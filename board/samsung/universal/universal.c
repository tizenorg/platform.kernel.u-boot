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
#include <asm/errno.h>
#include <fbutils.h>
#include <lcd.h>
#include <bmp_layout.h>

#include "animation_frames.h"
#include "gpio_setting.h"

DECLARE_GLOBAL_DATA_PTR;

#define C100_MACH_START			3000
#define C110_MACH_START			3100

/* FIXME Neptune workaround */
#define USE_NEPTUNE_BOARD
#undef USE_NEPTUNE_BOARD

static unsigned int board_rev;
static unsigned int battery_soc;
static struct s5pc110_gpio *s5pc110_gpio;

enum {
	I2C_2,
	I2C_GPIO3,
	I2C_PMIC,
	I2C_GPIO5,
	I2C_GPIO6,
	I2C_GPIO7,
	I2C_GPIO10,
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
 * i2c gpio7 - aries
 * SDA: MP05[1]
 * SCL: MP05[0]
 */
static struct i2c_gpio_bus_data i2c_gpio7 = {
	.sda_pin	= 1,
	.scl_pin	= 0,
};

/*
 * i2c gpio7 - cypress
 * SDA: MP05[6]
 * SCL: MP05[4]
 */
static struct i2c_gpio_bus_data i2c_cypress_gpio7 = {
	.sda_pin	= 6,
	.scl_pin	= 4,
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
	MACH_P1P2,	/* Don't remove it */
	MACH_GEMINUS,
	MACH_CYPRESS,

	MACH_WMG160 = 160,
};

#define SPLIT_SCREEN_FEATURE	0x100

/* board is MACH_AQUILA and board is like below. */
#define J1_B2_BOARD		0x0200
#define LIMO_UNIVERSAL_BOARD	0x0400
#define LIMO_REAL_BOARD		0x0800
#define MEDIA_BOARD		0x1000
#define BAMBOO_BOARD		0x2000
#define ARIES_BOARD		0x4000
#define NEPTUNE_BOARD		0x8000

#define BOARD_MASK		0xFF00

static int c110_machine_id(void)
{
	return gd->bd->bi_arch_number - C110_MACH_START;
}

static int machine_is_aquila(void)
{
	return c110_machine_id() == MACH_AQUILA;
}

static int machine_is_tickertape(void)
{
	return c110_machine_id() == MACH_TICKERTAPE;
}

static int machine_is_geminus(void)
{
	return c110_machine_id() == MACH_GEMINUS;
}

static int machine_is_cypress(void)
{
	return c110_machine_id() == MACH_CYPRESS;
}

static int board_is_limo_universal(void)
{
	return machine_is_aquila() && (board_rev & LIMO_UNIVERSAL_BOARD);
}

static int board_is_limo_real(void)
{
	return machine_is_aquila() && (board_rev & LIMO_REAL_BOARD);
}

static int board_is_media(void)
{
	return machine_is_aquila() && (board_rev & MEDIA_BOARD);
}

static int board_is_bamboo(void)
{
	return machine_is_aquila() && (board_rev & BAMBOO_BOARD);
}

static int board_is_j1b2(void)
{
	return machine_is_aquila() && (board_rev & J1_B2_BOARD);
}

static int board_is_aries(void)
{
	return machine_is_aquila() && (board_rev & ARIES_BOARD);
}

static int board_is_neptune(void)
{
	return machine_is_aquila() && (board_rev & NEPTUNE_BOARD);
}

/* DLNA Dongle */
static int machine_is_wmg160(void)
{
	return c110_machine_id() == MACH_WMG160;
}

static void enable_battery(void);

void i2c_init_board(void)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;
	int num_bus;

	if (cpu_is_s5pc100())
		return;

	num_bus = ARRAY_SIZE(i2c_gpio);

	if (machine_is_aquila()) {
		if (board_is_aries()) {
			i2c_gpio[I2C_GPIO7].bus->gpio_base =
				(unsigned int)&gpio->gpio_mp0_5;
		} else {
			i2c_gpio[I2C_GPIO6].bus->gpio_base = 0;
			i2c_gpio[I2C_GPIO7].bus->gpio_base = 0;
		}
	} else if (machine_is_cypress()) {
		i2c_gpio[I2C_GPIO7].bus = &i2c_cypress_gpio7;
		i2c_gpio[I2C_GPIO7].bus->gpio_base =
			(unsigned int)&gpio->gpio_mp0_5;
	} else {
		i2c_gpio[I2C_GPIO7].bus->gpio_base = 0;
	}

	i2c_gpio[I2C_2].bus->gpio_base = (unsigned int)&gpio->gpio_d1;
	i2c_gpio[I2C_GPIO3].bus->gpio_base = (unsigned int)&gpio->gpio_j3;
	i2c_gpio[I2C_PMIC].bus->gpio_base = (unsigned int)&gpio->gpio_j4;
	i2c_gpio[I2C_GPIO5].bus->gpio_base = (unsigned int)&gpio->gpio_mp0_5;
	i2c_gpio[I2C_GPIO6].bus->gpio_base = (unsigned int)&gpio->gpio_j3;

	i2c_gpio_init(i2c_gpio, num_bus, I2C_PMIC);

	/* Reset on max17040 early */
	if (battery_soc == 0)
		enable_battery();
}

#ifdef CONFIG_MISC_INIT_R
#define DEV_INFO_LEN		512
static char device_info[DEV_INFO_LEN];
static int display_info;

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
	"P1P2",		/* Don't remove it */
	"Geminus",
	"Cypress",
	"Neptune",
};

enum {
	MEM_4G1G1G,
	MEM_4G2G1G,
	MEM_4G3G1G,
	MEM_4G4G1G,
};

static char feature_buffer[32];

static char *display_features(int board, int board_rev)
{
	int count = 0;
	char *buf = feature_buffer;
	char *name = NULL;

	if (board == MACH_AQUILA) {
		if (board_rev & SPLIT_SCREEN_FEATURE)
			name = "SplitScreen";
		if (board_rev & J1_B2_BOARD)
			name = "J1 B2";
		/* Limo Real or Universal */
		if (board_rev & LIMO_REAL_BOARD)
			name = "Limo Real";
		else if (board_rev & LIMO_UNIVERSAL_BOARD)
			name = "Limo Universal";
		if (board_rev & MEDIA_BOARD)
			name = "Media";
		if (board_rev & BAMBOO_BOARD)
			name = "Bamboo";
		if (board_rev & ARIES_BOARD)
			name = "Aries";
		if (board_rev & NEPTUNE_BOARD)
			name = "Neptune";

		if (name)
			count += sprintf(buf + count, " - %s", name);
	}

	return buf;
}

static char *get_board_name(int board)
{
	if (board == MACH_WMG160)
		return "WMG160";
	return (char *) board_name[board];
}

static void check_board_revision(int board, int rev)
{
	switch (board) {
	case MACH_AQUILA:
		/* Limo Real or Universal */
		if (rev & LIMO_UNIVERSAL_BOARD)
			board_rev &= ~J1_B2_BOARD;
		if (rev & LIMO_REAL_BOARD)
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD);
		if (rev & MEDIA_BOARD)
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD);
		if (rev & BAMBOO_BOARD)
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD |
					LIMO_REAL_BOARD |
					MEDIA_BOARD);
		if (rev & ARIES_BOARD)
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD);
		if (rev & NEPTUNE_BOARD)
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD);
		break;
	case MACH_CYPRESS:
	case MACH_TICKERTAPE:
	case MACH_GEMINUS:
	case MACH_WMG160:
		board_rev &= ~BOARD_MASK;
		break;
	default:
		break;
	}
}

static unsigned int get_hw_revision(struct s5pc1xx_gpio_bank *bank, int hwrev3)
{
	unsigned int rev;
	int mode3 = 1;

	if (hwrev3)
		mode3 = 7;

	gpio_direction_input(bank, 2);
	gpio_direction_input(bank, 3);
	gpio_direction_input(bank, 4);
	gpio_direction_input(bank, mode3);

	gpio_set_pull(bank, 2, GPIO_PULL_NONE);		/* HWREV_MODE0 */
	gpio_set_pull(bank, 3, GPIO_PULL_NONE);		/* HWREV_MODE1 */
	gpio_set_pull(bank, 4, GPIO_PULL_NONE);		/* HWREV_MODE2 */
	gpio_set_pull(bank, mode3, GPIO_PULL_NONE);	/* HWREV_MODE3 */

	rev = gpio_get_value(bank, 2);
	rev |= (gpio_get_value(bank, 3) << 1);
	rev |= (gpio_get_value(bank, 4) << 2);
	rev |= (gpio_get_value(bank, mode3) << 3);

	return rev;
}

