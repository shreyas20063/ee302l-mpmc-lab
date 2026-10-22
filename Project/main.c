#include <AT89C513xA.h>

#define IR_IN P3_2
#define IR_OUT P1_0
#define LED_RX P1_7
#define BTN P3_5
#define REC P3_3
#define LCD_data P2
#define LCD_rs P0_0
#define LCD_rw P0_1
#define LCD_en P0_2
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
unsigned char lcd_col;
unsigned char sel_last=0xFF;
__code const char hexd[]="0123456789ABCDEF";
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
uart_tx(hexd[b>>4]);
uart_tx(hexd[b&0x0F]);
}

void delay_ms(unsigned int ms)
{
while(ms--)
{
TR0=0;
TH0=0xF8;
TL0=0x30;
TF0=0;
TR0=1;
while(!TF0);
}
TR0=0;
}

void lcd_cmd(unsigned char c)
{
LCD_data=c;
LCD_rs=0;
LCD_rw=0;
LCD_en=1;
delay_ms(1);
LCD_en=0;
delay_ms(2);
}

void lcd_data(unsigned char c)
{
LCD_data=c;
LCD_rs=1;
LCD_rw=0;
LCD_en=1;
delay_ms(1);
LCD_en=0;
delay_ms(1);
}

void lcd_init(void)
{
delay_ms(50);
lcd_cmd(0x38);
lcd_cmd(0x38);
lcd_cmd(0x0C);
lcd_cmd(0x01);
lcd_cmd(0x06);
}

void lcd_at(unsigned char row)
{
if(row)
lcd_cmd(0xC0);
else
lcd_cmd(0x80);
lcd_col=0;
}

void lcd_ch(char c)
{
if(lcd_col<16)
{
lcd_data(c);
lcd_col++;
}
}

void lcd_str(char *s)
{
while(*s)
{
lcd_ch(*s);
s++;
}
}

void lcd_end(void)
{
while(lcd_col<16)
lcd_ch(' ');
}

void lcd_num(unsigned long n)
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
lcd_ch(buf[i]);
}
}

void lcd_hex(unsigned char b)
{
lcd_ch(hexd[b>>4]);
lcd_ch(hexd[b&0x0F]);
}

