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
#define S5P_CLOCK_BASE			S5P_PA_CLK

/* Clock Register */
#define S5PC100_APLL_LOCK_OFFSET	0x0
#define S5PC100_MPLL_LOCK_OFFSET	0x4
#define S5PC100_EPLL_LOCK_OFFSET	0x8
#define S5PC100_HPLL_LOCK_OFFSET	0xc

#define S5PC100_APLL_CON_OFFSET		0x100
#define S5PC100_MPLL_CON_OFFSET		0x104
#define S5PC100_EPLL_CON_OFFSET		0x108
#define S5PC100_HPLL_CON_OFFSET		0x10c

#define S5PC110_APLL_LOCK_OFFSET	0x00
#define S5PC110_MPLL_LOCK_OFFSET	0x08
#define S5PC110_EPLL_LOCK_OFFSET	0x10
#define S5PC110_VPLL_LOCK_OFFSET	0x20

#define S5PC110_APLL_CON_OFFSET		0x100
#define S5PC110_MPLL_CON_OFFSET		0x108
#define S5PC110_EPLL_CON_OFFSET		0x110
#define S5PC110_VPLL_CON_OFFSET		0x120

#define S5P_CLK_SRC0_OFFSET		0x200
#define S5P_CLK_SRC1_OFFSET		0x204
#define S5P_CLK_SRC2_OFFSET		0x208
#define S5P_CLK_SRC3_OFFSET		0x20c

#define S5P_CLK_DIV0_OFFSET		0x300
#define S5P_CLK_DIV1_OFFSET		0x304
#define S5P_CLK_DIV2_OFFSET		0x308
#define S5P_CLK_DIV3_OFFSET		0x30c
#define S5P_CLK_DIV4_OFFSET		0x310

#define S5P_CLK_OUT_OFFSET		0x400

#define S5P_CLK_GATE_D00_OFFSET		0x500
#define S5P_CLK_GATE_D01_OFFSET		0x504
#define S5P_CLK_GATE_D02_OFFSET		0x508

#define S5P_CLK_GATE_D10_OFFSET		0x520
#define S5P_CLK_GATE_D11_OFFSET		0x524
#define S5P_CLK_GATE_D12_OFFSET		0x528
#define S5P_CLK_GATE_D13_OFFSET		0x530
#define S5P_CLK_GATE_D14_OFFSET		0x534

#define S5P_CLK_GATE_D20_OFFSET		0x540

#define S5P_CLK_GATE_SCLK0_OFFSET	0x560
#define S5P_CLK_GATE_SCLK1_OFFSET	0x564

#endif
