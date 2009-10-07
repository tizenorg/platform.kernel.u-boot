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

static void __board_sleep_init(void) { }

void board_sleep_init(void)
	__attribute__((weak, alias("__board_sleep_init")));

static int s5pc110_sleep(void)
{
	unsigned int value;

	board_sleep_init();

	writel(0x0badcafe, S5PC110_INFORM0);

	value = readl(S5PC110_PWR_CFG);
	value &= ~S5PC110_CFG_STANDBYWFI_MASK;
	value |= S5PC110_CFG_STANDBYWFI_SLEEP;
	writel(value, S5PC110_PWR_CFG);

	value = readl(S5PC110_OTHERS);
	value |= S5PC110_OTHERS_SYSCON_INT_DISABLE;
	writel(value, S5PC110_OTHERS);

	printf("%s[%d] sleep enter\n\n", __func__, __LINE__);

	value = 0;
	asm("b	1f\n\t"
		".align 5\n\t"
		"1:\n\t"
		"mcr p15, 0, %0, c7, c10, 5\n\t"
		"mcr p15, 0, %0, c7, c10, 4\n\t"
		".word 0xe320f003" :: "r" (value));

	printf("%s[%d] sleep success\n", __func__, __LINE__);
	return 0;
}

int do_sleep(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (cpu_is_s5pc110())
		return s5pc110_sleep();

	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	sleep,		CONFIG_SYS_MAXARGS,	1, do_sleep,
	"S5PC110 sleep",
	"sleep - sleep mode"
);
