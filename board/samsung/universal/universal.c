/*
 * Copyright (C) 2009 Samsung Electronics
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
#include <lcd.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/keypad.h>
#include <asm/arch/mmc.h>
#include <asm/arch/power.h>
#include <asm/arch/mem.h>
#include <fbutils.h>

DECLARE_GLOBAL_DATA_PTR;

#define C100_MACH_START			3000
#define C110_MACH_START			3100

static unsigned int board_rev;
static unsigned int battery_soc;

#define I2C_GPIO3	0
#define I2C_PMIC	1
#define I2C_GPIO5	2
#define I2C_GPIO6	3

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
 * SDA: GPJ3[0]
 * SCL: GPJ3[0]
 */
static struct i2c_gpio_bus_data i2c_gpio6 = {
	.sda_pin	= 0,
	.scl_pin	= 1,
};


static struct i2c_gpio_bus i2c_gpio[] = {
	{
		.bus	= &i2c_gpio3,
	}, {
		.bus	= &i2c_pmic,
	}, {
		.bus	= &i2c_gpio5,
	}, {
		.bus	= &i2c_gpio6,
	},
};

static void enable_touchkey(void);
static void enable_battery(void);

void i2c_init_board(void)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;

	if (cpu_is_s5pc100())
		return;

	i2c_gpio[0].bus->gpio_base = (unsigned int)&gpio->gpio_j3;
	i2c_gpio[1].bus->gpio_base = (unsigned int)&gpio->gpio_j4;
	i2c_gpio[2].bus->gpio_base = (unsigned int)&gpio->gpio_mp0_5;
	i2c_gpio[3].bus->gpio_base = (unsigned int)&gpio->gpio_j3;

	i2c_gpio_init(i2c_gpio, ARRAY_SIZE(i2c_gpio), I2C_PMIC);

	/* XXX Power on Touckey early (it requires 100 msec power up time) */
	enable_touchkey();

	/* Reset on max17040 early */
	if (battery_soc == 0)
		enable_battery();
}

u32 get_board_rev(void)
{
	return board_rev;
}

static int hwrevision(int rev)
{
	return (board_rev & 0xf) == rev;
}

enum {
	MACH_UNIVERSAL,
	MACH_TICKERTAPE,
	MACH_AQUILA,
	MACH_P1P2,
	MACH_GEMINUS,
};

#define SCREEN_SPLIT_FEATURE	0x100

/* board is MACH_AQUILA and board is like below. */
#define J1_B2_BOARD		0x200
#define LIMO_UNIVERSAL_BOARD	0x400
#define LIMO_REAL_BOARD		0x800
/* board is MACH_P1P2 and board is like below. */
#define P1_REAL_BOARD		0x200
#define P2_REAL_BOARD		0x400

#define BOARD_MASK		0xF00

static int machine_is_aquila(void)
{
	int board;

	if (cpu_is_s5pc100())
		return 0;

	board = gd->bd->bi_arch_number - C110_MACH_START;

	return board == MACH_AQUILA;
}

static int machine_is_p1p2(void)
{
	int board;

	if (cpu_is_s5pc100())
		return 0;

	board = gd->bd->bi_arch_number - C110_MACH_START;

	return board == MACH_P1P2;
}

static int machine_is_tickertape(void)
{
	int board;

	if (cpu_is_s5pc100())
		return 0;

	board = gd->bd->bi_arch_number - C110_MACH_START;

	return board == MACH_TICKERTAPE;
}

static int machine_is_geminus(void)
{
	int board;

	if (cpu_is_s5pc100())
		return 0;

	board = gd->bd->bi_arch_number - C110_MACH_START;

	return board == MACH_GEMINUS;
}

static int board_is_limo_universal(void)
{
	int board;

	if (cpu_is_s5pc100())
		return 0;

	board = gd->bd->bi_arch_number - C110_MACH_START;

	return board == MACH_AQUILA && (board_rev & LIMO_UNIVERSAL_BOARD);
}

static int board_is_limo_real(void)
{
	int board;

	if (cpu_is_s5pc100())
		return 0;

	board = gd->bd->bi_arch_number - C110_MACH_START;

	return board == MACH_AQUILA && (board_rev & LIMO_REAL_BOARD);
}

static int board_is_j1b2(void)
{
	int board;

	if (cpu_is_s5pc100())
		return 0;

	board = gd->bd->bi_arch_number - C110_MACH_START;

	return board == MACH_AQUILA && (board_rev & J1_B2_BOARD);
}

#ifdef CONFIG_MISC_INIT_R
#define DEV_INFO_LEN		512
static char device_info[DEV_INFO_LEN];
static int display_info = 0;

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
		device_info[0] = 0;
	}
	strcat(device_info, buf);
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

static const char *board_name[] = {
	"Universal",
	"TickerTape",
	"Aquila",
	"P1P2",
	"Geminus",
};

enum {
	MEM_4G1G1G,
	MEM_4G2G1G,
	MEM_4G3G1G,
};

static char feature_buffer[32];

static char *display_features(int board, int board_rev)
{
	int count = 0;
	char *buf = feature_buffer;

	if (board == MACH_AQUILA) {
		if (board_rev & SCREEN_SPLIT_FEATURE)
			count += sprintf(buf + count, " - ScreenSplit");
		if (board_rev & J1_B2_BOARD)
			count += sprintf(buf + count, " - J1 B2 board");
		/* Limo Real or Universal */
		if (board_rev & LIMO_REAL_BOARD)
			count += sprintf(buf + count, " - Limo Real");
		else if (board_rev & LIMO_UNIVERSAL_BOARD)
			count += sprintf(buf + count, " - Limo Universal");
	} else if (board == MACH_P1P2) {
		/* P1P2 */
		if (board_rev & P1_REAL_BOARD)
			count += sprintf(buf + count, " - P1 Real");
		else if (board_rev & P2_REAL_BOARD)
			count += sprintf(buf + count, " - P2 Real");
		else
			count += sprintf(buf + count, " - Universal");
	}

	return buf;
}

static void check_board_revision(int board, int rev)
{
	switch (board) {
	case MACH_AQUILA:
		/* Limo Real or Universal */
		if (rev & LIMO_UNIVERSAL_BOARD)
			board_rev &= ~J1_B2_BOARD;
		if (rev & LIMO_REAL_BOARD) {
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD);
		}
		break;
	case MACH_P1P2:
		break;
	case MACH_TICKERTAPE:
	case MACH_GEMINUS:
		board_rev &= ~BOARD_MASK;
		break;
	default:
		break;
	}
}

