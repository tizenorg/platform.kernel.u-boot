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
#ifdef CONFIG_S5PC1XXFB
#include <fbutils.h>
#endif

#define TX_DATA_LEN	4
#define RX_DATA_LEN	64

/* required for DMA transfers */
__attribute__ ((__aligned__ (__alignof__ (long long))))
static char tx_data[TX_DATA_LEN] = "MPL";
__attribute__ ((__aligned__ (__alignof__ (long long))))
static char rx_data[RX_DATA_LEN];

static int downloading;

extern int s5p_receive_done;
extern int s5p_usb_connected;
extern otg_dev_t otg;

static int __usb_board_init(void)
{
	return 0;
}

int usb_board_init(void) __attribute__((weak, alias("__usb_board_init")));

extern int s5p_no_lcd_support(void);
extern void s5pc_fimd_lcd_off(unsigned int win_id);
extern void s5pc_fimd_window_off(unsigned int win_id);
extern void get_rev_info(char *rev_info);
extern int check_exit_key(void);

#ifdef CONFIG_GENERIC_MMC
#include <mmc.h>
static int mmc_type;
extern int s5p_no_mmc_support(void);

static void usbd_set_mmc_dev(struct usbd_ops *usbd)
{
	struct mmc *mmc;
	char mmc_name[4];

	if (s5p_no_mmc_support())
		return;

	usbd->mmc_dev = 0;

	mmc = find_mmc_device(usbd->mmc_dev);
	mmc_init(mmc);

	if (!mmc->read_bl_len) {
		mmc_type = -1;
		usbd->mmc_max = 0;
		usbd->mmc_total = 0;
		return;
	}

	/* FIXME */
	usbd->mmc_max = 0x8000;
	usbd->mmc_blk = mmc->read_bl_len;
	usbd->mmc_total = mmc->capacity / mmc->read_bl_len;

	sprintf(mmc_name, "%c%c%c", mmc->cid[0] & 0xff,
			(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff);

	if (!strncmp(mmc_name, "SEM", 3))
		mmc_type = 1;
	else
		mmc_type = 0;
}
#endif

void s5pc1xx_wdt_reset(void)
{
	unsigned long wdt_base = samsung_get_base_watchdog();

	/*
	 * WTCON
	 * Watchdog timer[5]	: 0(disable) / 1(enable)
	 * Reset[0]		: 0(disable) / 1(enable)
	 */
	writel((1 << 5) | (1 << 0), wdt_base);
}

/* clear download informations */
static void s5p_usb_clear_dnfile_info(void)
{
	otg.dn_addr = 0;
	otg.dn_filesize = 0;
	otg.dn_ptr = 0;
}

/* start the usb controller */
static int usb_init(void)
{
	if (usb_board_init()) {
		puts("Failed to usb_board_init\n");
		return 0;
	}

#ifdef CONFIG_S5PC1XXFB
	if (!s5p_no_lcd_support()) {
		char rev_info[64];

		init_font();
		set_font_color(FONT_WHITE);

		get_rev_info(rev_info);
		fb_printf(rev_info);

#ifdef CONFIG_GENERIC_MMC
		if (mmc_type == 1)
			fb_printf("MMC: iNAND\n");
		else if (mmc_type == 0)
			fb_printf("MMC: MoviNAND\n");

		fb_printf("\n");
#endif
		fb_printf("USB Download Mode\n");
		fb_printf("Press the POWERKEY to CANCEL the downloading\n");
	}
#endif

	s5p_usbctl_init();

	printf("USB Start!! - %s Speed\n",
			otg.speed ? "Full" : "High");

	while (!s5p_usb_connected) {
		if (s5p_usb_detect_irq()) {
			s5p_udc_int_hndlr();
			s5p_usb_clear_irq();
		}

		if (check_exit_key())
			return 0;
	}

	s5p_usb_clear_dnfile_info();

	puts("Connected!!\n");

	return 1;
}

static void usb_stop(void)
{
#ifdef CONFIG_MMC_ASYNC_WRITE
	/* We must wait until finish the writing */
	if (!s5p_no_mmc_support()) {
		struct mmc *mmc;
		mmc = find_mmc_device(0);
		mmc_init(mmc);
	}
#endif
#ifdef CONFIG_S5PC1XXFB
	if (!s5p_no_lcd_support()) {
		exit_font();

		/* it uses fb3 as default window. */
		s5pc_fimd_lcd_off(3);
		s5pc_fimd_window_off(3);
	}
#endif
}

static void down_start(void)
{
	downloading = 1;

#ifdef CONFIG_S5PC1XXFB
	if (!s5p_no_lcd_support()) {
		fb_printf("Download Start\n");
		draw_progress(95, 0, FONT_WHITE);
	}
#endif
}

static void down_cancel(int mode)
{
#ifdef CONFIG_S5PC1XXFB
	if (!s5p_no_lcd_support()) {
		/* clear fb */
		fb_clear(120);
		exit_font();
	}
#endif
	switch (mode) {
	case END_BOOT:
		run_command("run bootcmd", 0);
		break;
	case END_RETRY:
		run_command("usbdown", 0);
		break;
	default:
		break;
	}
}

/*
 * receive the packet from host PC
 * return received size
 */
static int usb_receive_packet(void)
{
	s5p_usb_set_dn_addr(otg.dn_addr, otg.dn_filesize);
	while (1) {
		if (s5p_usb_detect_irq()) {
			s5p_udc_int_hndlr();
			s5p_usb_clear_irq();
		}

		if (!s5p_usb_connected) {
			puts("Disconnected!!\n");
			return -1;
		}

		if (s5p_receive_done) {
			s5p_receive_done = 0;
			return otg.dn_filesize;
		}

		if (!downloading) {
			if (check_exit_key())
				return 0;
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

#ifdef CONFIG_S5PC1XXFB
static void set_progress(int progress)
{
	draw_progress(95, progress, FONT_WHITE);
}
#endif

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
	usbd->down_cancel = down_cancel;
	usbd->down_start = down_start;
#ifdef CONFIG_S5PC1XXFB
	if (!s5p_no_lcd_support())
		usbd->set_progress = set_progress;
#endif
#ifdef CONFIG_GENERIC_MMC
	usbd_set_mmc_dev(usbd);
#endif
	downloading = 0;

	return usbd;
}