static void check_hw_revision(void)
{
	unsigned int board = MACH_UNIVERSAL;	/* Default is Universal */

	if (cpu_is_s5pc100()) {
		struct s5pc100_gpio *gpio =
			(struct s5pc100_gpio *)S5PC100_GPIO_BASE;

		board_rev = get_hw_revision(&gpio->gpio_j0, 0);

		/* C100 TickerTape */
		if (board_rev == 3)
			board = MACH_TICKERTAPE;
	} else {
		struct s5pc110_gpio *gpio =
			(struct s5pc110_gpio *)S5PC110_GPIO_BASE;
		int hwrev3 = 0;

		board_rev = 0;

		/*
		 * Note Check 'Aquila' board first
		 *
		 * TT: TickerTape
		 * SS: SplitScreen
		 * LRA: Limo Real Aquila
		 * LUA: Limo Universal Aquila
		 * OA: Old Aquila
		 * CYP: Cypress
		 * BB: Bamboo
		 *
		 * ADDR = 0xE0200000 + OFF
		 *
		 * 	 OFF	Universal BB   LRA  LUA  OA   TT   SS	     CYP
		 *   J1: 0x0264	0x10      0x10 0x00 0x00 0x00 0x00 0x00	    
		 *   J2: 0x0284	          0x01 0x10 0x00 
		 *   H1: 0x0C24	   W           0x28 0xA8 0x1C		     0x0F
		 *   H3: 0x0C64	               0x03 0x07 0x0F		    
		 *   D1: 0x00C4	0x0F	       0x3F 0x3F 0x0F 0xXC 0x3F
		 *    I: 0x0224	                         0x02 0x00 0x08
		 * MP03: 0x0324	                         0x9x      0xbx 0x9x
		 * MP05: 0x0364	                         0x80      0x88
		 */

		/* C110 Aquila */
		if (gpio_get_value(&gpio->gpio_j1, 4) == 0) {
			board = MACH_AQUILA;
			board_rev |= J1_B2_BOARD;

			gpio_set_pull(&gpio->gpio_j2, 6, GPIO_PULL_NONE);
			gpio_direction_input(&gpio->gpio_j2, 6);

			/* Check board */
			if (gpio_get_value(&gpio->gpio_h1, 2) == 0)
				board_rev |= LIMO_UNIVERSAL_BOARD;

			if (gpio_get_value(&gpio->gpio_h3, 2) == 0)
				board_rev |= LIMO_REAL_BOARD;

			if (gpio_get_value(&gpio->gpio_j2, 6) == 1)
				board_rev |= MEDIA_BOARD;

			/* set gpio to default value. */
			gpio_set_pull(&gpio->gpio_j2, 6, GPIO_PULL_DOWN);
			gpio_direction_output(&gpio->gpio_j2, 6, 0);
		}

		/* Workaround: C110 Aquila Rev0.6 */
		if (board_rev == 6) {
			board = MACH_AQUILA;
			board_rev |= LIMO_REAL_BOARD;
		}

		/* C110 Aquila Bamboo */
		if (gpio_get_value(&gpio->gpio_j2, 0) == 1) {
			board = MACH_AQUILA;
			board_rev |= BAMBOO_BOARD;
		}

		/* C110 TickerTape */
		if (gpio_get_value(&gpio->gpio_d1, 0) == 0 &&
				gpio_get_value(&gpio->gpio_d1, 1) == 0)
			board = MACH_TICKERTAPE;

		/* WMG160 - GPH3[0:4] = 0x00 */
		if (board == MACH_TICKERTAPE) {
			int i, wmg160 = 1;

			for (i = 0; i < 4; i++) {
				if (gpio_get_value(&gpio->gpio_h3, i) != 0) {
					wmg160 = 0;
					break;
				}
			}
			if (wmg160) {
				board = MACH_WMG160;
				hwrev3 = 1;
			}
		}

		/* C110 Geminus for rev0.0 */
		gpio_set_pull(&gpio->gpio_j1, 2, GPIO_PULL_NONE);
		gpio_direction_input(&gpio->gpio_j1, 2);
		if (gpio_get_value(&gpio->gpio_j1, 2) == 1) {
			board = MACH_GEMINUS;
			if ((board_rev & ~BOARD_MASK) == 3)
				board_rev &= ~0xff;
		}
		gpio_set_pull(&gpio->gpio_j1, 2, GPIO_PULL_DOWN);
		gpio_direction_output(&gpio->gpio_j1, 2, 0);

		/* C110 Geminus for rev0.1 ~ */
		gpio_set_pull(&gpio->gpio_j0, 6, GPIO_PULL_NONE);
		gpio_direction_input(&gpio->gpio_j0, 6);
		if (gpio_get_value(&gpio->gpio_j0, 6) == 1) {
			board = MACH_GEMINUS;
			hwrev3 = 1;
		}
		gpio_set_pull(&gpio->gpio_j0, 6, GPIO_PULL_DOWN);

		/* Aquila - Aries MP0_5[6] == 1 */
		gpio_direction_input(&gpio->gpio_mp0_5, 6);
		if (gpio_get_value(&gpio->gpio_mp0_5, 6) == 1) {
			/* Cypress: Do this for cypress */
			gpio_set_pull(&gpio->gpio_j2, 2, GPIO_PULL_NONE);
			gpio_direction_input(&gpio->gpio_j2, 2);
			if (gpio_get_value(&gpio->gpio_j2, 2) == 1) {
				board = MACH_CYPRESS;
				gpio_direction_output(&gpio->gpio_mp0_5, 6, 0);
			} else {
				board = MACH_AQUILA;
				board_rev |= ARIES_BOARD;
#ifdef USE_NEPTUNE_BOARD
				board_rev &= ~ARIES_BOARD;
				board_rev |= NEPTUNE_BOARD;
#endif
			}
			gpio_set_pull(&gpio->gpio_j2, 2, GPIO_PULL_DOWN);
			hwrev3 = 1;
		} else
			gpio_direction_output(&gpio->gpio_mp0_5, 6, 0);

		board_rev |= get_hw_revision(&gpio->gpio_j0, hwrev3);
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
		setenv("bootk", "onenand read 0x30007FC0 0x60000 0x300000; "
				"bootm 0x30007FC0");
		setenv("updatek", "onenand erase 0x60000 0x300000; "
				  "onenand write 0x31008000 0x60000 0x300000");
	}
}

static void show_hw_revision(void)
{
	int board;

	/*
	 * Workaround for Rev 0.3 + CP Ver ES 3.1
	 * it's Rev 0.4
	 */
	if (board_is_limo_real()) {
		if (hwrevision(0)) {
			/* default is Rev 0.4 */
			board_rev &= ~0xf;
			board_rev |= 0x4;
		}
	}

	if (cpu_is_s5pc110())
		board = gd->bd->bi_arch_number - C110_MACH_START;
	else
		board = gd->bd->bi_arch_number - C100_MACH_START;

	check_board_revision(board, board_rev);

	/* Set CPU Revision */
	if (machine_is_aquila()) {
		if (board_is_limo_real()) {
			if ((board_rev & 0xf) < 8)
				s5pc1xx_set_cpu_rev(0);
		}
	} else if (machine_is_geminus()) {
		if ((board_rev & 0xf) < 1)
			s5pc1xx_set_cpu_rev(0);
	} else if (machine_is_cypress()) {
		s5pc1xx_set_cpu_rev(1);
	} else {
		s5pc1xx_set_cpu_rev(0);
	}

	dprintf("HW Revision:\t%x (%s%s) %s\n",
		board_rev, get_board_name(board),
		display_features(board, board_rev),
		s5pc1xx_get_cpu_rev() ? "" : "EVT0");
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
	gpio_cfg_pin(&s5pc110_gpio->gpio_h2, 6, GPIO_IRQ);
	gpio_set_pull(&s5pc110_gpio->gpio_h2, 6, GPIO_PULL_UP);
}

static void enable_ldos(void)
{
	if (cpu_is_s5pc100())
		return;

	/* TOUCH_EN: XMMC3DATA_3: GPG3[6] output high */
	gpio_direction_output(&s5pc110_gpio->gpio_g3, 6, 1);
}

static void enable_t_flash(void)
{
	if (!(board_is_limo_universal() || board_is_limo_real()))
		return;

	/* T_FLASH_EN : XM0ADDR_13: MP0_5[4] output high */
	gpio_direction_output(&s5pc110_gpio->gpio_mp0_5, 4, 1);
}

