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

int board_check_condition(void);
int board_load_bootloader(unsigned char *buf);
int board_lock_recoveryblock(void);
void board_update_init(void);
int board_update_image(u32 *buf, u32 len);
void board_recovery_init(void);

#endif
