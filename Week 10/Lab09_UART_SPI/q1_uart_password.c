#include <AT89C513xA.h>

#define LCD_data P2
#define LCD_rs P0_0
#define LCD_rw P0_1
#define LCD_en P0_2

void delay_ms(unsigned int ms)
{
while(ms--)
{
TH0=0xF8;
TL0=0x30;
TR0=1;
while(!TF0);
TR0=0;
TF0=0;
}
}

void uart_init(void)
{
TMOD=0x21;
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

void LCD_CmdWrite(unsigned char cmd)
{
LCD_data=cmd;
LCD_rs=0;
LCD_rw=0;
LCD_en=1;
delay_ms(1);
LCD_en=0;
delay_ms(2);
}

void LCD_DataWrite(unsigned char dat)
{
LCD_data=dat;
LCD_rs=1;
LCD_rw=0;
LCD_en=1;
delay_ms(1);
LCD_en=0;
delay_ms(2);
}

void LCD_Print(char *s)
{
while(*s)
{
LCD_DataWrite(*s);
s++;
}
}

void LCD_Init(void)
{
delay_ms(20);
LCD_CmdWrite(0x38);
LCD_CmdWrite(0x0C);
LCD_CmdWrite(0x01);
LCD_CmdWrite(0x06);
}

void LCD_Show(char *s)
{
LCD_CmdWrite(0x01);
LCD_CmdWrite(0x80);
LCD_Print(s);
}

void main(void)
{
char pass[4];
char c;
unsigned char i;
unsigned char wrong=0;
P1=0x00;
uart_init();
LCD_Init();
while(1)
{
uart_print("\r\nEnter Password: ");
LCD_Show("ENTER PASSWORD");
i=0;
while(i<4)
{
c=uart_rx();
if(c=='\r' || c=='\n')
continue;
pass[i]=c;
uart_tx('*');
i++;
}
if(pass[0]=='1' && pass[1]=='2' && pass[2]=='3' && pass[3]=='4')
{
wrong=0;
uart_print("\r\nAccess Granted\r\n");
LCD_Show("ACCESS OK");
P1=0xF0;
delay_ms(5000);
P1=0x00;
}
else
{
wrong++;
uart_print("\r\nAccess Denied\r\n");
LCD_Show("WRONG PASSWORD");
for(i=0;i<3;i++)
{
P1=0xF0;
delay_ms(300);
P1=0x00;
delay_ms(300);
}
uart_print("Attempts remaining: ");
uart_tx('0'+3-wrong);
uart_print("\r\n");
if(wrong==3)
{
uart_print("SYSTEM LOCKED\r\n");
LCD_Show("SYSTEM LOCKED");
P1=0xF0;
while(1)
{
if(RI)
RI=0;
}
}
}
}
}