static unsigned int get_hw_revision(struct s5pc1xx_gpio_bank *bank)
{
	unsigned int rev;

	gpio_direction_input(bank, 1);
	gpio_direction_input(bank, 2);
	gpio_direction_input(bank, 3);
	gpio_direction_input(bank, 4);

	gpio_set_pull(bank, 1, GPIO_PULL_NONE);		/* HWREV_MODE3 */
	gpio_set_pull(bank, 2, GPIO_PULL_NONE);		/* HWREV_MODE0 */
	gpio_set_pull(bank, 3, GPIO_PULL_NONE);		/* HWREV_MODE1 */
	gpio_set_pull(bank, 4, GPIO_PULL_NONE);		/* HWREV_MODE2 */

	rev = gpio_get_value(bank, 2);
	rev |= (gpio_get_value(bank, 3) << 1);
	rev |= (gpio_get_value(bank, 4) << 2);
	rev |= (gpio_get_value(bank, 1) << 3);

	return rev;
}

static void check_hw_revision(void)
{
	unsigned int board = MACH_UNIVERSAL;	/* Default is Universal */

	if (cpu_is_s5pc100()) {
		struct s5pc100_gpio *gpio =
			(struct s5pc100_gpio *)S5PC100_GPIO_BASE;

		board_rev = get_hw_revision(&gpio->gpio_j0);

		/* C100 TickerTape */
		if (board_rev == 3)
			board = MACH_TICKERTAPE;
	} else {
		struct s5pc110_gpio *gpio =
			(struct s5pc110_gpio *)S5PC110_GPIO_BASE;

		board_rev = get_hw_revision(&gpio->gpio_j0);

		/*
		 * Note Check 'Aquila' board first
		 *
		 * TT: TickerTape
		 * SS: ScreenSplit
		 * LRA: Limo Real Aquila
		 * LUA: Limo Universal Aquila
		 * OA: Old Aquila
		 * P1P2: Smart Book
		 *
		 * ADDR = 0xE0200000 + OFF
		 *
		 * 	 OFF	Universal LRA  LUA  OA   TT   SS	P1P2
		 *   J1: 0x0264	0x10      0x00 0x00 0x00 0x00 0x00	0x00
		 *   H1: 0x0C24	    W      0x28 0xA8 0x1C		0x18
		 *   H3: 0x0C64	          0x03 0x07 0x0F		0xff
		 *   D1: 0x00C4	0x0F	  0x3F 0x3F 0x0F 0xXC 0x3F
		 *    I: 0x0224	                    0x02 0x00 0x08
		 * MP03: 0x0324	                    0x9x      0xbx 0x9x
		 * MP05: 0x0364	                    0x80      0x88
		 */

		/* C110 Aquila */
		if (gpio_get_value(&gpio->gpio_j1, 4) == 0) {
			board = MACH_AQUILA;
			board_rev |= J1_B2_BOARD;

			/* Check board */
			if (gpio_get_value(&gpio->gpio_h1, 2) == 0)
				board_rev |= LIMO_UNIVERSAL_BOARD;

			if (gpio_get_value(&gpio->gpio_h3, 2) == 0)
				board_rev |= LIMO_REAL_BOARD;
#if 0
			/* C110 Aquila ScreenSplit */
			if (gpio_get_value(&gpio->gpio_mp0_3, 5))
				board_rev |= SCREEN_SPLIT_FEATURE;
			else {
				if (gpio_get_value(&gpio->gpio_i, 3))
					board_rev |= SCREEN_SPLIT_FEATURE;
			}
#endif
		}

		/* C110 TickerTape */
		if (gpio_get_value(&gpio->gpio_d1, 0) == 0 &&
			gpio_get_value(&gpio->gpio_d1, 1) == 0)
			board = MACH_TICKERTAPE;

		/* C110 P1P2 */
		if (gpio_get_value(&gpio->gpio_h3, 7) == 1) {
			board = MACH_P1P2;
			board_rev &= ~BOARD_MASK;
		} else if (gpio_get_value(&gpio->gpio_j0, 7) == 1) {
			board = MACH_P1P2;
			board_rev &= ~BOARD_MASK;
			if (gpio_get_value(&gpio->gpio_j0, 6) == 1)
				board_rev |= P1_REAL_BOARD;
			if (gpio_get_value(&gpio->gpio_j0, 6) == 0)
				board_rev |= P2_REAL_BOARD;
		}

		/* C110 Geminus */
		gpio_set_pull(&gpio->gpio_j1, 2, GPIO_PULL_NONE);
		gpio_direction_input(&gpio->gpio_j1, 2);
		if (gpio_get_value(&gpio->gpio_j1, 2) == 1)
			board = MACH_GEMINUS;
		gpio_set_pull(&gpio->gpio_j1, 2, GPIO_PULL_DOWN);
		gpio_direction_output(&gpio->gpio_j1, 2, 0);
	}

	/* Set machine id */
	if (cpu_is_s5pc110())
		gd->bd->bi_arch_number = C110_MACH_START + board;
	else
		gd->bd->bi_arch_number = C100_MACH_START + board;

	/* Architecture Common settings */
	if (cpu_is_s5pc110()) {
		setenv("mtdparts", MTDPARTS_DEFAULT_4KB);
	} else {
		setenv("bootk", "onenand read 0x30007FC0 0x60000 0x300000; bootm 0x30007FC0");
		setenv("updatek", "onenand erase 0x60000 0x300000; onenand write 0x31008000 0x60000 0x300000");
	}
}

void show_hw_revision(void)
{
	int board;

	/*
	 * Workaround for Rev 0.3 + CP Ver ES 3.1
	 * it's Rev 0.4
	 */
	if (board_is_limo_real()) {
		if (hwrevision(0)) {
			struct s5pc110_gpio *gpio =
				(struct s5pc110_gpio *)S5PC110_GPIO_BASE;

			/* default is Rev 0.4 */
			board_rev &= ~0xf;
			board_rev |= 0x4;

			udelay(2000);

			/* if GPJ0[4] is low, it's Rev 0.0 */
			if (gpio_get_value(&gpio->gpio_j0, 4) == 0)
				board_rev &= ~0xf;
		}
	}

	if (cpu_is_s5pc110())
		board = gd->bd->bi_arch_number - C110_MACH_START;
	else
		board = gd->bd->bi_arch_number - C100_MACH_START;

	check_board_revision(board, board_rev);
	dprintf("HW Revision:\t%x (%s%s)\n", board_rev, board_name[board],
		display_features(board, board_rev));
}

