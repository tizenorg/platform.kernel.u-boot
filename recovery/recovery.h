/*
 * Copyright (C) 2010 Samsung Electronics
 * Sanghee Kim <sh0130.kim@samsung.com>
 *
 * Common header for Recovery Block
 */

#ifndef _RECOVERY_H
#define _RECOVERY_H

#define DEBUG_MARK	"Recovery: "

/* board/.../... */

extern int board_check_condition(void);
extern int board_load_uboot(unsigned char *buf);
extern void board_recovery_init(void);

#endif
