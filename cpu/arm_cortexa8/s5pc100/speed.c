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

/*
 * This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same PLL and clock machinery inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <common.h>

#define APLL 0
#define MPLL 1
#define EPLL 2
#define HPLL 3

/* ------------------------------------------------------------------------- */
/*
 * NOTE: This describes the proper use of this file.
 *
 * CONFIG_SYS_CLK_FREQ should be defined as the input frequency of the PLL.
 *
 * get_FCLK(), get_HCLK(), get_PCLK() and get_UCLK() return the clock of
 * the specified bus in HZ.
 */
/* ------------------------------------------------------------------------- */

static ulong get_PLLCLK(int pllreg)
{
	ulong r, m, p, s, mask;

	switch (pllreg) {
	case APLL:
		r = S5P_APLL_CON_REG;
		break;
	case MPLL:
		r = S5P_MPLL_CON_REG;
		break;
	case EPLL:
		r = S5P_EPLL_CON_REG;
		break;
	case HPLL:
		r = S5P_HPLL_CON_REG;
		break;
	default:
		hang();
	}

	if (pllreg == APLL)
		mask = 0x3ff;
	else
		mask = 0x0ff;

	m = (r >> 16) & mask;
	p = (r >> 8) & 0x3f;
	s = r & 0x7;

	return m * (CONFIG_SYS_CLK_FREQ / (p * (1 << s)));
}

/* return ARMCORE frequency */
ulong get_ARMCLK(void)
{
	ulong div;
	unsigned long dout_apll, armclk;
	unsigned int apll_ratio, arm_ratio;;

	div = S5P_CLK_DIV0_REG;
	/* ARM_RATIO: [6:4] */
	arm_ratio = (div >> 4) & 0x7;
	/* APLL_RATIO: [0] */
	apll_ratio = div & 0x1;

	dout_apll = get_PLLCLK(APLL) / (apll_ratio + 1);
	armclk = dout_apll / (arm_ratio + 1);

	return armclk;
}

/* return FCLK frequency */
ulong get_FCLK(void)
{
	return get_PLLCLK(APLL);
}

/* return MCLK frequency */
ulong get_MCLK(void)
{
	return get_PLLCLK(MPLL);
}

/* return HCLKD0 frequency */
ulong get_HCLK(void)
{
	ulong hclkd0;
	uint div, d0_bus_ratio;

	div = S5P_CLK_DIV0_REG;
	/* D0_BUS_RATIO: [10:8] */
	d0_bus_ratio = (div >> 8) & 0x7;

	hclkd0 = get_ARMCLK() / (d0_bus_ratio + 1);

	return hclkd0;
}

/* return PCLKD0 frequency */
ulong get_PCLKD0(void)
{
	ulong pclkd0;
	uint div, pclkd0_ratio;

	div = S5P_CLK_DIV0_REG;
	/* PCLKD0_RATIO: [14:12] */
	pclkd0_ratio = (div >> 12) & 0x7;

	pclkd0 = get_HCLK() / (pclkd0_ratio + 1);

	return pclkd0;
}

/* return PCLKD1 frequency */
ulong get_PCLK(void)
{
	ulong d1_bus, pclkd1;
	uint div, d1_bus_ratio, pclkd1_ratio;

	div = S5P_CLK_DIV1_REG;
	/* D1_BUS_RATIO: [14:12] */
	d1_bus_ratio = (div >> 12) & 0x7;
	/* PCLKD1_RATIO: [18:16] */
	pclkd1_ratio = (div >> 16) & 0x7;

	/* ASYNC Mode */
	d1_bus = get_PLLCLK(MPLL) / (d1_bus_ratio + 1);
	pclkd1 = d1_bus / (pclkd1_ratio + 1);

	return pclkd1;
}

/* return UCLK frequency */
ulong get_UCLK(void)
{
	return get_PLLCLK(EPLL);
}

int print_cpuinfo(void)
{
	printf("CPU:\tS5PC100@%luMHz\n", get_ARMCLK() / 1000000);
	printf("\tFclk = %luMHz, HclkD0 = %luMHz, PclkD0 = %luMHz,"
		" PclkD1 = %luMHz\n",
			get_FCLK() / 1000000, get_HCLK() / 1000000,
			get_PCLKD0() / 1000000, get_PCLK() / 1000000);

	return 0;
}