static void setup_limo_real_gpios(void)
{
	if (!board_is_limo_real())
		return;

	/*
	 * Note: Please write GPIO alphabet order
	 */
	/* CODEC_LDO_EN: XVVSYNC_LDI: GPF3[4] output high */
	gpio_direction_output(&s5pc110_gpio->gpio_f3, 4, 1);

	if (hwrevision(0))
		/* RESET_REQ_N: XM0BEN_1: MP0_2[1] output high */
		gpio_direction_output(&s5pc110_gpio->gpio_mp0_2, 1, 1);
	else
		/* RESET_REQ_N: XM0CSn_2: MP0_1[2] output high */
		gpio_direction_output(&s5pc110_gpio->gpio_mp0_1, 2, 1);

	/* T_FLASH_DETECT: EINT28: GPH3[4] interrupt mode */
	gpio_cfg_pin(&s5pc110_gpio->gpio_h3, 4, GPIO_IRQ);
	gpio_set_pull(&s5pc110_gpio->gpio_h3, 4, GPIO_PULL_UP);
}

static void setup_media_gpios(void)
{
	if (!board_is_media())
		return;

	/*
	 * Note: Please write GPIO alphabet order
	 */
	/* RESET_REQ_N: XM0CSn_2: MP0_1[2] output high */
	gpio_direction_output(&s5pc110_gpio->gpio_mp0_1, 2, 1);

	/* T_FLASH_DETECT: EINT28: GPH3[4] interrupt mode */
	gpio_cfg_pin(&s5pc110_gpio->gpio_h3, 4, GPIO_IRQ);
	gpio_set_pull(&s5pc110_gpio->gpio_h3, 4, GPIO_PULL_UP);
}

#define KBR3		(1 << 3)
#define KBR2		(1 << 2)
#define KBR1		(1 << 1)
#define KBR0		(1 << 0)

static void check_keypad(void)
{
	unsigned int reg, value;
	unsigned int col_num, row_num;
	unsigned int col_mask;
	unsigned int col_mask_shift;
	unsigned int row_state[4];
	unsigned int i;
	unsigned int auto_download = 0;

	if (machine_is_wmg160())
		return;

	if (cpu_is_s5pc100()) {
		struct s5pc100_gpio *gpio =
			(struct s5pc100_gpio *)S5PC100_GPIO_BASE;

		row_num = 3;
		col_num = 3;

		/* Set GPH2[2:0] to KP_COL[2:0] */
		gpio_cfg_pin(&gpio->gpio_h2, 0, 0x3);
		gpio_cfg_pin(&gpio->gpio_h2, 1, 0x3);
		gpio_cfg_pin(&gpio->gpio_h2, 2, 0x3);

		/* Set GPH3[2:0] to KP_ROW[2:0] */
		gpio_cfg_pin(&gpio->gpio_h3, 0, 0x3);
		gpio_cfg_pin(&gpio->gpio_h3, 1, 0x3);
		gpio_cfg_pin(&gpio->gpio_h3, 2, 0x3);

		reg = S5PC100_KEYPAD_BASE;
		col_mask = S5PC1XX_KEYIFCOL_MASK;
		col_mask_shift = 0;
	} else {
		if (board_is_limo_real() || board_is_limo_universal()) {
			row_num = 2;
			col_num = 3;
		} else {
			row_num = 4;
			col_num = 4;
		}

		for (i = 0; i < row_num; i++) {
			/* Set GPH3[3:0] to KP_ROW[3:0] */
			gpio_cfg_pin(&s5pc110_gpio->gpio_h3, i, 0x3);
			gpio_set_pull(&s5pc110_gpio->gpio_h3, i, GPIO_PULL_UP);
		}

		for (i = 0; i < col_num; i++)
			/* Set GPH2[3:0] to KP_COL[3:0] */
			gpio_cfg_pin(&s5pc110_gpio->gpio_h2, i, 0x3);

		reg = S5PC110_KEYPAD_BASE;
		col_mask = S5PC110_KEYIFCOLEN_MASK;
		col_mask_shift = 8;
	}

	/* KEYIFCOL reg clear */
	writel(0, reg + S5PC1XX_KEYIFCOL_OFFSET);

	/* key_scan */
	for (i = 0; i < col_num; i++) {
		value = col_mask;
		value &= ~(1 << i) << col_mask_shift;

		writel(value, reg + S5PC1XX_KEYIFCOL_OFFSET);
		udelay(1000);

		value = readl(reg + S5PC1XX_KEYIFROW_OFFSET);
		row_state[i] = ~value & ((1 << row_num) - 1);
		printf("[%d col] row_state: 0x%x\n", i, row_state[i]);
	}

	/* KEYIFCOL reg clear */
	writel(0, reg + S5PC1XX_KEYIFCOL_OFFSET);

	if (machine_is_aquila()) {
		/* cam full shot & volume down */
		if ((row_state[0] & 0x1) && (row_state[1] & 0x2))
			auto_download = 1;
		/* volume down */
		else if ((row_state[1] & 0x2))
			display_info = 1;
	} else if (machine_is_geminus())
		/* volume down & home */
		if ((row_state[1] & 0x2) && (row_state[2] & 0x1))
			auto_download = 1;

	if (auto_download)
		setenv("bootcmd", "usbdown");
}

static void enable_battery(void)
{
	unsigned char val[2];
	unsigned char addr = 0x36;	/* max17040 fuel gauge */

	i2c_set_bus_num(I2C_GPIO3);

	if (machine_is_aquila()) {
		if (board_is_aries() || board_is_neptune())
			i2c_set_bus_num(I2C_GPIO7);
		else if (board_is_j1b2())
			return;
	} else if (machine_is_tickertape()) {
		return;
	} else if (machine_is_cypress()) {
		i2c_set_bus_num(I2C_GPIO7);
	} else if (machine_is_geminus()) {
		if (hwrevision(1))
			i2c_set_bus_num(I2C_GPIO7);
	}

	if (i2c_probe(addr)) {
		printf("Can't found max17040 fuel gauge\n");
		return;
	}

	val[0] = 0x54;
	val[1] = 0x00;
	i2c_write(addr, 0xfe, 1, val, 2);
}

