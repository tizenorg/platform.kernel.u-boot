/*
	 * Copyright (C) 2014 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/sections.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <asm/arch/cpu.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <power/battery.h>
#include <power/max77693_pmic.h>
#include <power/max77693_muic.h>
#include <power/max77693_fg.h>
#include <errno.h>
#include <usb.h>
#include <usb/s3c_udc.h>
#include <usb_mass_storage.h>
#include <fdt.h>
#include <lcd.h>
#include <asm/arch/mipi_dsim.h>
#include <samsung/misc.h>
#include <libtizen.h>
#include "setup.h"

DECLARE_GLOBAL_DATA_PTR;

#define HWREV_TRATS2_CFG_REG	0x11000280
#define HWREV_TRATS2_CFG	0x0
#define HWREV_TRATS2_PULL_REG	0x11000288
#define HWREV_TRATS2_PULL	0x33000033
#define HWREV_TRATS2_DAT_REG	0x11000284
#define HWREV_TRATS2_DAT_MASK	0xf0

#define CPU_MAIN_REV_MASK	0xf0
#define CPU_MAIN_REV_SHIFT	0x4
#define CPU_MAIN_REV(x)		((x >> CPU_MAIN_REV_SHIFT) & CPU_MAIN_REV_MASK)

#define DTB_PADDING		0x4

/* For global battery and charger functions */
static struct power_battery *pbat;
static struct pmic *p_chrg, *p_muic, *p_fg, *p_bat;
static int power_init_done;

#ifdef CONFIG_BOARD_TYPES
/* Supported Exynos4 boards */
enum {
	BOARD_TYPE_TRATS2,
	BOARD_TYPE_ODROID_U3,
	BOARD_TYPE_ODROID_X2,
	BOARD_TYPES_NUM,
};

static const int board_arch_num[] = {
	MACH_TYPE_SMDK4412,
	MACH_TYPE_ODROIDX,
	MACH_TYPE_ODROIDX,
};

static const char *board_compat[] = {
	"samsung,trats2",
	"samsung,odroid",
	"samsung,odroid",
};

static const char *board_name[] = {
	"trats",
	"odroid",
	"odroid",
};

static const char *board_model[] = {
	"2",
	"u3",
	"x2",
};

extern void sdelay(unsigned long);

void set_board_type(void)
{
	int hwrev = 0;
	unsigned int addr;

	/* GPM1[5:2] as input */
	writel(HWREV_TRATS2_CFG, HWREV_TRATS2_CFG_REG);
	writel(HWREV_TRATS2_PULL, HWREV_TRATS2_PULL_REG);
	hwrev = readl(HWREV_TRATS2_DAT_REG) & HWREV_TRATS2_DAT_MASK;

	/* Any Trats2 revision is valid */
	if (hwrev) {
		gd->board_type = BOARD_TYPE_TRATS2;
		return;
	}

	/* Set GPA1 pin 1 to HI - enable XCL205 output */
	/* GPA1CON 0x11400020 */
	addr = 0x11400020;
	writel(0x10, addr);

	/* GPA1DAT 0x11400024 */
	addr = 0x11400024;
	writel(0x2, addr);

	/* GPA1PUD 0x11400028 */
	addr = 0x11400028;
	writel(0xc, addr);

	/* GPA1DRV 0x1140002c */
	addr = 0x1140002c;
	writel(0xc, addr);

	/* Check GPC1 pin 2 input */
	/* GPC1CON 0x11400020 */
	addr = 0x11400080;
	writel(0x0, addr);

	/* GPC1PUD 0x11400028 */
	addr = 0x11400088;
	writel(0x0, addr);

	/* XCL205 - needs some latch time */
	sdelay(200000);

	/* GPC1DAT 0x11400024 */
	addr = 0x11400084;

	/* Check GPC1 pin2 - LED supplied by XCL205 - X2 only */
	if (readl(addr) & 0x4)
		gd->board_type = BOARD_TYPE_ODROID_X2;
	else
		gd->board_type = BOARD_TYPE_ODROID_U3;
}

int board_is_trats2(void)
{
	if (gd->board_type == BOARD_TYPE_TRATS2)
		return 1;

	return 0;
}

int board_is_odroid_x2(void)
{
	if (gd->board_type == BOARD_TYPE_ODROID_X2)
		return 1;

	return 0;
}

int board_is_odroid_u3(void)
{
	if (gd->board_type == BOARD_TYPE_ODROID_U3)
		return 1;

	return 0;
}

const char *get_board_name(void)
{
	return board_name[gd->board_type];
}

const char *get_board_type(void)
{
	return board_model[gd->board_type];
}
#endif

#ifdef CONFIG_OF_MULTI
unsigned long *get_board_fdt(void)
{
	unsigned long *fdt = (unsigned long *)&_end;
	unsigned long size;
	char *compat = NULL;
	int i;

	set_board_type();

	for (i = 0; i < BOARD_TYPES_NUM; i++) {
		if (!fdt || fdt_magic(fdt) != FDT_MAGIC)
			break;

		compat = (char *) fdt_getprop(fdt, 0, "compatible", NULL);
		if (compat && !strcmp(board_compat[gd->board_type], compat))
				return fdt;

		/* Byte size */
		size = fdt_totalsize(fdt);

		fdt += (unsigned long)(roundup(size, DTB_PADDING) >> 2);
	}

	return NULL;
}
#endif

#ifdef CONFIG_SET_DFU_ALT_INFO
char *get_dfu_alt_system(void)
{
	char *alt_system;

	if (board_is_trats2())
		alt_system = getenv("dfu_alt_system_trats2");
	else
		alt_system = getenv("dfu_alt_system_odroid");

	return alt_system;
}

