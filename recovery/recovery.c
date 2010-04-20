/*
 * (C) Copyright 2010 Samsung Electronics
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
#include <malloc.h>
#include "common.h"
#include "usbd.h"
#include "onenand.h"

typedef int (init_fnc_t)(void);

static void normal_boot(void)
{
	uchar *buf;

	buf = (uchar *)CONFIG_SYS_BOOT_ADDR;

	board_load_uboot(buf);

	((init_fnc_t *)CONFIG_SYS_BOOT_ADDR)();
}

static void recovery_boot(void)
{
	/* usb download and write image */
	do_usbd_down();

	/* reboot */
	reset_cpu(0);
}

void start_recovery_boot(void)
{
	/* armboot_start is defined in the board-specific linker script */
	mem_malloc_init(_armboot_start - CONFIG_SYS_MALLOC_LEN,
			CONFIG_SYS_MALLOC_LEN);

	onenand_init();

	board_recovery_init();

	if (board_check_condition())
		recovery_boot();
	else
		normal_boot();

	/* NOTREACHED - no way out of command loop except booting */
}

/*
 * origin at lib_arm/eabi_compat.c to support EABI
 */
int raise(int signum)
{
	return 0;
}