static void check_battery(void)
{
	unsigned char val[2];
	unsigned char addr = 0x36;	/* max17040 fuel gauge */

	i2c_set_bus_num(I2C_GPIO3);

	if (machine_is_aquila()) {
		if (board_is_aries() || board_is_neptune())
			i2c_set_bus_num(I2C_GPIO7);
		else if (board_is_j1b2())
			return;
	} else if (machine_is_cypress()) {
		i2c_set_bus_num(I2C_GPIO7);
	} else if (machine_is_geminus()) {
		if (hwrevision(1))
			i2c_set_bus_num(I2C_GPIO7);
	} else
		return;

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

	/* MHL Power enable */
	/* HDMI_EN : GPJ2[2] XMSMDATA_2 output mode */
	gpio_direction_output(&s5pc110_gpio->gpio_j2, 2, 1);

	/* MHL_RST : MP0_4[7] XM0ADDR_7 output mode */
	gpio_direction_output(&s5pc110_gpio->gpio_mp0_4, 7, 0);

	/* 10ms required after reset */
	udelay(10000);

	/* output enable */
	gpio_set_value(&s5pc110_gpio->gpio_mp0_4, 7, 1);

	i2c_set_bus_num(I2C_GPIO5);

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

#define CHARGER_ANIMATION_FRAME		6
static int max8998_power_key(void)
{
	unsigned char addr, val[2];
	i2c_set_bus_num(I2C_PMIC);
	addr = 0xCC >> 1;
	if (i2c_probe(addr)) {
		printf("Can't found max8998\n");
		return 0;
	}

	/* Accessing IRQ1 register */
	i2c_read(addr, 0x00, 1, val, 1);
	if (val[0] & (1 << 6))
		return 1;

	return 0;
}

extern void lcd_display_clear(void);
extern int lcd_display_bitmap(ulong bmp_image, int x, int y);

static void into_charge_mode(void)
{
	unsigned char addr = 0xCC >> 1;	/* max8998 */;
	unsigned char val[2];
	unsigned int level;
	int i, j;
	bmp_image_t *bmp;
	unsigned long len;
	ulong bmp_addr[CHARGER_ANIMATION_FRAME];

	i2c_set_bus_num(I2C_PMIC);

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
	init_font();

	/* TODO: write the image-text for the charger */

	level = battery_soc * CHARGER_ANIMATION_FRAME / 100;
	if (level >= CHARGER_ANIMATION_FRAME)
		level = CHARGER_ANIMATION_FRAME - 1;

	for (i = 0; i < CHARGER_ANIMATION_FRAME; i++)
		bmp_addr[i] = (ulong)battery_charging_animation[i];

	lcd_display_clear();
	for (i = 0; i < 3; i++) {
		for (j = level; j < CHARGER_ANIMATION_FRAME; j++) {
			int k;

			bmp = gunzip_bmp(bmp_addr[j], &len);
			lcd_display_bitmap((ulong) bmp, 140, 202);
			free(bmp);

			for (k = 0; k < 10; k++)
				if (max8998_power_key()) {
					lcd_display_clear();
					/* FIXME don't use static function */
					/* draw_samsung_logo(lcd_base); */
					return;
				} else
					udelay(100 * 1000);
		}
	}
	exit_font();
#endif

	/* EVT0: sleep 1, EVT1: sleep */
	if (s5pc1xx_get_cpu_rev() == 0) {
		run_command("sleep 1", 0);
		return;
	}

	run_command("sleep", 0);
}

static void check_micro_usb(int intr)
{
	unsigned char addr;
	unsigned char val[2];
	static int started_charging_once = 0;
	char *path;

	if (cpu_is_s5pc100())
		return;

	if (board_is_limo_real()) {
		if (hwrevision(0) || hwrevision(1))
			return;
	}

	i2c_set_bus_num(I2C_PMIC);

	if (machine_is_aquila()) {
		if (board_is_aries() || board_is_neptune())
			i2c_set_bus_num(I2C_GPIO6);
	} else if (machine_is_cypress()) {
		i2c_set_bus_num(I2C_GPIO6);
	} else if (machine_is_geminus()) {
		if (hwrevision(1))
			i2c_set_bus_num(I2C_GPIO6);
	} else if (machine_is_wmg160())
		i2c_set_bus_num(I2C_GPIO6);

	addr = 0x25;		/* fsa9480 */
	if (i2c_probe(addr)) {
		printf("Can't found fsa9480\n");
		return;
	}

	/* Clear Interrupt */
	if (intr) {
		i2c_read(addr, 0x03, 1, val, 2);
		udelay(500 * 1000);
	}

	/* Read Device Type 1 */
	i2c_read(addr, 0x0a, 1, val, 1);

#define FSA_DEV1_CHARGER	(1 << 6)
#define FSA_DEV1_UART		(1 << 3)
#define FSA_DEV1_USB		(1 << 2)
#define FSA_DEV2_JIG_USB_OFF	(1 << 1)
#define FSA_DEV2_JIG_USB_ON	(1 << 0)

	/*
	 * If USB, use default 475mA
	 * If Charger, use 600mA and go to charge mode
	 */
	if ((val[0] & FSA_DEV1_CHARGER) && !started_charging_once) {
		started_charging_once = 1;
		into_charge_mode();
	}

	/* If Factory Mode is Boot ON-USB, go to download mode */
	i2c_read(addr, 0x07, 1, val, 1);

#define FSA_ADC_FAC_USB_OFF	0x18
#define FSA_ADC_FAC_USB_ON	0x19
#define FSA_ADC_FAC_UART	0x1d

	if (val[0] == FSA_ADC_FAC_USB_ON || val[0] == FSA_ADC_FAC_USB_OFF)
		setenv("bootcmd", "usbdown");

	path = getenv("usb");

	if (!strncmp(path, "cp", 2))
		run_command("microusb cp", 0);
}

static void micro_usb_switch(int path)
{
	unsigned char addr;
	unsigned char val[2];

	i2c_set_bus_num(I2C_PMIC);

	if (machine_is_aquila()) {
		if (board_is_aries() || board_is_neptune())
			i2c_set_bus_num(I2C_GPIO6);
	} else if (machine_is_cypress()) {
		i2c_set_bus_num(I2C_GPIO6);
	} else if (machine_is_geminus()) {
		if (hwrevision(1))
			i2c_set_bus_num(I2C_GPIO6);
	} else if (machine_is_wmg160()) {
		i2c_set_bus_num(I2C_GPIO6);
		return;
	}

	addr = 0x25;		/* fsa9480 */
	if (i2c_probe(addr)) {
		printf("Can't found fsa9480\n");
		return;
	}

	if (path)
		val[0] = 0x90;	/* VAUDIO */
	else
		val[0] = (1 << 5) | (1 << 2);	/* DHOST */

	i2c_write(addr, 0x13, 1, val, 1);	/* MANSW1 */

	i2c_read(addr, 0x2, 1, val, 1);
	val[0] &= ~(1 << 2);			/* Manual switching */
	i2c_write(addr, 0x2, 1, val, 1);
}

#define MAX8998_REG_ONOFF1	0x11
#define MAX8998_REG_ONOFF2	0x12
#define MAX8998_REG_ONOFF3	0x13
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

	if (cpu_is_s5pc100())
		return;

	i2c_set_bus_num(I2C_PMIC);

	addr = 0xCC >> 1;	/* max8998 */
	if (i2c_probe(addr)) {
		if (i2c_probe(addr)) {
			printf("Can't found max8998\n");
			return;
		}
	}

	/* ONOFF1 */
	i2c_read(addr, MAX8998_REG_ONOFF1, 1, val, 1);
	val[0] &= ~MAX8998_LDO3;
	i2c_write(addr, MAX8998_REG_ONOFF1, 1, val, 1);

	/* ONOFF2 */
	i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	/*
	 * Disable LDO10(VPLL_1.1V), LDO11(CAM_IO_2.8V),
	 * LDO12(CAM_ISP_1.2V), LDO13(CAM_A_2.8V)
	 */
	val[0] &= ~(MAX8998_LDO10 | MAX8998_LDO11 |
			MAX8998_LDO12 | MAX8998_LDO13);

	if (board_is_aries() || board_is_neptune())
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

	if (board_is_aries() || board_is_neptune())
		val[0] |= MAX8998_LDO17;	/* LDO17: VCC_3.0V_LCD */

	i2c_write(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
}

static void setup_power_down_mode_registers(void)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;
	struct s5pc1xx_gpio_bank *bank;
	struct gpio_powermode *p;
	int n_p;
	struct gpio_external *ge;
	int n_ge;
	struct s5pc1xx_gpio_item *mr;
	int n_mr;
	int i;

	if (cpu_is_s5pc100())
		return;

	/* Only Limo real and aries supports worked for sleep currnet */
	if (machine_is_aquila()) {
		if (board_is_limo_real())
			/* Support */;
		else if (board_is_aries() || board_is_neptune())
			/* Support */;
		else
			return;
	} else if (machine_is_geminus()) {
		/* Support */;
	} else
		return;

	if (machine_is_aquila()) {
		if (board_is_aries() || board_is_neptune()) {
			/* Aquila rev 0.9 */
			p = aries_powerdown_modes;
			ge = aries_external_powerdown_modes;
			mr = aries_mirror_powerdown_mode;
			n_p = ARRAY_SIZE(aries_powerdown_modes);
			n_ge = ARRAY_SIZE(aries_external_powerdown_modes);
			n_mr = ARRAY_SIZE(aries_mirror_powerdown_mode);
		} else {
			/* Aquila rev 0.8 or lower */
			p = aquila_powerdown_modes;
			ge = aquila_external_powerdown_modes;
			mr = aquila_mirror_powerdown_mode;
			n_p = ARRAY_SIZE(aquila_powerdown_modes);
			n_ge = ARRAY_SIZE(aquila_external_powerdown_modes);
			n_mr = ARRAY_SIZE(aquila_mirror_powerdown_mode);
		}
	} else if (machine_is_geminus()) {
		if (hwrevision(1)) {
			/* Same as Aquila rev 0.9 */
#if 0
			p = aries_powerdown_modes;
			ge = aries_external_powerdown_modes;
			mr = aries_mirror_powerdown_mode;
			n_p = ARRAY_SIZE(aries_powerdown_modes);
			n_ge = ARRAY_SIZE(aries_external_powerdown_modes);
			n_mr = ARRAY_SIZE(aries_mirror_powerdown_mode);
#else
			p = aquila_powerdown_modes;
			ge = aquila_external_powerdown_modes;
			mr = aquila_mirror_powerdown_mode;
			n_p = ARRAY_SIZE(aquila_powerdown_modes);
			n_ge = ARRAY_SIZE(aquila_external_powerdown_modes);
			n_mr = ARRAY_SIZE(aquila_mirror_powerdown_mode);
#endif
		} else if (hwrevision(0)) {
			/* Same as Aquila rev 0.8 or lower */
			p = aquila_powerdown_modes;
			ge = aquila_external_powerdown_modes;
			mr = aquila_mirror_powerdown_mode;
			n_p = ARRAY_SIZE(aquila_powerdown_modes);
			n_ge = ARRAY_SIZE(aquila_external_powerdown_modes);
			n_mr = ARRAY_SIZE(aquila_mirror_powerdown_mode);
		} else {
			return; /* Not supported */
		}
	}

	bank = &gpio->gpio_a0;

	for (i = 0; i < n_p; i++, p++, bank++) {
		writel(p->conpdn, &bank->pdn_con);
		writel(p->pudpdn, &bank->pdn_pull);
	}
	/* M299 */
	writel(0xff0022b0, (unsigned int *)0xF0000000);
	writel(0xff0022b0, (unsigned int *)0xF1400000);

	bank = &gpio->gpio_h0;

	for (i = 0; i < n_ge; i++) {
		writel(ge->con, &bank->con);
		writel(ge->dat, &bank->dat);
		writel(ge->pud, &bank->pull);

		bank++;
		ge++;
	}

	for (i = 0; i < n_mr; i++) {
		unsigned int reg = readl(&mr->bank->pdn_con);
		reg &= ~(1 << mr->number);
		if (readl(&mr->bank->dat) & (1 << mr->number))
			reg |= 1 << mr->number;
		writel(reg, &mr->bank->pdn_con);

		printf("[%8.8X] = %8.8X\n", (unsigned int) (&mr->bank->pdn_con), reg);

		mr++;
	}
}

