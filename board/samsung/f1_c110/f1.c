/*
 * Copyright (C) 2010 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
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
#include <i2c.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/keypad.h>
#include <asm/arch/mmc.h>
#include <asm/arch/power.h>
#include <asm/arch/mem.h>
#include <asm/arch/hs_otg.h>
#include <asm/arch/regs-otg.h>
#include <asm/arch/rtc.h>
#include <asm/arch/adc.h>
#include <mipi_ddi.h>
#include <asm/errno.h>
#include <fbutils.h>
#include <lcd.h>
#include <dsim.h>
#include <spi.h>
#include <bmp_layout.h>
#include <ramoops.h>
#include <pmic.h>

#include "animation_frames.h"
#include "gpio_setting.h"
#include "usb_mass_storage.h"

DECLARE_GLOBAL_DATA_PTR;

static unsigned int board_rev;
static unsigned int battery_soc;
static struct s5pc110_gpio *gpio;

enum {
	I2C_2,
	I2C_GPIO3,
	I2C_PMIC,
	I2C_GPIO5,
	I2C_GPIO6,
	I2C_GPIO7,
	I2C_GPIO10,
	I2C_NUM,
};

/*
 * i2c 2
 * SDA: GPD1[4]
 * SCL: GPD1[5]
 */
static struct i2c_gpio_bus_data i2c_2 = {
	.sda_pin	= 4,
	.scl_pin	= 5,
};

/*
 * i2c gpio3
 * SDA: GPJ3[6]
 * SCL: GPJ3[7]
 */
static struct i2c_gpio_bus_data i2c_gpio3 = {
	.sda_pin	= 6,
	.scl_pin	= 7,
};

/*
 * i2c pmic
 * SDA: GPJ4[0]
 * SCL: GPJ4[3]
 */
static struct i2c_gpio_bus_data i2c_pmic = {
	.sda_pin	= 0,
	.scl_pin	= 3,
};

/*
 * i2c gpio5
 * SDA: MP05[3]
 * SCL: MP05[2]
 */
static struct i2c_gpio_bus_data i2c_gpio5 = {
	.sda_pin	= 3,
	.scl_pin	= 2,
};

/*
 * i2c gpio6
 * SDA: GPJ3[4]
 * SCL: GPJ3[5]
 */
static struct i2c_gpio_bus_data i2c_gpio6 = {
	.sda_pin	= 4,
	.scl_pin	= 5,
};

/*
 * i2c gpio7
 * SDA: MP05[1]
 * SCL: MP05[0]
 */
static struct i2c_gpio_bus_data i2c_gpio7 = {
	.sda_pin	= 1,
	.scl_pin	= 0,
};

/*
 * i2c gpio10
 * SDA: GPJ3[0]
 * SCL: GPJ3[1]
 */
static struct i2c_gpio_bus_data i2c_gpio10 = {
	.sda_pin	= 0,
	.scl_pin	= 1,
};

static struct i2c_gpio_bus i2c_gpio[I2C_NUM];

u32 get_board_rev(void)
{
	return board_rev;
}

static int hwrevision(int rev)
{
	return (board_rev & 0xf) == rev;
}

static void check_battery(int mode);

static struct i2c_gpio_bus i2c_gpio[] = {
	{
		.bus	= &i2c_2,
	}, {
		.bus	= &i2c_gpio3,
	}, {
		.bus	= &i2c_pmic,
	}, {
		.bus	= &i2c_gpio5,
	}, {
		.bus	= &i2c_gpio6,
	}, {
		.bus	= &i2c_gpio7,
	}, {
		.bus	= &i2c_gpio10,
	},
};