static void check_auto_burn(void)
{
	unsigned long magic_base = CONFIG_SYS_SDRAM_BASE + 0x02000000;
	unsigned int count = 0;
	char buf[64];

	if (readl(magic_base) == 0x426f6f74) {	/* ASICC: Boot */
		printf("Auto burning bootloader\n");
		count += sprintf(buf + count, "run updateb; ");
	}
	if (readl(magic_base + 0x04) == 0x4b65726e) {	/* ASICC: Kern */
		printf("Auto burning kernel\n");
		count += sprintf(buf + count, "run updatek; ");
	}

	if (count) {
		count += sprintf(buf + count, "reset");
		setenv("bootcmd", buf);
	}

	/* Clear the magic value */
	writel(0xa5a55a5a, magic_base);
	writel(0xa5a55a5a, magic_base + 0x4);
}

static void pmic_pin_init(void)
{
	unsigned int reg, value;
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;

	if (cpu_is_s5pc100())
		return;

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
	gpio_cfg_pin(&gpio->gpio_h2, 6, GPIO_IRQ);
	gpio_set_pull(&gpio->gpio_h2, 6, GPIO_PULL_UP);
}

static void enable_ldos(void)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;

	if (cpu_is_s5pc100())
		return;

	if (machine_is_p1p2())
		return;

	/* TOUCH_EN: XMMC3DATA_3: GPG3[6] output high */
	gpio_direction_output(&gpio->gpio_g3, 6, 1);
}

static void enable_t_flash(void)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;

	if (!(board_is_limo_universal() || board_is_limo_real()))
		return;

	if (machine_is_p1p2())
		return;

	/* T_FLASH_EN : XM0ADDR_13: MP0_5[4] output high */
	gpio_direction_output(&gpio->gpio_mp0_5, 4, 1);
}

static void setup_limo_real_gpios(void)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;

	if (!board_is_limo_real())
		return;

	/*
	 * Note: Please write GPIO alphabet order
	 */
	/* CODEC_LDO_EN: XVVSYNC_LDI: GPF3[4] output high */
	gpio_direction_output(&gpio->gpio_f3, 4, 1);

	if (hwrevision(0))
		/* RESET_REQ_N: XM0BEN_1: MP0_2[1] output high */
		gpio_direction_output(&gpio->gpio_mp0_2, 1, 1);
	else
		/* RESET_REQ_N: XM0CSn_2: MP0_1[2] output high */
		gpio_direction_output(&gpio->gpio_mp0_1, 2, 1);

	/* T_FLASH_DETECT: EINT28: GPH3[4] interrupt mode */
	gpio_cfg_pin(&gpio->gpio_h3, 4, GPIO_IRQ);
	gpio_set_pull(&gpio->gpio_h3, 4, GPIO_PULL_UP);
}

static void setup_p1p2_gpios(void)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *) S5PC110_GPIO_BASE;

	if (!machine_is_p1p2())
		return;

	/*
	 * Note: Please write GPIO alphabet order
	 */
	/* RESET_REQ_N: XM0FRnB[1]: MP0_3[5] output high */
	gpio_direction_output(&gpio->gpio_mp0_3, 5, 1);
	/* CODEC_LDO_EN: XM0FRnB[2]: MP0_3[6] output high */
	gpio_direction_output(&gpio->gpio_mp0_3, 6, 1);
}

#define KBR3		(1 << 3)
#define KBR2		(1 << 2)
#define KBR1		(1 << 1)
#define KBR0		(1 << 0)

static void enable_touchkey(void)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;

	/* TOUCH_EN - GPIO_J3(0) : (J1B2) */
	/* TOUCH_EN - GPIO_J3(5) : (not J1B2) */
	if (board_rev & J1_B2_BOARD)
		gpio_direction_output(&gpio->gpio_j3, 0, 1);
	else
		gpio_direction_output(&gpio->gpio_j3, 5, 1);

	/* TOUCH_CE - GPIO_J2(6) */
	gpio_direction_output(&gpio->gpio_j2, 6, 1);	/* TOUCH_CE */
}

static void check_keypad(void)
{
	unsigned int reg, value;
	unsigned int col_mask, row_mask;
	unsigned int auto_download = 0;
	unsigned int col_value[4], i;
	unsigned char val[2];
	unsigned char addr = 0x20;		/* mcs5000 3-touchkey */
	if (cpu_is_s5pc100()) {
		struct s5pc100_gpio *gpio =
			(struct s5pc100_gpio *)S5PC100_GPIO_BASE;

		/* Set GPH2[2:0] to KP_COL[2:0] */
		gpio_cfg_pin(&gpio->gpio_h2, 0, 0x3);
		gpio_cfg_pin(&gpio->gpio_h2, 1, 0x3);
		gpio_cfg_pin(&gpio->gpio_h2, 2, 0x3);

		/* Set GPH3[2:0] to KP_ROW[2:0] */
		gpio_cfg_pin(&gpio->gpio_h3, 0, 0x3);
		gpio_cfg_pin(&gpio->gpio_h3, 1, 0x3);
		gpio_cfg_pin(&gpio->gpio_h3, 2, 0x3);

		reg = S5PC100_KEYPAD_BASE;
	} else {
		struct s5pc110_gpio *gpio =
			(struct s5pc110_gpio *)S5PC110_GPIO_BASE;

		if (board_is_limo_real() || board_is_limo_universal()) {
			row_mask = 0x00FF;
			col_mask = 0x0FFF;
		} else {
			row_mask = 0xFFFF;
			col_mask = 0xFFFF;
		}

		for (i = 0; i < 4; i++) {
			/* Set GPH3[3:0] to KP_ROW[3:0] */
			if (row_mask & (0xF << (i << 2))) {
				gpio_cfg_pin(&gpio->gpio_h3, i, 0x3);
				if (!machine_is_p1p2())
					gpio_set_pull(&gpio->gpio_h3, i, GPIO_PULL_UP);
			}

			/* Set GPH2[3:0] to KP_COL[3:0] */
			if (col_mask & (0xF << (i << 2)))
				gpio_cfg_pin(&gpio->gpio_h2, i, 0x3);
			if (machine_is_p1p2())
				gpio_set_pull(&gpio->gpio_h2, i, GPIO_PULL_UP);
		}

		reg = S5PC110_KEYPAD_BASE;
	}
	/* init col */
	value = 0x00;
	writel(value, reg + S5PC1XX_KEYIFCOL_OFFSET);
	value = readl(reg + S5PC1XX_KEYIFROW_OFFSET);
	/* VOLUMEDOWN and CAM(Half shot) Button */
	if ((value & KBR1) == 0) {
		i = 0;
		while (i < 4) {
			value = readl(reg + S5PC1XX_KEYIFCOL_OFFSET);
			value |= 0xff;
			value &= ~(1 << i);
			writel(value, reg + S5PC1XX_KEYIFCOL_OFFSET);
			udelay(10*1000);
			col_value[i++] = readl(reg + S5PC1XX_KEYIFROW_OFFSET);
		}
		writel(0x00, reg + S5PC1XX_KEYIFCOL_OFFSET);

		/* expected value is  row_value[0] = 0x00  row_value[1] = 0x01 */
		/* workaround */
		if ((col_value[0] & 0x3) == 0x3 && (col_value[1] & 0x3) == 0x3)
			auto_download = 1;

		if ((col_value[0] & 0x3) == 0x3 && (col_value[1] & 0x3) != 0x3)
			display_info = 1;
		if (machine_is_p1p2()) {
			if ((col_value[0] & 0xd) == 0xd)
				auto_download = 1;
		}
	}

	if (auto_download)
		setenv("bootcmd", "usbdown");

	if (machine_is_aquila()) {
		/* 3 touchkey */
		i2c_gpio_set_bus(I2C_GPIO6);

		if (i2c_probe(addr)) {
			printf("Can't found 3 touchkey\n");
			return;
		}

#define MCS5000_TK_HW_VERSION  0x06
#define MCS5000_TK_FW_VERSION  0x0A
#define MCS5000_TK_MI_VERSION  0x0B

		reg = MCS5000_TK_MI_VERSION;
		i2c_read(addr, reg, 1, val, 1);
		dprintf("3-touchkey M/I 0x%x, ", val[0]);
		reg = MCS5000_TK_HW_VERSION;
		i2c_read(addr, reg, 1, val, 1);
		dprintf("H/W 0x%x, ", val[0]);
		reg = MCS5000_TK_FW_VERSION;
		i2c_read(addr, reg, 1, val, 1);
		dprintf("F/W 0x%x\n", val[0]);
	}
}

