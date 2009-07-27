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
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>

#define APLL 0
#define MPLL 1
#define EPLL 2
#define HPLL 3
#define VPLL 4

static int s5p1xx_clock_read_reg(int offset)
{
	return readl(S5P_CLOCK_BASE + offset);
}

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

unsigned long get_pll_clk(int pllreg)
{
	unsigned long r, m, p, s, mask, fout;

	switch (pllreg) {
	case APLL:
		if (cpu_is_s5pc110())
			r = s5p1xx_clock_read_reg(S5PC110_APLL_CON_OFFSET);
		else
			r = s5p1xx_clock_read_reg(S5PC100_APLL_CON_OFFSET);
		break;
	case MPLL:
		if (cpu_is_s5pc110())
			r = s5p1xx_clock_read_reg(S5PC110_MPLL_CON_OFFSET);
		else
			r = s5p1xx_clock_read_reg(S5PC100_MPLL_CON_OFFSET);
		break;
	case EPLL:
		if (cpu_is_s5pc110())
			r = s5p1xx_clock_read_reg(S5PC110_EPLL_CON_OFFSET);
		else
			r = s5p1xx_clock_read_reg(S5PC100_EPLL_CON_OFFSET);
		break;
	case HPLL:
		if (cpu_is_s5pc110())
			hang();
		r = s5p1xx_clock_read_reg(S5PC100_HPLL_CON_OFFSET);
		break;
	case VPLL:
		if (cpu_is_s5pc100())
			hang();
		r = s5p1xx_clock_read_reg(S5PC110_VPLL_CON_OFFSET);
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

	if (cpu_is_s5pc110()) {
		if (pllreg == APLL)
			fout = m * CONFIG_SYS_CLK_FREQ / (p * (s * 2 - 1));
		else
			fout = m * CONFIG_SYS_CLK_FREQ / (p * s * 2);
	}
	else {
		fout = m * CONFIG_SYS_CLK_FREQ / (p * (1 << s));
	}

	return fout;
}

/* return ARMCORE frequency */
unsigned long get_arm_clk(void)
{
	unsigned long div;
	unsigned long dout_apll, armclk;
	unsigned int apll_ratio, arm_ratio;;

	div = s5p1xx_clock_read_reg(S5P_CLK_DIV0_OFFSET);
	/* ARM_RATIO: [6:4] */
	arm_ratio = (div >> 4) & 0x7;
	/* APLL_RATIO: [0] */
	apll_ratio = div & 0x1;

	dout_apll = get_pll_clk(APLL) / (apll_ratio + 1);
	armclk = dout_apll / (arm_ratio + 1);

	return armclk;
}

/* return FCLK frequency */
unsigned long get_fclk(void)
{
	return get_pll_clk(APLL);
}

/* return MCLK frequency */
unsigned long get_mclk(void)
{
	return get_pll_clk(MPLL);
}

/* return HCLKD0 frequency */
unsigned long get_hclk(void)
{
	unsigned long hclkd0;
	uint div, d0_bus_ratio;

	div = s5p1xx_clock_read_reg(S5P_CLK_DIV0_OFFSET);
	/* D0_BUS_RATIO: [10:8] */
	d0_bus_ratio = (div >> 8) & 0x7;

	hclkd0 = get_arm_clk() / (d0_bus_ratio + 1);

	return hclkd0;
}

/* return PCLKD0 frequency */
unsigned long get_pclkd0(void)
{
	unsigned long pclkd0;
	uint div, pclkd0_ratio;

	div = s5p1xx_clock_read_reg(S5P_CLK_DIV0_OFFSET);
	/* PCLKD0_RATIO: [14:12] */
	pclkd0_ratio = (div >> 12) & 0x7;

	pclkd0 = get_hclk() / (pclkd0_ratio + 1);

	return pclkd0;
}

/* return PCLKD1 frequency */
unsigned long get_pclk(void)
{
	unsigned long d1_bus, pclkd1;
	uint div, d1_bus_ratio, pclkd1_ratio;

	div = s5p1xx_clock_read_reg(S5P_CLK_DIV1_OFFSET);
	/* D1_BUS_RATIO: [14:12] */
	d1_bus_ratio = (div >> 12) & 0x7;
	/* PCLKD1_RATIO: [18:16] */
	pclkd1_ratio = (div >> 16) & 0x7;

	/* ASYNC Mode */
	d1_bus = get_pll_clk(MPLL) / (d1_bus_ratio + 1);
	pclkd1 = d1_bus / (pclkd1_ratio + 1);

	return pclkd1;
}

/* return UCLK frequency */
unsigned long get_uclk(void)
{
	return get_pll_clk(EPLL);
}