#ifdef CONFIG_LCD
struct spi_platform_data {
	struct s5pc1xx_gpio_bank *cs_bank;
	struct s5pc1xx_gpio_bank *clk_bank;
	struct s5pc1xx_gpio_bank *si_bank;
	struct s5pc1xx_gpio_bank *so_bank;

	unsigned int cs_num;
	unsigned int clk_num;
	unsigned int si_num;
	unsigned int so_num;

	unsigned int board_is_media;
	unsigned int board_is_cypress;
};

extern void s6e63m0_set_platform_data(struct spi_platform_data *pd);
extern void s6d16a0x_set_platform_data(struct spi_platform_data *pd);

struct spi_platform_data spi_pd;

struct s5pc110_gpio *gpio_base = (struct s5pc110_gpio *) S5PC110_GPIO_BASE;

void lcd_cfg_gpio(void)
{
	unsigned int i, f3_end = 4;

	for (i = 0; i < 8; i++) {
		/* set GPF0,1,2[0:7] for RGB Interface and Data lines (32bit) */
		gpio_cfg_pin(&gpio_base->gpio_f0, i, GPIO_FUNC(2));
		gpio_cfg_pin(&gpio_base->gpio_f1, i, GPIO_FUNC(2));
		gpio_cfg_pin(&gpio_base->gpio_f2, i, GPIO_FUNC(2));
		/* pull-up/down disable */
		gpio_set_pull(&gpio_base->gpio_f0, i, GPIO_PULL_NONE);
		gpio_set_pull(&gpio_base->gpio_f1, i, GPIO_PULL_NONE);
		gpio_set_pull(&gpio_base->gpio_f2, i, GPIO_PULL_NONE);

		/* drive strength to max (24bit) */
		gpio_set_drv(&gpio_base->gpio_f0, i, GPIO_DRV_4x);
		gpio_set_rate(&gpio_base->gpio_f0, i, GPIO_DRV_SLOW);
		gpio_set_drv(&gpio_base->gpio_f1, i, GPIO_DRV_4x);
		gpio_set_rate(&gpio_base->gpio_f1, i, GPIO_DRV_SLOW);
		gpio_set_drv(&gpio_base->gpio_f2, i, GPIO_DRV_4x);
		gpio_set_rate(&gpio_base->gpio_f2, i, GPIO_DRV_SLOW);
	}

	/* set DISPLAY_DE_B pin for dual rgb mode. */
	if (board_is_media())
		f3_end = 5;

	for (i = 0; i < f3_end; i++) {
		/* set GPF3[0:3] for RGB Interface and Data lines (32bit) */
		gpio_cfg_pin(&gpio_base->gpio_f3, i, GPIO_PULL_UP);
		/* pull-up/down disable */
		gpio_set_pull(&gpio_base->gpio_f3, i, GPIO_PULL_NONE);
		/* drive strength to max (24bit) */
		gpio_set_drv(&gpio_base->gpio_f3, i, GPIO_DRV_4x);
		gpio_set_rate(&gpio_base->gpio_f3, i, GPIO_DRV_SLOW);
	}
	/* display output path selection (only [1:0] valid) */
	writel(0x2, 0xE0107008);

	/* gpio pad configuration for LCD reset. */
	gpio_cfg_pin(&gpio_base->gpio_mp0_5, 5, GPIO_OUTPUT);

	/* gpio pad configuration for LCD ON. */
	gpio_cfg_pin(&gpio_base->gpio_j1, 3, GPIO_OUTPUT);

	/* LCD_BACKLIGHT_EN */
	if (machine_is_geminus())
		gpio_cfg_pin(&gpio_base->gpio_mp0_5, 0, GPIO_OUTPUT);

	/*
	 * gpio pad configuration for
	 * DISPLAY_CS, DISPLAY_CLK, DISPLAY_SO, DISPLAY_SI.
	 */
	gpio_cfg_pin(&gpio_base->gpio_mp0_1, 1, GPIO_OUTPUT);
	gpio_cfg_pin(&gpio_base->gpio_mp0_4, 1, GPIO_OUTPUT);
	gpio_cfg_pin(&gpio_base->gpio_mp0_4, 2, GPIO_INPUT);
	gpio_cfg_pin(&gpio_base->gpio_mp0_4, 3, GPIO_OUTPUT);

	if (machine_is_aquila()) {
		spi_pd.cs_bank = &gpio_base->gpio_mp0_1;
		spi_pd.cs_num = 1;
		spi_pd.clk_bank = &gpio_base->gpio_mp0_4;
		spi_pd.clk_num = 1;
		spi_pd.si_bank = &gpio_base->gpio_mp0_4;
		spi_pd.si_num = 3;
		spi_pd.so_bank = &gpio_base->gpio_mp0_4;
		spi_pd.so_num = 2;

		if (board_is_neptune())
			s6d16a0x_set_platform_data(&spi_pd);
		else {
			s6e63m0_set_platform_data(&spi_pd);
			if (board_is_media())
				spi_pd.board_is_media = 1;
		}
	}

	if (machine_is_cypress()) {
#if 0		/* universal cypress */
		/* FLCD_CS */
		gpio_cfg_pin(&gpio_base->gpio_mp0_1, 0, GPIO_OUTPUT);
#endif
		/* FLCD_CS_S */
		gpio_cfg_pin(&gpio_base->gpio_mp0_5, 1, GPIO_OUTPUT);
		/* FLCD_CLK */
		gpio_cfg_pin(&gpio_base->gpio_mp0_4, 0, GPIO_OUTPUT);
		/* FLCD_SDI */
		gpio_cfg_pin(&gpio_base->gpio_mp0_4, 2, GPIO_OUTPUT);
		/* FLCD_RST_S */
		gpio_cfg_pin(&gpio_base->gpio_mp0_4, 5, GPIO_OUTPUT);
		/* FLCD_ON_S */
		gpio_cfg_pin(&gpio_base->gpio_g2, 2, GPIO_OUTPUT);
#if 0		/* universal cypress */
		pd_cs.bank = &gpio_base->gpio_mp0_1;
		pd_cs.num = 0;
#endif
		spi_pd.cs_bank = &gpio_base->gpio_mp0_5;
		spi_pd.cs_num = 1;
		spi_pd.clk_bank = &gpio_base->gpio_mp0_4;
		spi_pd.clk_num = 0;
		spi_pd.si_bank = &gpio_base->gpio_mp0_4;
		spi_pd.si_num = 2;

		spi_pd.board_is_cypress = 1;

		/* these data would be sent to s6e63m0 lcd panel driver. */
		s6e63m0_set_platform_data(&spi_pd);
	}

	return;
}

void backlight_on(unsigned int onoff)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *) S5PC110_GPIO_BASE;

	if (onoff) {
		if (machine_is_geminus())
			gpio_set_value(&gpio->gpio_mp0_5, 0, 1);
	} else {
		if (machine_is_geminus())
			gpio_set_value(&gpio->gpio_mp0_5, 0, 0);
	}
}

void reset_lcd(void)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *) S5PC110_GPIO_BASE;

	if (machine_is_aquila() || machine_is_geminus())
		gpio_set_value(&gpio->gpio_mp0_5, 5, 1);
	if (machine_is_cypress())
		gpio_set_value(&gpio->gpio_mp0_4, 5, 1);
}