static void enable_battery(void)
{
	unsigned char val[2];
	unsigned char addr = 0x36;	/* max17040 fuel gauge */

	if (!board_is_limo_universal() && !board_is_limo_real())
		return;

	i2c_gpio_set_bus(I2C_GPIO3);

	if (i2c_probe(addr)) {
		printf("Can't found max17040 fuel gauge\n");
		return;
	}

	val[0] = 0x00;
	val[1] = 0x54;
	i2c_write(addr, 0xfe, 1, val, 2);
}

static void check_battery(void)
{
	unsigned char val[2];
	unsigned char addr = 0x36;	/* max17040 fuel gauge */

	if (!board_is_limo_universal() && !board_is_limo_real())
		return;

	i2c_gpio_set_bus(I2C_GPIO3);

	if (i2c_probe(addr)) {
		printf("Can't found max17040 fuel gauge\n");
		return;
	}

	i2c_read(addr, 0x04, 1, val, 1);

	dprintf("battery:\t%d%%\n", val[0]);

	battery_soc = val[0];
}

static void check_mhl(void)
{
	unsigned char val[2];
	unsigned char addr = 0x39;	/* SIL9230 */
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;

	/* MHL Power enable */
	/* HDMI_EN : GPJ2[2] XMSMDATA_2 output mode */
	gpio_direction_output(&gpio->gpio_j2, 2, 1);

	/* MHL_RST : MP0_4[7] XM0ADDR_7 output mode */
	gpio_direction_output(&gpio->gpio_mp0_4, 7, 0);

	/* 10ms required after reset */
	udelay(10000);

	/* output enable */
	gpio_set_value(&gpio->gpio_mp0_4, 7, 1);

	i2c_gpio_set_bus(I2C_GPIO5);

	/* set usb path */
	if (i2c_probe(addr)) {
		printf("Can't found MHL Chip\n");
		return;
	}

	/*
	 * System Control #1
	 * set to Normal operation
	 */
	val[0] = 0x35;
	i2c_write((0x72 >> 1), 0x08, 1, val, 1);
	i2c_read((0x72 >> 1), 0x08, 1, val, 1);

	/*
	 * MHL TX Control #1
	 * TERM_MODE [7:6]
	 * 00 = MHL termination ON
	 * 11 = MHL termination OFF
	 */
	val[0] = 0xd0;
	i2c_write((0x72 >> 1), 0xa0, 1, val, 1);
	i2c_read((0x72 >> 1), 0xa0, 1, val, 1);
}

static void into_charge_mode(void)
{
	unsigned char addr = 0xCC >> 1;	/* max8998 */;
	unsigned char val[2];
	unsigned int level;
	int i, j;

	if (i2c_probe(addr)) {
		printf("Can't found max8998\n");
		return;
	}

	printf("Charge Mode\n");

	i2c_read(addr, 0x0C, 1, val, 1);
	val[0] &= ~(0x7 << 0);
	val[0] |= 5;		/* 600mA */
	i2c_write(addr, 0x0C, 1, val, 1);

#ifdef CONFIG_S5PC1XXFB
	/* TODO: change to Image animation */
	init_font();
	set_font_xy(0, 0);
	set_font_color(FONT_WHITE);
	fb_printf("charging");

	level = battery_soc / 25;

	for (i = 0; i < 3; i++) {
		if (level == 0)
			udelay(1 * 1000 * 1000);

		for (j = 0; j < 4; j++) {
			fb_printf("..");

			if (j >= level)
				udelay(1 * 1000 * 1000);
		}

		if (level <= 4)
			udelay(1 * 1000 * 1000);

		set_font_xy(0, 0);
		set_font_color(FONT_XOR);
		fb_printf("charging........");

		set_font_xy(0, 0);
		set_font_color(FONT_WHITE);
		fb_printf("charging");
	}

	exit_font();
#endif

	run_command("sleep", 0);
}

static void check_micro_usb(void)
{
	unsigned char addr;
	unsigned char val[2];

	if (cpu_is_s5pc100())
		return;

	if (board_is_limo_real()) {
		if (hwrevision(0) || hwrevision(1))
			return;
	}

	i2c_gpio_set_bus(I2C_PMIC);

	addr = 0x25;		/* fsa9480 */
	if (i2c_probe(addr)) {
		printf("Can't found fsa9480\n");
		return;
	}

	/* Clear Interrupt */
	i2c_read(addr, 0x03, 1, val, 2);

	/* Read Device Type 1 */
	i2c_read(addr, 0x0a, 1, val, 1);

#define FSA_DEDICATED_CHARGER	(1 << 6)
#define FSA_UART		(1 << 3)
#define FSA_USB			(1 << 2)

	/*
	 * If USB, use default 475mA
	 * If Charger, use 600mA and go to charge mode
	 */
	if (val[0] & FSA_DEDICATED_CHARGER)
		into_charge_mode();

	/* If Factory Mode is Boot ON-USB, go to download mode */
	i2c_read(addr, 0x07, 1, val, 1);

#define FSA_ADC_FAC_USB		0x19
#define FSA_ADC_FAC_UART	0x1d

	if (val[0] == FSA_ADC_FAC_USB)
		setenv("bootcmd", "usbdown");
}

