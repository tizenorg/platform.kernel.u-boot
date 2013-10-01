/*
 * cmd_tizendown.c -- USB TIZEN "THOR" Downloader gadget
 *
 * Copyright (C) 2013 Lukasz Majewski <l.majewski@samsung.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <thor.h>
#include <dfu.h>
#include <g_dnl.h>

int do_thor_down(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *s = "thor";
	int ret;

	if (argc < 3)
		return CMD_RET_USAGE;

	puts("TIZEN \"THOR\" Downloader\n");

	ret = dfu_init_env_entities(argv[1], simple_strtoul(argv[2], NULL, 10));
	if (ret)
		return ret;

	board_usb_init();
	g_dnl_register(s);

	ret = thor_init();
	if (ret) {
		error("THOR DOWNLOAD failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	ret = thor_handle();
	if (ret) {
		error("THOR failed: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

exit:
	g_dnl_unregister();
	dfu_free_entities();

	return ret;
}

U_BOOT_CMD(tizendown, CONFIG_SYS_MAXARGS, 1, do_thor_down,
	   "TIZEN \"THOR\" downloader",
	   "<interface> <dev>\n"
	   "  - device software upgrade via LTHOR TIZEN dowload\n"
	   "    program at <dev> attached to interface <interface>\n"
);
