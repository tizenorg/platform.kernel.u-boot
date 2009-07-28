/*
 * Copyright (C) 2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __ASM_ARM_ARCH_PWM_H_
#define __ASM_ARM_ARCH_PWM_H_

/*
 * PWM Timer
 */
#define S5PC100_PWMTIMER_BASE	0xEA000000
#define S5PC110_PWMTIMER_BASE	0xE2500000

/* PWM timer addressing */
#define S5PC100_TIMER_BASE	S5PC100_PWMTIMER_BASE
#define S5PC110_TIMER_BASE	S5PC110_PWMTIMER_BASE

/* Interval mode(Auto Reload) of PWM Timer 4 */
#define S5PC1XX_TCON4_AUTO_RELOAD	(1 << 22)
/* Update TCNTB4 */
#define S5PC1XX_TCON4_UPDATE		(1 << 21)
/* start bit of PWM Timer 4 */
#define S5PC1XX_TCON4_START		(1 << 20)

#ifndef __ASSEMBLY__
typedef struct s5pc1xx_timer {
	volatile unsigned long	TCFG0;
	volatile unsigned long	TCFG1;
	volatile unsigned long	TCON;
	volatile unsigned long	TCNTB0;
	volatile unsigned long	TCMPB0;
	volatile unsigned long	TCNTO0;
	volatile unsigned long	TCNTB1;
	volatile unsigned long	TCMPB1;
	volatile unsigned long	TCNTO1;
	volatile unsigned long	TCNTB2;
	volatile unsigned long	TCMPB2;
	volatile unsigned long	TCNTO2;
	volatile unsigned long	TCNTB3;
	volatile unsigned long	res1;
	volatile unsigned long	TCNTO3;
	volatile unsigned long	TCNTB4;
	volatile unsigned long	TCNTO4;
	volatile unsigned long	TINTCSTAT;
} s5pc1xx_timers_t;
#endif	/* __ASSEMBLY__ */

#endif
