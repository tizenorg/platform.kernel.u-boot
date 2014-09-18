/*
 * (C) Copyright 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _LIBTIZEN_H_
#define _LIBTIZEN_H_

#include <lcd.h>
#include <samsung/misc.h>

#define HD_RESOLUTION	0

void get_tizen_logo_info(vidinfo_t *vid);
void draw_thor_progress(unsigned long long int total_file_size, unsigned long long int downloaded_file_size);
void draw_thor_screen(void);
void draw_thor_connected(void);
void draw_thor_fail_screen(void);
void draw_thor_cable_info(void);

/* Interactive charger */
void draw_battery_screen(void);
void draw_charge_screen(void);
void clean_charge_screen(void);
void draw_charge_animation(void);
void draw_battery_state(unsigned int soc);
void draw_connect_charger_animation(void);

#endif	/* _LIBTIZEN_H_ */
