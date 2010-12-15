/*
 * (C) Copyright 2010 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 */
#include <common.h>
#include <info_action.h>
#include <mmc.h>

void info_action_check(void)
{
	struct info_action *ia = (struct info_action *) CONFIG_INFO_ADDRESS;
	char buf[64];
	unsigned int count = 0;
	struct mmc *mmc;

	if (ia->magic != INFO_ACTION_MAGIC)
		return;

	switch (ia->action) {
	case INFO_ACTION_SDCARD_BOOT:
		puts("sdcard boot\n");

		/*
		 * We need to initialize SD card for SD-boot.
		 * SD card device index : 2
		 */
		mmc = find_mmc_device(2);
		if (!mmc) {
			printf("Not found SD-card..insert SD-card!!\n");
			return;
		}
		mmc_init(mmc);

		/* Please add the parameters and just 'run mmcboot' */
		count += sprintf(buf + count, "run mmcboot; ");
		setenv("bootcmd", buf);
		setenv("mmcdev", "2");
		setenv("mmcrootpart", "0");
		setenv("mmcbootpart", "1");
		break;
	case INFO_ACTION_LCD_CONSOLE:
		printf("lcd console mode\n");
		break;
	case INFO_ACTION_UMS:
		printf("ums mode\n");
		break;
	default:
		break;
	}
}
