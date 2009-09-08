/*
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
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
#include <asm/arch/clock.h>

#define APLL 0
#define MPLL 1
#define EPLL 2
#define HPLL 3
#define VPLL 4

#ifndef CONFIG_SYS_CLK_FREQ_C100
#define CONFIG_SYS_CLK_FREQ_C100	12000000
#endif
#ifndef CONFIG_SYS_CLK_FREQ_C110
#define CONFIG_SYS_CLK_FREQ_C110	24000000
#endif

static int s5pc1xx_clock_read_reg(int offset)
{
	return readl(S5PC1XX_CLOCK_BASE + offset);
}

/*
 * CONFIG_SYS_CLK_FREQ_C1xx should be defined as the input frequency
 *
 * get_FCLK(), get_HCLK(), get_PCLK() and get_UCLK() return the clock of
 * the specified bus in HZ
 */
unsigned long get_pll_clk(int pllreg)
{
	unsigned long r, m, p, s, mask, fout;
	unsigned int freq;

	switch (pllreg) {
	case APLL:
		if (cpu_is_s5pc110())
			r = s5pc1xx_clock_read_reg(S5PC110_APLL_CON_OFFSET);
		else
			r = s5pc1xx_clock_read_reg(S5PC100_APLL_CON_OFFSET);
		break;
	case MPLL:
		if (cpu_is_s5pc110())
			r = s5pc1xx_clock_read_reg(S5PC110_MPLL_CON_OFFSET);
		else
			r = s5pc1xx_clock_read_reg(S5PC100_MPLL_CON_OFFSET);
		break;
	case EPLL:
		if (cpu_is_s5pc110())
			r = s5pc1xx_clock_read_reg(S5PC110_EPLL_CON_OFFSET);
		else
			r = s5pc1xx_clock_read_reg(S5PC100_EPLL_CON_OFFSET);
		break;
	case HPLL:
		if (cpu_is_s5pc110()) {
			puts("s5pc110 don't use HPLL\n");
			return 0;
		}
		r = s5pc1xx_clock_read_reg(S5PC100_HPLL_CON_OFFSET);
		break;
	case VPLL:
		if (cpu_is_s5pc100()) {
			puts("s5pc100 don't use VPLL\n");
			return 0;
		}
		r = s5pc1xx_clock_read_reg(S5PC110_VPLL_CON_OFFSET);
		break;
	default:
		printf("Unsupported PLL (%d)\n", pllreg);
		return 0;
	}

	if (cpu_is_s5pc110()) {
		/*
		 * APLL_CON: MIDV [25:16]
		 * MPLL_CON: MIDV [25:16]
		 * EPLL_CON: MIDV [24:16]
		 * VPLL_CON: MIDV [24:16]
		 */
		if (pllreg == APLL || pllreg == MPLL)
			mask = 0x3ff;
		else
			mask = 0x1ff;
	} else {
		/*
		 * APLL_CON: MIDV [25:16]
		 * MPLL_CON: MIDV [23:16]
		 * EPLL_CON: MIDV [23:16]
		 * HPLL_CON: MIDV [23:16]
		 */
		if (pllreg == APLL)
			mask = 0x3ff;
		else
			mask = 0x0ff;
	}
	m = (r >> 16) & mask;

	/* PDIV [13:8] */
	p = (r >> 8) & 0x3f;
	/* SDIV [2:0] */
	s = r & 0x7;

	if (cpu_is_s5pc110()) {
		freq = CONFIG_SYS_CLK_FREQ_C110;
		if (pllreg == APLL) {
			if (s < 1)
				s = 1;
			/* FOUT = MDIV * FIN / (PDIV * 2^(SDIV - 1)) */
			fout = m * (freq / (p * (1 << (s - 1))));
		} else
			/* FOUT = MDIV * FIN / (PDIV * 2^SDIV) */
			fout = m * (freq / (p * (1 << s)));
	} else {
		/* FOUT = MDIV * FIN / (PDIV * 2^SDIV) */
		freq = CONFIG_SYS_CLK_FREQ_C100;
		fout = m * (freq / (p * (1 << s)));
	}

	return fout;
}