void i2c_init_board(void)
{
	gpio = (struct s5pc110_gpio *)samsung_get_base_gpio();

	i2c_gpio[I2C_2].bus = &i2c_2;
	i2c_gpio[I2C_GPIO3].bus = &i2c_gpio3;
	i2c_gpio[I2C_PMIC].bus = &i2c_pmic;
	i2c_gpio[I2C_GPIO5].bus = &i2c_gpio5;
	i2c_gpio[I2C_GPIO6].bus = &i2c_gpio6;
	i2c_gpio[I2C_GPIO7].bus = &i2c_gpio7;
	i2c_gpio[I2C_GPIO10].bus = &i2c_gpio10;

	i2c_gpio[I2C_2].bus->gpio_base =
		(unsigned int)&gpio->d1;
	i2c_gpio[I2C_GPIO3].bus->gpio_base =
		(unsigned int)&gpio->j3;
	i2c_gpio[I2C_PMIC].bus->gpio_base =
		(unsigned int)&gpio->j4;
	i2c_gpio[I2C_GPIO5].bus->gpio_base =
		(unsigned int)&gpio->mp0_5;
	i2c_gpio[I2C_GPIO6].bus->gpio_base =
		(unsigned int)&gpio->j3;
	i2c_gpio[I2C_GPIO7].bus->gpio_base =
		(unsigned int)&gpio->mp0_5;

	i2c_gpio_init(i2c_gpio, I2C_NUM, I2C_PMIC);

	/* Reset on fuelgauge early */
	check_battery(1);
}

#ifdef CONFIG_MISC_INIT_R
#define DEV_INFO_LEN		256
static char device_info[DEV_INFO_LEN];
static int display_info;

static void empty_device_info_buffer(void)
{
	memset(device_info, 0x0, DEV_INFO_LEN);
}

static void dprintf(const char *fmt, ...)
{
	va_list args;
	uint i;
	char buf[128];

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);

	buf[127] = 0;

	if ((strlen(device_info) + strlen(buf)) > (DEV_INFO_LEN - 1)) {
		puts("Flushing device info...\n");
		puts(device_info);
		empty_device_info_buffer();
	}
	strncat(device_info, buf, 127);
	puts(buf);
}

#ifdef CONFIG_S5PC1XXFB
static void display_device_info(void)
{
	if (!display_info)
		return;

	init_font();
	set_font_xy(0, 450);
	set_font_color(FONT_WHITE);
	fb_printf(device_info);
	exit_font();

	memset(device_info, 0x0, DEV_INFO_LEN);

	udelay(5 * 1000 * 1000);
}
#endif

static char feature_buffer[32];

static unsigned int get_hw_revision(struct s5p_gpio_bank *bank, int hwrev3)
{
	unsigned int rev;

	gpio_direction_input(bank, 2);
	gpio_direction_input(bank, 3);
	gpio_direction_input(bank, 4);
	gpio_direction_input(bank, hwrev3);

	gpio_set_pull(bank, 2, GPIO_PULL_NONE);		/* HWREV_MODE0 */
	gpio_set_pull(bank, 3, GPIO_PULL_NONE);		/* HWREV_MODE1 */
	gpio_set_pull(bank, 4, GPIO_PULL_NONE);		/* HWREV_MODE2 */
	gpio_set_pull(bank, hwrev3, GPIO_PULL_NONE);	/* HWREV_MODE3 */

	rev = gpio_get_value(bank, 2);
	rev |= (gpio_get_value(bank, 3) << 1);
	rev |= (gpio_get_value(bank, 4) << 2);
	rev |= (gpio_get_value(bank, hwrev3) << 3);

	return rev;
}

static const char * const pcb_rev[] = {
	"AQUILA_2.1",
};

static void check_hw_revision(void)
{
	int hwrev;

	hwrev = get_hw_revision(&gpio->j0, 7);

	board_rev |= hwrev;
}

static void show_hw_revision(void)
{
	printf("HW Revision:\t0x%x\n", board_rev);
	printf("PCB Revision:\t%s\n", pcb_rev[(board_rev & 0xf) - 0xa]);
}

void get_rev_info(char *rev_info)
{
	sprintf(rev_info, "HW Revision: 0x%x (%s)\n",
			board_rev, pcb_rev[(board_rev & 0xf) - 0xa]);
}

