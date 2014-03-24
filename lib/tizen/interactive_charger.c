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

#include "battery_frame.h"
#include "battery_bar.h"
#include "battery_bar_red.h"
#include "charge_screen.h"
#include "charge_screen_clc.h"
#include "charge_screen_indicator.h"

#define CHARGE_SCREEN_SIZE_X	130
#define CHARGE_SCREEN_SIZE_Y	120

#define BATT_FRAME_SIZE_X	200
#define BATT_FRAME_SIZE_Y	380

#define BATT_BAR_SIZE_X		160
#define BATT_BAR_SIZE_Y		50
#define BATT_BAR_FIRST_GAP	8
#define BATT_BAR_GAP		8

#define BATT_FRAME_EMPTY_Y_OFFSET	356

#define BATT_FRAME_X_POS	((720 - BATT_FRAME_SIZE_X) / 2)
#define BATT_FRAME_Y_POS	((1280 - BATT_FRAME_SIZE_Y) / 2)

#define CHARGE_SCREEN_X_POS	((720 - CHARGE_SCREEN_SIZE_X) / 2)
#define CHARGE_SCREEN_Y_POS	(BATT_FRAME_Y_POS + BATT_FRAME_SIZE_Y + 100)

#define LOGO_X_OFS		((720 - 480) / 2)
#define LOGO_Y_OFS		((1280 - 800) / 2)
#define PROGRESS_BAR_WIDTH	4

#define ADD_ANIMATION_IMG(_x, _y, _img, _clc_img) { \
	.x_offs = _x, \
	.y_offs = _y, \
	.img = _img, \
	.clc_img = _clc_img, \
}

/* x, y: Offsets relative to charge image position */
struct animation_image {
	unsigned x_offs;
	unsigned y_offs;
	bmp_image_t **img;
	bmp_image_t **clc_img;
};

static int last_bar_cnt;

/* Common bar */
static bmp_image_t *bmp_bar_state;

/* Low state bar */
static bmp_image_t *bmp_bar_red_state;

/* Charge animation screen */
static bmp_image_t *charge_bmp;
static bmp_image_t *charge_clc_bmp;

/* Indicator images */
static bmp_image_t *indicator_bmp;
static bmp_image_t *indicator_clc_horiz_bmp;
static bmp_image_t *indicator_clc_vert_bmp;

/* Image: x, y, clean image ptr*/
static struct animation_image anim_chrg[] = {
	ADD_ANIMATION_IMG(113, 101, &indicator_bmp, &indicator_clc_vert_bmp),
	ADD_ANIMATION_IMG(85, 110, &indicator_bmp, &indicator_clc_horiz_bmp),
	ADD_ANIMATION_IMG(48, 110, &indicator_bmp, &indicator_clc_horiz_bmp),
	ADD_ANIMATION_IMG(22, 101, &indicator_bmp, &indicator_clc_vert_bmp)
};

void draw_battery_screen(void)
{
	static bmp_image_t *battery;
	void *frame_bitmap;

	if (!battery)
		battery = gunzip_bmp((long unsigned int)battery_frame,
				     NULL, &frame_bitmap);

	if (!battery)
		battery = (bmp_image_t *)battery_frame;

	lcd_display_bitmap((unsigned long)battery,
			   BATT_FRAME_X_POS,
			   BATT_FRAME_Y_POS);

	last_bar_cnt = 0;
}

static void prepare_battery_state(void)
{
	void *bar_bitmap;
	void *bar_red_bitmap;

	/* Check gzip version */
	if (!bmp_bar_state)
		bmp_bar_state = gunzip_bmp((long unsigned int)
					   battery_bar,
					   NULL, &bar_bitmap);
	if (!bmp_bar_state)
		bmp_bar_state = (bmp_image_t *)battery_bar;

	/* Check gzip version */
	if (!bmp_bar_red_state)
		bmp_bar_red_state = gunzip_bmp((long unsigned int)
					       battery_bar_red,
					       NULL, &bar_red_bitmap);
	if (!bmp_bar_red_state)
		bmp_bar_red_state = (bmp_image_t *)battery_bar_red;
}

void draw_battery_state(unsigned int current_percent)
{
	unsigned long bar_image;
	unsigned bar_x;
	unsigned bar_y;
	unsigned bar_y_tmp;
	static int img_ready;
	int bar_cnt;
	int i;

	if (!img_ready) {
		prepare_battery_state();
		img_ready = 1;
	}

	bar_x = BATT_FRAME_X_POS + (BATT_FRAME_SIZE_X - BATT_BAR_SIZE_X) / 2;
	bar_y = BATT_FRAME_Y_POS + BATT_FRAME_EMPTY_Y_OFFSET;
	bar_y -= (BATT_BAR_SIZE_Y + BATT_BAR_FIRST_GAP);

	bar_image = (unsigned long)bmp_bar_state;

	if (current_percent <= 20) {
		/* Draw red 20% */
		bar_cnt = 1;
		bar_image = (unsigned long)bmp_bar_red_state;
	} else if (current_percent <= 40) {
		/* Draw gray 20% */
		bar_cnt = 1;
		lcd_display_bitmap(bar_image, bar_x, bar_y);
	} else if (current_percent <= 60) {
		/* Draw gray 40% */
		bar_cnt = 2;
	} else if (current_percent <= 80) {
		/* Draw gray 60% */
		bar_cnt = 3;
	} else if (current_percent <= 90) {
		/* Draw gray 80% */
		bar_cnt = 4;
	} else {
		/* Draw gray 100% */
		bar_cnt = 5;
	}

	for (i = last_bar_cnt; i < bar_cnt; i++) {
		bar_y_tmp = bar_y - i * (BATT_BAR_SIZE_Y + BATT_BAR_GAP);
		lcd_display_bitmap(bar_image, bar_x, bar_y_tmp);
	}

	last_bar_cnt = bar_cnt;
}