char *get_dfu_alt_boot(void)
{
	char *alt_boot;

	switch (get_boot_mode()) {
	case BOOT_MODE_SD:
		if (board_is_trats2())
			alt_boot = DFU_ALT_BOOT_SD_TRATS2;
		else
			alt_boot = DFU_ALT_BOOT_SD_ODROID;
		break;
	case BOOT_MODE_EMMC:
	case BOOT_MODE_EMMC_SD:
		if (board_is_trats2())
			alt_boot = DFU_ALT_BOOT_EMMC_TRATS2;
		else
			alt_boot = DFU_ALT_BOOT_EMMC_ODROID;
		break;
	default:
		alt_boot = NULL;
		break;
	}

	if (!alt_boot)
		return alt_boot;

	return alt_boot;
}
#endif

static void board_clock_init(void)
{
	unsigned int set, clr, clr_src_cpu, clr_pll_con0, clr_src_dmc;
	struct exynos4x12_clock *clk = (struct exynos4x12_clock *)
						samsung_get_base_clock();

	/*
	 * CMU_CPU clocks src to MPLL
	 * Bit values:                 0  ; 1
	 * MUX_APLL_SEL:        FIN_PLL   ; FOUT_APLL
	 * MUX_CORE_SEL:        MOUT_APLL ; SCLK_MPLL
	 * MUX_HPM_SEL:         MOUT_APLL ; SCLK_MPLL_USER_C
	 * MUX_MPLL_USER_SEL_C: FIN_PLL   ; SCLK_MPLL
	*/
	clr_src_cpu = MUX_APLL_SEL(0x1) | MUX_CORE_SEL(0x1) |
		      MUX_HPM_SEL(0x1) | MUX_MPLL_USER_SEL_C(0x1);
	set = MUX_APLL_SEL(0) | MUX_CORE_SEL(1) | MUX_HPM_SEL(1) |
	      MUX_MPLL_USER_SEL_C(1);

	clrsetbits_le32(&clk->src_cpu, clr_src_cpu, set);

	/* Wait for mux change */
	while (readl(&clk->mux_stat_cpu) & MUX_STAT_CPU_CHANGING)
		continue;

	/* Set APLL to 1000MHz */
	clr_pll_con0 = SDIV(0x7) | PDIV(0x3f) | MDIV(0x3ff) | FSEL(0x1);
	set = SDIV(0) | PDIV(3) | MDIV(125) | FSEL(1);

	clrsetbits_le32(&clk->apll_con0, clr_pll_con0, set);

	/* Wait for PLL to be locked */
	while (!(readl(&clk->apll_con0) & PLL_LOCKED_BIT))
		continue;

	/* Set CMU_CPU clocks src to APLL */
	set = MUX_APLL_SEL(1) | MUX_CORE_SEL(0) | MUX_HPM_SEL(0) |
	      MUX_MPLL_USER_SEL_C(1);
	clrsetbits_le32(&clk->src_cpu, clr_src_cpu, set);

	/* Wait for mux change */
	while (readl(&clk->mux_stat_cpu) & MUX_STAT_CPU_CHANGING)
		continue;

	set = CORE_RATIO(0) | COREM0_RATIO(2) | COREM1_RATIO(5) |
	      PERIPH_RATIO(0) | ATB_RATIO(4) | PCLK_DBG_RATIO(1) |
	      APLL_RATIO(0) | CORE2_RATIO(0);
	/*
	 * Set dividers for MOUTcore = 1000 MHz
	 * coreout =      MOUT / (ratio + 1) = 1000 MHz (0)
	 * corem0 =     armclk / (ratio + 1) = 333 MHz (2)
	 * corem1 =     armclk / (ratio + 1) = 166 MHz (5)
	 * periph =     armclk / (ratio + 1) = 1000 MHz (0)
	 * atbout =       MOUT / (ratio + 1) = 200 MHz (4)
	 * pclkdbgout = atbout / (ratio + 1) = 100 MHz (1)
	 * sclkapll = MOUTapll / (ratio + 1) = 1000 MHz (0)
	 * core2out = core_out / (ratio + 1) = 1000 MHz (0) (armclk)
	*/
	clr = CORE_RATIO(0x7) | COREM0_RATIO(0x7) | COREM1_RATIO(0x7) |
	      PERIPH_RATIO(0x7) | ATB_RATIO(0x7) | PCLK_DBG_RATIO(0x7) |
	      APLL_RATIO(0x7) | CORE2_RATIO(0x7);

	clrsetbits_le32(&clk->div_cpu0, clr, set);

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_cpu0) & DIV_STAT_CPU0_CHANGING)
		continue;

	/*
	 * For MOUThpm = 1000 MHz (MOUTapll)
	 * doutcopy = MOUThpm / (ratio + 1) = 200 (4)
	 * sclkhpm = doutcopy / (ratio + 1) = 200 (4)
	 * cores_out = armclk / (ratio + 1) = 200 (4)
	 */
	clr = COPY_RATIO(0x7) | HPM_RATIO(0x7) | CORES_RATIO(0x7);
	set = COPY_RATIO(4) | HPM_RATIO(4) | CORES_RATIO(4);

	clrsetbits_le32(&clk->div_cpu1, clr, set);

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_cpu1) & DIV_STAT_CPU1_CHANGING)
		continue;

	/*
	 * Set CMU_DMC clocks src to APLL
	 * Bit values:             0  ; 1
	 * MUX_C2C_SEL:      SCLKMPLL ; SCLKAPLL
	 * MUX_DMC_BUS_SEL:  SCLKMPLL ; SCLKAPLL
	 * MUX_DPHY_SEL:     SCLKMPLL ; SCLKAPLL
	 * MUX_MPLL_SEL:     FINPLL   ; MOUT_MPLL_FOUT
	 * MUX_PWI_SEL:      0110 (MPLL); 0111 (EPLL); 1000 (VPLL); 0(XXTI)
	 * MUX_G2D_ACP0_SEL: SCLKMPLL ; SCLKAPLL
	 * MUX_G2D_ACP1_SEL: SCLKEPLL ; SCLKVPLL
	 * MUX_G2D_ACP_SEL:  OUT_ACP0 ; OUT_ACP1
	*/
	clr_src_dmc = MUX_C2C_SEL(0x1) | MUX_DMC_BUS_SEL(0x1) |
		      MUX_DPHY_SEL(0x1) | MUX_MPLL_SEL(0x1) |
		      MUX_PWI_SEL(0xf) | MUX_G2D_ACP0_SEL(0x1) |
		      MUX_G2D_ACP1_SEL(0x1) | MUX_G2D_ACP_SEL(0x1);
	set = MUX_C2C_SEL(1) | MUX_DMC_BUS_SEL(1) | MUX_DPHY_SEL(1) |
	      MUX_MPLL_SEL(0) | MUX_PWI_SEL(0) | MUX_G2D_ACP0_SEL(1) |
	      MUX_G2D_ACP1_SEL(1) | MUX_G2D_ACP_SEL(1);

	clrsetbits_le32(&clk->src_dmc, clr_src_dmc, set);

	/* Wait for mux change */
	while (readl(&clk->mux_stat_dmc) & MUX_STAT_DMC_CHANGING)
		continue;

	/* Set MPLL to 800MHz */
	set = SDIV(0) | PDIV(3) | MDIV(100) | FSEL(0) | PLL_ENABLE(1);

	clrsetbits_le32(&clk->mpll_con0, clr_pll_con0, set);

	/* Wait for PLL to be locked */
	while (!(readl(&clk->mpll_con0) & PLL_LOCKED_BIT))
		continue;

	/* Switch back CMU_DMC mux */
	set = MUX_C2C_SEL(0) | MUX_DMC_BUS_SEL(0) | MUX_DPHY_SEL(0) |
	      MUX_MPLL_SEL(1) | MUX_PWI_SEL(8) | MUX_G2D_ACP0_SEL(0) |
	      MUX_G2D_ACP1_SEL(0) | MUX_G2D_ACP_SEL(0);

	clrsetbits_le32(&clk->src_dmc, clr_src_dmc, set);

	/* Wait for mux change */
	while (readl(&clk->mux_stat_dmc) & MUX_STAT_DMC_CHANGING)
		continue;

	/* CLK_DIV_DMC0 */
	clr = ACP_RATIO(0x7) | ACP_PCLK_RATIO(0x7) | DPHY_RATIO(0x7) |
	      DMC_RATIO(0x7) | DMCD_RATIO(0x7) | DMCP_RATIO(0x7);
	/*
	 * For:
	 * MOUTdmc = 800 MHz
	 * MOUTdphy = 800 MHz
	 *
	 * aclk_acp = MOUTdmc / (ratio + 1) = 200 (3)
	 * pclk_acp = aclk_acp / (ratio + 1) = 100 (1)
	 * sclk_dphy = MOUTdphy / (ratio + 1) = 400 (1)
	 * sclk_dmc = MOUTdmc / (ratio + 1) = 400 (1)
	 * aclk_dmcd = sclk_dmc / (ratio + 1) = 200 (1)
	 * aclk_dmcp = aclk_dmcd / (ratio + 1) = 100 (1)
	 */
	set = ACP_RATIO(3) | ACP_PCLK_RATIO(1) | DPHY_RATIO(1) |
	      DMC_RATIO(1) | DMCD_RATIO(1) | DMCP_RATIO(1);

	clrsetbits_le32(&clk->div_dmc0, clr, set);

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_dmc0) & DIV_STAT_DMC0_CHANGING)
		continue;

	/* CLK_DIV_DMC1 */
	clr = G2D_ACP_RATIO(0xf) | C2C_RATIO(0x7) | PWI_RATIO(0xf) |
	      C2C_ACLK_RATIO(0x7) | DVSEM_RATIO(0x7f) | DPM_RATIO(0x7f);
	/*
	 * For:
	 * MOUTg2d = 800 MHz
	 * MOUTc2c = 800 Mhz
	 * MOUTpwi = 108 MHz
	 *
	 * sclk_g2d_acp = MOUTg2d / (ratio + 1) = 200 (3)
	 * sclk_c2c = MOUTc2c / (ratio + 1) = 400 (1)
	 * aclk_c2c = sclk_c2c / (ratio + 1) = 200 (1)
	 * sclk_pwi = MOUTpwi / (ratio + 1) = 18 (5)
	 */
	set = G2D_ACP_RATIO(3) | C2C_RATIO(1) | PWI_RATIO(5) |
	      C2C_ACLK_RATIO(1) | DVSEM_RATIO(1) | DPM_RATIO(1);

	clrsetbits_le32(&clk->div_dmc1, clr, set);

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_dmc1) & DIV_STAT_DMC1_CHANGING)
		continue;

	/* CLK_SRC_PERIL0 */
	clr = UART0_SEL(0xf) | UART1_SEL(0xf) | UART2_SEL(0xf) |
	      UART3_SEL(0xf) | UART4_SEL(0xf);
	/*
	 * Set CLK_SRC_PERIL0 clocks src to MPLL
	 * src values: 0(XXTI); 1(XusbXTI); 2(SCLK_HDMI24M); 3(SCLK_USBPHY0);
	 *             5(SCLK_HDMIPHY); 6(SCLK_MPLL_USER_T); 7(SCLK_EPLL);
	 *             8(SCLK_VPLL)
	 *
	 * Set all to SCLK_MPLL_USER_T
	 */
	set = UART0_SEL(6) | UART1_SEL(6) | UART2_SEL(6) | UART3_SEL(6) |
	      UART4_SEL(6);

	clrsetbits_le32(&clk->src_peril0, clr, set);

	/* CLK_DIV_PERIL0 */
	clr = UART0_RATIO(0xf) | UART1_RATIO(0xf) | UART2_RATIO(0xf) |
	      UART3_RATIO(0xf) | UART4_RATIO(0xf);
	/*
	 * For MOUTuart0-4: 800MHz
	 *
	 * SCLK_UARTx = MOUTuartX / (ratio + 1) = 100 (7)
	*/
	set = UART0_RATIO(7) | UART1_RATIO(7) | UART2_RATIO(7) |
	      UART3_RATIO(7) | UART4_RATIO(7);

	clrsetbits_le32(&clk->div_peril0, clr, set);

	while (readl(&clk->div_stat_peril0) & DIV_STAT_PERIL0_CHANGING)
		continue;

	/* CLK_DIV_FSYS1 */
	clr = MMC0_RATIO(0xf) | MMC0_PRE_RATIO(0xff) | MMC1_RATIO(0xf) |
	      MMC1_PRE_RATIO(0xff);
	/*
	 * For MOUTmmc0-3 = 800 MHz (MPLL)
	 *
	 * DOUTmmc1 = MOUTmmc1 / (ratio + 1) = 100 (7)
	 * sclk_mmc1 = DOUTmmc1 / (ratio + 1) = 50 (1)
	 * DOUTmmc0 = MOUTmmc0 / (ratio + 1) = 100 (7)
	 * sclk_mmc0 = DOUTmmc0 / (ratio + 1) = 50 (1)
	*/
	set = MMC0_RATIO(7) | MMC0_PRE_RATIO(1) | MMC1_RATIO(7) |
	      MMC1_PRE_RATIO(1);

	clrsetbits_le32(&clk->div_fsys1, clr, set);

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_fsys1) & DIV_STAT_FSYS1_CHANGING)
		continue;

	/* CLK_DIV_FSYS2 */
	clr = MMC2_RATIO(0xf) | MMC2_PRE_RATIO(0xff) | MMC3_RATIO(0xf) |
	      MMC3_PRE_RATIO(0xff);
	/*
	 * For MOUTmmc0-3 = 800 MHz (MPLL)
	 *
	 * DOUTmmc3 = MOUTmmc3 / (ratio + 1) = 100 (7)
	 * sclk_mmc3 = DOUTmmc3 / (ratio + 1) = 50 (1)
	 * DOUTmmc2 = MOUTmmc2 / (ratio + 1) = 100 (7)
	 * sclk_mmc2 = DOUTmmc2 / (ratio + 1) = 50 (1)
	*/
	set = MMC2_RATIO(7) | MMC2_PRE_RATIO(1) | MMC3_RATIO(7) |
	      MMC3_PRE_RATIO(1);

	clrsetbits_le32(&clk->div_fsys2, clr, set);

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_fsys2) & DIV_STAT_FSYS2_CHANGING)
		continue;

	/* CLK_DIV_FSYS3 */
	clr = MMC4_RATIO(0xf) | MMC4_PRE_RATIO(0xff);
	/*
	 * For MOUTmmc4 = 800 MHz (MPLL)
	 *
	 * DOUTmmc4 = MOUTmmc4 / (ratio + 1) = 100 (7)
	 * sclk_mmc4 = DOUTmmc4 / (ratio + 1) = 100 (0)
	*/
	set = MMC4_RATIO(7) | MMC4_PRE_RATIO(0);

	clrsetbits_le32(&clk->div_fsys3, clr, set);

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_fsys3) & DIV_STAT_FSYS3_CHANGING)
		continue;

	return;
}

