/*
 * (C) Copyright 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <malloc.h>
#include <lcd.h>
#include <version.h>
#include <libtizen.h>

#include "tizen_logo_16bpp.h"
#include "tizen_logo_16bpp_gzip.h"

#ifdef CONFIG_VIDEO_BMP_GZIP
#include "download_logos_16bpp_gzip.h"
#else
#include "download_logos_16bpp.h"
#endif

#define LOGO_X_OFS	((720 - 480) / 2)
#define LOGO_Y_OFS	((1280 - 800) / 2)
#define PROGRESS_BAR_WIDTH	4

void get_tizen_logo_info(vidinfo_t *vid)
{
	switch (vid->vl_bpix) {
	case 4:
		vid->logo_width = TIZEN_LOGO_16BPP_WIDTH;
		vid->logo_height = TIZEN_LOGO_16BPP_HEIGHT;
		vid->logo_x_offset = TIZEN_LOGO_16BPP_X_OFFSET;
		vid->logo_y_offset = TIZEN_LOGO_16BPP_Y_OFFSET;
#if defined(CONFIG_VIDEO_BMP_GZIP)
		vid->logo_addr = (ulong)tizen_logo_16bpp_gzip;
#else
		vid->logo_addr = (ulong)tizen_logo_16bpp;
#endif
		break;
	default:
		vid->logo_addr = 0;
		break;
	}
}

void draw_thor_fail_screen(void)
{
	int x, y;

	lcd_clear();

	x = 196 + LOGO_X_OFS;
	y = 280 + LOGO_Y_OFS;

	bmp_display((unsigned long)download_noti_image, x, y);

	x = 70 + LOGO_X_OFS;
	y = 370 + LOGO_Y_OFS;

	bmp_display((unsigned long)download_fail_text, x, y);
}

static unsigned int tmp_msg_x, tmp_msg_y;

void draw_thor_connected(void)
{
	unsigned int prev_console_x, prev_console_y;

	/* Get curent lcd xy */
	lcd_get_position_cursor(&prev_console_x, &prev_console_y);

	/* Set proper lcd xy position for clean unneeded messages */
	lcd_set_position_cursor(tmp_msg_x, tmp_msg_y);
	lcd_puts("\n\n                                          ");
	lcd_puts("\n\n                                          ");

	lcd_set_position_cursor(tmp_msg_x, tmp_msg_y);
	lcd_printf("\n\n\tConnection established.");

	/* Restore last lcd x y position */
	lcd_set_position_cursor(prev_console_x, prev_console_y);
}

void draw_thor_screen(void)
{
	int x, y;

	lcd_clear();
	lcd_printf("\n\n\tTHOR Tizen Downloader");

	lcd_printf("\n\n\tU-BOOT bootloader info:");
	lcd_printf("\n\tCompilation date: %s\n\tBinary version: %s\n",
		   U_BOOT_DATE, PLAIN_VERSION);

	char *pit_compatible = getenv("dfu_alt_pit_compatible");
	if (pit_compatible) {
		lcd_printf("\tPlatform compatible with PIT Version: %s\n",
			   pit_compatible);
	};

	lcd_get_position_cursor(&tmp_msg_x, &tmp_msg_y);

	lcd_printf("\n\n\tPlease connect USB cable.");
	lcd_printf("\n\n\tFor EXIT press POWERKEY 3 times.\n");

	x = 199 + LOGO_X_OFS;
	y = 272 + LOGO_Y_OFS;

	bmp_display((unsigned long)download_image, x, y);

	x = 90 + LOGO_X_OFS;
	y = 360 + LOGO_Y_OFS;

	bmp_display((unsigned long)download_text, x, y);

	x = 39 + LOGO_X_OFS;
	y = 445 + LOGO_Y_OFS;

	bmp_display((unsigned long)prog_base, x, y);

}

void draw_thor_progress(unsigned long long int total_file_size,
		   unsigned long long int downloaded_file_size)
{
	static bmp_image_t *progress_bar = NULL;
	static int last_percent;

	void *bar_bitmap;
	int current_percent = 0;
	int draw_percent;
	int i, tmp_x, x, y;

	/* Make shift to prevent int overflow */
	int divisor = total_file_size >> 7;
	int divident = 100 * (downloaded_file_size >> 7);

	if (!divisor || !divident)
		return;

	while (divident > 0) {
		divident -= divisor;
		if (divident >= 0)
			current_percent++;
	}

	if (current_percent > 100)
		current_percent = 100;

	if (!current_percent)
		last_percent = 0;

	draw_percent = current_percent - last_percent;
	if (!draw_percent)
		return;

	if (!progress_bar)
		progress_bar = gunzip_bmp((long unsigned int)prog_middle,
					  NULL, &bar_bitmap);

	if (!progress_bar)
		progress_bar = (bmp_image_t *)prog_middle;

	/* set bar position */
	x = 40 + LOGO_X_OFS;
	y = 446 + LOGO_Y_OFS;

	/* set last bar position */
	x += last_percent * PROGRESS_BAR_WIDTH;

	for (i=0; i < draw_percent; i++) {
		tmp_x = x + i * PROGRESS_BAR_WIDTH;
		lcd_display_bitmap((unsigned long)progress_bar, tmp_x, y);
	}

	last_percent = current_percent;
}
