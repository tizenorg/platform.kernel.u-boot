/*
 * cmd_dfu.c -- dfu command
 *
 * Copyright (C) 2015
 * Lukasz Majewski <l.majewski@majess.pl>
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *	    Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <dfu.h>
#include <console.h>
#include <g_dnl.h>
#include <usb.h>
#include <net.h>

static int do_dfu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *usb_controller;
	char *interface;
	char *devstring;
	int argv_list = 0;
	int ret, i = 0;
	bool dfu_reset = false;

	switch (argc) {
	case 2:
		argv_list = 1;
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
	case 5:
		argv_list = 4;
	case 4:
		usb_controller = argv[1];
		interface = argv[2];
		devstring = argv[3];
		break;
	default:
		return CMD_RET_USAGE;
	}
#ifdef CONFIG_DFU_TFTP
	unsigned long addr = 0;
	if (!strcmp(argv[1], "tftp")) {
		if (argc == 5)
			addr = simple_strtoul(argv[4], NULL, 0);

		return update_tftp(addr, interface, devstring);
	}
#endif
	ret = dfu_init_env_entities(interface, devstring);
	if (ret)
		goto done;

	if (argv_list && !strcmp(argv[argv_list], "list")) {
		dfu_show_entities();
		goto done;
	}

	int controller_index = simple_strtoul(usb_controller, NULL, 0);
	board_usb_init(controller_index, USB_INIT_DEVICE);
	g_dnl_clear_detach();
	g_dnl_register("usb_dnl_dfu");
	while (1) {
		if (g_dnl_detach()) {
			/*
			 * Check if USB bus reset is performed after detach,
			 * which indicates that -R switch has been passed to
			 * dfu-util. In this case reboot the device
			 */
			if (dfu_usb_get_reset()) {
				dfu_reset = true;
				goto exit;
			}

			/*
			 * This extra number of usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			if (++i == 10000)
				goto exit;
		}

		if (ctrlc())
			goto exit;

		WATCHDOG_RESET();
		usb_gadget_handle_interrupts(controller_index);
	}
exit:
	g_dnl_unregister();
	board_usb_cleanup(controller_index, USB_INIT_DEVICE);
done:
	dfu_free_entities();

	if (dfu_reset)
		run_command("reset", 0);

	g_dnl_clear_detach();
bad_args:
	if (argc == 1) {
		free(usb_controller);
		free(interface);
		free(devstring);
	}
	return ret;
}

U_BOOT_CMD(dfu, CONFIG_SYS_MAXARGS, 1, do_dfu,
	"Device Firmware Upgrade",
	"<USB_controller> <interface> <dev> [list]\n"
	"  - device firmware upgrade via <USB_controller>\n"
	"    on device <dev>, attached to interface\n"
	"    <interface>\n"
	"    [list] - list available alt settings\n"
#ifdef CONFIG_DFU_TFTP
	"dfu tftp <interface> <dev> [<addr>]\n"
	"  - device firmware upgrade via TFTP\n"
	"    on device <dev>, attached to interface\n"
	"    <interface>\n"
	"    [<addr>] - address where FIT image has been stored\n"
#endif
);
