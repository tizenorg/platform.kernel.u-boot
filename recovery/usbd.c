/*
 * USB Downloader for SAMSUNG Platform
 *
 * Copyright (C) 2007-2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 */

#include <common.h>
#include "usbd.h"
#include "onenand.h"

#define OPS_READ	0
#define OPS_WRITE	1

static struct usbd_ops usbd_ops;
static unsigned long down_ram_addr;

int update_boot_image(void)
{
	struct onenand_op *onenand_ops = onenand_get_interface();
	ulong len, offset;

#if 1
	/* case: IPL, Recovery, u-boot are one file */
	offset = 0;
	len = CONFIG_RECOVERY_SIZE + CONFIG_RECOVERY_ADDR;
#else
	/* case: IPL, Recover are one file and u-boot is another */
	offset = CONFIG_RECOVERY_ADDR;
	len = CONFIG_RECOVERY_SIZE;
#endif
	/* Erase */
	onenand_ops->erase(offset, len, 0);
	/* Write */
	onenand_ops->write(offset, len, NULL, (u_char *)down_ram_addr);

	return 0;
}

/* Parsing received data packet and Process data */
static int process_data(struct usbd_ops *usbd)
{
	ulong cmd = 0, arg = 0, len = 0, flag = 0;
	int recvlen = 0;
	int ret = 0;
	int img_type;

	/* Parse command */
	cmd  = *((ulong *) usbd->rx_data + 0);
	arg  = *((ulong *) usbd->rx_data + 1);
	len  = *((ulong *) usbd->rx_data + 2);
	flag = *((ulong *) usbd->rx_data + 3);

	/* Reset tx buffer */
	*((ulong *) usbd->tx_data) = 0;

	switch (cmd) {
	case COMMAND_DOWNLOAD_IMAGE:
		usbd->recv_setup((char *)down_ram_addr, (int)len);

		/* response */
		usbd->send_data(usbd->tx_data, usbd->tx_len);

		/* Receive image */
		recvlen = usbd->recv_data();

		/* Retry this commad */
		if (recvlen < len)
			*((ulong *) usbd->tx_data) = STATUS_RETRY;
		else
			*((ulong *) usbd->tx_data) = STATUS_DONE;

		usbd->send_data(usbd->tx_data, usbd->tx_len);
		return 1;

	case COMMAND_PARTITION_SYNC:
		*((ulong *) usbd->tx_data) = CONFIG_RECOVERY_BOOT_BLOCKS - 1;
		usbd->send_data(usbd->tx_data, usbd->tx_len);
		return 1;

	case COMMAND_WRITE_PART_1:
		img_type = IMG_BOOT;
		break;

	/* Download complete -> reset */
	case COMMAND_RESET_PDA:
		/* Stop USB */
		usbd->usb_stop();

		if (usbd->cpu_reset)
			usbd->cpu_reset();

		return 0;

	/* Error */
	case COMMAND_RESET_USB:
		/* Stop USB */
		usbd->usb_stop();
		return 0;

	default:
		return 1;
	}

	/* Erase and Write to NAND */
	switch (img_type) {
	case IMG_BOOT:
		update_boot_image();
		break;

	default:
		/* Retry? */
		break;
	}

	if (ret) {
		/* Retry this commad */
		*((ulong *) usbd->tx_data) = STATUS_RETRY;
		usbd->send_data(usbd->tx_data, usbd->tx_len);
		return 1;
	} else
		*((ulong *) usbd->tx_data) = STATUS_DONE;

	/* Write image success -> Report status */
	usbd->send_data(usbd->tx_data, usbd->tx_len);

	return 1;
}

int do_usbd_down(void)
{
	struct usbd_ops *usbd;

	/* interface setting */
	usbd = usbd_set_interface(&usbd_ops);
	down_ram_addr = usbd->ram_addr;

	/* init the usb controller */
	usbd->usb_init();

	/* receive setting */
	usbd->recv_setup(usbd->rx_data, usbd->rx_len);

	/* detect the download request from Host PC */
	if (usbd->recv_data())
		usbd->send_data(usbd->tx_data, usbd->tx_len);
	else
		return 0;

	/* receive the data from Host PC */
	while (1) {
		usbd->recv_setup(usbd->rx_data, usbd->rx_len);

		if (usbd->recv_data()) {
			if (process_data(usbd) == 0)
				return 0;
		}
	}

	return 0;
}
