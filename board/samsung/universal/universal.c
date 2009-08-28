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

#ifdef CONFIG_MISC_INIT_R
enum {
	MACH_UNIVERSAL,
	MACH_TICKERTAPE,
	MACH_AQUILA,
	MACH_SCREENSPLIT,
};

static const char *board_name[] = {
	"Universal",
	"TickerTape",
	"Aquila",
	"ScreenSplit",
};

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
	switch (board_rev) {
	case 1:
		if (cpu_is_s5pc110()) {
			/*
			 * Note Check 'Aquila' board first
			 *
			 * 		Universal Aquila TickerTape ScreenSplit
			 * 0xE02000C4	0x0F	  0x0F   0xXC       0x3F
			 * 0xE0200264	0x10      0x00   0x00       0x00
			 * 0xE02002a4	0xc0      0x80   0x??       0xc0
			 * 0xE0200324	0xFF	  0x9F   0xFD       0x[9b][fd]
			 */

			/* C110 Aquila */
			pin = S5PC110_GPIO_BASE(S5PC110_GPIO_J1_OFFSET);
			pin += S5PC1XX_GPIO_DAT_OFFSET;
			if ((readl(pin) & 0xf0) == 0) {
				board = MACH_AQUILA;

#if 0
				/* C110 ScreenSplit */
				pin = S5PC110_GPIO_BASE(S5PC110_GPIO_J3_OFFSET);
				pin += S5PC1XX_GPIO_DAT_OFFSET;
				if ((readl(pin) & 0xf0) == 0xc0)
					board = MACH_SCREENSPLIT;
#endif
			}

			/* C110 TickerTape */
			pin = S5PC110_GPIO_BASE(S5PC110_GPIO_D1_OFFSET);
			pin += S5PC1XX_GPIO_DAT_OFFSET;
			if ((readl(pin) & 0x03) == 0)
				board = MACH_TICKERTAPE;
		}
		break;
	case 3:
		/* C100 TickerTape */
		board = MACH_TICKERTAPE;
		/* Workaround: OneDRAM is broken at s5pc100 tickertape */
		if (cpu_is_s5pc100() && board == MACH_TICKERTAPE)
			setenv("meminfo", "mem=128M");
		break;
	default:
		break;
	}
	/* Set machine id */
	if (cpu_is_s5pc110())
		gd->bd->bi_arch_number = 3100 + board;
	else
		gd->bd->bi_arch_number = 3000 + board;
	printf("HW Revision:\t%x (%s)\n", board_rev, board_name[board]);

	/* Architecture Common settings */
	if (cpu_is_s5pc110()) {
		/* In S5PC110, we can't swap the DMC0/1 */
		gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
		gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
		gd->bd->bi_dram[1].start = S5PC110_PHYS_SDRAM_2;
		gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
		setenv("meminfo", "mem=80M mem=128M@0x40000000");
		setenv("mtdparts", MTDPARTS_DEFAULT_4KB);
	} else {
		setenv("bootk", "onenand read 0x30007FC0 0x60000 0x300000; bootm 0x30007FC0");
		setenv("updatek", "onenand erase 0x60000 0x300000; onenand write 0x31008000 0x60000 0x300000");
	}
}

static void check_auto_burn(void)
{
	unsigned long magic_base = CONFIG_SYS_SDRAM_BASE + 0x02000000;
	if (readl(magic_base) == 0x426f6f74) {	/* ASICC: Boot */
		printf("Auto burning bootloader\n");
		setenv("bootcmd", "run updateb; reset");
	}
	if (readl(magic_base) == 0x4b65726e) {	/* ASICC: Kern */
		printf("Auto burning kernel\n");
		setenv("bootcmd", "run updatek; reset");
	}
	/* Clear the magic value */
	writel(0xa5a55a5a, magic_base);
}

static void enable_touch_ldo(void)
{
	unsigned int pin, value;

	if (cpu_is_s5pc100())
		return;

	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_G3_OFFSET);

	/* TOUCH_EN: XMMC3DATA_3: GPG3[6] output mode */
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
}

static void check_keypad(void)
{
	unsigned int reg, value;
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
		/* Set GPH2[3:0] to KP_COL[3:0] */
		reg = S5PC110_GPIO_BASE(S5PC110_GPIO_H2_OFFSET);
		reg += S5PC1XX_GPIO_CON_OFFSET;
		value = readl(reg);
		value &= ~(0xFFFF);
		value |= (0x3333);
		writel(value, reg);

		/* Set GPH3[3:0] to KP_ROW[3:0] */
		reg = S5PC110_GPIO_BASE(S5PC110_GPIO_H3_OFFSET);
		reg += S5PC1XX_GPIO_CON_OFFSET;
		value = readl(reg);
		value &= ~(0xFFFF);
		value |= (0x3333);
		writel(value, reg);

		reg = S5PC110_KEYPAD_BASE;
	}

	value = 0x00;
	writel(value, reg + S5PC1XX_KEYIFCOL_OFFSET);

	value = readl(reg + S5PC1XX_KEYIFROW_OFFSET);

	if ((value & 0x1) == 0)
		auto_download = 1;

	if (auto_download)
		setenv("bootcmd", "usbdown");
}

int misc_init_r(void)
{
	check_hw_revision();

	/* Check auto burning */
	check_auto_burn();

	/* To power up I2C2 */
	enable_touch_ldo();

	/* To usbdown automatically */
	check_keypad();

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
