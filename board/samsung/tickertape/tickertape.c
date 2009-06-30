/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
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

DECLARE_GLOBAL_DATA_PTR;

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
	printf("Board:\tTickerTape\n");
	return 0;
}
#endif

void raise(void)
{
}

#ifdef CONFIG_MISC_INIT_R
static const char *board_name[] = {
	"Unknown",
	"Universal",
	"Unknown",
	"TickerTape",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
};

int misc_init_r(void)
{
	/* check H/W revision */
	unsigned int rev  = __REG(S5P_GPIO_J0_DAT);

	/* GPJ0[4:2] */
	rev >>= 2;
	rev &= 0x7;
	printf("HW Revision:\t%x (%s)\n", rev, board_name[rev]);
	if (rev == 1)
		gd->bd->bi_arch_number = 3000;	/* Universal */
	if (rev == 3)
		gd->bd->bi_arch_number = 3001;	/* Tickertape */
}
#endif

#ifdef CONFIG_CMD_USBDOWN
#include <i2c.h>

int usb_board_init(void)
{
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
#endif
