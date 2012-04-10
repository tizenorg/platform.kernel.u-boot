/*
 *  Copyright (C) 2012 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __G_DOWNLOAD_H_
#define __G_DOWNLOAD_H_

#include <mmc.h>
#include <linux/usb/ch9.h>
#include <usbdescriptors.h>
#include <linux/usb/gadget.h>

enum { THOR = 1 };
enum {MMC = 1, FAT = 2, RAW = 3};

struct g_dnl {
	int prot;

	char *file_name;
	int file_size;

	int (*init) (struct g_dnl *dnl);
	int (*info) (void);

	int (*dl) (struct g_dnl *dnl);
	int (*ul) (struct g_dnl *dnl);

	unsigned int *rx_buf;
	unsigned int *tx_buf;

	int (*cmd) (struct g_dnl *dnl);

	int medium;
	int (*store) (struct g_dnl *dnl, int medium);
	unsigned int p; /* private */

	int packet_size;
};

/**
 * dnl_init - initialize code downloader
 * @dnl: Interface to download functions
 *
 * returns zero, or a negative error code.
 */
static inline int dnl_init(struct g_dnl *dnl)
{
	return dnl->init(dnl);
}

/**
 * dnl_download - handle download operation
 * @dnl: Interface to download functions
 *
 * returns zero on success or a negative error code.
 *
 */
static inline int dnl_download(struct g_dnl *dnl)
{
	return dnl->dl(dnl);
}

/**
 * dnl_command - handle various commands
 * @dnl: Interface to download functions
 *
 * returns zero on success or a negative error code.
 *
 */
static inline int dnl_command(struct g_dnl *dnl)
{
	return dnl->cmd(dnl);
}

int g_dnl_init(char *s, struct g_dnl *dnl);
void set_udc_gadget_private_data(void *);
struct g_dnl *get_udc_gadget_private_data(struct usb_gadget *gadget);

void usbd_thor_udc_probe(void);

#endif /* __G_DOWNLOAD_H_ */
