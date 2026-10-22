#include <AT89C513xA.h>

#define IR_IN P3_2
#define IR_OUT P1_0
#define LED_RX P1_7
#define MAX_EDGES 400
#define REPLAY_FIX 212
#define MAX_HEARD 100
#define MATCH_TICKS 400
#define MARK_FIX 180
#define SPACE_FIX 140
#define MIN_EDGES 8
#define END_OVF 5
#define DOUBLE_GAP 12000000

__xdata unsigned int dur[MAX_EDGES];
unsigned int n_edges=0;
__xdata unsigned int heard[MAX_HEARD];
unsigned int n_heard;
unsigned int last_t;
unsigned char rx_level;
unsigned char rx_started;
unsigned char comp=0;

void uart_init(void)
{
SCON=0x50;
PCON=PCON|0x80;
TH1=0xF3;
TL1=0xF3;
TR1=1;
}

void uart_tx(char c)
{
SBUF=c;
while(!TI);
TI=0;
}

void uart_print(char *s)
{
while(*s)
{
uart_tx(*s);
s++;
}
}

void uart_num(unsigned long n)
{
char buf[10];
unsigned char i=0;
do
{
buf[i]='0'+n%10;
n=n/10;
i++;
}
while(n);
while(i)
{
i--;
uart_tx(buf[i]);
}
}

void carrier_set(unsigned int reload)
{
TR2=0;
T2CON=0x00;
RCAP2H=reload>>8;
RCAP2L=reload&0xFF;
TH2=RCAP2H;
TL2=RCAP2L;
TR2=1;
}

void carrier_on(void)
{
T2MOD=0x02;
IR_OUT=1;
}

void carrier_off(void)
{
IR_OUT=0;
T2MOD=0x00;
}

void measure_carrier(void)
{
unsigned int count;
unsigned char ovf=0;
unsigned long hz;
carrier_on();
TR0=0;
TMOD=0x25;
TH0=0;
TL0=0;
TF0=0;
CMOD=0x00;
CL=0;
CH=0;
CF=0;
CR=1;
TR0=1;
while(ovf<4)
{
if(CF)
{
CF=0;
ovf++;
}
}
TR0=0;
CR=0;
carrier_off();
count=((unsigned int)TH0<<8)|TL0;
TMOD=0x21;
hz=(unsigned long)count*15625/2048;
uart_print("carrier measured on P3.4: ");
uart_num(hz);
uart_print(" Hz (needs a jumper P1.0 -> P3.4)\r\n");
}

unsigned long dur_ticks(unsigned int i)
{
if(dur[i]&0x8000)
return (unsigned long)(dur[i]&0x7FFF)<<6;
return dur[i];
}

void ir_dump(void)
{
unsigned int i;
uart_num(n_edges);
uart_print(" durations, microseconds (mark space mark ...):\r\n");
for(i=0;i<n_edges;i++)
{
uart_num(dur_ticks(i)/2);
if(i%10==9)
uart_print("\r\n");
else
uart_tx(' ');
}
uart_print("\r\n");
}

void ir_record(void)
{
unsigned char level;
unsigned char ovf;
unsigned long t;
uart_print("\r\nwaiting for a remote (any key cancels)\r\n");
do
{
n_edges=0;
level=0;
ovf=0;
while(IR_IN)
{
if(RI)
{
RI=0;
uart_print("cancelled\r\n");
return;
}
}
TR0=0;
TH0=0;
TL0=0;
TF0=0;
TR0=1;
while(n_edges<MAX_EDGES && ovf<END_OVF)
{
if(TF0)
{
TF0=0;
ovf++;
}
if(IR_IN!=level)
{
TR0=0;
t=((unsigned long)ovf<<16)|((unsigned int)TH0<<8)|TL0;
if(t<0x8000)
dur[n_edges]=t;
else
dur[n_edges]=0x8000|(t>>6);
n_edges++;
TH0=0;
TL0=0;
TF0=0;
ovf=0;
TR0=1;
level=!level;
}
}
TR0=0;
}
while(n_edges<MIN_EDGES);
ir_dump();
}

void wait_ticks(unsigned long t)
{
unsigned int d;
while(t)
{
if(t>60000)
d=60000;
else
d=t;
t=t-d;
d=0-d;
TR0=0;
TH0=d>>8;
TL0=d&0xFF;
TF0=0;
TR0=1;
while(!TF0);
}
TR0=0;
}

unsigned long replay_ticks(unsigned int i)
{
unsigned long d;
d=dur_ticks(i);
if(comp)
{
if(i&1)
d=d+SPACE_FIX;
else if(d>MARK_FIX)
d=d-MARK_FIX;
}
if(d>REPLAY_FIX)
d=d-REPLAY_FIX;
return d;
}

