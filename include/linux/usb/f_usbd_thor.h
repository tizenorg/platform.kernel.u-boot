/*
 * USB THOR Downloader
 *
 * Copyright (C) 2012 Samsung Electronics
 * Lukasz Majewski  <l.majewski@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _USB_THOR_H_
#define _USB_THOR_H_

#include <linux/usb/composite.h>

/* ACM -> THOR GADGET download */
#define DEACTIVATE_CARRIER 0x0000
#define ACTIVATE_CARRIER 0x0003

#define SET_LINE_CODING 0x0020
#define SET_CONTROL_LINE_STATE 0x0022

/* THOR Composite Gadget */

#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

/* Samsung's IDs */
#define DRIVER_VENDOR_NUM 0x04E8
#define DRIVER_PRODUCT_NUM 0x6601

#define STRING_MANUFACTURER 25
#define STRING_PRODUCT 2
#define STRING_USBDOWN 2
#define	CONFIG_USBDOWNLOADER 2

int f_usbd_add(struct usb_configuration *c);

/* Interface to THOR protocol */
#define TX_DATA_LEN 128
#define RX_DATA_LEN 1024

extern char *usbd_tx_data_buf;
extern char *usbd_rx_data_buf;

extern void usbd_set_dma(char *addr, int len);
extern unsigned int usbd_rx_data(void);
extern void usbd_tx_data(char *data, int len);

#endif /* _USB_THOR_H_ */