static void board_gpio_init(void)
{
	if (board_is_trats2()) {
		/* PS_ALS_INT */
		gpio_set_pull(EXYNOS4X12_GPIO_X02, S5P_GPIO_PULL_NONE);
		/* TSP_nINT */
		gpio_set_pull(EXYNOS4X12_GPIO_X04, S5P_GPIO_PULL_NONE);
		/* AP_PMIC_IRQ*/
		gpio_set_pull(EXYNOS4X12_GPIO_X07, S5P_GPIO_PULL_NONE);
		/* IF_PMIC_IRQ*/
		gpio_set_pull(EXYNOS4X12_GPIO_X15, S5P_GPIO_PULL_NONE);
		/* VOL_UP */
		gpio_set_pull(EXYNOS4X12_GPIO_X20, S5P_GPIO_PULL_NONE);
		/* VOL_DOWN */
		gpio_set_pull(EXYNOS4X12_GPIO_X21, S5P_GPIO_PULL_NONE);
		/* FUEL_ALERT */
		gpio_set_pull(EXYNOS4X12_GPIO_X23, S5P_GPIO_PULL_NONE);
		/* ADC_INT */
		gpio_set_pull(EXYNOS4X12_GPIO_X24, S5P_GPIO_PULL_NONE);
		/* nPOWER */
		gpio_set_pull(EXYNOS4X12_GPIO_X27, S5P_GPIO_PULL_NONE);
		/* WPC_INT */
		gpio_set_pull(EXYNOS4X12_GPIO_X30, S5P_GPIO_PULL_NONE);
		/* OK_KEY */
		gpio_set_pull(EXYNOS4X12_GPIO_X35, S5P_GPIO_PULL_NONE);
		/* HDMI_HPD */
		gpio_set_pull(EXYNOS4X12_GPIO_X37, S5P_GPIO_PULL_NONE);
	} else {
		/* eMMC reset */
		gpio_cfg_pin(EXYNOS4X12_GPIO_K12, S5P_GPIO_FUNC(0x1));
		gpio_set_pull(EXYNOS4X12_GPIO_K12, S5P_GPIO_PULL_NONE);
		gpio_set_drv(EXYNOS4X12_GPIO_K12, S5P_GPIO_DRV_4X);

		/* Enable FAN (Odroid U3) */
		gpio_set_pull(EXYNOS4X12_GPIO_D00, S5P_GPIO_PULL_UP);
		gpio_set_drv(EXYNOS4X12_GPIO_D00, S5P_GPIO_DRV_4X);
		gpio_direction_output(EXYNOS4X12_GPIO_D00, 1);

		/* OTG Vbus output (Odroid U3+) */
		gpio_set_pull(EXYNOS4X12_GPIO_L20, S5P_GPIO_PULL_NONE);
		gpio_set_drv(EXYNOS4X12_GPIO_L20, S5P_GPIO_DRV_4X);
		gpio_direction_output(EXYNOS4X12_GPIO_L20, 0);

		/* OTG INT (Odroid U3+) */
		gpio_set_pull(EXYNOS4X12_GPIO_X31, S5P_GPIO_PULL_UP);
		gpio_set_drv(EXYNOS4X12_GPIO_X31, S5P_GPIO_DRV_4X);
		gpio_direction_input(EXYNOS4X12_GPIO_X31);
	}
}

