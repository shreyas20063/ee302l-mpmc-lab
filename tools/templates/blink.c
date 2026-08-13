/* Pt-51 LED blink - P1 upper nibble is the four LEDs */
/* SDCC header; on lab machines include <AT89C513xA.h> instead */
#include <8051.h>

void delay(void)
{
	int i, j;
	for (i = 0; i < 0xff; i++)
		for (j = 0; j < 0xff; j++)
			;
}

void main(void)
{
	while (1) {
		P1 = 0xF0;
		delay();
		P1 = 0x00;
		delay();
	}
}
