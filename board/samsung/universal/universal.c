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
#include <asm/arch/gpio.h>
#include <asm/arch/keypad.h>

DECLARE_GLOBAL_DATA_PTR;

#define C100_MACH_START			3000
#define C110_MACH_START			3100

#define I2C_GPIO3	0
#define I2C_PMIC	1
#define I2C_GPIO5	2

static unsigned int board_rev;

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int dram_init(void)
{
	/* In mem setup, we swap the bank. So below size is correct */
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[1].start = S5PC100_PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

u32 get_board_rev(void)
{
	return board_rev;
}

enum {
	MACH_UNIVERSAL,
	MACH_TICKERTAPE,
	MACH_AQUILA,
};

#define SCREEN_SPLIT_FEATURE	0x100
#define J1_B2_BOARD_FEATURE	0x200
#define LIMO_UNIVERSAL_BOARD	0x400
#define LIMO_REAL_BOARD		0x800
#define FEATURE_MASK		0xF00

static int machine_is_aquila(void)
{
	int board;

	if (cpu_is_s5pc100())
		return 0;

	board = gd->bd->bi_arch_number - C110_MACH_START;

	return board == MACH_AQUILA;
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

#ifdef CONFIG_MISC_INIT_R
static const char *board_name[] = {
	"Universal",
	"TickerTape",
	"Aquila",
};

enum {
	MEM_4G1G1G,
	MEM_4G2G1G,
	MEM_4G3G1G,
};

static char feature_buffer[32];

static char *display_features(int board_rev)
{
	int count = 0;
	char *buf = feature_buffer;

	if (board_rev & SCREEN_SPLIT_FEATURE)
		count += sprintf(buf + count, " - ScreenSplit");
	if (board_rev & J1_B2_BOARD_FEATURE)
		count += sprintf(buf + count, " - J1 B2 board");
	/* Limo Real or Universal */
	if (board_rev & LIMO_REAL_BOARD)
		count += sprintf(buf + count, " - Limo Real");
	else if (board_rev & LIMO_UNIVERSAL_BOARD)
		count += sprintf(buf + count, " - Limo Universal");

	return buf;
}

static void check_board_revision(int board, int rev)
{
	switch (board) {
	case MACH_AQUILA:
		/* Limo Real or Universal */
		if (rev & LIMO_UNIVERSAL_BOARD)
			board_rev &= ~J1_B2_BOARD_FEATURE;
		if (rev & LIMO_REAL_BOARD) {
			board_rev &= ~(J1_B2_BOARD_FEATURE |
					LIMO_UNIVERSAL_BOARD);
		}
		break;
	case MACH_TICKERTAPE:
		board_rev &= ~FEATURE_MASK;
		break;
	default:
		break;
	}
}

static void set_board_meminfo(void)
{
	if (board_is_limo_real()) {
		setenv("meminfo", "mem=80M mem=256M@0x40000000");
		gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE + SZ_128M;
		return;
	}

	setenv("meminfo", "mem=80M mem=128M@0x40000000");
}

static void check_hw_revision(void)
{
	unsigned int board = MACH_UNIVERSAL;	/* Default is Universal */
	unsigned long pin;

	if (cpu_is_s5pc110())
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_J0_OFFSET);
	else
		pin = S5PC100_GPIO_BASE(S5PC100_GPIO_J0_OFFSET);

	pin += S5PC1XX_GPIO_DAT_OFFSET;

	/* check H/W revision */
	board_rev  = readl(pin);

	/* GPJ0[4:2] */
	board_rev >>= 2;
	board_rev &= 0x7;

	if (cpu_is_s5pc100()) {
		if (board_rev == 3) {
			/* C100 TickerTape */
			board = MACH_TICKERTAPE;
			/* Workaround: OneDRAM is broken at tickertape */
			setenv("meminfo", "mem=128M");
		}
	} else {
		/*
		 * Note Check 'Aquila' board first
		 *
		 * TT: TickerTape
		 * SS: ScreenSplit
		 * LRA: Limo Real Aquila
		 * LUA: Limo Universal Aquila
		 * OA: Old Aquila
		 *
		 * 			Universal LRA  LUA  OA   TT   SS
		 *   J1: 0xE0200264	0x10      0x00 0x00 0x00 0x00 0x00
		 *   H1: 0xE0200C24	          0x28 0xA8 0x1C
		 *   H3: 0xE0200C64	          0x02 0x07 0x0F
		 *   D1: 0xE02000C4	0x0F	  0x3F 0x3F 0x0F 0xXC 0x3F
		 *    I: 0xE0200224	                    0x02 0x00 0x08
		 * MP03: 0xE0200324	                    0x9x      0xbx 0x9x
		 * MP05: 0xE0200364	                    0x80      0x88
		 */

		/* C110 Aquila */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_J1_OFFSET);
		pin += S5PC1XX_GPIO_DAT_OFFSET;
		if ((readl(pin) & 0xf0) == 0) {
			board = MACH_AQUILA;
			board_rev |= J1_B2_BOARD_FEATURE;

			/* Check board */
			pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET);
			pin += S5PC1XX_GPIO_DAT_OFFSET;
			if ((readl(pin) & (1 << 2)) == 0)
				board_rev |= LIMO_UNIVERSAL_BOARD;

			pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H3_OFFSET);
			pin += S5PC1XX_GPIO_DAT_OFFSET;
			if ((readl(pin) & (1 << 2)) == 0)
				board_rev |= LIMO_REAL_BOARD;
#if 0
			/* C110 Aquila ScreenSplit */
			pin = S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_3_OFFSET);
			pin += S5PC1XX_GPIO_DAT_OFFSET;
			if ((readl(pin) & (1 << 5)))
				board_rev |= SCREEN_SPLIT_FEATURE;
			else {
				pin = S5PC110_GPIO_BASE(S5PC110_GPIO_I_OFFSET);
				pin += S5PC1XX_GPIO_DAT_OFFSET;
				if ((readl(pin) & (1 << 3)))
					board_rev |= SCREEN_SPLIT_FEATURE;
			}