#define MAX8998_REG_ONOFF1	0x11
#define MAX8998_REG_ONOFF2	0x12
#define MAX8998_REG_ONOFF3	0x13
#define MAX8998_LDO10		(1 << 3)
#define MAX8998_LDO11		(1 << 2)
#define MAX8998_LDO12		(1 << 1)
#define MAX8998_LDO13		(1 << 0)
#define MAX8998_LDO14		(1 << 7)
#define MAX8998_LDO15		(1 << 6)
#define MAX8998_LDO16		(1 << 5)
#define MAX8998_LDO17		(1 << 4)

static void init_pmic(void)
{
	unsigned char addr;
	unsigned char val[2];

	if (cpu_is_s5pc100())
		return;

	i2c_gpio_set_bus(I2C_PMIC);

	addr = 0xCC >> 1;	/* max8998 */
	if (i2c_probe(addr)) {
		printf("Can't found max8998\n");
		return;
	}

	/* ONOFF1 */
	/* ONOFF2 */
	i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	/*
	 * Disable LDO10(VPLL_1.1V), LDO11(CAM_IO_2.8V),
	 * LDO12(CAM_ISP_1.2V), LDO13(CAM_A_2.8V)
	 */
	val[0] &= ~(MAX8998_LDO10 | MAX8998_LDO11 |
			MAX8998_LDO12 | MAX8998_LDO13);
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
	i2c_write(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
}

#define PDN_MASK(x)		(0x3 << ((x) << 1))

#define CON_INPUT(x)		(0x0 << ((x) << 2))
#define CON_OUTPUT(x)		(0x1 << ((x) << 2))
#define CON_IRQ(x)		(0xf << ((x) << 2))

#define DAT_SET(x)		(0x1 << (x))
#define DAT_CLEAR(x)		(0x0 << (x))

#define OUTPUT0(x)		(0x0 << ((x) << 1))
#define OUTPUT1(x)		(0x1 << ((x) << 1))
#define INPUT(x)		(0x2 << ((x) << 1))

#define PULL_DIS(x)		(0x0 << ((x) << 1))
#define PULL_DOWN(x)		(0x1 << ((x) << 1))
#define PULL_UP(x)		(0x2 << ((x) << 1))

#define PREVIOUS(x)		(0x3 << ((x) << 1))

#define R225_229_755_756_NOT_REMOVED
#undef R225_229_755_756_NOT_REMOVED

struct gpio_powermode {
	unsigned int	conpdn;
	unsigned int	pudpdn;
};

struct gpio_external {
	unsigned int	con;
	unsigned int	dat;
	unsigned int	pud;
};

static struct gpio_powermode powerdown_modes[] = {
	{	/* S5PC110_GPIO_A0_OFFSET */
		INPUT(0) | OUTPUT0(1) | INPUT(2) | OUTPUT0(3) |
		INPUT(4) | OUTPUT0(5) | INPUT(6) | OUTPUT0(7),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6) | PULL_DIS(7),
	}, {	/* S5PC110_GPIO_A1_OFFSET */
		INPUT(0) | OUTPUT0(1) | INPUT(2) | OUTPUT0(3),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3),
	}, {	/* S5PC110_GPIO_B_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		INPUT(4) | OUTPUT0(5) | OUTPUT0(6) | OUTPUT0(7),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6) | PULL_DIS(7),
	}, {	/* S5PC110_GPIO_C0_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | INPUT(3) |
		OUTPUT0(4),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4),
	}, {	/* S5PC110_GPIO_C1_OFFSET */
		/* OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		OUTPUT0(4), */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | INPUT(3) |
		OUTPUT0(4),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4),
	}, {	/* S5PC110_GPIO_D0_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3),
	}, {	/* S5PC110_GPIO_D1_OFFSET */
#ifdef R225_229_755_756_NOT_REMOVED
		/* Original */
		INPUT(0) | INPUT(1) | INPUT(2) | INPUT(3) |
		OUTPUT0(4) | OUTPUT0(5),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5),
#else
		/* For current test board only with R225-229 / R755-756 removed  */
		INPUT(0) | INPUT(1) | INPUT(2) | INPUT(3) |
		INPUT(4) | INPUT(5),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5),
#endif
	}, {	/* S5PC110_GPIO_E0_OFFSET */
		INPUT(0) | INPUT(1) | INPUT(2) | INPUT(3) |
		INPUT(4) | INPUT(5) | INPUT(6) | INPUT(7),
		PULL_DOWN(0) | PULL_DOWN(1) | PULL_DOWN(2) | PULL_DOWN(3) |
		PULL_DOWN(4) | PULL_DOWN(5) | PULL_DOWN(6) | PULL_DOWN(7),
	}, {	/* S5PC110_GPIO_E1_OFFSET */
		INPUT(0) | INPUT(1) | INPUT(2) | OUTPUT0(3) |
		OUTPUT0(4),
		PULL_DOWN(0) | PULL_DOWN(1) | PULL_DOWN(2) | PULL_DIS(3) |
		PULL_DIS(4),
	}, {	/* S5PC110_GPIO_F0_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		OUTPUT0(4) | OUTPUT0(5) | OUTPUT0(6) | OUTPUT0(7),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6) | PULL_DIS(7),
	}, {	/* S5PC110_GPIO_F1_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		OUTPUT0(4) | OUTPUT0(5) | OUTPUT0(6) | OUTPUT0(7),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6) | PULL_DIS(7),
	}, {	/* S5PC110_GPIO_F2_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		OUTPUT0(4) | OUTPUT0(5) | OUTPUT0(6) | OUTPUT0(7),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6) | PULL_DIS(7),
	}, {	/* S5PC110_GPIO_F3_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		OUTPUT0(4) | OUTPUT0(5),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5),
	}, {	/* S5PC110_GPIO_G0_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		OUTPUT0(4) | OUTPUT0(5) | OUTPUT0(6),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6),
	}, {	/* S5PC110_GPIO_G1_OFFSET */
#ifdef R225_229_755_756_NOT_REMOVED
		/* Original */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		OUTPUT0(4) | OUTPUT0(5) | OUTPUT0(6),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6),
#else
		/* For the current test board */
		OUTPUT0(0) | INPUT(1) | OUTPUT0(2) | INPUT(3) |
		INPUT(4) | INPUT(5) | INPUT(6),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6),
