/*
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
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
 *
 */

#include <common.h>
#include "usbd.h"
#include "usb-hs-otg.h"

static unsigned char tx_data[8] = "MPL";
static unsigned long tx_len = 4;

static unsigned char rx_data[2048];
static unsigned long rx_len = 64;

struct usbd_ops *usbd_set_interface(struct usbd_ops *);

void usb_init(void);
int usb_receive_packet(void);

void recv_setup(char *, int);

extern int s3c_receive_done;
extern int s3c_usb_connected;
extern otg_dev_t otg;

/* This function is interfaced between
 * USB Device Controller and USB Downloader
 */
struct usbd_ops *usbd_set_interface(struct usbd_ops *usbd)
{
	usbd->usb_init = usb_init;
	usbd->usb_stop = s3c_usb_stop;
	usbd->send_data = s3c_usb_tx;
	usbd->recv_data = usb_receive_packet;
	usbd->recv_setup = recv_setup;
	usbd->tx_data = tx_data;
	usbd->rx_data = rx_data;
	usbd->tx_len = tx_len;
	usbd->rx_len = rx_len;
	usbd->ram_addr = CONFIG_SYS_DOWN_ADDR;

	return usbd;
}

/* clear download informations */
void s3c_usb_clear_dnfile_info(void)
{
	otg.dn_addr = 0;
	otg.dn_filesize = 0;
	otg.dn_ptr = 0;
}

/* clear upload informations */
void s3c_usb_clear_upfile_info(void)
{
	otg.up_addr = 0;
	otg.up_size = 0;
	otg.up_ptr = 0;
}

/* start the usb controller */
void usb_init(void)
{
	if (usb_board_init()) {
		printf("Failed to usb_board_init\n");
		return;
	}

	s3c_usbctl_init();
	s3c_usbc_activate();

	printf("USB Start!! - %s Speed\n",
			otg.speed ? "Full" : "High");

	while (!s3c_usb_connected) {
		if (S5P_USBD_DETECT_IRQ()) {
			s3c_udc_int_hndlr();
			S5P_USBD_CLEAR_IRQ();
		}
	}

	s3c_usb_clear_dnfile_info();

	printf("Connected!!\n");
}

/* receive the packet from host PC
 * return received size
 */
int usb_receive_packet(void)
{
	while (1) {
		if (S5P_USBD_DETECT_IRQ()) {
			s3c_udc_int_hndlr();
			S5P_USBD_CLEAR_IRQ();
		}

		if (s3c_receive_done) {
			s3c_receive_done = 0;
			return otg.dn_filesize;
		}
	}
}

/* setup the download informations */
void recv_setup(char *addr, int len)
{
	s3c_usb_clear_dnfile_info();

	otg.dn_addr = addr;
	otg.dn_ptr = addr;
	otg.dn_filesize = len;
}