#endif
		}

		/* C110 TickerTape */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_D1_OFFSET);
		pin += S5PC1XX_GPIO_DAT_OFFSET;
		if ((readl(pin) & 0x03) == 0)
			board = MACH_TICKERTAPE;
	}

	/* Set machine id */
	if (cpu_is_s5pc110())
		gd->bd->bi_arch_number = C110_MACH_START + board;
	else
		gd->bd->bi_arch_number = C100_MACH_START + board;

	check_board_revision(board, board_rev);
	printf("HW Revision:\t%x (%s%s)\n", board_rev, board_name[board],
		display_features(board_rev));

	/* Architecture Common settings */
	if (cpu_is_s5pc110()) {
		/* In S5PC110, we can't swap the DMC0/1 */
		gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
		gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
		gd->bd->bi_dram[1].start = S5PC110_PHYS_SDRAM_2;
		gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
		set_board_meminfo();
		setenv("mtdparts", MTDPARTS_DEFAULT_4KB);
	} else {
		setenv("meminfo", "mem=80M mem=128M@0x38000000");
		setenv("bootk", "onenand read 0x30007FC0 0x60000 0x300000; bootm 0x30007FC0");
		setenv("updatek", "onenand erase 0x60000 0x300000; onenand write 0x31008000 0x60000 0x300000");
	}
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

static void enable_ldos(void)
{
	unsigned int pin, value;

	if (cpu_is_s5pc100())
		return;

	/* TOUCH_EN: XMMC3DATA_3: GPG3[6] output mode */
	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_G3_OFFSET);

	value = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	value &= ~(0xf << 24);			/* 24 = 6 * 4 */
	value |= (1 << 24);
	writel(value, pin + S5PC1XX_GPIO_CON_OFFSET);

	/* output enable */
	value = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
	value |= (1 << 6);			/* 6 = 6 * 1 */
	writel(value, pin + S5PC1XX_GPIO_DAT_OFFSET);

#if 0
	value = readl(pin + S5PC1XX_GPIO_PULL_OFFSET);
	value &= ~(3 << 12);			/* 12 = 6 * 2 */
	value |= (0 << 12);			/* Pull-up/down disable */
	writel(value, pin + S5PC1XX_GPIO_PULL_OFFSET);
#endif

	if (board_is_limo_real()) {
		/* CODEC_LDO_EN: XVVSYNC_LDI: GPF3[4] output mode */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_F3_OFFSET);

		value = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
		value &= ~(0xf << 16);			/* 16 = 4 * 4 */
		value |= (1 << 16);
		writel(value, pin + S5PC1XX_GPIO_CON_OFFSET);

		/* output enable */
		value = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
		value |= (1 << 4);			/* 4 = 4 * 1 */
		writel(value, pin + S5PC1XX_GPIO_DAT_OFFSET);
	}
}

