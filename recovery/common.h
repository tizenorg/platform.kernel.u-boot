/*
 * (C) Copyright 2010 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __COMMON_H
#define __COMMON_H

/* board/.../... */

extern int board_check_condition(void);
extern int board_load_uboot(unsigned char *buf);
extern int board_update_uboot(void);
extern void board_recovery_init(void);

#endif
