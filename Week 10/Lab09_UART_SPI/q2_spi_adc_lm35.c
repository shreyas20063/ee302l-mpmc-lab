#include <AT89C513xA.h>

#define LCD_data P2
#define LCD_rs P0_0
#define LCD_rw P0_1
#define LCD_en P0_2
#define CS P1_4
#define MISO P1_5
#define SCK P1_6
#define MOSI P1_7
#define VREF_MV 3300

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

unsigned char spi_byte(unsigned char out)
{
unsigned char i;
unsigned char in=0;
for(i=0;i<8;i++)
{
if(out&0x80)
MOSI=1;
else
MOSI=0;
out=out<<1;
SCK=1;
in=in<<1;
if(MISO)
in=in|1;
SCK=0;
}
return in;
}

unsigned int adc_read(unsigned char ch)
{
unsigned char hi;
unsigned char lo;
CS=0;
spi_byte(0x01);
hi=spi_byte(0x80|(ch<<4))&0x03;
lo=spi_byte(0x00);
CS=1;
return ((unsigned int)hi<<8)|lo;
}

void main(void)
{
unsigned int raw;
unsigned int t;
CS=1;
SCK=0;
MISO=1;
TMOD=0x01;
LCD_Init();
LCD_CmdWrite(0x82);
LCD_Print("TEMPERATURE");
while(1)
{
raw=adc_read(0);
t=(unsigned long)raw*VREF_MV/1024;
LCD_CmdWrite(0xC4);
if(t>=1000)
LCD_DataWrite('0'+t/1000);
else
LCD_DataWrite(' ');
LCD_DataWrite('0'+t/100%10);
LCD_DataWrite('0'+t/10%10);
LCD_DataWrite('.');
LCD_DataWrite('0'+t%10);
LCD_DataWrite(0xDF);
LCD_DataWrite('C');
delay_ms(500);
}
}