void lcd_power_on(unsigned int onoff)
{
	struct s5pc110_gpio *gpio = (struct s5pc110_gpio *) S5PC110_GPIO_BASE;
	if (onoff) {
		/* TSP_LDO_ON */
		if (machine_is_aquila() || machine_is_geminus())
			gpio_set_value(&gpio->gpio_j1, 3, 1);

		if (machine_is_cypress())
			gpio_set_value(&gpio->gpio_g2, 2, 1);

		if (board_is_aries() || board_is_neptune()) {
			unsigned char addr;
			unsigned char val[2];
			unsigned char val2[2];

			i2c_set_bus_num(I2C_PMIC);
			addr = 0xCC >> 1;	/* max8998 */
			if (i2c_probe(addr)) {
				printf("Can't found max8998\n");
				return;
			}
			i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
			val[0] &= ~(MAX8998_LDO17);
			val[0] |= MAX8998_LDO17;	/* LDO17: VCC_3.0V_LCD */
			i2c_write(addr, MAX8998_REG_ONOFF3, 1, val, 1);

			i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
			val[0] |= MAX8998_LDO17;
			val2[0] = 0xE;
			i2c_write(addr, MAX8998_REG_LDO17, 1, val2, 1);
			i2c_write(addr, MAX8998_REG_ONOFF3, 1, val, 1);

			i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
			val[0] |= MAX8998_LDO7;
			val2[0] = 0x2;
			i2c_write(addr, MAX8998_REG_LDO7, 1, val2, 1);
			i2c_write(addr, MAX8998_REG_ONOFF2, 1, val, 1);
		}
	} else {
		if (machine_is_aquila() || machine_is_geminus())
			gpio_set_value(&gpio->gpio_j1, 3, 0);

		if (machine_is_cypress())
			gpio_set_value(&gpio->gpio_g2, 2, 0);

		if (board_is_aries() || board_is_neptune()) {
			unsigned char addr;
			unsigned char val[2];

			i2c_set_bus_num(I2C_PMIC);
			addr = 0xCC >> 1;	/* max8998 */
			if (i2c_probe(addr)) {
				printf("Can't found max8998\n");
				return;
			}

			i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
			val[0] &= ~(1 << 7);
			i2c_write(addr, MAX8998_REG_ONOFF2, 1, val, 1);
			i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);

			i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
			val[0] &= ~MAX8998_LDO17;
			i2c_write(addr, MAX8998_REG_ONOFF3, 1, val, 1);
			i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
		}
	}
}

extern void s6e63m0_cfg_ldo(void);
extern void s6e63m0_enable_ldo(unsigned int onoff);
extern void s6d16a0x_cfg_ldo(void);
extern void s6d16a0x_enable_ldo(unsigned int onoff);

int s5p_no_lcd_support(void)
{
	if (machine_is_wmg160())
		return 1;
	return 0;
}

void init_panel_info(vidinfo_t *vid)
{
	vid->cfg_gpio = NULL;
	vid->reset_lcd = NULL;
	vid->backlight_on = NULL;
	vid->lcd_power_on = NULL;

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

	if (board_is_media()) {
		vid->vl_col	= 960;
		vid->vl_row	= 800;
		vid->vl_width	= 960;
		vid->vl_height	= 800;

		/* enable dual lcd mode. */
		vid->dual_lcd_enabled = 1;
	}

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

	if (machine_is_aquila() || machine_is_cypress()) {
		vid->cfg_gpio = lcd_cfg_gpio;
		vid->reset_lcd = reset_lcd;
		vid->backlight_on = backlight_on;
		vid->lcd_power_on = lcd_power_on;
		vid->cfg_ldo = s6e63m0_cfg_ldo;
		vid->enable_ldo = s6e63m0_enable_ldo;

		vid->init_delay = 25000;
		vid->reset_delay = 120000;
	}

	if (board_is_neptune()) {
		vid->vl_freq	= 100;
		vid->vl_col	= 320;
		vid->vl_row	= 480;
		vid->vl_width	= 320;
		vid->vl_height	= 480;

		vid->vl_clkp	= CONFIG_SYS_HIGH;
		vid->vl_hsp	= CONFIG_SYS_HIGH;
		vid->vl_vsp	= CONFIG_SYS_HIGH;
		vid->vl_dp	= CONFIG_SYS_LOW;
		vid->vl_bpix	= 32;

		/* disable dual lcd mode. */
		vid->dual_lcd_enabled = 0;

		/* S6D16A0X LCD Panel */
		vid->vl_hspw	= 16;
		vid->vl_hbpd	= 24;
		vid->vl_hfpd	= 16;

		vid->vl_vspw	= 2;
		vid->vl_vbpd	= 2;
		vid->vl_vfpd	= 4;

		vid->cfg_gpio = lcd_cfg_gpio;
		vid->backlight_on = NULL;
		vid->lcd_power_on = lcd_power_on;
		vid->reset_lcd = reset_lcd;
		vid->cfg_ldo = s6d16a0x_cfg_ldo;
		vid->enable_ldo = s6d16a0x_enable_ldo;

		vid->init_delay = 10000;
		vid->power_on_delay = 10000;
		vid->reset_delay = 1000;

	}

	if (machine_is_geminus()) {
		vid->vl_freq	= 60;
		vid->vl_col	= 1024,
		vid->vl_row	= 600,
		vid->vl_width	= 1024,
		vid->vl_height	= 600,
		vid->vl_clkp	= CONFIG_SYS_LOW,
		vid->vl_hsp	= CONFIG_SYS_HIGH,
		vid->vl_vsp	= CONFIG_SYS_HIGH,
		vid->vl_dp	= CONFIG_SYS_LOW,
		vid->vl_bpix	= 32,

		vid->vl_hspw	= 32,
		vid->vl_hfpd	= 48,
		vid->vl_hbpd	= 80,

		vid->vl_vspw	= 1,
		vid->vl_vfpd	= 3,
		vid->vl_vbpd	= 4,

		vid->cfg_gpio = lcd_cfg_gpio;
		vid->reset_lcd = reset_lcd;
		vid->backlight_on = backlight_on;
		vid->lcd_power_on = lcd_power_on;
	}
#if 0
	vid->vl_freq	= 60;
	vid->vl_col	= 480,
	vid->vl_row	= 800,
	vid->vl_width	= 480,
	vid->vl_height	= 800,
	vid->vl_clkp	= CONFIG_SYS_HIGH,
	vid->vl_hsp	= CONFIG_SYS_LOW,
	vid->vl_vsp	= CONFIG_SYS_LOW,
	vid->vl_dp	= CONFIG_SYS_HIGH,
	vid->vl_bpix	= 32,

	/* tl2796 panel. */
	vid->vl_hpw	= 4,
	vid->vl_blw	= 8,
	vid->vl_elw	= 8,

	vid->vl_vpw	= 4,
	vid->vl_bfw	= 8,
	vid->vl_efw	= 8,
#endif
#if 0
	vid->vl_freq	= 60;
	vid->vl_col	= 1024,
	vid->vl_row	= 600,
	vid->vl_width	= 1024,
	vid->vl_height	= 600,
	vid->vl_clkp	= CONFIG_SYS_HIGH,
	vid->vl_hsp	= CONFIG_SYS_HIGH,
	vid->vl_vsp	= CONFIG_SYS_HIGH,
	vid->vl_dp	= CONFIG_SYS_LOW,
	vid->vl_bpix	= 32,

	/* AMS701KA AMOLED Panel. */
	vid->vl_hpw	= 30,
	vid->vl_blw	= 114,
	vid->vl_elw	= 48,

	vid->vl_vpw	= 2,
	vid->vl_bfw	= 6,
	vid->vl_efw	= 8,
#endif
}
#endif

static void setup_meminfo(void)
{
	char meminfo[64] = {0, };
	int count = 0, size, real;

	size = gd->bd->bi_dram[0].size >> 20;
	count += sprintf(meminfo + count, "mem=%dM", size);

	/* Each Chip Select can't exceed the 256MiB */
	size = gd->bd->bi_dram[1].size >> 20;
	real = min(size, 256);
	count += sprintf(meminfo + count, " mem=%dM@0x%x",
		real, (unsigned int)gd->bd->bi_dram[1].start);

	size -= real;
	if (size > 0) {
		count += sprintf(meminfo + count, " mem=%dM@0x%x", size,
			(unsigned int)gd->bd->bi_dram[1].start + (real << 20));
	}

	setenv("meminfo", meminfo);
}

/*
 * CSA partition Migration
 * It will be deleted
 */
