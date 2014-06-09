/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * Power off command
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

/*
 * Power off the board by simple command
 * This requires implementation of function board_poweroff() which is declared
 * in include/common.h
 *
 * Implementation of board_poweroff() should:
 * 1.a. turn off the device
 *      or
 * 1.b. print info to user about turn off ability as it is in few boards
 * 2.   never back to caller
 */
int do_power_off(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	if (!strcmp("off", argv[1])) {
		board_poweroff();
		/* This function should not back here */
		return CMD_RET_SUCCESS;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(power, CONFIG_SYS_MAXARGS, 1, do_power_off,
	"Device power off",
	"<off>\n"
	"It turn off the power supply of the board"
);
