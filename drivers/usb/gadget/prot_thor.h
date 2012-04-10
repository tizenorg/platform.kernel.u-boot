/*
 * THOR protocol internals
 *
 * Copyright (C) 2012 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
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

#ifndef __PROT_THOR_H_
#define __PROT_THOR_H_

#include <common.h>
#include <linux/usb/f_usbd_thor.h>

#define VER_PROTOCOL_MAJOR	4
#define VER_PROTOCOL_MINOR	0

enum rqt {
	RQT_INFO = 200,
	RQT_CMD,
	RQT_DL,
	RQT_UL,
};

enum rqt_data {
	/* RQT_INFO */
	RQT_INFO_VER_PROTOCOL = 1,
	RQT_INIT_VER_HW,
	RQT_INIT_VER_BOOT,
	RQT_INIT_VER_KERNEL,
	RQT_INIT_VER_PLATFORM,
	RQT_INIT_VER_CSC,

	/* RQT_CMD */
	RQT_CMD_REBOOT = 1,
	RQT_CMD_POWEROFF,
	RQT_CMD_EFSCLEAR,

	/* RQT_DL */
	RQT_DL_INIT = 1,
	RQT_DL_FILE_INFO,
	RQT_DL_FILE_START,
	RQT_DL_FILE_END,
	RQT_DL_EXIT,

	/* RQT_UL */
	RQT_UL_INIT = 1,
	RQT_UL_START,
	RQT_UL_END,
	RQT_UL_EXIT,
};

typedef struct _rqt_box {	/* total: 256B */
	s32 rqt;		/* request id */
	s32 rqt_data;		/* request data id */
	s32 int_data[14];	/* int data */
	char str_data[5][32];	/* string data */
	char md5[32];		/* md5 checksum */
} __attribute__((packed)) rqt_box;

typedef struct _rsp_box {	/* total: 128B */
	s32 rsp;		/* response id (= request id) */
	s32 rsp_data;		/* response data id */
	s32 ack;		/* ack */
	s32 int_data[5];	/* int data */
	char str_data[3][32];	/* string data */
} __attribute__((packed)) rsp_box;

typedef struct _data_rsp_box {	/* total: 8B */
	s32 ack;		/* response id (= request id) */
	s32 count;		/* response data id */
} __attribute__((packed)) data_rsp_box;

enum {
	FILE_TYPE_NORMAL,
	FILE_TYPE_PIT,
};

#define F_NAME_BUF_SIZE 32

/* download packet size */
#define PKT_DOWNLOAD_SIZE (1 << 20) /* 1 MiB */
#define PKT_DOWNLOAD_CHUNK_SIZE (32 << 20) /* 32 MiB */

int process_data(struct g_dnl *dnl);

static inline int pkt_download(void *dest, const int size)
{
	usbd_set_dma((char *)dest, size);
	return usbd_rx_data();
}

static inline void pkt_upload(void *src, const int size)
{
	usbd_tx_data((char *)src, size);
}

#endif /*  __PROT_THOR_H__ */
