/*
 * prot_thor.c -- USB THOR Downloader protocol
 *
 * Copyright (C) 2012 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
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
 *
 */
#undef DEBUG
#include <common.h>
#include <errno.h>
#include <g_dnl.h>
#include "prot_thor.h"

static void send_rsp(const rsp_box *rsp)
{
	/* should be copy on dma duffer */
	memcpy(usbd_tx_data_buf, rsp, sizeof(rsp_box));
	pkt_upload(usbd_tx_data_buf, sizeof(rsp_box));

	debug("-RSP: %d, %d\n", rsp->rsp, rsp->rsp_data);
}

static void send_data_rsp(s32 ack, s32 count)
{
	data_rsp_box rsp;

	rsp.ack = ack;
	rsp.count = count;

	/* should be copy on dma duffer */
	memcpy(usbd_tx_data_buf, &rsp, sizeof(data_rsp_box));
	pkt_upload(usbd_tx_data_buf, sizeof(data_rsp_box));

	debug("-DATA RSP: %d, %d\n", ack, count);
}

static int process_rqt_info(const rqt_box *rqt)
{
	rsp_box rsp = {0, };

	rsp.rsp = rqt->rqt;
	rsp.rsp_data = rqt->rqt_data;

	switch (rqt->rqt_data) {
	case RQT_INFO_VER_PROTOCOL:
		rsp.int_data[0] = VER_PROTOCOL_MAJOR;
		rsp.int_data[1] = VER_PROTOCOL_MINOR;
		break;
	case RQT_INIT_VER_HW:
		sprintf(rsp.str_data[0], "%x", checkboard());
		break;
	case RQT_INIT_VER_BOOT:
		sprintf(rsp.str_data[0], "%s", getenv("ver"));
		break;
	case RQT_INIT_VER_KERNEL:
		sprintf(rsp.str_data[0], "%s", "k unknown");
		break;
	case RQT_INIT_VER_PLATFORM:
		sprintf(rsp.str_data[0], "%s", "p unknown");
		break;
	case RQT_INIT_VER_CSC:
		sprintf(rsp.str_data[0], "%s", "c unknown");
		break;
	default:
		return 0;
	}

	send_rsp(&rsp);
	return 1;
}

static int process_rqt_cmd(const rqt_box *rqt)
{
	rsp_box rsp = {0, };

	rsp.rsp = rqt->rqt;
	rsp.rsp_data = rqt->rqt_data;

	switch (rqt->rqt_data) {
	case RQT_CMD_REBOOT:
		debug("TARGET RESET\n");
		send_rsp(&rsp);
		run_command("reset", 0);
		break;
	case RQT_CMD_POWEROFF:
	case RQT_CMD_EFSCLEAR:
		send_rsp(&rsp);
	default:
		printf("Command not supported -> cmd: %d\n", rqt->rqt_data);
		return -1;
	}

	return 0;
}

static unsigned long download(unsigned int total, unsigned int packet_size,
			      struct g_dnl *dnl)
{
	int count = 0;
	unsigned int rcv_cnt = 0;
	static int sect_start = 92160; /* Hardcoded -> will be fixed -> */
	unsigned int dma_buffer_address = CONFIG_SYS_DOWN_ADDR;

	do {
		if (packet_size == PKT_DOWNLOAD_SIZE)
			dma_buffer_address =
				CONFIG_SYS_DOWN_ADDR + (count * packet_size);

		usbd_set_dma((char *) dma_buffer_address,
			     packet_size);

		rcv_cnt += usbd_rx_data();
		debug("RCV data count: %u\n", rcv_cnt);

		/* Store data after receiving a "chunk" packet */
		if (packet_size == PKT_DOWNLOAD_CHUNK_SIZE &&
		    (rcv_cnt % PKT_DOWNLOAD_CHUNK_SIZE) == 0) {
			dnl->p = (sect_start + count *
				  (PKT_DOWNLOAD_CHUNK_SIZE >> 9));
			debug("DNL STORE dnl->p: %d\n", dnl->p);
			dnl->store(dnl, dnl->medium);
		}
		send_data_rsp(0, ++count);
	} while (rcv_cnt < total);

	debug("rcv: %d dnl: %d\n", rcv_cnt, total);

	return rcv_cnt;
}

static int process_rqt_download(const rqt_box *rqt, struct g_dnl *dnl)
{
	static unsigned long download_total_size, cnt;
	static char f_name[F_NAME_BUF_SIZE];
	rsp_box rsp = {0, };
	int file_type;
	int ret = 1;

	rsp.rsp = rqt->rqt;
	rsp.rsp_data = rqt->rqt_data;

	switch (rqt->rqt_data) {
	case RQT_DL_INIT:
		download_total_size = rqt->int_data[0];

		debug("INIT: total %d bytes\n", rqt->int_data[0]);
		break;
	case RQT_DL_FILE_INFO:
		file_type = rqt->int_data[0];
		if (file_type == FILE_TYPE_PIT) {
			puts("PIT table file - not supported\n");
			return -1;
		}

		dnl->file_size = rqt->int_data[1];
		memcpy(f_name, rqt->str_data[0], sizeof(f_name));

		debug("INFO: name(%s, %d), size(%d), type(%d)\n",
		       f_name, 0, dnl->file_size, file_type);

		if (dnl->file_size > PKT_DOWNLOAD_CHUNK_SIZE)
			dnl->packet_size = PKT_DOWNLOAD_CHUNK_SIZE;
		else
			dnl->packet_size = PKT_DOWNLOAD_SIZE;

		printf("%s: dnl->file_size: %d dnl->packet_size: %d\n",
		       __func__, dnl->file_size, dnl->packet_size);

		rsp.int_data[0] = dnl->packet_size;

		dnl->file_name = f_name;
		ret = 0;

		break;
	case RQT_DL_FILE_START:
		send_rsp(&rsp);

		cnt = download(download_total_size, dnl->packet_size, dnl);

		dnl->store(dnl, dnl->medium);

		return cnt;
	case RQT_DL_FILE_END:
		debug("DL FILE_END\n");
		break;
	case RQT_DL_EXIT:
		debug("DL EXIT\n");
		ret = 0;

		break;
	default:
		printf("Operation not supported: %d\n", rqt->rqt_data);
		return -1;
	}

	send_rsp(&rsp);
	return ret;
}

int process_data(struct g_dnl *dnl)
{
	rqt_box rqt;
	int ret = 1;

	memset(&rqt, 0, sizeof(rqt));
	memcpy(&rqt, usbd_rx_data_buf, sizeof(rqt));

	debug("+RQT: %d, %d\n", rqt.rqt, rqt.rqt_data);

	switch (rqt.rqt) {
	case RQT_INFO:
		ret = process_rqt_info(&rqt);
		break;
	case RQT_CMD:
		ret = process_rqt_cmd(&rqt);
		break;
	case RQT_DL:
		ret = process_rqt_download(&rqt, dnl);
		break;
	case RQT_UL:
		puts("RQT: UPLOAD not supported!\n");
		break;
	default:
		printf("unknown request (%d)\n", rqt.rqt);
		ret = 0;
	}

	/* exit code: */
	/* 0 - success */
	/* < 0 - Error code */

	return ret;
}