#endif
	}, {	/* S5PC110_GPIO_G2_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		OUTPUT0(4) | OUTPUT0(5) | OUTPUT0(6),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6),
	}, {	/* S5PC110_GPIO_G3_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT1(2) | INPUT(3) |
		OUTPUT0(4) | OUTPUT0(5) | OUTPUT0(6),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6),
	}, {	/* S5PC110_GPIO_I_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | INPUT(3) |
		OUTPUT0(4) | OUTPUT0(5) | OUTPUT0(6),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6),
	}, {	/* S5PC110_GPIO_J0_OFFSET */
		OUTPUT1(0) | OUTPUT0(1) | INPUT(2) | INPUT(3) |
		INPUT(4) | INPUT(5) | OUTPUT0(6) | OUTPUT0(7),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6) | PULL_DIS(7),
	}, {	/* S5PC110_GPIO_J1_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		OUTPUT0(4) | OUTPUT0(5) | OUTPUT0(6) | OUTPUT0(7),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6) | PULL_DIS(7),
	}, {	/* S5PC110_GPIO_J2_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		INPUT(4) | OUTPUT0(5) | OUTPUT0(6) | INPUT(7),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6) | PULL_DOWN(7),
	}, {	/* S5PC110_GPIO_J3_OFFSET */
		OUTPUT0(0) | OUTPUT0(1) | OUTPUT0(2) | OUTPUT0(3) |
		OUTPUT1(4) | OUTPUT0(5) | INPUT(6) | INPUT(7),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5) | PULL_DIS(6) | PULL_DIS(7),
	}, {	/* S5PC110_GPIO_J4_OFFSET */
		INPUT(0) | OUTPUT0(1) | OUTPUT0(2) | INPUT(3) |
		OUTPUT0(4),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4),
	},
};

static struct gpio_external external_powerdown_modes[] = {
	{	/* S5PC110_GPIO_H0_OFFSET */
		CON_OUTPUT(0) | CON_INPUT(1) | CON_OUTPUT(2) | CON_OUTPUT(3) |
		CON_OUTPUT(4) | CON_OUTPUT(5) | CON_INPUT(6) | CON_INPUT(7),
		DAT_SET(0) | DAT_CLEAR(2) | DAT_CLEAR(3) |
		DAT_CLEAR(4) | DAT_CLEAR(5),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DIS(4) | PULL_DIS(5),
	}, {	/* S5PC110_GPIO_H1_OFFSET */
		/* CON_INPUT(0) | CON_INPUT(1) | CON_OUTPUT(2) | CON_IRQ(3) |
		CON_IRQ(4) | CON_INPUT(5) | CON_INPUT(6) | CON_INPUT(7),
		DAT_CLEAR(2),
		PULL_DOWN(0) | PULL_DOWN(1) |
		PULL_DOWN(6),		*/
		CON_INPUT(0) | CON_INPUT(1) | CON_OUTPUT(2) | CON_IRQ(3) |
		CON_INPUT(4) | CON_INPUT(5) | CON_OUTPUT(6) | CON_INPUT(7),
		DAT_SET(0),
		PULL_DIS(0) | PULL_DIS(1) | PULL_DIS(2) | PULL_DIS(3) |
		PULL_DOWN(4),
	}, {	/* S5PC110_GPIO_H2_OFFSET */
		CON_OUTPUT(0) | CON_OUTPUT(1) | CON_OUTPUT(2) | CON_OUTPUT(3) |
		CON_IRQ(4) | CON_IRQ(5) | CON_IRQ(6) | CON_IRQ(7),
		DAT_CLEAR(0) | DAT_CLEAR(1) | DAT_CLEAR(2) | DAT_CLEAR(3),
		0,
	}, {	/* S5PC110_GPIO_H3_OFFSET */
		CON_IRQ(0) | CON_IRQ(1) | CON_IRQ(2) | CON_OUTPUT(3) |
		CON_IRQ(4) | CON_INPUT(5) | CON_IRQ(6) | CON_OUTPUT(7),
		DAT_CLEAR(3) | DAT_CLEAR(7),
		0,
	},
};

static void setup_power_down_mode_registers(void)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;
	struct s5pc1xx_gpio_bank *bank;
	struct gpio_powermode *p;
	struct gpio_external *ge;
	int i;

	if (cpu_is_s5pc100())
		return;

	if (!(machine_is_aquila() && board_is_limo_real()))
		return;

	bank = &gpio->gpio_a0;
	p = powerdown_modes;

	for (i = 0; i < ARRAY_SIZE(powerdown_modes); i++, p++, bank++) {
		writel(p->conpdn, &bank->pdn_con);
		writel(p->pudpdn, &bank->pdn_pull);
	}
	bank = &gpio->gpio_i;
	writel(0x0008, &bank->dat);
	bank = &gpio->gpio_mp0_1;
	writel(0x5100, &bank->pdn_con);
	writel(0x0000, &bank->pdn_pull);
	bank = &gpio->gpio_mp0_2;
	writel(0x0020, &bank->pdn_con);
	writel(0x0000, &bank->pdn_pull);
	bank = &gpio->gpio_mp0_3;
	writel(0x0210, &bank->pdn_con);
	writel(0x0000, &bank->pdn_pull);
	bank = &gpio->gpio_mp0_4;
	writel(0x2280, &bank->pdn_con);
	writel(0x1140, &bank->pdn_pull);
	bank = &gpio->gpio_mp0_5;
	writel(0x00a2, &bank->pdn_con);
	writel(0x0001, &bank->pdn_pull);
	bank = &gpio->gpio_mp0_6;
	writel(0x0000, &bank->pdn_con);
	writel(0x0000, &bank->pdn_pull);
	bank = &gpio->gpio_mp0_7;
	writel(0x0000, &bank->pdn_con);
	writel(0x0000, &bank->pdn_pull);

	/* M299 */
	writel(0xff0022b0, (unsigned int *)0xF0000000);
	writel(0xff0022b0, (unsigned int *)0xF1400000);


	bank = &gpio->gpio_h0;
	ge = external_powerdown_modes;

	for (i = 0; i < ARRAY_SIZE(external_powerdown_modes); i++) {
		writel(ge->con, &bank->con);
		writel(ge->dat, &bank->dat);
		writel(ge->pud, &bank->pull);

		bank++;
		ge++;
	}
}

