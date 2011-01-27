/*
 * Copyright (C) 2009 Samsung Electronics
 *
 * Donghwa Lee <dh09.lee@samsung.com>
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
#include <errno.h>
#include <pwm.h>
#include <asm/io.h>
#include <asm/arch/pwm.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

#define PRESCALER_0		(8 - 1)		/* prescaler of timer 0, 1 */
#define MUX_DIV_1		0		/* 1/1 period */
#define MUX_DIV_2		1		/* 1/2 period */
#define MUX_DIV_4		2		/* 1/4 period */
#define MUX_DIV_8		3		/* 1/8 period */
#define MUX_DIV_16		4		/* 1/16 period */

#define MUX0_DIV_SHIFT		0

#define TCON0_TIMER_SHIFT	0


int pwm_enable(int pwm_id)
{
	const struct s5p_timer *pwm = (struct s5p_timer *)samsung_get_base_timer();
	unsigned long tcon;

	tcon = readl(&pwm->tcon);

	if (pwm_id == 0)
		tcon |= TCON0_START;
	else
		tcon |= TCON_START(pwm_id);
	writel(tcon, &pwm->tcon);

	return 0;
}

void pwm_disable(int pwm_id)
{
	const struct s5p_timer *pwm = (struct s5p_timer *)samsung_get_base_timer();
	unsigned long tcon;

	tcon = readl(&pwm->tcon);
	if (pwm_id == 0)
		tcon &= ~TCON0_START;
	else
		tcon &= ~TCON_START(pwm_id);

	writel(tcon, &pwm->tcon);

}

static unsigned long pwm_calc_tin(int pwm_id, unsigned long freq)
{
	unsigned long tin_parent_rate;
	unsigned int div;

	tin_parent_rate = get_pwm_clk();

	for (div = 2; div <= 16; div *= 2) {
		if ((tin_parent_rate / (div << 16)) < freq)
			return tin_parent_rate / div;
	}

	return tin_parent_rate / 16;
}

#define NS_IN_HZ (1000000000UL)

int pwm_config(int pwm_id, int duty_ns, int period_ns)
{
	const struct s5p_timer *pwm = (struct s5p_timer *)samsung_get_base_timer();
	unsigned int offset;
	unsigned long tin_rate;
	unsigned long tin_ns;
	unsigned long period;
	unsigned long tcon;
	unsigned long tcnt;
	long tcmp;

	/* We currently avoid using 64bit arithmetic by using the
	 * fact that anything faster than 1Hz is easily representable
	 * by 32bits. */

	if (period_ns > NS_IN_HZ || duty_ns > NS_IN_HZ)
		return -ERANGE;

	if (duty_ns > period_ns)
		return -EINVAL;

	/* The TCMP and TCNT can be read without a lock, they're not
	 * shared between the timers. */
	offset = pwm_id * 0xc;
	tcmp = readl(&pwm->tcmpb0 + offset);
	tcnt = readl(&pwm->tcntb0 + offset);

	period = NS_IN_HZ / period_ns;

	/* Check to see if we are changing the clock rate of the PWM */

	tin_rate = pwm_calc_tin(pwm_id, period);
	gd->timer_rate_hz = tin_rate;

	tin_ns = NS_IN_HZ / tin_rate;
	tcnt = period_ns / tin_ns;

	/* Note, counters count down */

	tcmp = duty_ns / tin_ns;
	tcmp = tcnt - tcmp;
	/* the pwm hw only checks the compare register after a decrement,
	   so the pin never toggles if tcmp = tcnt */
	if (tcmp == tcnt)
		tcmp--;

	if (tcmp < 0)
		tcmp = 0;

	/* Update the PWM register block. */
	writel(tcmp, &pwm->tcmpb0 + offset);
	writel(tcnt, &pwm->tcntb0 + offset);

	tcon = readl(&pwm->tcon);
	if (pwm_id == 0) {
		tcon |= TCON0_UPDATE;
		tcon |= TCON0_AUTO_RELOAD;
	} else {
		tcon |= TCON_UPDATE(pwm_id);
		tcon |= TCON_AUTO_RELOAD(pwm_id);
	}

	writel(tcon, &pwm->tcon);

	if (pwm_id == 0)
		tcon &= ~TCON0_UPDATE;
	else
		tcon &= ~TCON_UPDATE(pwm_id);

	writel(tcon, &pwm->tcon);

	return 0;
}

int pwm_init(void)
{
	u32 val;
	const struct s5p_timer *pwm = (struct s5p_timer *)samsung_get_base_timer();
	//pwm = (struct s5p_timer *)samsung_get_base_timer();

	/*
	 * @ PWM Timer 0
	 * Timer Freq(HZ) =
	 *	PWM_CLK / { (prescaler_value + 1) * (divider_value) }
	 */

	/* set prescaler : 16 */
	/* set divider : 2 */
	val = readl(&pwm->tcfg0);
	val &= ~(0xff << 8);
	val |= (PRESCALER_0 & 0xff) << 8;
	writel(val, &pwm->tcfg0);
	val = readl(&pwm->tcfg1);
	val &= ~(0xf << MUX0_DIV_SHIFT);
	val |= (MUX_DIV_1 & 0xF) << MUX0_DIV_SHIFT;
	writel(val, &pwm->tcfg1);

	/* timer_rate_hz = 44444444(HZ) (per 1 sec)*/
	gd->timer_rate_hz = get_pwm_clk() / ((PRESCALER_0 + 1) *
			(MUX_DIV_1 + 1));

	/* timer_rate_hz / 100 = 444444.44(HZ) (per 10 msec) */
	gd->timer_rate_hz = gd->timer_rate_hz / 100;

	/* set count value */
	writel(gd->timer_rate_hz, &pwm->tcntb0);
	gd->lastinc = gd->timer_rate_hz;

	val = (readl(&pwm->tcon) & ~(0x07 << TCON0_TIMER_SHIFT)) |
		TCON0_INVERTER;

	/* start PWM timer 0 */
	writel(val | TCON0_START, &pwm->tcon);

	gd->timer_reset_value = 0;

	return 0;
}