static int pmic_init_max77686(void)
{
	struct pmic *p = pmic_get("MAX77686_PMIC");

	if (pmic_probe(p))
		return -ENODEV;

	if (board_is_trats2()) {
		/* BUCK/LDO Output Voltage */
		max77686_set_ldo_voltage(p, 21, 2800000);	/* LDO21 VTF_2.8V */
		max77686_set_ldo_voltage(p, 23, 3300000);	/* LDO23 TSP_AVDD_3.3V*/
		max77686_set_ldo_voltage(p, 24, 1800000);	/* LDO24 TSP_VDD_1.8V */

		/* BUCK/LDO Output Mode */
		max77686_set_buck_mode(p, 1, OPMODE_STANDBY);	/* BUCK1 VMIF_1.1V_AP */
		max77686_set_buck_mode(p, 2, OPMODE_ON);	/* BUCK2 VARM_1.0V_AP */
		max77686_set_buck_mode(p, 3, OPMODE_ON);	/* BUCK3 VINT_1.0V_AP */
		max77686_set_buck_mode(p, 4, OPMODE_ON);	/* BUCK4 VG3D_1.0V_AP */
		max77686_set_buck_mode(p, 5, OPMODE_ON);	/* BUCK5 VMEM_1.2V_AP */
		max77686_set_buck_mode(p, 6, OPMODE_ON);	/* BUCK6 VCC_SUB_1.35V*/
		max77686_set_buck_mode(p, 7, OPMODE_ON);	/* BUCK7 VCC_SUB_2.0V */
		max77686_set_buck_mode(p, 8, OPMODE_OFF);	/* VMEM_VDDF_2.85V */
		max77686_set_buck_mode(p, 9, OPMODE_OFF);	/* CAM_ISP_CORE_1.2V*/

		max77686_set_ldo_mode(p, 1, OPMODE_LPM);	/* LDO1 VALIVE_1.0V_AP*/
		max77686_set_ldo_mode(p, 2, OPMODE_STANDBY);	/* LDO2 VM1M2_1.2V_AP */
		max77686_set_ldo_mode(p, 3, OPMODE_LPM);	/* LDO3 VCC_1.8V_AP */
		max77686_set_ldo_mode(p, 4, OPMODE_LPM);	/* LDO4 VCC_2.8V_AP */
		max77686_set_ldo_mode(p, 5, OPMODE_OFF);	/* LDO5_VCC_1.8V_IO */
		max77686_set_ldo_mode(p, 6, OPMODE_STANDBY);	/* LDO6 VMPLL_1.0V_AP */
		max77686_set_ldo_mode(p, 7, OPMODE_STANDBY);	/* LDO7 VPLL_1.0V_AP */
		max77686_set_ldo_mode(p, 8, OPMODE_LPM);	/* LDO8 VMIPI_1.0V_AP */
		max77686_set_ldo_mode(p, 9, OPMODE_OFF);	/* CAM_ISP_MIPI_1.2*/
		max77686_set_ldo_mode(p, 10, OPMODE_LPM);	/* LDO10 VMIPI_1.8V_AP*/
		max77686_set_ldo_mode(p, 11, OPMODE_STANDBY);	/* LDO11 VABB1_1.8V_AP*/
		max77686_set_ldo_mode(p, 12, OPMODE_LPM);	/* LDO12 VUOTG_3.0V_AP*/
		max77686_set_ldo_mode(p, 13, OPMODE_OFF);	/* LDO13 VC2C_1.8V_AP */
		max77686_set_ldo_mode(p, 14, OPMODE_STANDBY);	/* VABB02_1.8V_AP */
		max77686_set_ldo_mode(p, 15, OPMODE_STANDBY);	/* LDO15 VHSIC_1.0V_AP*/
		max77686_set_ldo_mode(p, 16, OPMODE_STANDBY);	/* LDO16 VHSIC_1.8V_AP*/
		max77686_set_ldo_mode(p, 17, OPMODE_OFF);	/* CAM_SENSOR_CORE_1.2*/
		max77686_set_ldo_mode(p, 18, OPMODE_OFF);	/* CAM_ISP_SEN_IO_1.8V*/
		max77686_set_ldo_mode(p, 19, OPMODE_OFF);	/* LDO19 VT_CAM_1.8V */
		max77686_set_ldo_mode(p, 20, OPMODE_ON);	/* LDO20 VDDQ_PRE_1.8V*/
		max77686_set_ldo_mode(p, 21, OPMODE_OFF);	/* LDO21 VTF_2.8V */
		max77686_set_ldo_mode(p, 22, OPMODE_OFF);	/* LDO22 VMEM_VDD_2.8V*/
		max77686_set_ldo_mode(p, 23, OPMODE_OFF);	/* LDO23 TSP_AVDD_3.3V*/
		max77686_set_ldo_mode(p, 24, OPMODE_OFF);	/* LDO24 TSP_VDD_1.8V */
		max77686_set_ldo_mode(p, 25, OPMODE_OFF);	/* LDO25 VCC_3.3V_LCD */
		max77686_set_ldo_mode(p, 26, OPMODE_OFF);	/*LDO26 VCC_3.0V_MOTOR*/
	} else {
		/* Set LDO Voltage */
		max77686_set_ldo_voltage(p, 20, 1800000);	/* LDO20 eMMC */
		max77686_set_ldo_voltage(p, 21, 2800000);	/* LDO21 SD */
		max77686_set_ldo_voltage(p, 22, 2800000);	/* LDO22 eMMC */
	}

	return 0;
}

