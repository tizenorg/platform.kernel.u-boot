/*
 * (C) Copyright 2009
 * Inki Dae, SAMSUNG Electronics, <inki.dae@samsung.com>
 * Heungjun Kim, SAMSUNG Electronics, <riverful.kim@samsung.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clk.h>

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	unsigned int pid = readl(S5P_PRO_ID);

	pid >>= 12;
	pid &= 0x00fff;

	printf("CPU:\tS5PC%x@%luMHz\n", pid, get_arm_clk() / 1000000);
	printf("\tFclk = %luMHz, HclkD0 = %luMHz, PclkD0 = %luMHz,"
		" PclkD1 = %luMHz\n",
		get_fclk() / 1000000, get_hclk() / 1000000,
		get_pclkd0() / 1000000, get_pclk() / 1000000);

	return 0;
}
#endif