/* return ARMCORE frequency */
unsigned long get_arm_clk(void)
{
	unsigned long div;
	unsigned long dout_apll, armclk;
	unsigned int apll_ratio, arm_ratio;

	div = s5pc1xx_clock_read_reg(S5P_CLK_DIV0_OFFSET);
	if (cpu_is_s5pc110()) {
		/* ARM_RATIO: don't use */
		arm_ratio = 0;
		/* APLL_RATIO: [2:0] */
		apll_ratio = div & 0x7;
	} else {
		/* ARM_RATIO: [6:4] */
		arm_ratio = (div >> 4) & 0x7;
		/* APLL_RATIO: [0] */
		apll_ratio = div & 0x1;
	}

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

/* s5pc100: return HCLKD0 frequency */
unsigned long get_hclk(void)
{
	unsigned long hclkd0;
	uint div, d0_bus_ratio;

	div = s5pc1xx_clock_read_reg(S5P_CLK_DIV0_OFFSET);
	/* D0_BUS_RATIO: [10:8] */
	d0_bus_ratio = (div >> 8) & 0x7;

	hclkd0 = get_arm_clk() / (d0_bus_ratio + 1);

	return hclkd0;
}

/* s5pc100: return PCLKD0 frequency */
unsigned long get_pclkd0(void)
{
	unsigned long pclkd0;
	uint div, pclkd0_ratio;

	div = s5pc1xx_clock_read_reg(S5P_CLK_DIV0_OFFSET);
	/* PCLKD0_RATIO: [14:12] */
	pclkd0_ratio = (div >> 12) & 0x7;

	pclkd0 = get_hclk() / (pclkd0_ratio + 1);

	return pclkd0;
}

/* s5pc100: return PCLKD1 frequency */
unsigned long get_pclkd1(void)
{
	unsigned long d1_bus, pclkd1;
	uint div, d1_bus_ratio, pclkd1_ratio;

	div = s5pc1xx_clock_read_reg(S5P_CLK_DIV1_OFFSET);
	/* D1_BUS_RATIO: [14:12] */
	d1_bus_ratio = (div >> 12) & 0x7;
	/* PCLKD1_RATIO: [18:16] */
	pclkd1_ratio = (div >> 16) & 0x7;

	/* ASYNC Mode */
	d1_bus = get_pll_clk(MPLL) / (d1_bus_ratio + 1);
	pclkd1 = d1_bus / (pclkd1_ratio + 1);

	return pclkd1;
}

/* s5pc110: return HCLKs frequency */
unsigned long get_hclk_sys(int clk)
{
	unsigned long hclk;
	unsigned int div;
	unsigned int offset;
	unsigned int hclk_sys_ratio;

	if (clk == CLK_M)
		return get_hclk();

	div = s5pc1xx_clock_read_reg(S5P_CLK_DIV0_OFFSET);

	/*
	 * HCLK_MSYS_RATIO: [10:8]
	 * HCLK_DSYS_RATIO: [19:16]
	 * HCLK_PSYS_RATIO: [27:24]
	 */
	offset = 8 + (clk << 0x3);

	hclk_sys_ratio = (div >> offset) & 0xf;

	hclk = get_pll_clk(MPLL) / (hclk_sys_ratio + 1);

	return hclk;
}

/* s5pc110: return PCLKs frequency */
unsigned long get_pclk_sys(int clk)
{
	unsigned long pclk;
	unsigned int div;
	unsigned int offset;
	unsigned int pclk_sys_ratio;

	div = s5pc1xx_clock_read_reg(S5P_CLK_DIV0_OFFSET);

	/*
	 * PCLK_MSYS_RATIO: [14:12]
	 * PCLK_DSYS_RATIO: [22:20]
	 * PCLK_PSYS_RATIO: [30:28]
	 */
	offset = 12 + (clk << 0x3);

	pclk_sys_ratio = (div >> offset) & 0x7;

	pclk = get_hclk_sys(clk) / (pclk_sys_ratio + 1);

	return pclk;
}

/* return peripheral clock */
unsigned long get_pclk(void)
{
	if (cpu_is_s5pc110())
		return get_pclk_sys(CLK_P);
	else
		return get_pclkd1();
}

/* return UCLK frequency */
unsigned long get_uclk(void)
{
	return get_pll_clk(EPLL);
}
