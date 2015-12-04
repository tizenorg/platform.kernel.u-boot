/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 * Sanghee Kim <sh0130.kim@samsung.com>
 * Piotr Wilczek <p.wilczek@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <asm/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/clock.h>
#include <asm/arch/mipi_dsim.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <power/battery.h>
#include <power/max77693_pmic.h>
#include <power/max77693_muic.h>
#include <power/max77693_fg.h>
#include <libtizen.h>
#include <samsung/misc.h>
#include <errno.h>
#include <usb.h>
#include <usb/s3c_udc.h>
#include <usb_mass_storage.h>
#include "setup.h"

DECLARE_GLOBAL_DATA_PTR;

/* For global battery and charger functions */
static struct power_battery *pbat;
static struct pmic *p_chrg, *p_muic, *p_fg, *p_bat;
static int power_init_done;

static unsigned int board_rev = -1;
static inline u32 get_model_rev(void);

static void check_hw_revision(void)
{
	int modelrev = 0;
	char str[12];
	int i;

	/*
	 * GPM1[1:0]: MODEL_REV[1:0]
	 * Don't set as pull-none for these N/C pin.
	 * TRM say that it may cause unexcepted state and leakage current.
	 * and pull-none is only for output function.
	 */
	for (i = 0; i < 2; i++) {
		int pin = i + EXYNOS4X12_GPIO_M10;

		sprintf(str, "model_rev%d", i);
		gpio_request(pin, str);
		gpio_cfg_pin(pin, S5P_GPIO_INPUT);
	}

	/* GPM1[5:2]: HW_REV[3:0] */
	for (i = 0; i < 4; i++) {
		int pin = i + EXYNOS4X12_GPIO_M12;

		sprintf(str, "hw_rev%d", i);
		gpio_request(pin, str);
		gpio_cfg_pin(pin, S5P_GPIO_INPUT);
		gpio_set_pull(pin, S5P_GPIO_PULL_NONE);
	}

	/* GPM1[1:0]: MODEL_REV[1:0] */
	for (i = 0; i < 2; i++)
		modelrev |= (gpio_get_value(EXYNOS4X12_GPIO_M10 + i) << i);

	/* board_rev[15:8] = model */
	board_rev = modelrev << 8;
}

u32 get_board_rev(void)
{
	return board_rev;
}

static inline u32 get_model_rev(void)
{
	return (board_rev >> 8) & 0xff;
}

static void board_external_gpio_init(void)
{
	/*
	 * some pins which in alive block are connected with external pull-up
	 * but it's default setting is pull-down.
	 * if that pin set as input then that floated
	 */

	gpio_set_pull(EXYNOS4X12_GPIO_X02, S5P_GPIO_PULL_NONE);	/* PS_ALS_INT */
	gpio_set_pull(EXYNOS4X12_GPIO_X04, S5P_GPIO_PULL_NONE);	/* TSP_nINT */
	gpio_set_pull(EXYNOS4X12_GPIO_X07, S5P_GPIO_PULL_NONE);	/* AP_PMIC_IRQ*/
	gpio_set_pull(EXYNOS4X12_GPIO_X15, S5P_GPIO_PULL_NONE);	/* IF_PMIC_IRQ*/
	gpio_set_pull(EXYNOS4X12_GPIO_X20, S5P_GPIO_PULL_NONE);	/* VOL_UP */
	gpio_set_pull(EXYNOS4X12_GPIO_X21, S5P_GPIO_PULL_NONE);	/* VOL_DOWN */
	gpio_set_pull(EXYNOS4X12_GPIO_X23, S5P_GPIO_PULL_NONE);	/* FUEL_ALERT */
	gpio_set_pull(EXYNOS4X12_GPIO_X24, S5P_GPIO_PULL_NONE);	/* ADC_INT */
	gpio_set_pull(EXYNOS4X12_GPIO_X27, S5P_GPIO_PULL_NONE);	/* nPOWER */
	gpio_set_pull(EXYNOS4X12_GPIO_X30, S5P_GPIO_PULL_NONE);	/* WPC_INT */
	gpio_set_pull(EXYNOS4X12_GPIO_X35, S5P_GPIO_PULL_NONE);	/* OK_KEY */
	gpio_set_pull(EXYNOS4X12_GPIO_X37, S5P_GPIO_PULL_NONE);	/* HDMI_HPD */
}