static void prepare_charge_screen(void)
{
	/* Charge animation screen */
	void *charge_bitmap;
	void *charge_clc_bitmap;

	/* Charge animation screen */
	if (!charge_bmp)
		charge_bmp = gunzip_bmp((long unsigned int)charge_screen,
					NULL,
					&charge_bitmap);
	if (!charge_bmp)
		charge_bmp = (bmp_image_t *)charge_screen;

	/* Charge animation clear screen */
	if (!charge_clc_bmp)
		charge_clc_bmp = gunzip_bmp((long unsigned)charge_screen_clc,
					    NULL,
					    &charge_clc_bitmap);
	if (!charge_clc_bmp)
		charge_clc_bmp = (bmp_image_t *)charge_screen_clc;
}

void draw_charge_screen(void)
{
	static int img_ready;

	if (!img_ready) {
		prepare_charge_screen();
		img_ready = 1;
	}

	lcd_display_bitmap((unsigned long)charge_bmp,
			   CHARGE_SCREEN_X_POS,
			   CHARGE_SCREEN_Y_POS);
}

void clean_charge_screen(void)
{
	lcd_display_bitmap((unsigned long)charge_clc_bmp,
			   CHARGE_SCREEN_X_POS,
			   CHARGE_SCREEN_Y_POS);
}

void draw_connect_charger_animation(void)
{
	static int screen;
	static int img_ready;

	if (!img_ready) {
		prepare_charge_screen();
		img_ready = 1;
	}

	if (!screen) {
		draw_charge_screen();
		screen++;
	} else {
		clean_charge_screen();
		screen = 0;
	}
}

static void prepare_charge_animation(void)
{
	/* Indicator main */
	void *indicator_bitmap;
	/* Indicator clean horizontal */
	void *clc_horiz_bitmap;
	/* Indicator clean vertical */
	void *clc_vert_bitmap;

	/* Indicator main */
	if (!indicator_bmp)
		indicator_bmp = gunzip_bmp((long unsigned int)indicator, NULL,
					   &indicator_bitmap);
	if (!indicator_bmp)
		indicator_bmp = (bmp_image_t *)indicator;

	/* Indicator clean horizontal */
	if (!indicator_clc_horiz_bmp)
		indicator_clc_horiz_bmp = gunzip_bmp((long unsigned int)
						     indicator_clc_horiz,
						     NULL,
						     &clc_horiz_bitmap);
	if (!indicator_clc_horiz_bmp)
		indicator_clc_horiz_bmp = (bmp_image_t *)indicator_clc_horiz;

	/* Indicator clean vertical */
	if (!indicator_clc_vert_bmp)
		indicator_clc_vert_bmp = gunzip_bmp((long unsigned int)
						    indicator_clc_vert,
						    NULL,
						    &clc_vert_bitmap);
	if (!indicator_clc_vert_bmp)
		indicator_clc_vert_bmp = (bmp_image_t *)indicator_clc_vert;
}

void draw_charge_animation(void)
{
	/* Indicator current and last position */
	int anim_frame_pos_x, anim_frame_pos_y;
	static int last_img_ptr;
	static int img_ptr;
	static int img_ready;

	if (!img_ready) {
		prepare_charge_animation();
		img_ready = 1;
	}

	anim_frame_pos_x = CHARGE_SCREEN_X_POS;
	anim_frame_pos_y = CHARGE_SCREEN_Y_POS;

	if (img_ptr >= ARRAY_SIZE(anim_chrg))
		img_ptr = 0;

	/* Clean previous indicator */
	lcd_display_bitmap((unsigned long) *anim_chrg[last_img_ptr].clc_img,
			   anim_frame_pos_x + anim_chrg[last_img_ptr].x_offs,
			   anim_frame_pos_y + anim_chrg[last_img_ptr].y_offs);

	/* Draw indicator at new position */
	lcd_display_bitmap((unsigned long) *anim_chrg[img_ptr].img,
			   anim_frame_pos_x + anim_chrg[img_ptr].x_offs,
			   anim_frame_pos_y + anim_chrg[img_ptr].y_offs);

	last_img_ptr = img_ptr++;
}
