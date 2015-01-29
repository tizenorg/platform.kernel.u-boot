/*
 * cmd_thordown.c -- USB TIZEN "THOR" Downloader gadget
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
#include <usb.h>
#include <libtizen.h>

int do_thor_down(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *usb_controller;
	char *interface;
	char *devstring;
	int ret;

	puts("TIZEN \"THOR\" Downloader\n");

	switch (argc) {
	case 1:
		usb_controller = strdup(getenv("dfu_usb_con"));
		interface = strdup(getenv("dfu_interface"));
		devstring = strdup(getenv("dfu_device"));

		if (!usb_controller || !interface || !devstring) {
			puts("DFU: default device environment is not set.\n");
			ret = CMD_RET_USAGE;
			goto bad_args;
		}
		break;
	case 4:
		usb_controller = argv[1];
		interface = argv[2];
		devstring = argv[3];
		break;
	default:
		return CMD_RET_USAGE;
	}

	ret = dfu_init_env_entities(interface, devstring);
	if (ret) {
		board_error_report(ERROR_DFU_ENV_ENTITIES, argc, argv);
		goto done;
	}

	int controller_index = simple_strtoul(usb_controller, NULL, 0);
	ret = board_usb_init(controller_index, USB_INIT_DEVICE);
	if (ret) {
		error("USB init failed: %d", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	g_dnl_register("usb_dnl_thor");

	ret = thor_init();
	if (ret) {
		error("THOR DOWNLOAD failed: %d", ret);
		if (ret == -EINTR)
			ret = CMD_RET_SUCCESS;
		else
			ret = CMD_RET_FAILURE;

		goto exit;
	}

	ret = thor_handle();
	if (ret) {
		error("THOR failed: %d", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

exit:
	g_dnl_unregister();
done:
	dfu_free_entities();

#ifdef CONFIG_TIZEN
	if (ret != CMD_RET_SUCCESS) {
#ifdef CONFIG_OF_MULTI
		if (board_is_trats2())
			draw_thor_fail_screen();
#else
		draw_thor_fail_screen();
#endif
	} else
		lcd_clear();
#endif
bad_args:
	if (argc == 1) {
		free(usb_controller);
		free(interface);
		free(devstring);
	}
	return ret;
}

U_BOOT_CMD(thordown, CONFIG_SYS_MAXARGS, 1, do_thor_down,
	   "TIZEN \"THOR\" downloader",
	   "<USB_controller> <interface> <dev>\n"
	   "  - device software upgrade via LTHOR TIZEN dowload\n"
	   "    program via <USB_controller> on device <dev>,\n"
	   "	attached to interface <interface>\n"
);
