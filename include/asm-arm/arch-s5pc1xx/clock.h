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

#ifndef __ASM_ARM_ARCH_CLOCK_H_
#define __ASM_ARM_ARCH_CLOCK_H_

/*
 * Clock control
 */
#define S5P_CLKREG(x)           (S5P_PA_CLK + (x))

/* Clock Register */
#define S5P_APLL_LOCK           S5P_CLKREG(0x0)
#define S5P_MPLL_LOCK           S5P_CLKREG(0x4)
#define S5P_EPLL_LOCK           S5P_CLKREG(0x8)
#define S5P_HPLL_LOCK           S5P_CLKREG(0xc)

#define S5P_APLL_CON            S5P_CLKREG(0x100)
#define S5P_MPLL_CON            S5P_CLKREG(0x104)
#define S5P_EPLL_CON            S5P_CLKREG(0x108)
#define S5P_HPLL_CON            S5P_CLKREG(0x10c)

#define S5P_CLK_SRC0            S5P_CLKREG(0x200)
#define S5P_CLK_SRC1            S5P_CLKREG(0x204)
#define S5P_CLK_SRC2            S5P_CLKREG(0x208)
#define S5P_CLK_SRC3            S5P_CLKREG(0x20c)

#define S5P_CLK_DIV0            S5P_CLKREG(0x300)
#define S5P_CLK_DIV1            S5P_CLKREG(0x304)
#define S5P_CLK_DIV2            S5P_CLKREG(0x308)
#define S5P_CLK_DIV3            S5P_CLKREG(0x30c)
#define S5P_CLK_DIV4            S5P_CLKREG(0x310)

#define S5P_CLK_OUT             S5P_CLKREG(0x400)

#define S5P_CLK_GATE_D00        S5P_CLKREG(0x500)
#define S5P_CLK_GATE_D01        S5P_CLKREG(0x504)
#define S5P_CLK_GATE_D02        S5P_CLKREG(0x508)

#define S5P_CLK_GATE_D10        S5P_CLKREG(0x520)
#define S5P_CLK_GATE_D11        S5P_CLKREG(0x524)
#define S5P_CLK_GATE_D12        S5P_CLKREG(0x528)
#define S5P_CLK_GATE_D13        S5P_CLKREG(0x530)
#define S5P_CLK_GATE_D14        S5P_CLKREG(0x534)

#define S5P_CLK_GATE_D20        S5P_CLKREG(0x540)

#define S5P_CLK_GATE_SCLK0      S5P_CLKREG(0x560)
#define S5P_CLK_GATE_SCLK1      S5P_CLKREG(0x564)

#endif