void ir_replay(void)
{
unsigned int i;
for(i=0;i<n_edges;i++)
{
if(i&1)
carrier_off();
else
carrier_on();
wait_ticks(replay_ticks(i));
}
carrier_off();
uart_print("\r\nplayed ");
uart_num(n_edges);
uart_print(" durations\r\n");
}

void nec_frame(unsigned char addr,unsigned char cmd)
{
unsigned char b[4];
unsigned char i;
unsigned char j;
b[0]=addr;
b[1]=~addr;
b[2]=cmd;
b[3]=~cmd;
n_edges=0;
dur[n_edges++]=18000;
dur[n_edges++]=9000;
for(i=0;i<4;i++)
{
for(j=0;j<8;j++)
{
dur[n_edges++]=1120;
if(b[i]&(1<<j))
dur[n_edges++]=3380;
else
dur[n_edges++]=1120;
}
}
dur[n_edges++]=1120;
uart_print("\r\nloaded NEC test frame: address 04, command 08 (LG TV power)\r\n");
}

unsigned int pca_now(void)
{
unsigned char h;
unsigned char l;
do
{
h=CH;
l=CL;
}
while(h!=CH);
return ((unsigned int)h<<8)|l;
}

void listen_until_tf0(void)
{
unsigned int t;
while(!TF0)
{
if(IR_IN!=rx_level)
{
t=pca_now();
if(rx_started && n_heard<MAX_HEARD)
{
heard[n_heard]=t-last_t;
n_heard++;
}
rx_started=1;
last_t=t;
rx_level=!rx_level;
}
}
}

void ir_loopback(void)
{
unsigned int i;
unsigned int d;
unsigned int ok=0;
n_heard=0;
rx_level=1;
rx_started=0;
CMOD=0x00;
CL=0;
CH=0;
CR=1;
for(i=0;i<n_edges && !(dur[i]&0x8000);i++)
{
if(i&1)
carrier_off();
else
carrier_on();
d=dur[i];
if(d>REPLAY_FIX)
d=d-REPLAY_FIX;
d=0-d;
TR0=0;
TH0=d>>8;
TL0=d&0xFF;
TF0=0;
TR0=1;
listen_until_tf0();
}
carrier_off();
TR0=0;
TH0=0xB1;
TL0=0xE0;
TF0=0;
TR0=1;
listen_until_tf0();
TR0=0;
CR=0;
uart_print("\r\nsent  heard (us)\r\n");
for(i=0;i<n_edges && i<n_heard;i++)
{
uart_num(dur[i]/2);
uart_print("  ");
uart_num(heard[i]/2);
if(heard[i]+MATCH_TICKS>=dur[i] && heard[i]<=dur[i]+MATCH_TICKS)
ok++;
else
uart_print("  <- off");
uart_print("\r\n");
}
uart_print("sent ");
uart_num(n_edges);
uart_print(", heard ");
uart_num(n_heard);
uart_print(", within 200 us: ");
uart_num(ok);
uart_print("\r\n");
}

void main(void)
{
char c;
IR_OUT=0;
LED_RX=0;
TMOD=0x21;
uart_init();
carrier_set(0xFF62);
uart_print("\r\nIR cloner dev build, 38 kHz\r\n");
uart_print("r record  p play  o double press (6 s)  d dump  t load NEC test  l loopback  f measure carrier  k timing fix on/off  c/x carrier on/off  6/8/0 = 36/38/40 kHz\r\n");
while(1)
{
LED_RX=!IR_IN;
if(RI)
{
c=SBUF;
RI=0;
if(c=='r')
ir_record();
else if(c=='p')
ir_replay();
else if(c=='o')
{
ir_replay();
uart_print("waiting 6 s\r\n");
wait_ticks(DOUBLE_GAP);
ir_replay();
}
else if(c=='d')
ir_dump();
else if(c=='t')
nec_frame(0x04,0x08);
else if(c=='l')
ir_loopback();
else if(c=='c')
{
carrier_on();
uart_print("carrier ON at P1.0\r\n");
}
else if(c=='x')
{
carrier_off();
uart_print("carrier off\r\n");
}
else if(c=='6')
{
carrier_set(0xFF59);
uart_print("36 kHz\r\n");
}
else if(c=='8')
{
carrier_set(0xFF62);
uart_print("38 kHz\r\n");
}
else if(c=='0')
{
carrier_set(0xFF6A);
uart_print("40 kHz\r\n");
}
else if(c=='f')
measure_carrier();
else if(c=='k')
{
comp=!comp;
if(comp)
uart_print("timing correction ON (marks -90 us, spaces +70 us)\r\n");
else
uart_print("timing correction OFF (raw replay)\r\n");
}
}
}
}