static void enable_t_flash(void)
{
	unsigned int pin, value;

	if (!(board_is_limo_universal() || board_is_limo_real()))
		return;

	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_5_OFFSET);

	/* T_FLASH_EN : XM0ADDR_13: MP0_5[4] output mode */
	value = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	value &= ~(0xf << 16);			/* 16 = 4 * 4 */
	value |= (1 << 16);
	writel(value, pin + S5PC1XX_GPIO_CON_OFFSET);

	/* output enable */
	value = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
	value |= (1 << 4);			/* 4 = 4 * 1 */
	writel(value, pin + S5PC1XX_GPIO_DAT_OFFSET);
}

static void adjust_pins(void)
{
	unsigned int pin, value;

	if (board_is_limo_real()) {
		/* RESET_REQ_N: XM0BEN_1: MP0_2[1] output mode */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_2_OFFSET);

		value = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
		value &= ~(0xf << 4);			/* 4 = 1 * 4 */
		value |= (1 << 4);
		writel(value, pin + S5PC1XX_GPIO_CON_OFFSET);

		/* output enable */
		value = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
		value |= (1 << 1);			/* 1 = 1 * 1 */
		writel(value, pin + S5PC1XX_GPIO_DAT_OFFSET);
	}
}

#define KBR3		(1 << 3)
#define KBR2		(1 << 2)
#define KBR1		(1 << 1)
#define KBR0		(1 << 0)

static void check_keypad(void)
{
	unsigned int reg, value;
	unsigned int col_mask, col_mode, row_mask, row_mode;
	unsigned int auto_download = 0;

	if (cpu_is_s5pc100()) {
		/* Set GPH2[2:0] to KP_COL[2:0] */
		reg = S5PC100_GPIO_BASE(S5PC100_GPIO_H2_OFFSET);
		reg += S5PC1XX_GPIO_CON_OFFSET;
		value = readl(reg);
		value &= ~(0xFFF);
		value |= (0x333);
		writel(value, reg);

		/* Set GPH3[2:0] to KP_ROW[2:0] */
		reg = S5PC100_GPIO_BASE(S5PC100_GPIO_H3_OFFSET);
		reg += S5PC1XX_GPIO_CON_OFFSET;
		value = readl(reg);
		value &= ~(0xFFF);
		value |= (0x333);
		writel(value, reg);

		reg = S5PC100_KEYPAD_BASE;
	} else {
		if (board_is_limo_real() || board_is_limo_universal()) {
			row_mask = 0x00FF;
			row_mode = 0x0033;
			col_mask = 0x0FFF;
			col_mode = 0x0333;
		} else {
			row_mask = col_mask = 0xFFFF;
			row_mode = col_mode = 0x3333;
		}

		/* Set GPH2[3:0] to KP_COL[3:0] */
		reg = S5PC110_GPIO_BASE(S5PC110_GPIO_H2_OFFSET);
		value = readl(reg + S5PC1XX_GPIO_CON_OFFSET);
		value &= ~col_mask;
		value |= col_mode;
		writel(value, reg + S5PC1XX_GPIO_CON_OFFSET);

		/* Set GPH3[3:0] to KP_ROW[3:0] */
		reg = S5PC110_GPIO_BASE(S5PC110_GPIO_H3_OFFSET);
		value = readl(reg + S5PC1XX_GPIO_CON_OFFSET);
		value &= ~row_mask;
		value |= row_mode;
		writel(value, reg + S5PC1XX_GPIO_CON_OFFSET);
		value = readl(reg + S5PC1XX_GPIO_PULL_OFFSET);
		value &= ~(0xFF);
		/* Pull-up enabled */
		value |= (0x2 << 6 | 0x2 << 4 | 0x2 << 2 | 0x2 << 0);
		writel(value, reg + S5PC1XX_GPIO_PULL_OFFSET);

		reg = S5PC110_KEYPAD_BASE;
	}

	value = 0x00;
	writel(value, reg + S5PC1XX_KEYIFCOL_OFFSET);

	value = readl(reg + S5PC1XX_KEYIFROW_OFFSET);

	/* OK or Send Button */
	if ((value & KBR0) == 0)
		auto_download = 1;

	if (auto_download)
		setenv("bootcmd", "usbdown");
}

