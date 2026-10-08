#include <AT89C513xA.h>

#define LCD_data P2
#define LCD_rs P0_0
#define LCD_rw P0_1
#define LCD_en P0_2

void delay(unsigned int n)
{
unsigned int i;
while(n--)
for(i=0;i<100;i++);
}

void LCD_CmdWrite(unsigned char cmd)
{
LCD_data=cmd;
LCD_rs=0;
LCD_rw=0;
LCD_en=1;
delay(4);
LCD_en=0;
delay(4);
}

void LCD_DataWrite(unsigned char dat)
{
LCD_data=dat;
LCD_rs=1;
LCD_rw=0;
LCD_en=1;
delay(4);
LCD_en=0;
delay(4);
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
delay(40);
LCD_CmdWrite(0x38);
LCD_CmdWrite(0x0C);
LCD_CmdWrite(0x01);
LCD_CmdWrite(0x06);
}

void main(void)
{
LCD_Init();
LCD_CmdWrite(0x81);
LCD_Print("Shreyas & Yash");
LCD_CmdWrite(0xC2);
LCD_Print("08 OCT 2026");
while(1);
}
