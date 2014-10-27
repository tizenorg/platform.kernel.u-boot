/*
 * Copyright (C) 2014 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/power.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/sromc.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned int get_board_rev(void)
{
	unsigned int rev = 0;
	return rev;
}

int exynos_init(void)
{
	return 0;
}

static int board_clock_init(void)
{
	unsigned int set, clr, clr_src_cpu, clr_pll_con0;
	struct exynos5420_clock *clk = (struct exynos5420_clock *)
						samsung_get_base_clock();
	/*
	 * CMU_CPU clocks src to MPLL
	 * Bit values:                 0  ; 1
	 * MUX_APLL_SEL:        FIN_PLL   ; FOUT_APLL
	 * MUX_CORE_SEL:        MOUT_APLL ; SCLK_MPLL
	 * MUX_HPM_SEL:         MOUT_APLL ; SCLK_MPLL_USER_C
	 * MUX_MPLL_USER_SEL_C: FIN_PLL   ; SCLK_MPLL
	*/

	/* Set CMU_CPU clocks src to OSCCLK */
	clr_src_cpu = MUX_APLL_SEL(1) | MUX_CORE_SEL(1);
	set = MUX_APLL_SEL(0) | MUX_CORE_SEL(1);

	clrsetbits_le32(&clk->src_cpu, clr_src_cpu, set);

	while (MUX_STAT_CPU_CHANGING(readl(&clk->mux_stat_cpu)))
		continue;

	/* Set APLL to 1200MHz */
	clr_pll_con0 = SDIV(7) | PDIV(63) | MDIV(1023) | FSEL(1) |
			PLL_ENABLE(1);
	set = SDIV(0) | PDIV(2) | MDIV(100) | PLL_ENABLE(1);

	clrsetbits_le32(&clk->apll_con0, clr_pll_con0, set);

	while (!(readl(&clk->apll_con0) & PLL_LOCKED_BIT))
		continue;

	/* Set CMU_CPU clocks src to APLL */
	set = MUX_APLL_SEL(1) | MUX_CORE_SEL(0);
	clrsetbits_le32(&clk->src_cpu, clr_src_cpu, set);

	while (MUX_STAT_CPU_CHANGING(readl(&clk->mux_stat_cpu)))
		continue;

	clr = ARM_RATIO(7) | CPUD_RATIO(7) | ATB_RATIO(7) |
	      PCLK_DBG_RATIO(7) | APLL_RATIO(7) | ARM2_RATIO(7);
	set = ARM_RATIO(0) | CPUD_RATIO(2) | ATB_RATIO(5) |
	      PCLK_DBG_RATIO(5) | APLL_RATIO(0) | ARM2_RATIO(0);

	clrsetbits_le32(&clk->div_cpu0, clr, set);

	while (readl(&clk->div_stat_cpu0) & DIV_STAT_CPU0_CHANGING)
		continue;

	/* Set MPLL to 800MHz */
	set = SDIV(1) | PDIV(3) | MDIV(200) | PLL_ENABLE(1);

	clrsetbits_le32(&clk->mpll_con0, clr_pll_con0, set);

	while (!(readl(&clk->mpll_con0) & PLL_LOCKED_BIT))
		continue;

	/* Set CLKMUX_UART src to MPLL */
	clr = UART0_SEL(7) | UART1_SEL(7) | UART2_SEL(7) | UART3_SEL(7);
	set = UART0_SEL(3) | UART1_SEL(3) | UART2_SEL(3) | UART3_SEL(3);

	clrsetbits_le32(&clk->src_peric0, clr, set);

	/* Set SCLK_UART to 400 MHz (MPLL / 2) */
	clr = UART0_RATIO(15) | UART1_RATIO(15) | UART2_RATIO(15) |
	      UART3_RATIO(15);
	set = UART0_RATIO(1) | UART1_RATIO(1) | UART2_RATIO(1) |
	      UART3_RATIO(1);

	clrsetbits_le32(&clk->div_peric0, clr, set);

	while (readl(&clk->div_stat_peric0) & DIV_STAT_PERIC0_CHANGING)
		continue;

	/* Set CLKMUX_MMC src to MPLL */
	clr = MUX_MMC0_SEL(7) | MUX_MMC1_SEL(7) | MUX_MMC2_SEL(7);
	set = MUX_MMC0_SEL(3) | MUX_MMC1_SEL(3) | MUX_MMC2_SEL(3);

	clrsetbits_le32(&clk->src_fsys, clr, set);

	clr = MMC0_RATIO(0x3ff) | MMC1_RATIO(0x3ff) | MMC1_RATIO(0x3ff);
	set = MMC0_RATIO(0) | MMC1_RATIO(0) | MMC1_RATIO(0);

	clrsetbits_le32(&clk->div_fsys1, clr, set);

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_fsys1) & DIV_STAT_FSYS1_CHANGING)
		continue;

	return 0;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int exynos_early_init_f(void)
{
	return board_clock_init();
}
#endif