int misc_init_r(void)
{
#ifdef CONFIG_LCD
	/* It should be located at first */
	lcd_is_enabled = 0;

	if (board_is_limo_real() ||
		board_is_limo_universal() ||
		board_is_j1b2())
		setenv("lcd", "lcd=s6e63m0");
	/* it can't classify tl2796 with single-lcd and dual-lcd.
	else
		setenv("lcd", "lcd=tl2796-dual");
	*/
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

	/* Setup Limo Real board GPIOs */
	setup_limo_real_gpios();

	/* Setup P1P2 board GPIOS */
	setup_p1p2_gpios();

	/* To usbdown automatically */
	check_keypad();

	/* check max8998 */
	init_pmic();

	/* check max17040 */
	check_battery();

#ifdef CONFIG_S5PC1XXFB
	display_device_info();
#endif

	setup_power_down_mode_registers();

	/* check fsa9480 */
	check_micro_usb();

	return 0;
}
#endif

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	/* Check H/W Revision */
	check_hw_revision();

	return 0;
}

int dram_init(void)
{
	char meminfo[64] = {0,};
	unsigned int base, memconfig0, size;

	if (cpu_is_s5pc100()) {
		/* In mem setup, we swap the bank. So below size is correct */
		gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
		gd->bd->bi_dram[0].size = PHYS_SDRAM_2_SIZE;
		gd->bd->bi_dram[1].start = S5PC100_PHYS_SDRAM_2;
		size = 128;
	} else {
		/* In S5PC110, we can't swap the DMC0/1 */
		gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
		gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

		base = S5PC110_DMC1_BASE;
		/* DMC configuration */
		memconfig0 = readl(base + MEMCONFIG0_OFFSET);
		gd->bd->bi_dram[1].start = memconfig0 & 0xFF000000;

		size = (memconfig0 >> 16) & 0xFF;
		size = ((unsigned char) ~size) + 1;

		/*
		 * (0x07 + 1) * 16 = 128 MiB
		 * (0x0f + 1) * 16 = 256 MiB
		 */
		size = size << 4;

	}
	gd->bd->bi_dram[1].size = size << 20;

	sprintf(meminfo, "mem=%dM mem=%dM@0x%x",
			(int)gd->bd->bi_dram[0].size >> 20,
			size, (unsigned int)gd->bd->bi_dram[1].start);
	setenv("meminfo", meminfo);

	return 0;
}

/* Used for sleep test */
static unsigned char saved_val[4][2];
static unsigned int gpio_CP;
static unsigned int gpio_T_FLASH;
void board_sleep_init(void)
{
	unsigned int value;
	unsigned char addr;
	unsigned char val[2];
	struct s5pc110_gpio *gpio =
		(struct s5pc110_gpio *)S5PC110_GPIO_BASE;

	/* Set wakeup mask register */
	value = 0xFFFF;
	value &= ~(1 << 4);     /* Keypad */
	value &= ~(1 << 3);     /* RTC */
	value &= ~(1 << 2);     /* RTC Alarm */
	writel(value, S5PC110_WAKEUP_MASK);

	/* Set external wakeup mask register */
	value = 0xFFFFFFFF;
	value &= ~(1 << 18);    /* T-Flash */
	writel(value, S5PC110_EINT_WAKEUP_MASK);

	i2c_gpio_set_bus(I2C_PMIC);
	addr = 0xCC >> 1;
	if (i2c_probe(addr)) {
		printf("Can't find max8998\n");
		return;
	}

	/* CP off */
	gpio_CP = gpio_get_value(&gpio->gpio_h3, 7);
	gpio_set_value(&gpio->gpio_h3, 7, 0);
	value = gpio_get_value(&gpio->gpio_h3, 7);
	/* MMC T_FLASH off */
	gpio_T_FLASH = gpio_get_value(&gpio->gpio_mp0_5, 4);
	gpio_set_value(&gpio->gpio_mp0_5, 4, 0);
	value = gpio_get_value(&gpio->gpio_mp0_5, 4);
	/* MHL off */
#define HDMI_EN1	&gpio->gpio_j2, 2
#define MHL_RST		&gpio->gpio_mp0_4, 7
	gpio_set_value(HDMI_EN1, 0);
	gpio_set_value(MHL_RST, 0);
	gpio_set_value(&gpio->gpio_j2, 3, 0); /* MHL_ON for REV02 or higher */

	/* Set ONOFF1 */
	i2c_read(addr, MAX8998_REG_ONOFF1, 1, val, 1);
	saved_val[0][0] = val[0];
	saved_val[0][1] = val[1];
	val[0] &= ~((1 << 7) | (1 << 6) | (1 << 4) | (1 << 2) |
			(1 << 1) | (1 << 0));
	i2c_write(addr, MAX8998_REG_ONOFF1, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF1, 1, val, 1);
	/* Set ONOFF2 */
	i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	saved_val[1][0] = val[0];
	saved_val[1][1] = val[1];
	val[0] &= ~((1 << 7) | (1 << 6) | (1 << 5) | (1 << 3) |
			(1 << 2) | (1 << 1) | (1 << 0));
	i2c_write(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	/* Set ONOFF3 */
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	saved_val[2][0] = val[0];
	saved_val[2][1] = val[1];
	val[0] &= ~((1 << 7) | (1 << 6) | (1 << 5) | (1 << 4));
	val[0] = 0x0;
	i2c_write(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	/* Set ONOFF4 */
	i2c_read(addr, MAX8998_REG_ONOFF3+1, 1, val, 1);
	saved_val[3][0] = val[0];
	saved_val[3][1] = val[1];
	val[0] &= ~((1 << 7) | (1 << 6) | (1 << 5) | (1 << 4));
	i2c_write(addr, MAX8998_REG_ONOFF3+1, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF3+1, 1, val, 1);
	printf("Turned off regulators. Preparing to sleep. [%s:%d]\n",
			__FILE__, __LINE__);
}
void board_sleep_resume(void)
{
	unsigned int value;
	unsigned char addr;
	unsigned char val[2];
	struct s5pc110_gpio *gpio =
		(struct s5pc110_gpio *)S5PC110_GPIO_BASE;

	i2c_gpio_set_bus(I2C_PMIC);
	addr = 0xCC >> 1;
	if (i2c_probe(addr)) {
		printf("Can't find max8998\n");
		return;
	}

	/* Set ONOFF1 */
	i2c_write(addr, MAX8998_REG_ONOFF1, 1, saved_val[0], 1);
	i2c_read(addr, MAX8998_REG_ONOFF1, 1, val, 1);
	/* Set ONOFF2 */
	i2c_write(addr, MAX8998_REG_ONOFF2, 1, saved_val[1], 1);
	i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	/* Set ONOFF3 */
	i2c_write(addr, MAX8998_REG_ONOFF3, 1, saved_val[2], 1);
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	/* Set ONOFF4 */
	i2c_write(addr, MAX8998_REG_ONOFF3+1, 1, saved_val[3], 1);
	i2c_read(addr, MAX8998_REG_ONOFF3+1, 1, val, 1);
	printf("Waked up.\n");

	/* CP */
	gpio_set_value(&gpio->gpio_h3, 7, gpio_CP);
	value = gpio_get_value(&gpio->gpio_h3, 7);
	/* MMC T_FLASH */
	gpio_set_value(&gpio->gpio_mp0_5, 4, gpio_T_FLASH);
	value = gpio_get_value(&gpio->gpio_mp0_5, 4);

	/* check max17040 */
	check_battery();

	/* check fsa9480 */
	check_micro_usb();
}

#ifdef CONFIG_CMD_USBDOWN
int usb_board_init(void)
{
	if (cpu_is_s5pc100()) {
#ifdef CONFIG_HARD_I2C
		uchar val[2] = {0,};

		/* PMIC */
		if (i2c_read(0x66, 0, 1, val, 2)) {
			printf("i2c_read error\n");
			return 1;
		}

		val[0] |= (1 << 3);
		val[1] |= (1 << 5);

		if (i2c_write(0x66, 0, 1, val, 2)) {
			printf("i2c_write error\n");
			return 1;
		}
		i2c_read(0x66, 0, 1, val, 2);
#endif
		return 0;
	}

	/* S5PC110 */
	if (board_is_limo_universal() || board_is_limo_real()) {
		/* check usb path */
		check_mhl();
	}

	if (machine_is_tickertape()) {
		struct s5pc110_gpio *gpio =
			(struct s5pc110_gpio *)S5PC110_GPIO_BASE;

		/* USB_SEL: XM0ADDR_0: MP04[0] output mode */
		gpio_direction_output(&gpio->gpio_mp0_4, 0, 0);
	}

	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	unsigned int reg;
	unsigned int clock;
	struct s5pc110_clock *clk = (struct s5pc110_clock *)S5PC1XX_CLOCK_BASE;
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;
	int i;

	/* MMC0 Clock source = SCLKMPLL */
	reg = readl(&clk->src4);
	reg &= ~0xf;
	reg |= 0x6;
	writel(reg, &clk->src4);

	reg = readl(&clk->div4);
	reg &= ~0xf;

	/* set div value near 50MHz */
	clock = get_pll_clk(MPLL) / 1000000;
	for (i = 0; i < 0xf; i++) {
		if ((clock / (i + 1)) <= 50) {
			reg |= i << 0;
			break;
		}
	}

	writel(reg, &clk->div4);

	/*
	 * MMC0 GPIO
	 * GPG0[0]	SD_0_CLK
	 * GPG0[1]	SD_0_CMD
	 * GPG0[2]	SD_0_CDn
	 * GPG0[3:6]	SD_0_DATA[0:3]
	 */
	for (i = 0; i < 7; i++) {
		/* GPG0[0:6] special function 2 */
		gpio_cfg_pin(&gpio->gpio_g0, i, 0x2);
		/* GPG0[0:6] pull up */
		gpio_set_pull(&gpio->gpio_g0, i, GPIO_PULL_UP);
	}

	return s5pc1xx_mmc_init(0);
}
#endif

static int pmic_status(void)
{
	unsigned char addr, val[2];
	int reg, i;

	i2c_gpio_set_bus(I2C_PMIC);
	addr = 0xCC >> 1;
	if (i2c_probe(addr)) {
		printf("Can't found max8998\n");
		return -1;
	}

	reg = 0x11;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 4; i--)
		printf("BUCK%d %s\n", 7 - i + 1, val[0] & (1 << i) ? "on" : "off");
	for (; i >= 0; i--)
		printf("LDO%d %s\n", 5 - i, val[0] & (1 << i) ? "on" : "off");
	reg = 0x12;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 0; i--)
		printf("LDO%d %s\n", 7 - i + 6, val[0] & (1 << i) ? "on" : "off");
	reg = 0x13;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 4; i--)
		printf("LDO%d %s\n", 7 - i + 14, val[0] & (1 << i) ? "on" : "off");
	return 0;
}