static void check_auto_burn(void)
{
	unsigned long magic_base = CONFIG_SYS_SDRAM_BASE + 0x02000000;
	unsigned int count = 0;
	char buf[64];

	/* OneNAND */
	if (readl(magic_base) == 0x426f6f74) {	/* ASICC: Boot */
		puts("Auto burning bootloader\n");
		count += sprintf(buf + count, "run updateb; ");
	}
	/* MMC */
	if (readl(magic_base) == 0x654D4D43) {	/* ASICC: eMMC */
		puts("Auto burning bootloader (eMMC)\n");
		count += sprintf(buf + count, "run updatemmc; ");
	}
	if (readl(magic_base + 0x04) == 0x4b65726e) {	/* ASICC: Kern */
		puts("Auto burning kernel\n");
		count += sprintf(buf + count, "run updatek; ");
	}
	/* Backup u-boot in eMMC */
	if (readl(magic_base + 0x8) == 0x4261636B) {	/* ASICC: Back */
		puts("Auto buring u-boot image (boot partition2 in eMMC)\n");
		count += sprintf(buf + count, "run updatebackup; ");
	}

	if (count) {
		count += sprintf(buf + count, "reset");
		setenv("bootcmd", buf);
	}

	/* Clear the magic value */
	memset((void*) magic_base, 0, 2);
}

static void pmic_pin_init(void)
{
	unsigned int reg, value;

	/* AP_PS_HOLD: XEINT_0: GPH0[0]
	 * Note: Don't use GPIO PS_HOLD it doesn't work
	 */
	reg = S5PC110_PS_HOLD_CONTROL;
	value = readl(reg);
	value |= S5PC110_PS_HOLD_DIR_OUTPUT |
		S5PC110_PS_HOLD_DATA_HIGH |
		S5PC110_PS_HOLD_OUT_EN;
	writel(value, reg);

	/* nPOWER: XEINT_22: GPH2[6] interrupt mode */
	gpio_cfg_pin(&gpio->h2, 6, GPIO_IRQ);
	gpio_set_pull(&gpio->h2, 6, GPIO_PULL_UP);
}

static void enable_ldos(void)
{
	/* TOUCH_EN: XMMC3DATA_3: GPG3[6] output high */
	gpio_direction_output(&gpio->g3, 6, 1);
}

static void enable_t_flash(void)
{
	/* T_FLASH_EN : XM0ADDR_13: MP0_5[4] output high */
	gpio_direction_output(&gpio->mp0_5, 4, 1);
}

static void check_battery(int mode)
{
	unsigned char val[2];
	unsigned char addr = 0x36;	/* max17042 fuel gauge */

	i2c_set_bus_num(I2C_GPIO7);

	if (i2c_probe(addr)) {
		puts("Can't found max17042(max8997) fuel gauge\n");
		return;
	}

	/* mode 0: check mode / 1: enable mode */
	if (mode) {
		val[0] = 0x40;
		val[1] = 0x00;
		i2c_write(addr, 0xfe, 1, val, 2);
	} else {
		i2c_read(addr, 0x04, 1, val, 1);
		dprintf("battery:\t%d%%\n", val[0]);
		battery_soc = val[0];
	}
}

static int max8998_probe(void)
{
	unsigned char addr = 0xCC >> 1;

	i2c_set_bus_num(I2C_PMIC);

	if (i2c_probe(addr)) {
		puts("Can't found max8998\n");
		return 1;
	}

	return 0;
}

#define CHARGER_ANIMATION_FRAME		6
int check_exit_key(void)
{
	return pmic_get_irq(PWRON1S);
}

#define KBR3		(1 << 3)
#define KBR2		(1 << 2)
#define KBR1		(1 << 1)
#define KBR0		(1 << 0)

static void check_keypad(void)
{
	unsigned int auto_download = 0;

	if (auto_download)
		setenv("bootcmd", "usbdown");
}

static int max8998_has_ext_power_source(void)
{
	unsigned char addr, val[2];
	addr = 0xCC >> 1;

	/* TODO */
	return 0;

	if (max8998_probe())
		return 0;

	/* Accessing STATUS2 register */
	i2c_read(addr, 0x09, 1, val, 1);
	if (val[0] & (1 << 5))
		return 1;

	return 0;
}

#define S5PC110_RST_STAT	0xE010A000

#define SWRESET			(1 << 3)
#define WDTRESET		(1 << 2)
#define WARMRESET		(1 << 1)
#define EXTRESET		(1 << 0)

static int get_reset_status(void)
{
	return readl(S5PC110_RST_STAT) & 0xf;
}

static void check_reset_status(void)
{
	int status = get_reset_status();

	puts("Reset Status: ");

	switch (status) {
	case EXTRESET:
		puts("Pin(Ext) Reset\n");
		break;
	case WARMRESET:
		puts("Warm Reset\n");
		break;
	case WDTRESET:
		puts("Watchdog Reset\n");
		break;
	case SWRESET:
		puts("S/W Reset\n");
		break;
	default:
		printf("Unknown (0x%x)\n", status);
	}
}