void lcd_two(char *a,char *b)
{
lcd_at(0);
lcd_str(a);
lcd_end();
lcd_at(1);
lcd_str(b);
lcd_end();
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
lcd_at(0);
lcd_str("CARRIER MEASURED");
lcd_end();
lcd_at(1);
lcd_num(hz);
lcd_str(" Hz");
lcd_end();
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

void ir_record(unsigned char by_button)
{
unsigned char level;
unsigned char rec_up=0;
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
n_edges=0;
return;
}
if(by_button)
{
if(REC)
rec_up=1;
else if(rec_up)
{
uart_print("cancelled\r\n");
n_edges=0;
while(!REC);
delay_ms(20);
return;
}
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

unsigned char read_sel(void)
{
unsigned char v=0;
if(!P1_1)
v=v|4;
if(!P1_2)
v=v|2;
if(!P1_3)
v=v|1;
return v;
}

void show_home(void)
{
unsigned char i;
unsigned char n=0;
unsigned char sel;
for(i=0;i<SLOTS;i++)
n=n+slot_used[i];
sel=read_sel();
sel_last=sel;
lcd_at(0);
lcd_str("SEL ");
lcd_ch('1'+sel);
lcd_str(": ");
if(slot_used[sel])
lcd_str(slot_name[sel]);
else
lcd_str("EMPTY");
lcd_end();
lcd_at(1);
lcd_str("IR ");
lcd_num(carrier_khz);
lcd_str("kHz  ");
lcd_num(n);
lcd_str("/8 SAVED");
lcd_end();
}

void lcd_slot(char *what,unsigned char i)
{
lcd_at(0);
lcd_str(what);
lcd_ch('1'+i);
lcd_end();
lcd_at(1);
lcd_str(slot_name[i]);
lcd_end();
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
slot_name[s][0]=0;
lcd_at(0);
lcd_str("NAME FOR SLOT ");
lcd_ch('1'+s);
lcd_end();
lcd_at(1);
lcd_end();
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
slot_name[s][n]=0;
uart_print("\b \b");
lcd_at(1);
lcd_str(slot_name[s]);
lcd_end();
continue;
}
if(c>=32 && c<127 && n<NAME_LEN)
{
slot_name[s][n]=c;
n++;
slot_name[s][n]=0;
uart_tx(c);
lcd_at(1);
lcd_str(slot_name[s]);
lcd_end();
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
lcd_at(0);
lcd_str("SLOT ");
lcd_ch('1'+i);
lcd_str(" EMPTY");
lcd_end();
lcd_at(1);
lcd_end();
return;
}
nec_build(slot_a0[i],slot_a1[i],slot_cmd[i]);
old=comp;
comp=0;
ir_replay();
comp=old;
uart_print("sent ");
slot_show(i);
lcd_slot("SENT SLOT ",i);
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
lcd_at(0);
lcd_str("RAW ");
lcd_num(n_edges);
lcd_str(" DURATIONS");
lcd_end();
lcd_at(1);
lcd_str("NOT NEC, P PLAYS");
lcd_end();
return;
}
uart_print("NEC address ");
uart_hex(a0);
uart_tx(' ');
uart_hex(a1);
uart_print(" command ");
uart_hex(cmd);
uart_print("\r\nsave to slot 1-8 (any other key skips): ");
lcd_at(0);
lcd_str("NEC ");
lcd_hex(a0);
lcd_hex(a1);
lcd_str(" CMD ");
lcd_hex(cmd);
lcd_end();
lcd_at(1);
lcd_str("SAVE TO SLOT 1-8");
lcd_end();
c=uart_rx();
if(c<'1' || c>'8')
{
uart_print("not saved\r\n");
lcd_at(1);
lcd_str("NOT SAVED");
lcd_end();
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
lcd_slot("SAVED SLOT ",c);
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
unsigned int m=0;
while(m<n_edges && !(dur[m]&0x8000))
m++;
n_heard=0;
rx_level=1;
rx_started=0;
CMOD=0x00;
CL=0;
CH=0;
CR=1;
for(i=0;i<m;i++)
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
for(i=0;i<m && i<n_heard;i++)
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
uart_num(m);
uart_print(", heard ");
uart_num(n_heard);
uart_print(", within 200 us: ");
uart_num(ok);
uart_print("\r\n");
lcd_at(0);
lcd_str("LOOPBACK HEARD ");
lcd_end();
lcd_at(1);
lcd_str("MATCH ");
lcd_num(ok);
lcd_str(" OF ");
lcd_num(m);
lcd_end();
}

void button_send(void)
{
unsigned char sel;
unsigned int held=0;
delay_ms(20);
if(BTN)
return;
sel=read_sel();
slot_play(sel);
while(!BTN && held<1000)
{
delay_ms(10);
held=held+10;
}
if(held>=1000 && slot_used[sel])
{
lcd_two("DOUBLE PRESS","WAIT 6 S...");
uart_print("button held: second press in 6 s\r\n");
wait_ticks(DOUBLE_GAP-2000UL*held);
slot_play(sel);
}
while(!BTN);
delay_ms(20);
delay_ms(800);
show_home();
}

void rec_button(void)
{
unsigned char sel;
unsigned int held=0;
unsigned char a0;
unsigned char a1;
unsigned char cmd;
delay_ms(20);
if(REC)
return;
sel=read_sel();
lcd_at(0);
lcd_str("REC SLOT ");
lcd_ch('1'+sel);
lcd_end();
lcd_at(1);
lcd_str("RELEASE=START");
lcd_end();
while(!REC && held<3000)
{
delay_ms(10);
held=held+10;
}
if(held>=3000)
{
slot_used[sel]=0;
slot_name[sel][0]=0;
slots_save();
uart_print("button: erased slot ");
uart_tx('1'+sel);
uart_print("\r\n");
lcd_at(0);
lcd_str("ERASED SLOT ");
lcd_ch('1'+sel);
lcd_end();
lcd_at(1);
lcd_end();
while(!REC);
delay_ms(1500);
show_home();
return;
}
delay_ms(20);
lcd_at(1);
lcd_str("PRESS REMOTE...");
lcd_end();
ir_record(1);
if(n_edges==0)
{
lcd_two("RECORD","CANCELLED");
}
else if(nec_decode(&a0,&a1,&cmd))
{
slot_a0[sel]=a0;
slot_a1[sel]=a1;
slot_cmd[sel]=cmd;
slot_used[sel]=1;
slot_name[sel][0]=hexd[a0>>4];
slot_name[sel][1]=hexd[a0&0x0F];
slot_name[sel][2]=hexd[a1>>4];
slot_name[sel][3]=hexd[a1&0x0F];
slot_name[sel][4]='-';
slot_name[sel][5]=hexd[cmd>>4];
slot_name[sel][6]=hexd[cmd&0x0F];
slot_name[sel][7]=0;
slots_save();
uart_print("button: saved to EEPROM: ");
slot_show(sel);
lcd_slot("SAVED SLOT ",sel);
}
else
{
uart_print("button: not NEC, not saved\r\n");
lcd_two("NOT NEC","NOT SAVED");
}
delay_ms(1500);
show_home();
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
lcd_init();
show_home();
uart_print("\r\nIR cloner dev build, 38 kHz\r\n");
uart_print("r record+save  1-8 send slot  L list  e erase  p play last  o double press  d dump  t test frame  l loopback  f measure  k fix  q 36/38/40  c/x carrier\r\n");
slots_list();
while(1)
{
LED_RX=!IR_IN;
if(!BTN)
button_send();
if(!REC)
rec_button();
if(read_sel()!=sel_last)
show_home();
if(RI)
{
c=SBUF;
RI=0;
if(c=='r')
{
lcd_two("RECORDING...","PRESS A BUTTON");
ir_record(0);
if(n_edges)
offer_save();
else
lcd_two("RECORD","CANCELLED");
}
else if(c>='1' && c<='8')
slot_play(c-'1');
else if(c=='L')
{
slots_list();
show_home();
}
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
show_home();
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
show_home();
}
else if(c=='p')
{
ir_replay();
lcd_at(0);
lcd_str("PLAYED RAW");
lcd_end();
lcd_at(1);
lcd_num(n_edges);
lcd_str(" DURATIONS");
lcd_end();
}
else if(c=='o')
{
ir_replay();
lcd_two("DOUBLE PRESS","WAIT 6 S...");
uart_print("waiting 6 s\r\n");
wait_ticks(DOUBLE_GAP);
ir_replay();
lcd_two("DOUBLE PRESS","DONE");
}
else if(c=='d')
ir_dump();
else if(c=='t')
{
nec_build(0x04,0xFB,0x08);
uart_print("\r\nloaded NEC test frame: address 04, command 08 (LG TV power)\r\n");
lcd_two("TEST FRAME","LG POWER 04 08");
}
else if(c=='l')
ir_loopback();
else if(c=='c')
{
carrier_on();
uart_print("carrier ON at P1.0\r\n");
lcd_two("CARRIER ON","P1.0, X STOPS");
}
else if(c=='x')
{
carrier_off();
uart_print("carrier off\r\n");
show_home();
}
else if(c=='f')
measure_carrier();
else if(c=='k')
{
comp=!comp;
if(comp)
{
uart_print("timing correction ON (marks -90 us, spaces +70 us)\r\n");
lcd_two("TIMING FIX","ON");
}
else
{
uart_print("timing correction OFF (raw replay)\r\n");
lcd_two("TIMING FIX","OFF");
}
}
}
}
}