static void check_battery(void)
{
	unsigned char val[2];
	unsigned char addr = 0x36;	/* max17040 fuel gauge */

	i2c_gpio_set_bus(I2C_GPIO3);

	if (i2c_probe(addr)) {
		printf("Can't found max17040 fuel gauge\n");
		return;
	}

	/* quick-start */
	val[0] = 0x40;
	val[1] = 0x00;
	if (i2c_write(addr, 0x06, 1, val, 2)) {
		printf("i2c_write error: %x\n", addr);
		return;
	}

	if (i2c_read(addr, 0x04, 1, val, 1)) {
		printf("i2c_read error: %x\n", addr);
		return;
	}
	printf("battery:\t%d%%\n", val[0]);

	/* TODO */
	/* If battery level is low then entering charge mode */
}

static void check_mhl(void)
{
	unsigned char val[2];
	unsigned char addr = 0x39;	/* SIL9230 */
	unsigned int pin, reg;

	/* MHL Power enable */
	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_J2_OFFSET);

	/* HDMI_EN : GPJ2[2] output mode */
	reg = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	reg &= ~(0xf << 8);			/* 8 = 2 * 4 */
	reg |= (1 << 8);
	writel(reg, pin + S5PC1XX_GPIO_CON_OFFSET);

	/* output enable */
	reg = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
	reg |= (1 << 2);			/* 2 = 2 * 1 */
	writel(reg, pin + S5PC1XX_GPIO_DAT_OFFSET);

	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET);

	/* MHL_RST : MP0_4[7] output mode */
	reg = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	reg &= ~(0xf << 28);			/* 28 = 7 * 4 */
	reg |= (1 << 28);
	writel(reg, pin + S5PC1XX_GPIO_CON_OFFSET);

	reg = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
	reg &= ~(1 << 7);			/* 7 = 7 * 1 */
	writel(reg, pin + S5PC1XX_GPIO_DAT_OFFSET);

	/* 10ms required after reset */
	udelay(10000);

	/* output enable */
	reg = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
	reg |= (1 << 7);			/* 7 = 7 * 1 */
	writel(reg, pin + S5PC1XX_GPIO_DAT_OFFSET);

	i2c_gpio_set_bus(I2C_GPIO5);

	/* set usb path */
	if (i2c_probe(addr)) {
		printf("Can't found MHL Chip\n");
		return;
	}

	/* added */
	val[0] = 0x01;
	i2c_write((0x72 >> 1), 0x05, 1, val, 1);

	val[0] = 0x3f;
	i2c_write((0x7A >> 1), 0x3d, 1, val, 1);

	val[0] = 0x01;
	i2c_write((0x92 >> 1), 0x11, 1, val, 1);

	val[0] = 0x15;
	i2c_write((0x92 >> 1), 0x12, 1, val, 1);

	/*
	 * System Control #1
	 * set to Normal operation
	 */
	val[0] = 0x35;
	i2c_write((0x72 >> 1), 0x08, 1, val, 1);

	/*
	 * MHL TX Control #1
	 * TERM_MODE [7:6]
	 * 00 = MHL termination ON
	 * 11 = MHL termination OFF
	 */
	val[0] = 0xd0;
	i2c_write((0x72 >> 1), 0xa0, 1, val, 1);

	/* added */
	val[0] = 0x03;
	i2c_write((0x92 >> 1), 0x17, 1, val, 1);

	val[0] = 0x6A;
	i2c_write((0x92 >> 1), 0x23, 1, val, 1);
	val[0] = 0xAA;
	i2c_write((0x92 >> 1), 0x24, 1, val, 1);
	val[0] = 0xCA;
	i2c_write((0x92 >> 1), 0x25, 1, val, 1);
	val[0] = 0xEA;
	i2c_write((0x92 >> 1), 0x26, 1, val, 1);

	val[0] = 0xA0;
	i2c_write((0x92 >> 1), 0x4C, 1, val, 1);
	val[0] = 0x00;
	i2c_write((0x92 >> 1), 0x4D, 1, val, 1);

	val[0] = 0x14;
	i2c_write((0x72 >> 1), 0x80, 1, val, 1);
	val[0] = 0x44;
	i2c_write((0x92 >> 1), 0x45, 1, val, 1);
	val[0] = 0xFC;
	i2c_write((0x72 >> 1), 0xA1, 1, val, 1);
	val[0] = 0xFA;
	i2c_write((0x72 >> 1), 0xA3, 1, val, 1);

	val[0] = 0x17;
	i2c_write((0x72 >> 1), 0x90, 1, val, 1);
	val[0] = 0x66;
	i2c_write((0x72 >> 1), 0x94, 1, val, 1);
	val[0] = 0x1C;
	i2c_write((0x72 >> 1), 0xA5, 1, val, 1);
	val[0] = 0x21;
	i2c_write((0x72 >> 1), 0x95, 1, val, 1);
	val[0] = 0x22;
	i2c_write((0x72 >> 1), 0x96, 1, val, 1);
	val[0] = 0x86;
	i2c_write((0x72 >> 1), 0x92, 1, val, 1);
	val[0] = 0xf6;
	i2c_write((0xC8 >> 1), 0x07, 1, val, 1);
	val[0] = 0x02;
	i2c_write((0xC8 >> 1), 0x40, 1, val, 1);

	val[0] = 0x00;
	i2c_write((0x72 >> 1), 0xC7, 1, val, 1);

	/* 100ms delay */
	udelay(100000);

	val[0] = 0x00;
	i2c_write((0x72 >> 1), 0x1A, 1, val, 1);
	val[0] = 0x00;
	i2c_write((0x72 >> 1), 0x1E, 1, val, 1);
	val[0] = 0x81;
	i2c_write((0x72 >> 1), 0xBC, 1, val, 1);

	/* 100ms delay */
	udelay(100000);

	val[0] = 0x1C;
	i2c_write((0x72 >> 1), 0x0D, 1, val, 1);
	val[0] = 0x04;
	i2c_write((0x72 >> 1), 0x05, 1, val, 1);   /* Auto reset on */
	val[0] = 0xFF;
	i2c_write((0x72 >> 1), 0x74, 1, val, 1);

	i2c_read((0x72 >>1), 0x95, 1, val, 1);
	val[0] &= ~(1 << 4);
	i2c_write((0x72 >> 1), 0x95, 1, val, 1);

	val[0] = 0x01;
	i2c_write((0x72 >> 1), 0xBC, 1, val, 1);
	val[0] = 0xA1;
	i2c_write((0x72 >> 1), 0xBD, 1, val, 1);
	val[0] = 0x40;
	i2c_write((0x72 >> 1), 0xBE, 1, val, 1);
	val[0] = 0x01;
	i2c_write((0xC8 >> 1), 0x21, 1, val, 1);

	val[0] = 0x01;
	i2c_write((0x72 >> 1), 0xBC, 1, val, 1);
	val[0] = 0xA1;
	i2c_write((0x72 >> 1), 0xBD, 1, val, 1);
	val[0] = 0x7C;
	i2c_write((0x72 >> 1), 0xBE, 1, val, 1);

	val[0] = 0x01;
	i2c_write((0x72 >> 1), 0xBC, 1, val, 1);
	val[0] = 0x79;
	i2c_write((0x72 >> 1), 0xBD, 1, val, 1);

	i2c_read((0x72 >>1), 0x95, 1, val, 1);
	val[0] &= ~(1 << 4);
	i2c_write((0x72 >> 1), 0x95, 1, val, 1);

	val[0] = 0x81;
	i2c_write((0xC8 >> 1), 0x21, 1, val, 1);
}

void check_micro_usb(void)
{
	unsigned char addr = 0x25;	/* fsa9480 */

	i2c_gpio_set_bus(I2C_PMIC);

	if (i2c_probe(addr)) {
		printf("Can't found fsa9480\n");
		return;
	}
}

int misc_init_r(void)
{
	check_hw_revision();

	/* Check auto burning */
	check_auto_burn();

	/* To power up I2C2 */
	enable_ldos();

	/* Enable T-Flash at Limo Universal */
	enable_t_flash();

	/* Adjust the pins */
	adjust_pins();

	/* To usbdown automatically */
	check_keypad();

	/* check fsa9480 */
	check_micro_usb();

	if (board_is_limo_universal() || board_is_limo_real()) {
		/* check max17040 */
		check_battery();

		/* check usb path */
		check_mhl();
	}

	return 0;
}
#endif

#ifdef BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_LCD
	lcd_is_enabled = 0;
#endif
	return 0;
}
#endif

#ifdef CONFIG_CMD_USBDOWN
#ifdef CONFIG_HARD_I2C
int usb_board_init(void)
{
	if (cpu_is_s5pc100()) {
		uchar val[2] ={0,};

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
	}
	return 0;
}
#endif
#endif