#ifdef CONFIG_SYS_I2C_INIT_BOARD
static void board_init_i2c(void)
{
	int i2c_id;

	if (board_is_trats2()) {
		i2c_id = PERIPH_ID_I2C7;

		/* I2C_8 */
		gpio_direction_output(EXYNOS4X12_GPIO_F14, 1);
		gpio_direction_output(EXYNOS4X12_GPIO_F15, 1);

		/* I2C_9 */
		gpio_direction_output(EXYNOS4X12_GPIO_M21, 1);
		gpio_direction_output(EXYNOS4X12_GPIO_M20, 1);
	} else {
		i2c_id = PERIPH_ID_I2C0;
	}

	if (exynos_pinmux_config(i2c_id, PINMUX_FLAG_NONE))
		debug("I2C%d not configured\n", i2c_id - PERIPH_ID_I2C0);
}
#endif

#ifdef CONFIG_SYS_I2C_SOFT
int get_soft_i2c_scl_pin(void)
{
	if (I2C_ADAP_HWNR)
		return EXYNOS4X12_GPIO_M21; /* I2C9 */
	else
		return EXYNOS4X12_GPIO_F14; /* I2C8 */
}

int get_soft_i2c_sda_pin(void)
{
	if (I2C_ADAP_HWNR)
		return EXYNOS4X12_GPIO_M20; /* I2C9 */
	else
		return EXYNOS4X12_GPIO_F15; /* I2C8 */
}
#endif

