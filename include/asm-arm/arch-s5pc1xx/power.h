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

#ifndef __ASM_ARM_ARCH_POWER_H_
#define __ASM_ARM_ARCH_POWER_H_

/*
 * Power control
 */
#define S5P_PWRREG(x)			(S5P_PA_PWR + (x))

#define S5P_PWR_CFG			S5P_PWRREG(0x0)
#define S5P_EINT_WAKEUP_MASK		S5P_PWRREG(0x04)
#define S5P_NORMAL_CFG			S5P_PWRREG(0x10)
#define S5P_STOP_CFG			S5P_PWRREG(0x14)
#define S5P_SLEEP_CFG			S5P_PWRREG(0x18)
#define S5P_STOP_MEM_CFG		S5P_PWRREG(0x1c)
#define S5P_OSC_FREQ			S5P_PWRREG(0x100)
#define S5P_OSC_STABLE			S5P_PWRREG(0x104)
#define S5P_PWR_STABLE			S5P_PWRREG(0x108)
#define S5P_INTERNAL_PWR_STABLE		S5P_PWRREG(0x110)
#define S5P_CLAMP_STABLE		S5P_PWRREG(0x114)
#define S5P_OTHERS			S5P_PWRREG(0x200)
#define S5P_RST_STAT			S5P_PWRREG(0x300)
#define S5P_WAKEUP_STAT			S5P_PWRREG(0x304)
#define S5P_BLK_PWR_STAT		S5P_PWRREG(0x308)
#define S5P_INFORM0			S5P_PWRREG(0x400)
#define S5P_INFORM1			S5P_PWRREG(0x404)
#define S5P_INFORM2			S5P_PWRREG(0x408)
#define S5P_INFORM3			S5P_PWRREG(0x40c)
#define S5P_INFORM4			S5P_PWRREG(0x410)
#define S5P_INFORM5			S5P_PWRREG(0x414)
#define S5P_INFORM6			S5P_PWRREG(0x418)
#define S5P_INFORM7			S5P_PWRREG(0x41c)
#define S5P_DCGIDX_MAP0			S5P_PWRREG(0x500)
#define S5P_DCGIDX_MAP1			S5P_PWRREG(0x504)
#define S5P_DCGIDX_MAP2			S5P_PWRREG(0x508)
#define S5P_DCGPERF_MAP0		S5P_PWRREG(0x50c)
#define S5P_DCGPERF_MAP1		S5P_PWRREG(0x510)
#define S5P_DVCIDX_MAP			S5P_PWRREG(0x514)
#define S5P_FREQ_CPU			S5P_PWRREG(0x518)
#define S5P_FREQ_DPM			S5P_PWRREG(0x51c)
#define S5P_DVSEMCLK_EN			S5P_PWRREG(0x520)
#define S5P_APLL_CON_L8			S5P_PWRREG(0x600)
#define S5P_APLL_CON_L7			S5P_PWRREG(0x604)
#define S5P_APLL_CON_L6			S5P_PWRREG(0x608)
#define S5P_APLL_CON_L5			S5P_PWRREG(0x60c)
#define S5P_APLL_CON_L4			S5P_PWRREG(0x610)
#define S5P_APLL_CON_L3			S5P_PWRREG(0x614)
#define S5P_APLL_CON_L2			S5P_PWRREG(0x618)
#define S5P_APLL_CON_L1			S5P_PWRREG(0x61c)
#define S5P_EM_CONTROL			S5P_PWRREG(0x620)

#define S5P_CLKDIV_IEM_L8		S5P_PWRREG(0x700)
#define S5P_CLKDIV_IEM_L7		S5P_PWRREG(0x704)
#define S5P_CLKDIV_IEM_L6		S5P_PWRREG(0x708)
#define S5P_CLKDIV_IEM_L5		S5P_PWRREG(0x70c)
#define S5P_CLKDIV_IEM_L4		S5P_PWRREG(0x710)
#define S5P_CLKDIV_IEM_L3		S5P_PWRREG(0x714)
#define S5P_CLKDIV_IEM_L2		S5P_PWRREG(0x718)
#define S5P_CLKDIV_IEM_L1		S5P_PWRREG(0x71c)

#define S5P_IEM_HPMCLK_DIV		S5P_PWRREG(0x724)

#endif
