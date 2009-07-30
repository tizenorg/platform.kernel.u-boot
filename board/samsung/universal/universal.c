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
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board:\tUniversal\n");
	return 0;
}
#endif

void raise(void)
{
}

u32 get_board_rev(void)
{
	return board_rev;
}

#ifdef CONFIG_MISC_INIT_R
static const char *board_name[] = {
	"Unknown",
	"Universal",
	"Unknown",
	"TickerTape",
	"Unknown",
	"Aquila",
	"Unknown",
	"Unknown",
};

static void check_auto_burn(void)
{
	if (readl(0x22000000) == 0xa5a55a5a) {
		printf("Auto burning bootloader\n");
		setenv("bootcmd", "run updateb");
	}
}

int misc_init_r(void)
{
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
	printf("HW Revision:\t%x (%s)\n", board_rev, board_name[board_rev]);
	if (board_rev == 1) {
		if (cpu_is_s5pc110()) {
			gd->bd->bi_arch_number = 3100;	/* Universal */
			setenv("meminfo", "mem=80M,128M@0x40000000");
			setenv("mtdparts", MTDPARTS_DEFAULT_4KB);
		} else {
			gd->bd->bi_arch_number = 3000;	/* Universal */
			setenv("bootk", "onenand read 0x20007FC0 0x60000 0x300000; bootm 0x20007FC0");
		}
	}
	if (board_rev == 3) {
		gd->bd->bi_arch_number = 3001;	/* Tickertape */
		/* Workaround: OneDRAM is broken*/
		setenv("meminfo", "mem=128M");
	}

	/* Check auto burning */
	check_auto_burn();

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