#define MAX8998_REG_ONOFF1	0x11
#define MAX8998_REG_ONOFF2	0x12
#define MAX8998_REG_ONOFF3	0x13
#define MAX8998_REG_ONOFF4	0x14
#define MAX8998_REG_LDO7	0x21
#define MAX8998_REG_LDO17	0x29
/* ONOFF1 */
#define MAX8998_LDO3		(1 << 2)
/* ONOFF2 */
#define MAX8998_LDO6		(1 << 7)
#define MAX8998_LDO7		(1 << 6)
#define MAX8998_LDO8		(1 << 5)
#define MAX8998_LDO9		(1 << 4)
#define MAX8998_LDO10		(1 << 3)
#define MAX8998_LDO11		(1 << 2)
#define MAX8998_LDO12		(1 << 1)
#define MAX8998_LDO13		(1 << 0)
/* ONOFF3 */
#define MAX8998_LDO14		(1 << 7)
#define MAX8998_LDO15		(1 << 6)
#define MAX8998_LDO16		(1 << 5)
#define MAX8998_LDO17		(1 << 4)

static void init_pmic(void)
{
	unsigned char addr;
	unsigned char val[2];

	/* TODO */
	return;

	addr = 0xCC >> 1;	/* max8998 */
	if (max8998_probe())
		return;

	/* ONOFF1 */
	i2c_read(addr, MAX8998_REG_ONOFF1, 1, val, 1);
	val[0] &= ~MAX8998_LDO3;
	i2c_write(addr, MAX8998_REG_ONOFF1, 1, val, 1);

	/* ONOFF2 */
	i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	/*
	 * Disable LDO8(USB_3.3V), LDO10(VPLL_1.1V), LDO11(CAM_IO_2.8V),
	 * LDO12(CAM_ISP_1.2V), LDO13(CAM_A_2.8V)
	 */
	val[0] &= ~(MAX8998_LDO8 | MAX8998_LDO10 | MAX8998_LDO11 |
			MAX8998_LDO12 | MAX8998_LDO13);

	val[0] |= MAX8998_LDO7;		/* LDO7: VLCD_1.8V */

	i2c_write(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	/* ONOFF3 */
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	/*
	 * Disable LDO14(CAM_CIF_1.8), LDO15(CAM_AF_3.3V),
	 * LDO16(VMIPI_1.8V), LDO17(CAM_8M_1.8V)
	 */
	val[0] &= ~(MAX8998_LDO14 | MAX8998_LDO15 |
			MAX8998_LDO16 | MAX8998_LDO17);

	val[0] |= MAX8998_LDO17;	/* LDO17: VCC_3.0V_LCD */

	i2c_write(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
}

#ifdef CONFIG_LCD

void fimd_clk_set(void)
{
	struct s5pc110_clock *clk =
		(struct s5pc110_clock *)samsung_get_base_clock();
	unsigned int cfg = 0;

	/* set lcd src clock */
	cfg = readl(&clk->src1);
	cfg &= ~(0xf << 20);
	cfg |= (0x6 << 20);
	writel(cfg, &clk->src1);

	/* set fimd ratio */
	cfg = readl(&clk->div1);
	cfg &= ~(0xf << 20);
	cfg |= (0x2 << 20);
	writel(cfg, &clk->div1);
}

void mipi_power_on(void)
{
	run_command("pmic ldo 3 on", 0);
	run_command("pmic ldo 7 on", 0);
}


extern void s6e63m0_set_platform_data(struct spi_platform_data *pd);
extern void s6d16a0x_set_platform_data(struct spi_platform_data *pd);
extern void ld9040_set_platform_data(struct spi_platform_data *pd);

struct spi_platform_data spi_pd;

void lcd_cfg_gpio(void)
{
	unsigned int i, f3_end = 4;

	for (i = 0; i < 8; i++) {
		/* set GPF0,1,2[0:7] for RGB Interface and Data lines (32bit) */
		gpio_cfg_pin(&gpio->f0, i, GPIO_FUNC(2));
		gpio_cfg_pin(&gpio->f1, i, GPIO_FUNC(2));
		gpio_cfg_pin(&gpio->f2, i, GPIO_FUNC(2));
		/* pull-up/down disable */
		gpio_set_pull(&gpio->f0, i, GPIO_PULL_NONE);
		gpio_set_pull(&gpio->f1, i, GPIO_PULL_NONE);
		gpio_set_pull(&gpio->f2, i, GPIO_PULL_NONE);

		/* drive strength to max (24bit) */
		gpio_set_drv(&gpio->f0, i, GPIO_DRV_4X);
		gpio_set_rate(&gpio->f0, i, GPIO_DRV_SLOW);
		gpio_set_drv(&gpio->f1, i, GPIO_DRV_4X);
		gpio_set_rate(&gpio->f1, i, GPIO_DRV_SLOW);
		gpio_set_drv(&gpio->f2, i, GPIO_DRV_4X);
		gpio_set_rate(&gpio->f2, i, GPIO_DRV_SLOW);
	}

	for (i = 0; i < f3_end; i++) {
		/* set GPF3[0:3] for RGB Interface and Data lines (32bit) */
		gpio_cfg_pin(&gpio->f3, i, GPIO_PULL_UP);
		/* pull-up/down disable */
		gpio_set_pull(&gpio->f3, i, GPIO_PULL_NONE);
		/* drive strength to max (24bit) */
		gpio_set_drv(&gpio->f3, i, GPIO_DRV_4X);
		gpio_set_rate(&gpio->f3, i, GPIO_DRV_SLOW);
	}
	/* display output path selection (only [1:0] valid) */
	writel(0x2, 0xE0107008);

	/* gpio pad configuration for LCD reset. */
	gpio_cfg_pin(&gpio->mp0_5, 5, GPIO_OUTPUT);

	/* gpio pad configuration for LCD ON. */
	gpio_cfg_pin(&gpio->j1, 3, GPIO_OUTPUT);

	/*
	 * gpio pad configuration for
	 * DISPLAY_CS, DISPLAY_CLK, DISPLAY_SO, DISPLAY_SI.
	 */
	gpio_cfg_pin(&gpio->mp0_1, 1, GPIO_OUTPUT);
	gpio_cfg_pin(&gpio->mp0_4, 1, GPIO_OUTPUT);
	gpio_cfg_pin(&gpio->mp0_4, 2, GPIO_INPUT);
	gpio_cfg_pin(&gpio->mp0_4, 3, GPIO_OUTPUT);

	spi_pd.cs_bank = &gpio->mp0_1;
	spi_pd.cs_num = 1;
	spi_pd.clk_bank = &gpio->mp0_4;
	spi_pd.clk_num = 1;
	spi_pd.si_bank = &gpio->mp0_4;
	spi_pd.si_num = 3;
	spi_pd.so_bank = &gpio->mp0_4;
	spi_pd.so_num = 2;

	spi_pd.mode = SPI_MODE_3;

	spi_pd.cs_active = ACTIVE_LOW;
	spi_pd.word_len = 8;

	return;
}

#define SWRST_REG		0x00
#define LEDCON_REG		0x01
#define LED_CUR_SET_REG		0x03
#define LED_CUR_TR_REG		0x08

#define SWRST			0x01
#define NORMAL_MODE		0x09
#define CUR_SET			0x63
#define TR_SET			0x00

void backlight_on(unsigned int onoff)
{
}

void reset_lcd(void)
{
}

void lcd_power_on(unsigned int onoff)
{
}

extern void s6e63m0_cfg_ldo(void);
extern void s6e63m0_enable_ldo(unsigned int onoff);
extern void s6d16a0x_cfg_ldo(void);
extern void s6d16a0x_enable_ldo(unsigned int onoff);
extern void ld9040_cfg_ldo(void);
extern void ld9040_enable_ldo(unsigned int onoff);
extern void s3cfb_set_trigger(void);
extern int s3cfb_is_i80_frame_done(void);

int s5p_no_lcd_support(void)
{
	return 0;
}

static struct dsim_config dsim_info = {
	/* main frame fifo auto flush at VSYNC pulse */
	.auto_flush = DSIM_FALSE,
	.eot_disable = DSIM_FALSE,

	.auto_vertical_cnt = DSIM_FALSE,
	.hse = DSIM_FALSE,
	.hfp = DSIM_FALSE,
	.hbp = DSIM_FALSE,
	.hsa = DSIM_FALSE,

	.e_no_data_lane = DSIM_DATA_LANE_2,
	.e_byte_clk = DSIM_PLL_OUT_DIV8,

	/* 472MHz: LSI Recommended */
	.p = 3,
	.m = 118,
	.s = 1,

	/* D-PHY PLL stable time spec :min = 200usec ~ max 400usec */
	.pll_stable_time = 500,

	.esc_clk = 10 * 1000000,	/* escape clk : 10MHz */

	/* stop state holding counter after bta change count 0 ~ 0xfff */
	.stop_holding_cnt = 0x0f,
	.bta_timeout = 0xff,		/* bta timeout 0 ~ 0xff */
	.rx_timeout = 0xffff,		/* lp rx timeout 0 ~ 0xffff */

	.e_lane_swap = DSIM_NO_CHANGE,
};

static struct dsim_lcd_config dsim_lcd_info = {
	.e_interface		= DSIM_COMMAND,

	.parameter[DSI_VIRTUAL_CH_ID]	= (unsigned int) DSIM_VIRTUAL_CH_0,
	.parameter[DSI_FORMAT]		= (unsigned int) DSIM_24BPP_888,
	.parameter[DSI_VIDEO_MODE_SEL]	= (unsigned int) DSIM_BURST,

	.mipi_ddi_pd		= NULL,
};

struct s5p_platform_dsim s6e39a0x_platform_data = {
	.pvid = NULL,
	.clk_name = "dsim",
	.dsim_info = &dsim_info,
	.dsim_lcd_info = &dsim_lcd_info,
	.lcd_panel_name = "s6e39a0x",
	.platform_rev = 1,

	/*
	 * the stable time of needing to write data on SFR
	 * when the mipi mode becomes LP mode.
	 */
	.delay_for_stabilization = 600000,
};

extern void s5p_set_dsim_platform_data(struct s5p_platform_dsim *pd);
extern void s6e39a0x_init(void);

void init_panel_info(vidinfo_t *vid)
{
	vid->cfg_gpio = NULL;
	vid->reset_lcd = NULL;
	vid->backlight_on = NULL;
	vid->lcd_power_on = NULL;
	vid->mipi_power = NULL;

	vid->cfg_ldo = NULL;
	vid->enable_ldo = NULL;

	vid->init_delay = 0;
	vid->reset_delay = 0;
	vid->power_on_delay = 0;

	vid->vl_freq	= 60;
	vid->vl_col	= 480;
	vid->vl_row	= 800;
	vid->vl_width	= 480;
	vid->vl_height	= 800;

	vid->dual_lcd_enabled = 0;
	vid->interface_mode = FIMD_RGB_INTERFACE;

	vid->vl_clkp	= CONFIG_SYS_HIGH;
	vid->vl_hsp	= CONFIG_SYS_LOW;
	vid->vl_vsp	= CONFIG_SYS_LOW;
	vid->vl_dp	= CONFIG_SYS_HIGH;
	vid->vl_bpix	= 32;

	/* S6E63M0 LCD Panel */
	vid->vl_hspw	= 2;
	vid->vl_hbpd	= 16;
	vid->vl_hfpd	= 16;

	vid->vl_vspw	= 2;
	vid->vl_vbpd	= 3;
	vid->vl_vfpd	= 28;

	vid->cfg_gpio = lcd_cfg_gpio;
	vid->reset_lcd = reset_lcd;
	vid->backlight_on = backlight_on;
	vid->lcd_power_on = lcd_power_on;
	vid->cfg_ldo = s6e63m0_cfg_ldo;
	vid->enable_ldo = s6e63m0_enable_ldo;

	vid->cs_setup = 0;
	vid->wr_setup = 0;
	vid->wr_act = 1;
	vid->wr_hold = 0;

	vid->cfg_gpio = lcd_cfg_gpio;
	vid->backlight_on = NULL;
	vid->lcd_power_on = lcd_power_on;
	vid->reset_lcd = reset_lcd;
	vid->mipi_power = mipi_power_on;

	vid->init_delay = 0;
	vid->power_on_delay = 30000;
	vid->reset_delay = 20000;
	vid->interface_mode = FIMD_CPU_INTERFACE;

	s6e39a0x_platform_data.part_reset = s5p_dsim_part_reset;
	s6e39a0x_platform_data.init_d_phy = s5p_dsim_init_d_phy;
	s6e39a0x_platform_data.get_fb_frame_done = s3cfb_is_i80_frame_done;
	s6e39a0x_platform_data.trigger = s3cfb_set_trigger;
	s6e39a0x_platform_data.pvid = vid;
	s6e39a0x_init();
	s5p_set_dsim_platform_data(&s6e39a0x_platform_data);
}
#endif

#ifdef CONFIG_CMD_RAMOOPS
static void show_dump_msg(void)
{
	int ret;

	ret = ramoops_init(samsung_get_base_modem());

	if (!ret)
		setenv("bootdelay", "-1");
}
#endif

int misc_init_r(void)
{
	check_reset_status();
#ifdef CONFIG_CMD_RAMOOPS
	show_dump_msg();
#endif

	show_hw_revision();

	/* Set proper PMIC pins */
	pmic_pin_init();

	/* Check auto burning */
	check_auto_burn();

	/* To power up I2C2 */
	enable_ldos();

	/* Enable T-Flash at Limo Real or Limo Universal */
	enable_t_flash();

	/* To usbdown automatically */
	check_keypad();

	/* check max8998 */
	pmic_bus_init(I2C_PMIC);
	init_pmic();

#ifdef CONFIG_S5PC1XXFB
	display_device_info();
#endif

	/* check max17042 of max8997 */
	check_battery(0);

#ifdef CONFIG_INFO_ACTION
	info_action_check();
#endif

	return 0;
}
#endif

int board_init(void)
{
	/* Set Initial global variables */
	gpio = (struct s5pc110_gpio *)samsung_get_base_gpio();

	/* Check H/W Revision */
	check_hw_revision();

	gd->bd->bi_arch_number = MACH_TYPE_GONI;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE + PHYS_SDRAM_2_SIZE +
			PHYS_SDRAM_3_SIZE;

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
}

#ifdef CONFIG_CMD_USBDOWN
int usb_board_init(void)
{
	/* interrupt clear */
	pmic_get_irq(PWRON1S);

#ifdef CONFIG_CMD_PMIC
	run_command("pmic ldo 8 on", 0);
	run_command("pmic ldo 3 on", 0);
#endif

	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int s5p_no_mmc_support(void)
{
	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int i, err;
	int buswidth = 4;

	if (s5p_no_mmc_support())
		return -1;

	/* MASSMEMORY_EN: XMSMDATA7: GPJ2[7] output high */
	gpio_direction_output(&gpio->j2, 7, 1);

	/*
	 * MMC0 GPIO
	 * GPG0[0]	SD_0_CLK
	 * GPG0[1]	SD_0_CMD
	 * GPG0[2]	SD_0_CDn	-> Not used
	 * GPG0[3:6]	SD_0_DATA[0:3]
	 */
	for (i = 0; i < 7; i++) {
		if (i == 2)
			continue;
		/* GPG0[0:6] special function 2 */
		gpio_cfg_pin(&gpio->g0, i, 0x2);
		/* GPG0[0:6] pull disable */
		gpio_set_pull(&gpio->g0, i, GPIO_PULL_NONE);
		/* GPG0[0:6] drv 4x */
		gpio_set_drv(&gpio->g0, i, GPIO_DRV_4X);
	}

	/* T_FLASH_DETECT: EINT28: GPH3[4] interrupt mode */
	gpio_cfg_pin(&gpio->h3, 4, GPIO_IRQ);
	gpio_set_pull(&gpio->h3, 4, GPIO_PULL_UP);

	err = s5p_mmc_init(0, buswidth);

	if (!gpio_get_value(&gpio->h3, 4)) {
		for (i = 0; i < 7; i++) {
			if (i == 2)
				continue;
			/* GPG2[0:6] special function 2 */
			gpio_cfg_pin(&gpio->g2, i, 0x2);
			/* GPG2[0:6] pull disable */
			gpio_set_pull(&gpio->g2, i, GPIO_PULL_NONE);
			/* GPG2[0:6] drv 4x */
			gpio_set_drv(&gpio->g2, i, GPIO_DRV_4X);
		}
		err =s5p_mmc_init(2, 4);
	}

	return err;
}
#endif