static int pmic_ldo_control(int buck, int ldo, int on)
{
	unsigned char addr, val[2];
	unsigned int reg, shift;

	if (ldo) {
		if (ldo < 2)
			return -1;
		if (ldo <= 5) {
			reg = 0x11;
			shift = 5 - ldo;
		} else if (ldo <= 13) {
			reg = 0x12;
			shift = 13 - ldo;
		} else if (ldo <= 17) {
			reg = 0x13;
			shift = 17 - ldo + 4;
		} else
			return -1;
	} else if (buck) {
		if (buck > 4)
			return -1;
		reg = 0x11;
		shift = 4 - buck + 4;
	} else
		return -1;

	i2c_gpio_set_bus(I2C_PMIC);
	addr = 0xCC >> 1;
	if (i2c_probe(addr)) {
		printf("Can't found max8998\n");
		return -1;
	}

	i2c_read(addr, reg, 1, val, 1);
	if (on)
		val[0] |= (1 << shift);
	else
		val[0] &= ~(1 << shift);
	i2c_write(addr, reg, 1, val, 1);
	i2c_read(addr, reg, 1, val, 1);
	printf("%s %d value 0x%x, %s\n", buck ? "buck" : "ldo", buck ? : ldo,
		val[0], val[0] & (1 << shift) ? "on" : "off");

	return 0;
}

static int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int buck = 0, ldo = 0, on = -1;

	switch (argc) {
	case 2:
		if (strncmp(argv[1], "status", 6) == 0)
			return pmic_status();
		break;
	case 4:
		if (strncmp(argv[1], "ldo", 3) == 0) {
			ldo = simple_strtoul(argv[2], NULL, 10);
			if (strncmp(argv[3], "on", 2) == 0)
				on = 1;
			else if (strncmp(argv[3], "off", 3) == 0)
				on = 0;
			else
				break;
			return pmic_ldo_control(buck, ldo, on);
		}
		if (strncmp(argv[1], "buck", 4) == 0) {
			buck = simple_strtoul(argv[2], NULL, 10);
			if (strncmp(argv[3], "on", 2) == 0)
				on = 1;
			else if (strncmp(argv[3], "off", 3) == 0)
				on = 0;
			else
				break;
			return pmic_ldo_control(buck, ldo, on);
		}

	default:
		break;
	}

	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	pmic,		CONFIG_SYS_MAXARGS,	1, do_pmic,
	"PMIC LDO & BUCK control",
	"status - Display PMIC LDO & BUCK status\n"
	"pmic ldo num on/off - Turn on/off the LDO\n"
	"pmic buck num on/off - Turn on/off the BUCK\n"
);