int exynos_early_init_f(void)
{
	board_clock_init();
	board_gpio_init();

	return 0;
}

int exynos_init(void)
{
	struct exynos4_power *pwr =
		(struct exynos4_power *)samsung_get_base_power();

	gd->bd->bi_arch_number = board_arch_num[gd->board_type];

	if (!board_is_trats2())
		return 0;

	writel(0, &pwr->inform4);
	writel(0, &pwr->inform5);

	return 0;
}

int exynos_power_init(void)
{
#ifdef CONFIG_SYS_I2C_INIT_BOARD
	board_init_i2c();
#endif
	/* bus number taken from FDT */
	pmic_init(0);
	pmic_init_max77686();

	if (!board_is_trats2())
		goto done;

	/* I2C adapter 10 - bus name soft1 */
	pmic_init_max77693(I2C_10);
	/* I2C adapter 10 - bus name soft1 */
	power_muic_init(I2C_10);
	/* I2C adapter 9 - bus name soft0 */
	power_fg_init(I2C_9);
	power_bat_init(0);

	p_chrg = pmic_get("MAX77693_PMIC");
	if (!p_chrg) {
		puts("MAX77693_PMIC: Not found\n");
		return -ENODEV;
	}

	p_muic = pmic_get("MAX77693_MUIC");
	if (!p_muic) {
		puts("MAX77693_MUIC: Not found\n");
		return -ENODEV;
	}

	p_fg = pmic_get("MAX77693_FG");
	if (!p_fg) {
		puts("MAX17042_FG: Not found\n");
		return -ENODEV;
	}

	if (p_chrg->chrg->chrg_bat_present(p_chrg) == 0)
		puts("No battery detected\n");

	p_bat = pmic_get("BAT_TRATS2");
	if (!p_bat) {
		puts("BAT_TRATS2: Not found\n");
		return -ENODEV;
	}

	p_fg->parent =  p_bat;
	p_chrg->parent = p_bat;
	p_muic->parent = p_bat;

#ifdef CONFIG_INTERACTIVE_CHARGER
	p_bat->low_power_mode = board_low_power_mode;
#endif
	p_bat->pbat->battery_init(p_bat, p_fg, p_chrg, p_muic);

	pbat = p_bat->pbat;

done:
	power_init_done = 1;

	return 0;
}

#ifdef CONFIG_USB_GADGET
static int s5pc210_phy_control(int on)
{
	struct pmic *p_pmic;

	p_pmic = pmic_get("MAX77686_PMIC");
	if (!p_pmic)
		return -ENODEV;

	if (pmic_probe(p_pmic))
		return -1;

	if (on)
		return max77686_set_ldo_mode(p_pmic, 12, OPMODE_ON);
	else
		return max77686_set_ldo_mode(p_pmic, 12, OPMODE_LPM);
}

struct s3c_plat_otg_data s5pc210_otg_data = {
	.phy_control	= s5pc210_phy_control,
	.regs_phy	= EXYNOS4X12_USBPHY_BASE,
	.regs_otg	= EXYNOS4X12_USBOTG_BASE,
	.usb_phy_ctrl	= EXYNOS4X12_USBPHY_CONTROL,
	.usb_flags	= PHY0_SLEEP,
};

