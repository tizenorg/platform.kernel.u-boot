/*
 * (C) Copyright 2009
 * Samsung Electronics, <www.samsung.com/sec>
 * Heungjun Kim <riverful.kim@samsung.com>
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
 *
 */

#ifndef __ASM_ARM_ARCH_PWM_H_
#define __ASM_ARM_ARCH_PWM_H_

/*
 * PWM Timer
 */
#define S5P_PWMTIMER_BASE(x)	(S5P_PA_PWMTIMER + (x))

/* PWM timer offset */
#define PWM_TCFG0_OFFSET	0x0
#define PWM_TCFG1_OFFSET	0x04
#define PWM_TCON_OFFSET		0x08
#define PWM_TCNTB0_OFFSET	0x0c
#define PWM_TCMPB0_OFFSET	0x10
#define PWM_TCNTO0_OFFSET	0x14
#define PWM_TCNTB1_OFFSET	0x18
#define PWM_TCMPB1_OFFSET	0x1c
#define PWM_TCNTO1_OFFSET	0x20
#define PWM_TCNTB2_OFFSET	0x24
#define PWM_TCMPB2_OFFSET	0x28
#define PWM_TCNTO2_OFFSET	0x2c
#define PWM_TCNTB3_OFFSET	0x30
#define PWM_TCNTO3_OFFSET	0x38
#define PWM_TCNTB4_OFFSET	0x3c
#define PWM_TCNTO4_OFFSET	0x40
#define PWM_TINT_CSTAT_OFFSET	0x44

/* PWM timer register */
#define S5P_PWM_TCFG0		S5P_PWMTIMER_BASE(PWM_TCFG0_OFFSET)
#define S5P_PWM_TCFG1		S5P_PWMTIMER_BASE(PWM_TCFG1_OFFSET)
#define S5P_PWM_TCON		S5P_PWMTIMER_BASE(PWM_TCON_OFFSET)
#define S5P_PWM_TCNTB0		S5P_PWMTIMER_BASE(PWM_TCNTB0_OFFSET)
#define S5P_PWM_TCMPB0		S5P_PWMTIMER_BASE(PWM_TCMPB0_OFFSET)
#define S5P_PWM_TCNTO0		S5P_PWMTIMER_BASE(PWM_TCNTO0_OFFSET)
#define S5P_PWM_TCNTB1		S5P_PWMTIMER_BASE(PWM_TCNTB1_OFFSET)
#define S5P_PWM_TCMPB1		S5P_PWMTIMER_BASE(PWM_TCMPB1_OFFSET)
#define S5P_PWM_TCNTO1		S5P_PWMTIMER_BASE(PWM_TCNTO1_OFFSET)
#define S5P_PWM_TCNTB2		S5P_PWMTIMER_BASE(PWM_TCNTB2_OFFSET)
#define S5P_PWM_TCMPB2		S5P_PWMTIMER_BASE(PWM_TCMPB2_OFFSET)
#define S5P_PWM_TCNTO2		S5P_PWMTIMER_BASE(PWM_TCNTO2_OFFSET)
#define S5P_PWM_TCNTB3		S5P_PWMTIMER_BASE(PWM_TCNTB3_OFFSET)
#define S5P_PWM_TCNTO3		S5P_PWMTIMER_BASE(PWM_TCNTO3_OFFSET)
#define S5P_PWM_TCNTB4		S5P_PWMTIMER_BASE(PWM_TCNTB4_OFFSET)
#define S5P_PWM_TCNTO4		S5P_PWMTIMER_BASE(PWM_TCNTO4_OFFSET)
#define S5P_PWM_TINT_CSTAT	S5P_PWMTIMER_BASE(PWM_TINT_CSTAT_OFFSET)

/* PWM timer addressing */
#define S5P_TIMER_BASE		S5P_PWMTIMER_BASE(0x0)

/* PWM timer value */
#define S5P_TCON4_AUTO_RELOAD	(1 << 22)
				/* Interval mode(Auto Reload) of PWM Timer 4 */
#define S5P_TCON4_UPDATE	(1 << 21)  /* Update TCNTB4 */
#define S5P_TCON4_ON		(1 << 20)  /* start bit of PWM Timer 4 */

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