#ifdef CONFIG_SYS_I2C_INIT_BOARD
static void board_init_i2c(void)
{
	int err;

	/* I2C_7 */
	err = exynos_pinmux_config(PERIPH_ID_I2C7, PINMUX_FLAG_NONE);
	if (err) {
		debug("I2C%d not configured\n", (I2C_7));
		return;
	}

	/* I2C_8 */
	gpio_request(EXYNOS4X12_GPIO_F14, "i2c8_clk");
	gpio_request(EXYNOS4X12_GPIO_F15, "i2c8_data");
	gpio_direction_output(EXYNOS4X12_GPIO_F14, 1);
	gpio_direction_output(EXYNOS4X12_GPIO_F15, 1);

	/* I2C_9 */
	gpio_request(EXYNOS4X12_GPIO_M21, "i2c9_clk");
	gpio_request(EXYNOS4X12_GPIO_M20, "i2c9_data");
	gpio_direction_output(EXYNOS4X12_GPIO_M21, 1);
	gpio_direction_output(EXYNOS4X12_GPIO_M20, 1);
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
	board_external_gpio_init();

	return 0;
}

static int pmic_init_max77686(void);

int exynos_init(void)
{
	struct exynos4_power *pwr =
		(struct exynos4_power *)samsung_get_base_power();

	check_hw_revision();
	printf("HW Revision:\t0x%04x\n", board_rev);

	/*
	 * First bootloader on the TRATS2 platform uses
	 * INFORM4 and INFORM5 registers for recovery
	 *
	 * To indicate correct boot chain - those two
	 * registers must be cleared out
	 */
	writel(0, &pwr->inform4);
	writel(0, &pwr->inform5);

	return 0;
}

int exynos_power_init(void)
{
#ifdef CONFIG_SYS_I2C_INIT_BOARD
	board_init_i2c();
#endif
	pmic_init(I2C_7);		/* I2C adapter 7 - bus name s3c24x0_7 */
	pmic_init_max77686();
	pmic_init_max77693(I2C_10);	/* I2C adapter 10 - bus name soft1 */
	power_muic_init(I2C_10);	/* I2C adapter 10 - bus name soft1 */
	power_fg_init(I2C_9);		/* I2C adapter 9 - bus name soft0 */
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
	power_init_done = 1;

	return 0;
}

