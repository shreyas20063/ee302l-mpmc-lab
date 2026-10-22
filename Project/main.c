#include <AT89C513xA.h>

#define IR_IN P3_2
#define IR_OUT P1_0
#define LED_RX P1_7
#define MAX_EDGES 400
#define REPLAY_FIX 64

__xdata unsigned int dur[MAX_EDGES];
unsigned int n_edges=0;

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

void uart_num(unsigned int n)
{
char buf[5];
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

void ir_dump(void)
{
unsigned int i;
uart_num(n_edges);
uart_print(" durations, microseconds (mark space mark ...):\r\n");
for(i=0;i<n_edges;i++)
{
uart_num(dur[i]/2);
if(i%10==9)
uart_print("\r\n");
else
uart_tx(' ');
}
uart_print("\r\n");
}

void ir_record(void)
{
unsigned char level=0;
n_edges=0;
uart_print("\r\nwaiting for a remote (any key cancels)\r\n");
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
while(n_edges<MAX_EDGES && !TF0)
{
if(IR_IN!=level)
{
TR0=0;
dur[n_edges]=((unsigned int)TH0<<8)|TL0;
n_edges++;
TH0=0;
TL0=0;
TR0=1;
level=!level;
}
}
TR0=0;
ir_dump();
}

void ir_replay(void)
{
unsigned int i;
unsigned int d;
for(i=0;i<n_edges;i++)
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
while(!TF0);
}
carrier_off();
TR0=0;
uart_print("\r\nplayed ");
uart_num(n_edges);
uart_print(" durations\r\n");
}

void main(void)
{
char c;
IR_OUT=0;
LED_RX=0;
TMOD=0x21;
uart_init();
carrier_set(0xFFB1);
uart_print("\r\nIR cloner dev build, 38 kHz\r\n");
uart_print("r record  p play  d dump  c carrier on  x carrier off  6/8/0 = 36/38/40 kHz\r\n");
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
else if(c=='d')
ir_dump();
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
carrier_set(0xFFAD);
uart_print("36 kHz\r\n");
}
else if(c=='8')
{
carrier_set(0xFFB1);
uart_print("38 kHz\r\n");
}
else if(c=='0')
{
carrier_set(0xFFB5);
uart_print("40 kHz\r\n");
}
}
}
}
