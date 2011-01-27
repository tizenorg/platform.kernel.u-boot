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

/* Interval mode(Auto Reload) of PWM Timer 4 */
#define TCON4_AUTO_RELOAD	(1 << 22)
/* Update TCNTB4 */
#define TCON4_UPDATE		(1 << 21)
/* start bit of PWM Timer 4 */
#define TCON4_START		(1 << 20)

/* Interval mode(Auto Reload) of PWM Timer 0 */
#define TCON0_AUTO_RELOAD	(1 << 3)
/* Inverter On/Off */
#define TCON0_INVERTER		(1 << 2)
/* Update TCNTB0 */
#define TCON0_UPDATE		(1 << 1)
/* start bit of PWM Timer 0 */
#define TCON0_START		(1 << 0)

#define TCON_AUTO_RELOAD(x)	(1 << (((x + 1) * 4) + 3))
#define TCON_INVERT(x)		(1 << (((x + 1) * 4) + 2))
#define TCON_UPDATE(x)		(1 << (((x + 1) * 4) + 1))
#define TCON_START(x)		(1 << (((x + 1) * 4)))

#ifndef __ASSEMBLY__
struct s5p_timer {
	unsigned int	tcfg0;
	unsigned int	tcfg1;
	unsigned int	tcon;
	unsigned int	tcntb0;
	unsigned int	tcmpb0;
	unsigned int	tcnto0;
	unsigned int	tcntb1;
	unsigned int	tcmpb1;
	unsigned int	tcnto1;
	unsigned int	tcntb2;
	unsigned int	tcmpb2;
	unsigned int	tcnto2;
	unsigned int	tcntb3;
	unsigned int	res1;
	unsigned int	tcnto3;
	unsigned int	tcntb4;
	unsigned int	tcnto4;
	unsigned int	tintcstat;
};
#endif	/* __ASSEMBLY__ */

#endif