#ifdef CONFIG_USB_GADGET
static int s5pc210_phy_control(int on)
{
	int ret = 0;
	unsigned int val;
	struct pmic *p, *p_pmic, *p_muic;

	p_pmic = pmic_get("MAX77686_PMIC");
	if (!p_pmic)
		return -ENODEV;

	if (pmic_probe(p_pmic))
		return -1;

	p_muic = pmic_get("MAX77693_MUIC");
	if (!p_muic)
		return -ENODEV;

	if (pmic_probe(p_muic))
		return -1;

	if (on) {
		ret = max77686_set_ldo_mode(p_pmic, 12, OPMODE_ON);
		if (ret)
			return -1;

		p = pmic_get("MAX77693_PMIC");
		if (!p)
			return -ENODEV;

		if (pmic_probe(p))
			return -1;

		/* SAFEOUT */
		ret = pmic_reg_read(p, MAX77693_SAFEOUT, &val);
		if (ret)
			return -1;

		val |= MAX77693_ENSAFEOUT1;
		ret = pmic_reg_write(p, MAX77693_SAFEOUT, val);
		if (ret)
			return -1;

		/* PATH: USB */
		ret = pmic_reg_write(p_muic, MAX77693_MUIC_CONTROL1,
			MAX77693_MUIC_CTRL1_DN1DP2);

	} else {
		ret = max77686_set_ldo_mode(p_pmic, 12, OPMODE_LPM);
		if (ret)
			return -1;

		/* PATH: UART */
		ret = pmic_reg_write(p_muic, MAX77693_MUIC_CONTROL1,
			MAX77693_MUIC_CTRL1_UT1UR2);
	}

	if (ret)
		return -1;

	return 0;
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

int g_dnl_board_usb_cable_connected(void)
{
	struct pmic *muic = pmic_get("MAX77693_MUIC");
	if (!muic)
		return 0;

	return !!muic->chrg->chrg_type(muic);
}
#endif

static int pmic_init_max77686(void)
{
	struct pmic *p = pmic_get("MAX77686_PMIC");

	if (pmic_probe(p))
		return -1;

	/* BUCK/LDO Output Voltage */
	max77686_set_ldo_voltage(p, 21, 2800000);	/* LDO21 VTF_2.8V */
	max77686_set_ldo_voltage(p, 23, 3300000);	/* LDO23 TSP_AVDD_3.3V*/
	max77686_set_ldo_voltage(p, 24, 1800000);	/* LDO24 TSP_VDD_1.8V */

	/* BUCK/LDO Output Mode */
	max77686_set_buck_mode(p, 1, OPMODE_ON);	/* BUCK1 VMIF_1.1V_AP */
	max77686_set_buck_mode(p, 2, OPMODE_ON);	/* BUCK2 VARM_1.0V_AP */
	max77686_set_buck_mode(p, 3, OPMODE_ON);	/* BUCK3 VINT_1.0V_AP */
	max77686_set_buck_mode(p, 4, OPMODE_ON);	/* BUCK4 VG3D_1.0V_AP */
	max77686_set_buck_mode(p, 5, OPMODE_ON);	/* BUCK5 VMEM_1.2V_AP */
	max77686_set_buck_mode(p, 6, OPMODE_ON);	/* BUCK6 VCC_SUB_1.35V*/
	max77686_set_buck_mode(p, 7, OPMODE_ON);	/* BUCK7 VCC_SUB_2.0V */
	max77686_set_buck_mode(p, 8, OPMODE_OFF);	/* VMEM_VDDF_2.85V */
	max77686_set_buck_mode(p, 9, OPMODE_OFF);	/* CAM_ISP_CORE_1.2V*/

	max77686_set_ldo_mode(p, 1, OPMODE_ON_AUTO_LPM);  /* LDO1 VALIVE_1.0V_AP*/
	max77686_set_ldo_mode(p, 2, OPMODE_ON_AUTO_OFF);  /* LDO2 VM1M2_1.2V_AP */
	max77686_set_ldo_mode(p, 3, OPMODE_ON_AUTO_LPM);  /* LDO3 VCC_1.8V_AP */
	max77686_set_ldo_mode(p, 4, OPMODE_ON_AUTO_LPM);  /* LDO4 VCC_2.8V_AP */
	max77686_set_ldo_mode(p, 5, OPMODE_OFF);          /* LDO5 VCC_1.8V_IO */
	max77686_set_ldo_mode(p, 6, OPMODE_ON_AUTO_OFF);  /* LDO6 VMPLL_1.0V_AP */
	max77686_set_ldo_mode(p, 7, OPMODE_ON_AUTO_OFF);  /* LDO7 VPLL_1.0V_AP */
	max77686_set_ldo_mode(p, 8, OPMODE_ON);           /* LDO8 VMIPI_1.0V_AP */
	max77686_set_ldo_mode(p, 9, OPMODE_OFF);          /* LDO9 CAM_ISP_MIPI_1.2 */
	max77686_set_ldo_mode(p, 10, OPMODE_ON);          /* LDO10 VMIPI_1.8V_AP */
	max77686_set_ldo_mode(p, 11, OPMODE_ON_AUTO_OFF); /* LDO11 VABB1_1.8V_AP */
	max77686_set_ldo_mode(p, 12, OPMODE_ON_AUTO_LPM); /* LDO12 VUOTG_3.0V_AP */
	max77686_set_ldo_mode(p, 13, OPMODE_OFF);         /* LDO13 VC2C_1.8V_AP */
	max77686_set_ldo_mode(p, 14, OPMODE_ON_AUTO_OFF); /* LDO14 VABB02_1.8V_AP */
	max77686_set_ldo_mode(p, 15, OPMODE_ON_AUTO_OFF); /* LDO15 VHSIC_1.0V_AP */
	max77686_set_ldo_mode(p, 16, OPMODE_ON_AUTO_OFF); /* LDO16 VHSIC_1.8V_AP */
	max77686_set_ldo_mode(p, 17, OPMODE_OFF);         /* LDO17 CAM_SENSOR_CORE_1.2 */
	max77686_set_ldo_mode(p, 18, OPMODE_OFF);         /* LDO18 CAM_ISP_SEN_IO_1.8V */
	max77686_set_ldo_mode(p, 19, OPMODE_OFF);         /* LDO19 VT_CAM_1.8V */
	max77686_set_ldo_mode(p, 20, OPMODE_ON);          /* LDO20 VDDQ_PRE_1.8V */
	max77686_set_ldo_mode(p, 21, OPMODE_OFF);         /* LDO21 VTF_2.8V */
	max77686_set_ldo_mode(p, 22, OPMODE_OFF);         /* LDO22 VMEM_VDD_2.8V */
	max77686_set_ldo_mode(p, 23, OPMODE_OFF);         /* LDO23 TSP_AVDD_3.3V */
	max77686_set_ldo_mode(p, 24, OPMODE_OFF);         /* LDO24 TSP_VDD_1.8V */
	max77686_set_ldo_mode(p, 25, OPMODE_ON_AUTO_LPM); /* LDO25 VCC_3.3V_LCD */
	max77686_set_ldo_mode(p, 26, OPMODE_OFF);         /* LDO26 VCC_3.0V_MOTOR*/

	return 0;
}

/*
 * LCD
 */

#ifdef CONFIG_LCD
int mipi_power(void)
{
	struct pmic *p = pmic_get("MAX77686_PMIC");

	/* LDO8 VMIPI_1.0V_AP */
	max77686_set_ldo_mode(p, 8, OPMODE_ON);
	/* LDO10 VMIPI_1.8V_AP */
	max77686_set_ldo_mode(p, 10, OPMODE_ON);

	return 0;
}

void exynos_lcd_power_on(void)
{
	struct pmic *p = pmic_get("MAX77686_PMIC");

	/* LCD_2.2V_EN: GPC0[1] */
	gpio_request(EXYNOS4X12_GPIO_C01, "lcd_2v2_en");
	gpio_set_pull(EXYNOS4X12_GPIO_C01, S5P_GPIO_PULL_UP);
	gpio_direction_output(EXYNOS4X12_GPIO_C01, 1);

	/* LDO25 VCC_3.1V_LCD */
	pmic_probe(p);
	max77686_set_ldo_voltage(p, 25, 3100000);
	max77686_set_ldo_mode(p, 25, OPMODE_ON_AUTO_LPM);
}

void exynos_reset_lcd(void)
{
	/* reset lcd */
	gpio_request(EXYNOS4X12_GPIO_F21, "lcd_reset");
	gpio_direction_output(EXYNOS4X12_GPIO_F21, 0);
	udelay(10);
	gpio_set_value(EXYNOS4X12_GPIO_F21, 1);
}

void exynos_lcd_misc_init(vidinfo_t *vid)
{
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
			return -1;
		}
	}

	if (!pbat->battery_charge) {
		puts("Can't enable charger\n");
		return -1;
	}

	/* Enable charger */
	if (pbat->battery_charge(p_bat)) {
		puts("Charger enable error\n");
		return -1;
	}

	return 0;
}

int charger_type(void)
{
	if (!power_init_done) {
		if (exynos_power_init()) {
			puts("Can't init board power subsystem");
			return -1;
		}
	}

	if (!p_muic->chrg->chrg_type) {
		puts("Can't get charger type\n");
		return -1;
	}

	return p_muic->chrg->chrg_type(p_muic);
}

int battery_present(void)
{
	if (!power_init_done) {
		if (exynos_power_init()) {
			puts("Can't init board power subsystem");
			return -1;
		}
	}

	if (!p_chrg->chrg->chrg_bat_present) {
		puts("Can't get battery state\n");
		return -1;
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
			return -1;
		}
	}

	if (!p_fg->fg->fg_battery_update) {
		puts("Can't update battery state\n");
		return -1;
	}

	/* Check battery state */
	if (p_fg->fg->fg_battery_update(p_fg, p_bat)) {
		puts("Battery update error\n");
		return -1;
	}

	debug("[BAT]:\n#state:%u\n#soc:%3.1u\n#vcell:%u\n", bat->state,
							    bat->state_of_chrg,
							    bat->voltage_uV);

	*soc = bat->state_of_chrg;

	return 0;
}
#endif
