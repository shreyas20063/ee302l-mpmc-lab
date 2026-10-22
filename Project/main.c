#include <AT89C513xA.h>

#define IR_IN P3_2
#define IR_OUT P1_0
#define LED_RX P1_7
#define MAX_EDGES 360
#define REPLAY_FIX 212
#define MAX_HEARD 90
#define SLOTS 8
#define NAME_LEN 8
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
unsigned char carrier_khz=38;
__xdata unsigned char slot_used[SLOTS];
__xdata unsigned char slot_a0[SLOTS];
__xdata unsigned char slot_a1[SLOTS];
__xdata unsigned char slot_cmd[SLOTS];
__xdata char slot_name[SLOTS][NAME_LEN+1];

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

char uart_rx(void)
{
while(!RI);
RI=0;
return SBUF;
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

void uart_hex(unsigned char b)
{
char h[]="0123456789ABCDEF";
uart_tx(h[b>>4]);
uart_tx(h[b&0x0F]);
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

void nec_build(unsigned char a0,unsigned char a1,unsigned char cmd)
{
unsigned char b[4];
unsigned char i;
unsigned char j;
unsigned long total=0;
b[0]=a0;
b[1]=a1;
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
for(i=0;i<n_edges;i++)
total=total+dur[i];
dur[n_edges++]=0x8000|((216000-total)>>6);
dur[n_edges++]=18000;
dur[n_edges++]=4500;
dur[n_edges++]=1120;
}

unsigned char nec_decode(unsigned char *a0,unsigned char *a1,unsigned char *cmd)
{
unsigned char b[4];
unsigned char i;
unsigned char j;
unsigned char k=3;
if(n_edges<67)
return 0;
if(dur[0]<14000 || dur[0]>22000 || dur[1]<7000 || dur[1]>11000)
return 0;
for(i=0;i<4;i++)
{
b[i]=0;
for(j=0;j<8;j++)
{
if(dur[k]&0x8000)
return 0;
if(dur[k]>2200)
b[i]=b[i]|(1<<j);
k=k+2;
}
}
if((b[2]^b[3])!=0xFF)
return 0;
*a0=b[0];
*a1=b[1];
*cmd=b[2];
return 1;
}

unsigned char ee_read(unsigned int a)
{
unsigned char v;
EECON=0x02;
AUXR=AUXR|0x20;
v=*((__xdata unsigned char *)a);
AUXR=AUXR&0xDF;
EECON=0x00;
return v;
}

void ee_latch(unsigned int a,unsigned char v)
{
EECON=0x02;
*((__xdata unsigned char *)a)=v;
EECON=0x00;
}

void ee_program(void)
{
EECON=0x52;
EECON=0xA2;
while(EECON&0x01);
EECON=0x00;
}

void slots_save(void)
{
unsigned char i;
unsigned char j;
unsigned int a=0;
for(i=0;i<SLOTS;i++)
{
ee_latch(a++,slot_used[i]);
ee_latch(a++,slot_a0[i]);
ee_latch(a++,slot_a1[i]);
ee_latch(a++,slot_cmd[i]);
for(j=0;j<=NAME_LEN;j++)
ee_latch(a++,slot_name[i][j]);
}
ee_program();
}

void slots_load(void)
{
unsigned char i;
unsigned char j;
unsigned int a=0;
for(i=0;i<SLOTS;i++)
{
slot_used[i]=ee_read(a++);
slot_a0[i]=ee_read(a++);
slot_a1[i]=ee_read(a++);
slot_cmd[i]=ee_read(a++);
for(j=0;j<=NAME_LEN;j++)
slot_name[i][j]=ee_read(a++);
if(slot_used[i]!=1)
{
slot_used[i]=0;
slot_name[i][0]=0;
}
slot_name[i][NAME_LEN]=0;
}
}

void slot_show(unsigned char i)
{
uart_tx('1'+i);
uart_print("  ");
if(slot_used[i])
{
uart_print(slot_name[i]);
uart_print("  (");
uart_hex(slot_a0[i]);
uart_tx(' ');
uart_hex(slot_a1[i]);
uart_print(" cmd ");
uart_hex(slot_cmd[i]);
uart_print(")");
}
else
uart_print("empty");
uart_print("\r\n");
}

void slots_list(void)
{
unsigned char i;
uart_print("\r\n");
for(i=0;i<SLOTS;i++)
slot_show(i);
}

void read_name(unsigned char s)
{
char c;
unsigned char n=0;
uart_print("name (Enter to finish): ");
while(1)
{
c=uart_rx();
if(c=='\r')
break;
if(c=='\n')
continue;
if((c==8 || c==127) && n>0)
{
n--;
uart_print("\b \b");
continue;
}
if(c>=32 && c<127 && n<NAME_LEN)
{
slot_name[s][n]=c;
n++;
uart_tx(c);
}
}
slot_name[s][n]=0;
uart_print("\r\n");
}

void slot_play(unsigned char i)
{
unsigned char old;
if(!slot_used[i])
{
uart_print("slot empty\r\n");
return;
}
nec_build(slot_a0[i],slot_a1[i],slot_cmd[i]);
old=comp;
comp=0;
ir_replay();
comp=old;
uart_print("sent ");
slot_show(i);
}

void offer_save(void)
{
unsigned char a0;
unsigned char a1;
unsigned char cmd;
char c;
if(!nec_decode(&a0,&a1,&cmd))
{
uart_print("not NEC: kept as raw capture only (p plays it)\r\n");
return;
}
uart_print("NEC address ");
uart_hex(a0);
uart_tx(' ');
uart_hex(a1);
uart_print(" command ");
uart_hex(cmd);
uart_print("\r\nsave to slot 1-8 (any other key skips): ");
c=uart_rx();
if(c<'1' || c>'8')
{
uart_print("not saved\r\n");
return;
}
uart_tx(c);
uart_print("\r\n");
c=c-'1';
slot_a0[c]=a0;
slot_a1[c]=a1;
slot_cmd[c]=cmd;
slot_used[c]=1;
read_name(c);
slots_save();
uart_print("saved to EEPROM: ");
slot_show(c);
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
slots_load();
uart_print("\r\nIR cloner dev build, 38 kHz\r\n");
uart_print("r record+save  1-8 send slot  L list  e erase  p play last  o double press  d dump  t test frame  l loopback  f measure  k fix  q 36/38/40  c/x carrier\r\n");
slots_list();
while(1)
{
LED_RX=!IR_IN;
if(RI)
{
c=SBUF;
RI=0;
if(c=='r')
{
ir_record();
if(n_edges)
offer_save();
}
else if(c>='1' && c<='8')
slot_play(c-'1');
else if(c=='L')
slots_list();
else if(c=='e')
{
uart_print("erase slot 1-8: ");
c=uart_rx();
if(c>='1' && c<='8')
{
slot_used[c-'1']=0;
slot_name[c-'1'][0]=0;
slots_save();
uart_print("erased\r\n");
}
else
uart_print("cancelled\r\n");
}
else if(c=='q')
{
if(carrier_khz==36)
{
carrier_set(0xFF62);
carrier_khz=38;
}
else if(carrier_khz==38)
{
carrier_set(0xFF6A);
carrier_khz=40;
}
else
{
carrier_set(0xFF59);
carrier_khz=36;
}
uart_num(carrier_khz);
uart_print(" kHz\r\n");
}
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
{
nec_build(0x04,0xFB,0x08);
uart_print("\r\nloaded NEC test frame: address 04, command 08 (LG TV power)\r\n");
}
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
