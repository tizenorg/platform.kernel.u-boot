/*
 * Copyright (C) 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#include <common.h>
#include <malloc.h>
#include "recovery.h"
#include "usbd.h"
#include "onenand.h"

#ifdef RECOVERY_DEBUG
#define	PUTS(s)	serial_puts(DEBUG_MARK""s)
#else
#define PUTS(s)
#endif

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
	PUTS("Recovery Mode\n");

	/* usb download and write image */
	do_usbd_down();

	/* reboot */
	/* reset_cpu(0); */
}

void start_recovery_boot(void)
{
	/* armboot_start is defined in the board-specific linker script */
	mem_malloc_init(_armboot_start - CONFIG_SYS_MALLOC_LEN,
			CONFIG_SYS_MALLOC_LEN);

	board_recovery_init();

	onenand_init();

	serial_init();
	serial_puts("\n");

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