int board_usb_init(int index, enum usb_init_type init)
{
	debug("USB_udc_probe\n");
	return s3c_udc_probe(&s5pc210_otg_data);
}
#endif

void reset_misc(void)
{
	if (board_is_trats2())
		return;

	/* Reset eMMC*/
	gpio_set_value(EXYNOS4X12_GPIO_K12, 0);
	mdelay(10);
	gpio_set_value(EXYNOS4X12_GPIO_K12, 1);
}

/*
 * LCD
 */
#ifdef CONFIG_LCD
int mipi_power(void)
{
	struct pmic *p;

	if (!board_is_trats2())
		return 0;

	p = pmic_get("MAX77686_PMIC");

	/* LDO8 VMIPI_1.0V_AP */
	max77686_set_ldo_mode(p, 8, OPMODE_ON);
	/* LDO10 VMIPI_1.8V_AP */
	max77686_set_ldo_mode(p, 10, OPMODE_ON);

	return 0;
}

void exynos_lcd_power_on(void)
{
	struct pmic *p;

	if (!board_is_trats2())
		return;

	p = pmic_get("MAX77686_PMIC");

	/* LCD_2.2V_EN: GPC0[1] */
	gpio_set_pull(EXYNOS4X12_GPIO_C01, S5P_GPIO_PULL_UP);
	gpio_direction_output(EXYNOS4X12_GPIO_C01, 1);

	/* LDO25 VCC_3.1V_LCD */
	pmic_probe(p);
	max77686_set_ldo_voltage(p, 25, 3100000);
	max77686_set_ldo_mode(p, 25, OPMODE_LPM);
}

void exynos_reset_lcd(void)
{
	if (!board_is_trats2())
		return;

	/* reset lcd */
	gpio_direction_output(EXYNOS4X12_GPIO_F21, 0);
	udelay(10);
	gpio_set_value(EXYNOS4X12_GPIO_F21, 1);
}

void exynos_lcd_misc_init(vidinfo_t *vid)
{
	if (!board_is_trats2())
		return;
#ifdef CONFIG_TIZEN
	get_tizen_logo_info(vid);
#endif
#ifdef CONFIG_S6E8AX0
	s6e8ax0_init();
#endif
}
#endif /* LCD */

void low_clock_mode(void)
{
	struct exynos4x12_clock *clk = (struct exynos4x12_clock *)
					samsung_get_base_clock();

	unsigned int cfg_apll_con0;
	unsigned int cfg_src_cpu;
	unsigned int cfg_div_cpu0;
	unsigned int cfg_div_cpu1;
	unsigned int clk_gate_cfg;

	/* Turn off unnecessary clocks */
	clk_gate_cfg = 0x0;
	writel(clk_gate_cfg, &clk->gate_ip_image);		/* IMAGE */
	writel(clk_gate_cfg, &clk->gate_ip_cam);		/* CAM */
	writel(clk_gate_cfg, &clk->gate_ip_tv);			/* TV */
	writel(clk_gate_cfg, &clk->gate_ip_mfc);		/* MFC */
	writel(clk_gate_cfg, &clk->gate_ip_g3d);		/* G3D */
	writel(clk_gate_cfg, &clk->gate_ip_gps);		/* GPS */
	writel(clk_gate_cfg, &clk->gate_ip_isp1);		/* ISP1 */

	/*
	 * Set CMU_CPU clocks src to MPLL
	 * Bit values:                 0 ; 1
	 * MUX_APLL_SEL:        FIN_PLL;   FOUT_APLL
	 * MUX_CORE_SEL:        MOUT_APLL; SCLK_MPLL
	 * MUX_HPM_SEL:         MOUT_APLL; SCLK_MPLL_USER_C
	 * MUX_MPLL_USER_SEL_C: FIN_PLL;   SCLK_MPLL
	*/
	cfg_src_cpu = MUX_APLL_SEL(1) | MUX_CORE_SEL(1) | MUX_HPM_SEL(1) |
		      MUX_MPLL_USER_SEL_C(1);
	writel(cfg_src_cpu, &clk->src_cpu);

	/* Disable APLL */
	cfg_apll_con0 = readl(&clk->apll_con0);
	writel(cfg_apll_con0 & ~PLL_ENABLE(1), &clk->apll_con0);

	/* Set APLL to 200MHz */
	cfg_apll_con0 = SDIV(2) | PDIV(3) | MDIV(100) | FSEL(1) | PLL_ENABLE(1);
	writel(cfg_apll_con0, &clk->apll_con0);

	/* Wait for PLL to be locked */
	while (!(readl(&clk->apll_con0) & PLL_LOCKED_BIT))
		continue;

	/* Set CMU_CPU clock src to APLL */
	cfg_src_cpu = MUX_APLL_SEL(1) | MUX_CORE_SEL(0) | MUX_HPM_SEL(0) |
		      MUX_MPLL_USER_SEL_C(0);
	writel(cfg_src_cpu, &clk->src_cpu);

	/* Wait for MUX ready status */
	while (readl(&clk->src_cpu) & MUX_STAT_CPU_CHANGING)
		continue;

	/*
	 * Set dividers for MOUTcore = 200 MHz
	 * coreout =      MOUT / (ratio + 1) = 200 MHz
	 * corem0 =     armclk / (ratio + 1) = 200 MHz
	 * corem1 =     armclk / (ratio + 1) = 200 MHz
	 * periph =     armclk / (ratio + 1) = 200 MHz
	 * atbout =       MOUT / (ratio + 1) = 200 MHz
	 * pclkdbgout = atbout / (ratio + 1) = 200 MHz
	 * sclkapll = MOUTapll / (ratio + 1) = 50 MHz
	 * armclk =   core_out / (ratio + 1) = 200 MHz
	*/
	cfg_div_cpu0 = CORE_RATIO(0) | COREM0_RATIO(0) | COREM1_RATIO(0) |
		       PERIPH_RATIO(0) | ATB_RATIO(0) | PCLK_DBG_RATIO(1) |
		       APLL_RATIO(3) | CORE2_RATIO(0);
	writel(cfg_div_cpu0, &clk->div_cpu0);

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_cpu0) & DIV_STAT_CPU0_CHANGING)
		continue;

	/*
	 * For MOUThpm = 200 MHz (MOUTapll)
	 * doutcopy = MOUThpm / (ratio + 1) = 100
	 * sclkhpm = doutcopy / (ratio + 1) = 100
	 * cores_out = armclk / (ratio + 1) = 200
	 */
	cfg_div_cpu1 = COPY_RATIO(1) | HPM_RATIO(0) | CORES_RATIO(0);
	writel(cfg_div_cpu1, &clk->div_cpu1);	/* DIV_CPU1 */

	/* Wait for divider ready status */
	while (readl(&clk->div_stat_cpu1) & DIV_STAT_CPU1_CHANGING)
		continue;
}

