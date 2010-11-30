/*
 * (C) Copyright 2010 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 */
#include <common.h>
#include <info_action.h>

void info_action_check()
{
	struct info_action *ia = (struct info_action *) CONFIG_INFO_ADDRESS;
	char buf[64];
	unsigned int count = 0;

	if (ia->magic == INFO_ACTION_MAGIC) {
		switch (ia->action) {
		case INFO_ACTION_SDCARD_BOOT:
			printf("sdcard boot\n");
			count += sprintf(buf + count, "mmc rescan 1; ");
			count += sprintf(buf + count, "run loaduimage; ");
			count += sprintf(buf + count, "run sdboot; ");
			setenv("bootcmd", buf);
			break;
		case INFO_ACTION_LCD_CONSOLE:
			break;
		case INFO_ACTION_UMS:
			break;
		default:
			break;
		}
	}
}