static void csa_migration(void)
{
	unsigned int *ubi_id;
	int i;

	run_command("onenand read 0x40000000 0x400000 0x400000", 0);

	for (i = 0; i < 10; i++) {
		ubi_id = (void *) (0x40000000 + 0x40000 * i);
		if (*ubi_id == 0x23494255) /* 0x23494255 = UBI */ {
			printf("CSA Migration is already done....\n");
			return;
		}
	}
	run_command("onenand erase 0x400000 0x800000", 0);
}

int misc_init_r(void)
{
#ifdef CONFIG_LCD
	/* It should be located at first */
	lcd_is_enabled = 0;

	if (board_is_neptune())
		setenv("lcdinfo", "lcd=s6d16a0x");
	else if ((board_is_limo_real() ||
		board_is_limo_universal() ||
		board_is_j1b2()))
		setenv("lcdinfo", "lcd=s6e63m0");
	/* it can't classify tl2796 with single-lcd and dual-lcd.
	else
		setenv("lcdinfo", "lcd=tl2796-dual");
	*/

	/*
	 * env values below should be added in case that lcd panel of geminus.
	 * setenv means that lcd panel has been turned on at u-boot.
	 */
	if (machine_is_geminus())
		setenv("lcdinfo", "lcd=lms480jc01");
	if (board_is_media())
		setenv("lcdinfo", "lcd=media");
#endif
	setup_meminfo();

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

	/* Setup Media board GPIOs */
	setup_media_gpios();

	/* To usbdown automatically */
	check_keypad();

	/* check max8998 */
	init_pmic();

#ifdef CONFIG_S5PC1XXFB
	display_device_info();
#endif

	setup_power_down_mode_registers();

	/* check max17040 */
	check_battery();

	/* check fsa9480 */
	check_micro_usb(0);

	/* csa migration (temporary) */
	csa_migration();

	return 0;
}
#endif

int board_init(void)
{
	/* Set Initial global variables */
	s5pc110_gpio = (struct s5pc110_gpio *) S5PC110_GPIO_BASE;

	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	/* Check H/W Revision */
	check_hw_revision();

	return 0;
}

int dram_init(void)
{
	unsigned int base, memconfig0, size;
	unsigned int memconfig1, sz = 0;

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

		/*
		 * Aquila Rev0.5 4G3G1G
		 * Aquila Rev0.8 4G3G1G
		 * Aquila Rev0.9 4G3G1G
		 */
		if (machine_is_aquila() &&
		    (hwrevision(5) || hwrevision(8) || hwrevision(9))) {
			memconfig1 = readl(base + MEMCONFIG1_OFFSET);

			sz = (memconfig1 >> 16) & 0xFF;
			sz = ((unsigned char) ~sz) + 1;
			sz = sz << 4;
		}

	}
	/*
	 * bi_dram[1].size contains all DMC1 memory size
	 */
	gd->bd->bi_dram[1].size = (size + sz) << 20;

	return 0;
}

/* Used for sleep test */
static unsigned char saved_val[4][2];
void board_sleep_init_late(void)
{
	/* CODEC_LDO_EN: GPF3[4] */
	gpio_direction_output(&s5pc110_gpio->gpio_f3, 4, 0);
	/* CODEC_XTAL_EN: GPH3[2] */
	gpio_direction_output(&s5pc110_gpio->gpio_h3, 2, 0);

	/* MMC T_FLASH off */
	gpio_direction_output(&s5pc110_gpio->gpio_mp0_5, 4, 0);
	/* MHL off */
	gpio_direction_output(&s5pc110_gpio->gpio_j2, 2, 0);
	gpio_direction_output(&s5pc110_gpio->gpio_mp0_4, 7, 0);
	gpio_direction_output(&s5pc110_gpio->gpio_j2, 3, 0); /* MHL_ON for REV02 or higher */


}
void board_sleep_init(void)
{
	unsigned char addr;
	unsigned char val[2];

	i2c_set_bus_num(I2C_PMIC);
	addr = 0xCC >> 1;
	if (i2c_probe(addr)) {
		printf("Can't find max8998\n");
		return;
	}

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
	val[0] |= (1 << 7);
	i2c_write(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	/* Set ONOFF3 */
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	saved_val[2][0] = val[0];
	saved_val[2][1] = val[1];
	val[0] &= ~((1 << 7) | (1 << 6) | (1 << 5) | (1 << 4));
	i2c_write(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	/* Set ONOFF4 */
	i2c_read(addr, MAX8998_REG_ONOFF3+1, 1, val, 1);
	saved_val[3][0] = val[0];
	saved_val[3][1] = val[1];
	val[0] &= ~((1 << 7) | (1 << 6) | (1 << 4));
	i2c_write(addr, MAX8998_REG_ONOFF3+1, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF3+1, 1, val, 1);
	printf("Turned off regulators. Preparing to sleep. [%s:%d]\n",
			__FILE__, __LINE__);
}

void board_sleep_resume(void)
{
	unsigned char addr;
	unsigned char val[2];

	show_hw_revision();

	i2c_set_bus_num(I2C_PMIC);
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

	/* check max17040 */
	check_battery();

	/* check fsa9480 */
	check_micro_usb(1);
}

#ifdef CONFIG_CMD_USBDOWN
int usb_board_init(void)
{
#ifdef CONFIG_CMD_PMIC
	run_command("pmic ldo 3 on", 0);
#endif

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
	if (board_is_limo_universal() ||
		board_is_limo_real() ||
		board_is_media()) {
		/* check usb path */
		if (board_is_limo_real() && !hwrevision(6))
			check_mhl();
	}

	if (machine_is_tickertape())
		/* USB_SEL: XM0ADDR_0: MP04[0] output mode */
		gpio_direction_output(&s5pc110_gpio->gpio_mp0_4, 0, 0);

	/* USB Path to AP */
	micro_usb_switch(0);

	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int s5p_no_mmc_support(void)
{
	if (machine_is_wmg160())
		return 1;
	return 0;
}

int board_mmc_init(bd_t *bis)
{
	unsigned int reg;
	unsigned int clock;
	struct s5pc110_clock *clk = (struct s5pc110_clock *)S5PC1XX_CLOCK_BASE;
	int i;

	/* MASSMEMORY_EN: XMSMDATA7: GPJ2[7] output high */
	if (machine_is_aquila() && (board_is_aries() || board_is_neptune()))
		gpio_direction_output(&s5pc110_gpio->gpio_j2, 7, 1);

	if (machine_is_wmg160())
		return -1;

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
	 * GPG0[2]	SD_0_CDn	-> Not used
	 * GPG0[3:6]	SD_0_DATA[0:3]
	 */
	for (i = 0; i < 7; i++) {
		if (i == 2)
			continue;
		/* GPG0[0:6] special function 2 */
		gpio_cfg_pin(&s5pc110_gpio->gpio_g0, i, 0x2);
		/* GPG0[0:6] pull disable */
		gpio_set_pull(&s5pc110_gpio->gpio_g0, i, GPIO_PULL_NONE);
		/* GPG0[0:6] drv 4x */
		gpio_set_drv(&s5pc110_gpio->gpio_g0, i, GPIO_DRV_4x);
	}

	return s5pc1xx_mmc_init(0);
}
#endif

#ifdef CONFIG_CMD_PMIC
static int pmic_status(void)
{
	unsigned char addr, val[2];
	int reg, i;

	i2c_set_bus_num(I2C_PMIC);
	addr = 0xCC >> 1;
	if (i2c_probe(addr)) {
		printf("Can't found max8998\n");
		return -1;
	}

	reg = 0x11;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 4; i--)
		printf("BUCK%d %s\n", 7 - i + 1,
			val[0] & (1 << i) ? "on" : "off");
	for (; i >= 0; i--)
		printf("LDO%d %s\n", 5 - i,
			val[0] & (1 << i) ? "on" : "off");
	reg = 0x12;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 0; i--)
		printf("LDO%d %s\n", 7 - i + 6,
			val[0] & (1 << i) ? "on" : "off");
	reg = 0x13;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 4; i--)
		printf("LDO%d %s\n", 7 - i + 14,
			val[0] & (1 << i) ? "on" : "off");

	reg = 0xd;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 6; i--)
		printf("SAFEOUT%d %s\n", 7 - i + 1,
			val[0] & (1 << i) ? "on" : "off");
	return 0;
}

static int pmic_ldo_control(int buck, int ldo, int safeout, int on)
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
	} else if (safeout) {
		if (safeout > 3)
			return -1;
		reg = 0xd;
		shift = 8 - safeout;
	} else
		return -1;

	i2c_set_bus_num(I2C_PMIC);
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

	if (ldo)
		printf("ldo %d value 0x%x, %s\n", ldo, val[0],
			val[0] & (1 << shift) ? "on" : "off");
	else if (buck)
		printf("buck %d value 0x%x, %s\n", buck, val[0],
			val[0] & (1 << shift) ? "on" : "off");
	else if (safeout)
		printf("safeout %d value 0x%x, %s\n", safeout, val[0],
			val[0] & (1 << shift) ? "on" : "off");

	return 0;
}

