/*
 * Sleep command for S5PC110
 *
 *  Copyright (C) 2005-2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <i2c.h>

#include <asm/io.h>

#include <asm/arch/cpu.h>
#include <asm/arch/power.h>

enum {
	SLEEP_WFI,
	SLEEP_REGISTER,
};

static void __board_sleep_init(void) { }

void board_sleep_init(void)
	__attribute__((weak, alias("__board_sleep_init")));

static int s5pc110_sleep(int mode)
{
	unsigned int value;

	board_sleep_init();

	writel(0x0badcafe, S5PC110_INFORM0);

	value = readl(S5PC110_PWR_CFG);
	value &= ~S5PC110_CFG_STANDBYWFI_MASK;
	if (mode == SLEEP_WFI)
		value |= S5PC110_CFG_STANDBYWFI_SLEEP;
	else
		value |= S5PC110_CFG_STANDBYWFI_IGNORE;
	writel(value, S5PC110_PWR_CFG);

	value = readl(S5PC110_OTHERS);
	value |= S5PC110_OTHERS_SYSCON_INT_DISABLE;
	writel(value, S5PC110_OTHERS);

	printf("%s[%d] sleep enter mode %d\n\n", __func__, __LINE__, mode);

	if (mode == SLEEP_WFI) {
		value = 0;
		asm("b	1f\n\t"
			".align 5\n\t"
			"1:\n\t"
			"mcr p15, 0, %0, c7, c10, 5\n\t"
			"mcr p15, 0, %0, c7, c10, 4\n\t"
			".word 0xe320f003" :: "r" (value));
	} else {
		value = S5PC110_PWR_MODE_SLEEP;
		writel(value, S5PC110_PWR_MODE);
	}

	printf("%s[%d] sleep success\n", __func__, __LINE__);
	return 0;
}

int do_sleep(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int mode = SLEEP_WFI;

	if (argc >= 2)
		mode = SLEEP_REGISTER;

	if (cpu_is_s5pc110())
		return s5pc110_sleep(mode);

	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	sleep,		CONFIG_SYS_MAXARGS,	1, do_sleep,
	"S5PC110 sleep",
	"sleep - sleep mode"
);