#ifdef CONFIG_INTERACTIVE_CHARGER
static int low_power_mode_set;

void board_low_power_mode(void)
{
	struct exynos4x12_power *pwr = (struct exynos4x12_power *)
					samsung_get_base_power();

	unsigned int pwr_core_cfg = 0x0;
	unsigned int pwr_cfg = 0x0;

	/* Set low power mode only once */
	if (low_power_mode_set)
		return;

	/* Power down CORES: 1, 2, 3 */
	/* LOCAL_PWR_CFG [1:0] 0x3 EN, 0x0 DIS */
	writel(pwr_core_cfg, &pwr->arm_core1_configuration);
	writel(pwr_core_cfg, &pwr->arm_core2_configuration);
	writel(pwr_core_cfg, &pwr->arm_core3_configuration);

	/* Turn off unnecessary power domains */
	writel(pwr_cfg, &pwr->xxti_configuration);		/* XXTI */
	writel(pwr_cfg, &pwr->cam_configuration);		/* CAM */
	writel(pwr_cfg, &pwr->tv_configuration);		/* TV */
	writel(pwr_cfg, &pwr->mfc_configuration);		/* MFC */
	writel(pwr_cfg, &pwr->g3d_configuration);		/* G3D */
	writel(pwr_cfg, &pwr->gps_configuration);		/* GPS */
	writel(pwr_cfg, &pwr->gps_alive_configuration);		/* GPS_ALIVE */

	/* Set CPU clock to 200MHz */
	low_clock_mode();

	low_power_mode_set = 1;
}
#endif

#ifdef CONFIG_CMD_POWEROFF
void board_poweroff(void)
{
	unsigned int val;
	struct exynos4x12_power *power =
		(struct exynos4x12_power *)samsung_get_base_power();

	val = readl(&power->ps_hold_control);
	val |= EXYNOS_PS_HOLD_CONTROL_EN_OUTPUT; /* set to output */
	val &= ~EXYNOS_PS_HOLD_CONTROL_DATA_HIGH; /* set state to low */
	writel(val, &power->ps_hold_control);

	while (1);
	/* Should not reach here */
}
#endif

/* Functions for interactive charger in board/samsung/common/misc.c */
#ifdef CONFIG_INTERACTIVE_CHARGER
int charger_enable(void)
{
	if (!power_init_done) {
		if (exynos_power_init()) {
			puts("Can't init board power subsystem");
			return -EIO;
		}
	}

	if (!pbat) {
		puts("No such device!\n");
		return -ENODEV;
	}

	if (!pbat->battery_charge) {
		puts("Can't enable charger\n");
		return -ENODEV;
	}

	/* Enable charger */
	if (pbat->battery_charge(p_bat)) {
		puts("Charger enable error\n");
		return -EIO;
	}

	return 0;
}

int charger_type(void)
{
	if (!power_init_done) {
		if (exynos_power_init()) {
			puts("Can't init board power subsystem");
			return -EIO;
		}
	}

	if (!p_muic) {
		puts("No such device!\n");
		return -ENODEV;
	}

	if (!p_muic->chrg->chrg_type) {
		puts("Can't get charger type\n");
		return -ENODEV;
	}

	return p_muic->chrg->chrg_type(p_muic);
}

int battery_present(void)
{
	if (!power_init_done) {
		if (exynos_power_init()) {
			puts("Can't init board power subsystem");
			return 0;
		}
	}

	if (!p_chrg) {
		puts("No such device!\n");
		return 0;
	}

	if (!p_chrg->chrg->chrg_bat_present) {
		puts("Can't get battery state\n");
		return 0;;
	}

	if (!p_chrg->chrg->chrg_bat_present(p_chrg)) {
		puts("Battery not present.\n");
		return 0;
	}

	return 1;
}

int battery_state(unsigned int *soc)
{
	struct battery *bat = pbat->bat;

	if (!power_init_done) {
		if (exynos_power_init()) {
			printf("Can't init board power subsystem");
			return -EIO;
		}
	}

	if (!p_fg) {
		puts("No such device!\n");
		return -ENODEV;
	}

	if (!p_fg->fg->fg_battery_update) {
		puts("Can't update battery state\n");
		return -EIO;
	}

	/* Check battery state */
	if (p_fg->fg->fg_battery_update(p_fg, p_bat)) {
		puts("Battery update error\n");
		return -EIO;
	}

	debug("[BAT]:\n#state:%u\n#soc:%3.1u\n#vcell:%u\n", bat->state,
							    bat->state_of_chrg,
							    bat->voltage_uV);

	*soc = bat->state_of_chrg;

	return 0;
}
#endif