static int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int buck = 0, ldo = 0, safeout = 0, on = -1;

	switch (argc) {
	case 2:
		if (strncmp(argv[1], "status", 6) == 0)
			return pmic_status();
		break;
	case 4:
		if (strncmp(argv[1], "ldo", 3) == 0)
			ldo = simple_strtoul(argv[2], NULL, 10);
		else if (strncmp(argv[1], "buck", 4) == 0)
			buck = simple_strtoul(argv[2], NULL, 10);
		else if (strncmp(argv[1], "safeout", 7) == 0)
			safeout = simple_strtoul(argv[2], NULL, 10);
		else
			break;

		if (strncmp(argv[3], "on", 2) == 0)
			on = 1;
		else if (strncmp(argv[3], "off", 3) == 0)
			on = 0;
		else
			break;

		return pmic_ldo_control(buck, ldo, safeout, on);

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
	"pmic safeout num on/off - Turn on/off the SAFEOUT\n"
);
#endif

#ifdef CONFIG_CMD_DEVICE_POWER

enum {
	POWER_NONE,
	POWER_TOUCH,
	POWER_3_TOUCHKEY,
	POWER_LCD,
	POWER_HAPTIC,
	POWER_AUDIO_CODEC,
	POWER_FM_RADIO,
	POWER_BT_WIFI,
	POWER_HDMI,
};

static void power_display_devices(void)
{
	printf("devices:\n");
	printf("\t%d - touch\n", POWER_TOUCH);
	printf("\t%d - 3 touchkey\n", POWER_3_TOUCHKEY);
	printf("\t%d - LCD\n", POWER_LCD);
	printf("\t%d - Haptic\n", POWER_HAPTIC);
	printf("\t%d - Audio Codec\n", POWER_AUDIO_CODEC);
	printf("\t%d - FM Radio\n", POWER_FM_RADIO);
	printf("\t%d - BT/WiFi\n", POWER_BT_WIFI);
	printf("\t%d - HDMI\n", POWER_HDMI);
}

static int power_hdmi(int on)
{
	/* HDMI_EN1: GPJ2[2] */
	gpio_direction_output(&s5pc110_gpio->gpio_j2, 2, on);
	/* MHL_ON: GPJ2[3] */
	gpio_direction_output(&s5pc110_gpio->gpio_j2, 3, on);
	return 0;
}

static int power_bt_wifi(int on)
{
	/* WLAN_BT_EN: GPB[5] */
	gpio_direction_output(&s5pc110_gpio->gpio_b, 5, on);
	return 0;
}

static int power_fm_radio(int on)
{
	/* FM_BUS_nRST: GPJ2[5] */
	gpio_direction_output(&s5pc110_gpio->gpio_j2, 5, !on);
	return 0;
}

static int power_audio_codec(int on)
{
	/* CODEC_LDO_EN: GPF3[4] */
	gpio_direction_output(&s5pc110_gpio->gpio_f3, 4, on);
	/* CODEC_XTAL_EN: GPH3[2] */
	gpio_direction_output(&s5pc110_gpio->gpio_h3, 2, on);
	return 0;
}

static int power_haptic(int on)
{
	/* HAPTIC_ON: GPJ1[1] */
	gpio_direction_output(&s5pc110_gpio->gpio_j1, 1, on);
	return 0;
}

static int power_lcd(int on)
{
	/* MLCD_ON: GPJ1[3] */
	gpio_direction_output(&s5pc110_gpio->gpio_j1, 3, on);
	return 0;
}

static int power_touch(int on)
{
	/* TOUCH_EN: GPG3[6] */
	gpio_direction_output(&s5pc110_gpio->gpio_g3, 6, on);
	return 0;
}

static int power_3_touchkey(int on)
{
	if (on) {
		/* 3_TOUCH_EN - GPJ3[0] : (J1B2) */
		/* 3_TOUCH_EN - GPJ3[5] : (not J1B2) */
		if (board_rev & J1_B2_BOARD)
			gpio_direction_output(&s5pc110_gpio->gpio_j3, 0, on);
		else
			gpio_direction_output(&s5pc110_gpio->gpio_j3, 5, on);

		/* 3_TOUCH_CE - GPJ2[6] */
		gpio_direction_output(&s5pc110_gpio->gpio_j2, 6, on);	/* TOUCH_CE */
	} else {
		/* 3_TOUCH_CE - GPJ2[6] */
		gpio_direction_output(&s5pc110_gpio->gpio_j2, 6, on);	/* TOUCH_CE */
	}

	if (on) {
		unsigned int reg;
		unsigned char val[2];
		unsigned char addr = 0x20;		/* mcs5000 3-touchkey */

		/* Require 100ms */
		udelay(80 * 1000);

		/* 3 touchkey */
		i2c_set_bus_num(I2C_GPIO10);

		/* Workaround to probe */
		if (i2c_probe(addr)) {
			if (i2c_probe(addr)) {
				printf("Can't found 3 touchkey\n");
				return -ENODEV;
			}
		}

#define MCS5000_TK_HW_VERSION  0x06
#define MCS5000_TK_FW_VERSION  0x0A
#define MCS5000_TK_MI_VERSION  0x0B

		reg = MCS5000_TK_MI_VERSION;
		i2c_read(addr, reg, 1, val, 1);
		printf("3-touchkey:\tM/I 0x%x, ", val[0]);
		reg = MCS5000_TK_HW_VERSION;
		i2c_read(addr, reg, 1, val, 1);
		printf("H/W 0x%x, ", val[0]);
		reg = MCS5000_TK_FW_VERSION;
		i2c_read(addr, reg, 1, val, 1);
		printf("F/W 0x%x\n", val[0]);
	}
	return 0;
}

static int power_control(int device, int on)
{
	switch (device) {
	case POWER_TOUCH:
		return power_touch(on);
	case POWER_3_TOUCHKEY:
		return power_3_touchkey(on);
	case POWER_LCD:
		return power_lcd(on);
	case POWER_HAPTIC:
		return power_haptic(on);
	case POWER_AUDIO_CODEC:
		return power_audio_codec(on);
	case POWER_FM_RADIO:
		return power_fm_radio(on);
	case POWER_BT_WIFI:
		return power_bt_wifi(on);
	case POWER_HDMI:
		return power_hdmi(on);
	default:
		printf("I don't know device %d\n", device);
		break;
	}
	return 0;
}

static int power_on(int on)
{
	power_touch(on);
	power_3_touchkey(on);
	power_lcd(on);
	power_haptic(on);
	power_audio_codec(on);
	power_fm_radio(on);
	power_bt_wifi(on);
	power_hdmi(on);

	return 0;
}

static int do_power(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int device, on;

	if (!machine_is_aquila())
		goto out;

	switch (argc) {
	case 2:
		if (strncmp(argv[1], "on", 2) == 0)
			return power_on(1);
		if (strncmp(argv[1], "off", 3) == 0)
			return power_on(0);
		break;
	case 3:
		device = simple_strtoul(argv[1], NULL, 10);
		if (device < 0)
			break;

		if (strncmp(argv[2], "on", 2) == 0)
			on = 1;
		else if (strncmp(argv[2], "off", 3) == 0)
			on = 0;
		else
			break;
		return power_control(device, on);
	default:
		break;
	}
out:
	cmd_usage(cmdtp);
	power_display_devices();
	return 1;
}

U_BOOT_CMD(
	power,		CONFIG_SYS_MAXARGS,	1, do_power,
	"Device Power Management control",
	"device on/off - Turn on/off the device\n"
);

static int do_microusb(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	switch (argc) {
	case 2:
		if (strncmp(argv[1], "cp", 2) == 0) {
			micro_usb_switch(1);
			pmic_ldo_control(0, 0, 2, 1);
			setenv("usb", "cp");
		} else if (strncmp(argv[1], "ap", 2) == 0) {
			micro_usb_switch(0);
			pmic_ldo_control(0, 0, 2, 0);
			setenv("usb", "ap");
		}
		break;
	default:
		cmd_usage(cmdtp);
		return 1;
	}

	saveenv();

	printf("USB Path is set to %s\n", getenv("usb"));

	return 0;
}

U_BOOT_CMD(
	microusb,		CONFIG_SYS_MAXARGS,	1, do_microusb,
	"Micro USB Switch",
	"cp - switch to CP\n"
	"microusb ap - switch to AP\n"
);
#endif
