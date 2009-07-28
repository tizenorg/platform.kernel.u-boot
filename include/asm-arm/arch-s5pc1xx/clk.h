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

#ifndef __ASM_ARM_ARCH_CLK_H_
#define __ASM_ARM_ARCH_CLK_H_

unsigned long get_pll_clk(int pllreg);
unsigned long get_arm_clk(void);
unsigned long get_fclk(void);
unsigned long get_mclk(void);
unsigned long get_hclk(void);
unsigned long get_pclk(void);
unsigned long get_uclk(void);

/*s5pc110 */
#define CLK_M	0
#define CLK_D	1
#define CLK_P	2

unsigned long get_hclk_sys(int clk);
unsigned long get_pclk_sys(int clk);

/* s5pc100 */
unsigned long get_pclkd0(void);
unsigned long get_pclkd1(void);

#endif
