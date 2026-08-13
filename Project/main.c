/* IR remote cloner - Pt-51 (AT89C5131A), 24 MHz
 * receive: TSOP1738 out -> P3.2 (idles high, goes low during an IR mark)
 * send:    IR LED driver -> P1.4 (needs 38 kHz carrier, see carrier_on)
 * SW1 (P1.0) = record, SW2 (P1.1) = replay, LED4 (P1.7) = busy light
 */
#include <8051.h>

#define IR_IN	P3_2
#define SW1	P1_0
#define SW2	P1_1
#define LED	P1_7

#define MAX_EDGES 400			/* AC remotes send 300+ edges */
__xdata unsigned int dur[MAX_EDGES];	/* mark/space lengths in timer ticks */
unsigned char n_edges = 0;

/* timer 0, mode 1: one tick = one machine cycle = 0.5 us at 24 MHz */
static void t0_init(void)
{
	TMOD = (TMOD & 0xF0) | 0x01;
}

static void t0_restart(void)
{
	TR0 = 0;
	TH0 = 0;
	TL0 = 0;
	TF0 = 0;
	TR0 = 1;
}

static unsigned int t0_read(void)
{
	return ((unsigned int)TH0 << 8) | TL0;
}

/* wait for the first falling edge, then time every level change into dur[].
 * capture ends when the line sits idle long enough to overflow the timer
 * (~32 ms), which no remote protocol comes close to. */
static void ir_capture(void)
{
	unsigned char level;

	n_edges = 0;
	while (IR_IN)			/* wait for burst to start */
		;
	level = 0;
	t0_restart();
	while (n_edges < MAX_EDGES) {
		while (IR_IN == level) {
			if (TF0)	/* line stuck -> burst over */
				return;
		}
		dur[n_edges++] = t0_read();
		t0_restart();
		level = !level;
	}
}

/* 38 kHz on P1.4. Plan: PCA high-speed output on CEX1 so the hardware
 * makes the carrier and the CPU only gates it. PCA registers are not in
 * <8051.h> - add them from the AT89C5131A datasheet when wiring this up. */
static void carrier_on(void)
{
	P1_4 = 1;	/* placeholder: constant on, no carrier yet */
}

static void carrier_off(void)
{
	P1_4 = 0;
}

/* play the burst back: marks = carrier on, spaces = carrier off */
static void ir_replay(void)
{
	unsigned char i;

	for (i = 0; i < n_edges; i++) {
		if ((i & 1) == 0)
			carrier_on();
		else
			carrier_off();
		t0_restart();
		while (t0_read() < dur[i])
			;
	}
	carrier_off();
}

void main(void)
{
	t0_init();
	LED = 0;
	while (1) {
		if (!SW1) {
			LED = 1;
			ir_capture();
			LED = 0;
		}
		if (!SW2) {
			LED = 1;
			ir_replay();
			LED = 0;
		}
	}
}
