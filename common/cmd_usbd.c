/*
 * cmd_usbd.c -- USB THOR Downloader gadget
 *
 * Copyright (C) 2012 Lukasz Majewski <l.majewski@samsung.com>
 * All rights reserved.
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
#define DEBUG
#include <common.h>
#include <usbd_thor.h>
#include <asm/errno.h>
#include <malloc.h>
#include <g_dnl.h>

#define STR_SIZE 16

static char dnl_tab[4][STR_SIZE];

char *find_dnl_entry(char* s, char *name)
{
	char *st, *c;

	for (; s; strsep(&s, ";"), st = s) {
		st = strchr(s, ' ');

		if (!strncmp(s, name, st - s)) {
			for (c = s; c; strsep(&c, ";"))
				;
			return s;
		}
	}
	return NULL;
}

int img_store(struct g_dnl *dnl, int medium)
{
	struct mmc *mmc;
	char cmd_buf[128];

	memset(cmd_buf, '\0', sizeof(cmd_buf));

	switch (medium) {
	case MMC:
		sprintf(cmd_buf, "%s write 0x%x %s %s", &dnl_tab[1][0],
			(unsigned int) dnl->rx_buf, &dnl_tab[2][0],
			&dnl_tab[3][0]);
		break;
	case FAT:
		sprintf(cmd_buf, "%swrite mmc %s:%s 0x%x %s %x",
			&dnl_tab[1][0], &dnl_tab[2][0], &dnl_tab[3][0],
			(unsigned int) dnl->rx_buf, &dnl_tab[0][0],
			dnl->file_size);
		break;
	case RAW:
		mmc = find_mmc_device(0);
		if (!mmc) {
			puts("no mmc device at slot 0\n");
			return -1;
		}
		mmc_init(mmc);

		sprintf(cmd_buf, "mmc write 0x%x %x %x",
			(unsigned int) dnl->rx_buf, dnl->p,
			dnl->packet_size / mmc->write_bl_len);
		break;
	}

	debug("%s: %s\n", __func__, cmd_buf);
	run_command(cmd_buf, 0);

	return 0;
}

int do_usbd_down(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	/* Simple argv[0] passing is not working since 'usbdown' cmd can
	   be run by */
	/* 'usb', 'usbd' or 'usbdown' */
	char *str, *st, *str_env;

	int ret = 0, i = 0;
	static char *s = "thor";
	static struct g_dnl *dnl;

	dnl = memalign(CONFIG_SYS_CACHELINE_SIZE, sizeof(struct g_dnl));

	puts("THOR Downloader\n");

	g_dnl_init(s, dnl);

	ret = dnl_init(dnl);
	if (ret)
		printf("%s: USBDOWN failed\n", __func__);

	ret = dnl_command(dnl);
	if (ret < 0)
		printf("%s: COMMAND failed: %d\n", __func__, ret);

	debug("DNL: file:%s size:%d\n", dnl->file_name, dnl->file_size);

	str_env = getenv("dnl_info");
	if (str_env == NULL) {
		puts("DNL: \"dnl_info\" variable not defined!\n");
		return -1;
	}
	debug("dnl_info: %s\n", str_env);

	str = find_dnl_entry(str_env, dnl->file_name);
	if (str == NULL) {
		printf("File: %s not at \"dnl_info\"!\n", dnl->file_name);
		return -1;
	}

	debug("%s:str: %s\n", __func__, str);

	memset(dnl_tab, '\0', sizeof(dnl_tab));
	do {
		st = strsep(&str, " ");
		strncpy(&dnl_tab[i++][0], st, strlen(st));

	} while (str);

	if (strncmp(dnl->file_name, &dnl_tab[0][0], strlen(&dnl_tab[0][0]))) {
		printf("Parsed string not match file: %s!\n", dnl->file_name);
		return -1;
	}

	debug("%s %s %s %s\n", &dnl_tab[0][0], &dnl_tab[1][0],
	       &dnl_tab[2][0], &dnl_tab[3][0]);

	if (!strncmp(&dnl_tab[1][0], "mmc", strlen("mmc"))) {
		dnl->store = img_store;
		dnl->medium = MMC;
	} else if (!strncmp(&dnl_tab[1][0], "fat", strlen("fat"))) {
		dnl->store = img_store;
		dnl->medium = FAT;
	} else if (!strncmp(&dnl_tab[1][0], "raw", strlen("raw"))) {
		dnl->store = img_store;
		dnl->medium = RAW;
	} else {
		printf("DNL: Medium: %s not recognized!", &dnl_tab[1][0]);
	}

	ret = dnl_download(dnl);
	if (ret < 0)
		printf("%s: DOWNLOAD failed: %d\n", __func__, ret);

	ret = dnl_command(dnl);
	if (ret < 0)
		printf("%s: COMMAND failed: %d\n", __func__, ret);

	return 0;
}

U_BOOT_CMD(usbdown, CONFIG_SYS_MAXARGS, 1, do_usbd_down,
	   "Initialize USB device and Run  THOR USB downloader", NULL
);
