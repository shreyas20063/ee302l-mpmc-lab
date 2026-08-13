/*
 * Pt-51 (AT89C5131A) — blink all four onboard LEDs.
 *
 * P1 upper nibble (P1.4-P1.7) = LED1-LED4, lower nibble (P1.0-P1.3) = SW1-SW4,
 * so 0xF0 drives the LEDs without disturbing the switch inputs.
 *
 * The deck uses #include <AT89C513xA.h>, which is an MCU 8051 IDE header.
 * SDCC ships <8051.h>, which defines P1 for any MCS-51 part; use that here and
 * swap the include back if you rebuild this on a lab machine.
 */
#include <8051.h>

static void delay(void)
{
	int i, j;
	for (i = 0; i < 0xff; i++)
		for (j = 0; j < 0xff; j++)
			;
}

void main(void)
{
	while (1) {
		P1 = 0xF0;	/* all four LEDs on  */
		delay();
		P1 = 0x00;	/* all four LEDs off */
		delay();
	}
}
