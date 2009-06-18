/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * (C) Copyright 2009
 * Heungjun Kim, SAMSUNG Electronics, <riverful.kim@samsung.com>
 * Inki Dae, SAMSUNG Electronics, <inki.dae@samsung.com>
 * Minkyu Kang, SAMSUNG Electronics, <mk7.kang@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#define PRESCALER_1		(16 - 1)	/* prescaler of timer 2, 3, 4 */
#define MUX_DIV_2		(1)		/* 1/2 period */
#define MUX_DIV_4		(2)		/* 1/4 period */
#define MUX_DIV_8		(3)		/* 1/8 period */
#define MUX_DIV_16		(4)		/* 1/16 period */
#define MUX4_DIV_SHIFT		16

#define TCON_TIMER4_SHIFT	20

static ulong count_value;

/* Internal tick units */
static unsigned long long timestamp;	/* Monotonic incrementing timer */
static unsigned long lastdec;	/* Last decremneter snapshot */

/* macro to read the 16 bit timer */
static inline ulong READ_TIMER(void)
{
	const s5pc1xx_timers_t *timers = (s5pc1xx_timers_t *) S5P_TIMER_BASE;

	return timers->TCNTO4;
}

int interrupt_init(void)
{
	s5pc1xx_timers_t *timers = (s5pc1xx_timers_t *) S5P_TIMER_BASE;

	/*
	 * @ PWM Timer 4
	 * Timer Freq(HZ) =
	 *	PCLK / { (prescaler_value + 1) * (divider_value) }
	 */

	/* set prescaler : 16 */
	/* set divider : 2 */
	timers->TCFG0 = (PRESCALER_1 & 0xff) << 8;
	timers->TCFG1 = (MUX_DIV_2 & 0xf) << MUX4_DIV_SHIFT;

	if (count_value == 0) {

		/* reset initial value */
		/* count_value = 2085937.5(HZ) (per 1 sec)*/
		count_value = get_PCLK() / ((PRESCALER_1 + 1) *
				(MUX_DIV_2 + 1));

		/* count_value / 100 = 20859.375(HZ) (per 10 msec) */
		count_value = count_value / 100;
	}

	/* set count value */
	timers->TCNTB4 = count_value;
	lastdec = count_value;

	/* auto reload & manual update */
	timers->TCON = (timers->TCON & ~(0x07 << TCON_TIMER4_SHIFT)) |
		S5P_TCON4_AUTO_RELOAD | S5P_TCON4_UPDATE;

	/* start PWM timer 4 */
	timers->TCON = (timers->TCON & ~(0x07 << TCON_TIMER4_SHIFT)) |
		S5P_TCON4_AUTO_RELOAD | S5P_TCON4_ON;

	timestamp = 0;

	return 0;
}


/*
 * timer without interrupts
 */
void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	timestamp = t;
}

/* delay x useconds */
void udelay(unsigned long usec)
{
	ulong tmo, tmp;

	if (usec >= 1000) {
		/*
		 * if "big" number, spread normalization
		 * to seconds
		 * 1. start to normalize for usec to ticks per sec
		 * 2. find number of "ticks" to wait to achieve target
		 * 3. finish normalize.
		 */
		tmo = usec / 1000;
		tmo *= CONFIG_SYS_HZ;
		tmo /= 1000;
	} else {
		/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000 * 1000);
	}

	/* get current timestamp */
	tmp = get_timer(0);

	/* if setting this fordward will roll time stamp */
	/* reset "advancing" timestamp to 0, set lastdec value */
	/* else, set advancing stamp wake up time */
	if ((tmo + tmp + 1) < tmp)
		reset_timer_masked();
	else
		tmo += tmp;

	/* loop till event */
	while (get_timer_masked() < tmo)
		;	/* nop */
}

void reset_timer_masked(void)
{
	/* reset time */
	lastdec = READ_TIMER();
	timestamp = 0;
}

ulong get_timer_masked(void)
{
	/* current tick value */
	ulong now = READ_TIMER();

	if (lastdec >= now)
		timestamp += lastdec - now;
	else
		timestamp += lastdec + count_value - now;

	lastdec = now;

	return timestamp;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	/* We overrun in 100s */
	return CONFIG_SYS_HZ * 100;
}

void reset_cpu(ulong ignored)
{
	unsigned int pid = __REG(S5P_PRO_ID);

	pid >>= 12;
	pid &= 0x00fff;
	pid |= (0xC << 12);

	__REG(S5P_SW_RST) = pid;

	while (1) ;
}
