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

#define TX_DATA_LEN	4
#define RX_DATA_LEN	64

static char tx_data[TX_DATA_LEN] = "MPL";
static char rx_data[RX_DATA_LEN];

extern int s5p_receive_done;
extern int s5p_usb_connected;
extern otg_dev_t otg;

static int __usb_board_init(void)
{
	return 0;
}

int usb_board_init(void) __attribute__((weak, alias("__usb_board_init")));

static void cpu_reset(void)
{
	writel(0x1, S5PC110_SWRESET);
}

/* clear download informations */
static void s5p_usb_clear_dnfile_info(void)
{
	otg.dn_addr = 0;
	otg.dn_filesize = 0;
	otg.dn_ptr = 0;
}

/* start the usb controller */
static void usb_init(void)
{
	if (usb_board_init())
		return;

	s5p_usbctl_init();

	while (!s5p_usb_connected) {
		if (s5p_usb_detect_irq()) {
			s5p_udc_int_hndlr();
			s5p_usb_clear_irq();
		}
	}

	s5p_usb_clear_dnfile_info();
}

static void usb_stop(void)
{
}

/*
 * receive the packet from host PC
 * return received size
 */
static int usb_receive_packet(void)
{
	while (1) {
		if (s5p_usb_detect_irq()) {
			s5p_udc_int_hndlr();
			s5p_usb_clear_irq();
		}

		if (s5p_receive_done) {
			s5p_receive_done = 0;
			return otg.dn_filesize;
		}
	}
}

/* setup the download informations */
static void recv_setup(char *addr, int len)
{
	s5p_usb_clear_dnfile_info();

	otg.dn_addr = (u32)addr;
	otg.dn_ptr = (u8 *) addr;
	otg.dn_filesize = len;
}

/*
 * This function is interfaced between
 * USB Device Controller and USB Downloader
 */
struct usbd_ops *usbd_set_interface(struct usbd_ops *usbd)
{
	usbd->usb_init = usb_init;
	usbd->usb_stop = usb_stop;
	usbd->send_data = s5p_usb_tx;
	usbd->recv_data = usb_receive_packet;
	usbd->recv_setup = recv_setup;
	usbd->tx_data = tx_data;
	usbd->rx_data = rx_data;
	usbd->tx_len = TX_DATA_LEN;
	usbd->rx_len = RX_DATA_LEN;
	usbd->ram_addr = CONFIG_SYS_DOWN_ADDR;
	usbd->cpu_reset = cpu_reset;

	return usbd;
}
