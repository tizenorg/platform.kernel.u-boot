/*
 * Copyright (C) 2013 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <libtizen.h>
#include <samsung/misc.h>
#include <errno.h>
#include <version.h>
#include <malloc.h>
#include <linux/sizes.h>
#include <asm/arch/cpu.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SET_DFU_ALT_INFO
void set_dfu_alt_info(void)
{
	size_t buf_size = CONFIG_SET_DFU_ALT_BUF_LEN;
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, buf_size);
	char *alt_info = "Settings not found!";
	char *status = "error!\n";
	char *alt_setting;
	char *alt_sep;
	int offset = 0;

	puts("DFU alt info setting: ");

	alt_setting = get_dfu_alt_boot();
	if (alt_setting) {
		setenv("dfu_alt_boot", alt_setting);
		offset = snprintf(buf, buf_size, "%s", alt_setting);
	}

	alt_setting = get_dfu_alt_system();
	if (alt_setting) {
		if (offset)
			alt_sep = ";";
		else
			alt_sep = "";

		offset += snprintf(buf + offset, buf_size - offset,
				    "%s%s", alt_sep, alt_setting);
	}

	if (offset) {
		alt_info = buf;
		status = "done\n";
	}

	setenv("dfu_alt_info", alt_info);
	puts(status);
}
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
void set_board_info(void)
{
	char info[64];

	snprintf(info, ARRAY_SIZE(info), "%u.%u", (s5p_cpu_rev & 0xf0) >> 4,
		 s5p_cpu_rev & 0xf);
	setenv("soc_rev", info);

	snprintf(info, ARRAY_SIZE(info), "%x", s5p_cpu_id);
	setenv("soc_id", info);

#ifdef CONFIG_REVISION_TAG
	snprintf(info, ARRAY_SIZE(info), "%x", get_board_rev());
	setenv("board_rev", info);
#endif
#ifdef CONFIG_OF_LIBFDT
	const char *bdtype = "";
	const char *bdname = CONFIG_SYS_BOARD;

#ifdef CONFIG_BOARD_TYPES
	bdtype = get_board_type();
	sprintf(info, "%s%s", bdname, bdtype);
	setenv("boardname", info);
#endif
	snprintf(info, ARRAY_SIZE(info),  "%s%x-%s%s.dtb",
		 CONFIG_SYS_SOC, s5p_cpu_id, bdname, bdtype);
	setenv("fdtfile", info);
#endif
}
#endif /* CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG */

#if defined(CONFIG_LCD_MENU) || defined(CONFIG_INTERACTIVE_CHARGER)
static int power_key_pressed(u32 reg)
{
	struct pmic *pmic;
	u32 status;
	u32 mask;

	pmic = pmic_get(KEY_PWR_PMIC_NAME);
	if (!pmic) {
		printf("%s: Not found\n", KEY_PWR_PMIC_NAME);
		return 0;
	}

	if (pmic_probe(pmic))
		return 0;

	if (reg == KEY_PWR_STATUS_REG)
		mask = KEY_PWR_STATUS_MASK;
	else
		mask = KEY_PWR_INTERRUPT_MASK;

	if (pmic_reg_read(pmic, reg, &status))
		return 0;

	return !!(status & mask);
}

int key_pressed(int key)
{
	int value;

	switch (key) {
	case KEY_POWER:
		value = power_key_pressed(KEY_PWR_INTERRUPT_REG);
		break;
	case KEY_VOLUMEUP:
		value = !gpio_get_value(KEY_VOL_UP_GPIO);
		break;
	case KEY_VOLUMEDOWN:
		value = !gpio_get_value(KEY_VOL_DOWN_GPIO);
		break;
	default:
		value = 0;
		break;
	}

	return value;
}

int check_keys(void)
{
	int keys = 0;

	if (key_pressed(KEY_POWER))
		keys += KEY_POWER;
	if (key_pressed(KEY_VOLUMEUP))
		keys += KEY_VOLUMEUP;
	if (key_pressed(KEY_VOLUMEDOWN))
		keys += KEY_VOLUMEDOWN;

	return keys;
}
#endif /* CONFIG_LCD_MENU || CONFIG_INTERACTIVE_CHARGER */

#ifdef CONFIG_LCD_MENU
/*
 * 0 BOOT_MODE_INFO
 * 1 BOOT_MODE_THOR
 * 2 BOOT_MODE_UMS
 * 3 BOOT_MODE_DFU
 * 4 BOOT_MODE_EXIT
 */
static char *
mode_name[BOOT_MODE_EXIT + 1][2] = {
	{"DEVICE", ""},
	{"THOR", "thor"},
	{"UMS", "ums"},
	{"DFU", "dfu"},
	{"GPT", "gpt"},
	{"ENV", "env"},
	{"EXIT", ""},
};

static char *
mode_info[BOOT_MODE_EXIT + 1] = {
	"info",
	"downloader",
	"mass storage",
	"firmware update",
	"restore",
	"default",
	"and run normal boot"
};

static char *
mode_cmd[BOOT_MODE_EXIT + 1] = {
	"",
	"thor 0 mmc 0",
	"ums 0 mmc 0",
	"dfu 0 mmc 0",
	"gpt write mmc 0 $partitions",
	"env default -a; saveenv",
	"",
};

static void display_board_info(void)
{
#ifdef CONFIG_GENERIC_MMC
	struct mmc *mmc = find_mmc_device(0);
#endif
	vidinfo_t *vid = &panel_info;

	lcd_set_position_cursor(4, 4);

	lcd_printf("%s\n\t", U_BOOT_VERSION);
	lcd_puts("\n\t\tBoard Info:\n");
#ifdef CONFIG_SYS_BOARD
	lcd_printf("\tBoard name: %s\n", CONFIG_SYS_BOARD);
#endif
#ifdef CONFIG_REVISION_TAG
	lcd_printf("\tBoard rev: %u\n", get_board_rev());
#endif
	lcd_printf("\tDRAM banks: %u\n", CONFIG_NR_DRAM_BANKS);
	lcd_printf("\tDRAM size: %u MB\n", gd->ram_size / SZ_1M);

#ifdef CONFIG_GENERIC_MMC
	if (mmc) {
		if (!mmc->capacity)
			mmc_init(mmc);

		lcd_printf("\teMMC size: %llu MB\n", mmc->capacity / SZ_1M);
	}
#endif
	if (vid)
		lcd_printf("\tDisplay resolution: %u x % u\n",
			   vid->vl_col, vid->vl_row);

	lcd_printf("\tDisplay BPP: %u\n", 1 << vid->vl_bpix);
}

static int mode_leave_menu(int mode)
{
	char *exit_option;
	char *exit_reset = "reset";
	char *exit_back = "back";
	cmd_tbl_t *cmd;
	int cmd_result;
	int leave;

	lcd_clear();

	switch (mode) {
	case BOOT_MODE_EXIT:
		return 1;
	case BOOT_MODE_INFO:
		display_board_info();
		exit_option = exit_back;
		leave = 0;
		break;
	default:
		cmd = find_cmd(mode_name[mode][1]);
		if (cmd) {
			printf("Enter: %s %s\n", mode_name[mode][0],
						 mode_info[mode]);
			lcd_printf("\n\n\t%s %s\n", mode_name[mode][0],
						    mode_info[mode]);
			lcd_puts("\n\tDo not turn off device before finish!\n");

			cmd_result = run_command(mode_cmd[mode], 0);

			if (cmd_result == CMD_RET_SUCCESS) {
				printf("Command finished\n");
				lcd_clear();
				lcd_printf("\n\n\t%s finished\n",
					   mode_name[mode][0]);

				exit_option = exit_reset;
				leave = 1;
			} else {
				printf("Command error\n");
				lcd_clear();
				lcd_printf("\n\n\t%s command error\n",
					   mode_name[mode][0]);

				exit_option = exit_back;
				leave = 0;
			}
		} else {
			lcd_puts("\n\n\tThis mode is not supported.\n");
			exit_option = exit_back;
			leave = 0;
		}
	}

	lcd_printf("\n\n\tPress POWER KEY to %s\n", exit_option);

	/* Clear PWR button Rising edge interrupt status flag */
	power_key_pressed(KEY_PWR_INTERRUPT_REG);

	/* Wait for PWR key */
	while (!key_pressed(KEY_POWER))
		mdelay(1);

	lcd_clear();
	return leave;
}

static void display_download_menu(int mode)
{
	char *selection[BOOT_MODE_EXIT + 1];
	int i;

	for (i = 0; i <= BOOT_MODE_EXIT; i++)
		selection[i] = "[  ]";

	selection[mode] = "[=>]";

	lcd_clear();
	lcd_printf("\n\n\t\tDownload Mode Menu\n\n");

	for (i = 0; i <= BOOT_MODE_EXIT; i++)
		lcd_printf("\t%s  %s - %s\n\n", selection[i],
						mode_name[i][0],
						mode_info[i]);
}

static void download_menu(void)
{
	int mode = 0;
	int last_mode = 0;
	int run;
	int key = 0;
	int timeout = 15; /* sec */
	int i;

	display_download_menu(mode);

	lcd_puts("\n");

	/* Start count if no key is pressed */
	while (check_keys())
		continue;

	while (timeout--) {
		lcd_printf("\r\tNormal boot will start in: %2.d seconds.",
			   timeout);

		/* about 1000 ms in for loop */
		for (i = 0; i < 10; i++) {
			mdelay(100);
			key = check_keys();
			if (key)
				break;
		}
		if (key)
			break;
	}

	if (!key) {
		lcd_clear();
		return;
	}

	while (1) {
		run = 0;

		if (mode != last_mode)
			display_download_menu(mode);

		last_mode = mode;
		mdelay(200);

		key = check_keys();
		switch (key) {
		case KEY_POWER:
			run = 1;
			break;
		case KEY_VOLUMEUP:
			if (mode > 0)
				mode--;
			break;
		case KEY_VOLUMEDOWN:
			if (mode < BOOT_MODE_EXIT)
				mode++;
			break;
		default:
			break;
		}

		if (run) {
			if (mode_leave_menu(mode))
				run_command("reset", 0);

			display_download_menu(mode);
		}
	}

	lcd_clear();
}

#ifdef CONFIG_INTERACTIVE_CHARGER
static int interactive_charger(int command)
{
	int display_timeout_ms = 1000 * CHARGE_DISPLAY_TIMEOUT_SEC;
	int display_time_ms = 0;
	int animation_step_ms = 500;
	int no_charger_anim_timeout_ms = NO_CHARGER_ANIM_TIMEOUT_SEC * 1000;
	int power_key_timeout_ms = CHARGE_PWR_KEY_RESET_TIMEOUT * 1000;
	int power_key_time_ms = 0;
	int keys_exit_timeout_ms = CHARGE_KEYS_EXIT_TIMEOUT * 1000;
	int keys_exit_time_ms = 0;
	unsigned int soc_boot = CHARGE_TRESHOLD_BOOT;
	unsigned int soc = 0;
	int clear_screen = 0;
	int chrg = 0;
	int keys;
	int i;

	/* Enable charger */
	if (charger_enable()) {
		puts("Can't enable charger");
		return CMD_RET_FAILURE;
	}

	draw_charge_screen();

	/* Clear PWR button Rising edge interrupt status flag */
	power_key_pressed(KEY_PWR_INTERRUPT_REG);

	puts("Starting interactive charger (Ctrl+C to break).\n");
	puts("This mode can be also finished by keys: VOLUP + VOLDOWN.\n");

	while (1) {
		/* Check state of charge */
		if (battery_state(&soc)) {
			puts("Fuel Gauge Error\n");
			lcd_clear();
			return CMD_RET_FAILURE;
		}

		if (display_time_ms <= display_timeout_ms) {
			draw_battery_state(soc);
			draw_charge_animation();

			mdelay(animation_step_ms);
			display_time_ms += animation_step_ms;
			clear_screen = 1;
		} else {
			/* Clear screen only once */
			if (clear_screen) {
				lcd_clear();
				clear_screen = 0;
			}

			/* Check key if lcd is clear */
			keys = check_keys();
			if (keys) {
				display_time_ms = 0;
				draw_battery_screen();
				draw_battery_state(soc);
				draw_charge_screen();
				printf("Key pressed. Battery SOC: %.2d %%.\n",
				       soc);
			}
		}

		/* Check exit conditions */
		while (check_keys() == CHARGE_KEYS_EXIT) {
			/* 50 ms delay in check_keys() */
			keys_exit_time_ms += 50;
			if (keys_exit_time_ms >= keys_exit_timeout_ms)
				break;
		}

		if (ctrlc() || keys_exit_time_ms >= keys_exit_timeout_ms) {
			puts("Charge mode: aborted by user.\n");
			lcd_clear();
			return CMD_RET_SUCCESS;
		}

		keys_exit_time_ms = 0;

		/* Turn off if no charger */
		chrg = charger_type();
		if (!chrg) {
			puts("Charger disconnected\n");
			draw_battery_screen();
			draw_battery_state(soc);

			for (i = 0; i < no_charger_anim_timeout_ms; i += 600) {
				mdelay(700);
				draw_connect_charger_animation();

				if (ctrlc())
					return CMD_RET_SUCCESS;

				chrg = charger_type();
				if (chrg) {
					display_time_ms = 0;
					draw_battery_screen();
					draw_battery_state(soc);
					draw_charge_screen();
					puts("Charger connected\n");
					break;
				}
			}

			if (!chrg) {
				if (command == CMD_BATTERY_BOOT_CHECK) {
					puts("Power OFF.");
					board_poweroff();
				} else {
					puts("Please connect charger.\n");
					return CMD_RET_SUCCESS;
				}
			}
		}

		/* Check boot ability with 5% of reserve */
		if (soc >= soc_boot) {
			while (power_key_pressed(KEY_PWR_STATUS_REG)) {
				mdelay(100);
				power_key_time_ms += 100;
				/* Check for pwr key press */
				if (power_key_time_ms >= power_key_timeout_ms)
					run_command("reset", 0);
			}
		}

		power_key_time_ms = 0;
	}

	return CMD_RET_SUCCESS;
}

static int battery(int command)
{
	unsigned soc;
	unsigned soc_boot = CHARGE_TRESHOLD_BOOT;
	int no_charger_timeout = NO_CHARGER_TIMEOUT_SEC;
	int no_charger_anim_timeout = NO_CHARGER_ANIM_TIMEOUT_SEC;
	int chrg = 0;
	int keys = 0;
	char *pwr_mode_info = "Warning!\n"
			      "Battery charge level was too low and\n"
			      "device is now in LOW POWER mode.\n"
			      "This mode is not recommended for boot system!\n";

	if (!battery_present()) {
		puts("Battery not present.\n");
		return CMD_RET_FAILURE;
	}

	if (battery_state(&soc)) {
		puts("Fuel Gauge Error\n");
		return CMD_RET_FAILURE;
	}

	printf("Battery SOC: %.2d %%.\n", soc);

	switch (command) {
	case CMD_BATTERY_BOOT_CHECK:
		/*
		 * If (soc > soc_boot) then don't waste
		 * a time for draw battery screen.
		*/
		if (soc >= soc_boot)
			return CMD_RET_SUCCESS;

		/* Set low power mode only if do boot check */
		board_low_power_mode();
		puts("Battery SOC (< 20%) is too low for boot.\n");
		break;
	case CMD_BATTERY_STATE:
		lcd_clear();
		draw_battery_screen();
		draw_battery_state(soc);

		/* Wait some time before clear battery screen */
		mdelay(2000);
		lcd_clear();
		return CMD_RET_SUCCESS;
	case CMD_BATTERY_CHARGE:
		lcd_clear();
		break;
	default:
		return CMD_RET_SUCCESS;
	}

	draw_battery_screen();
	draw_battery_state(soc);

	do {
		/* Check charger */
		chrg = charger_type();
		if (chrg) {
			printf("CHARGER TYPE: %d\n", chrg);
			draw_battery_screen();
			draw_battery_state(soc);
			break;
		}

		if (no_charger_anim_timeout > 0)
			draw_connect_charger_animation();

		mdelay(800);

		/* Check exit conditions */
		keys = check_keys();
		if (keys) {
			no_charger_anim_timeout = NO_CHARGER_ANIM_TIMEOUT_SEC;
			draw_battery_screen();
			draw_battery_state(soc);
		}

		if (keys == CHARGE_KEYS_EXIT || ctrlc()) {
			puts("Charge mode aborted by user.\n");
			goto warning;
		}

		no_charger_anim_timeout--;
		if (!no_charger_anim_timeout)
			lcd_clear();

	} while (no_charger_timeout--);

	/* there is no charger and soc (%) is too low - POWER OFF */
	if (!chrg) {
		puts("No charger connected!\n");

		if (command == CMD_BATTERY_BOOT_CHECK) {
			puts("Power OFF.");
			board_poweroff();
		} else {
			puts("Please connect charger.\n");
			return CMD_RET_SUCCESS;
		}
	}

	/* Charger connected - enable charge mode */
	if (interactive_charger(command))
		printf("Charge failed\n");

warning:
	/* Charge mode aborted or failed */
	if (command == CMD_BATTERY_BOOT_CHECK)
		puts(pwr_mode_info);

	return CMD_RET_SUCCESS;
}
#endif

void check_boot_mode(void)
{
	int pwr_key;

#ifdef CONFIG_INTERACTIVE_CHARGER
	battery(CMD_BATTERY_BOOT_CHECK);
#endif
	pwr_key = power_key_pressed(KEY_PWR_STATUS_REG);
	if (!pwr_key)
		return;

	/* Clear PWR button Rising edge interrupt status flag */
	power_key_pressed(KEY_PWR_INTERRUPT_REG);

	if (key_pressed(KEY_VOLUMEUP))
		download_menu();
	else if (key_pressed(KEY_VOLUMEDOWN))
		mode_leave_menu(BOOT_MODE_THOR);
}

void keys_init(void)
{
	/* Set direction to input */
	gpio_request(KEY_VOL_UP_GPIO, "volume-up");
	gpio_request(KEY_VOL_DOWN_GPIO, "volume-down");
	gpio_direction_input(KEY_VOL_UP_GPIO);
	gpio_direction_input(KEY_VOL_DOWN_GPIO);
}
#endif /* CONFIG_LCD_MENU */

#ifdef CONFIG_CMD_BMP
void draw_logo(void)
{
	int x, y;
	ulong addr;

	addr = panel_info.logo_addr;
	if (!addr) {
		error("There is no logo data.");
		return;
	}

	if (panel_info.vl_width >= panel_info.logo_width) {
		x = ((panel_info.vl_width - panel_info.logo_width) >> 1);
		x += panel_info.logo_x_offset; /* For X center align */
	} else {
		x = 0;
		printf("Warning: image width is bigger than display width\n");
	}

	if (panel_info.vl_height >= panel_info.logo_height) {
		y = ((panel_info.vl_height - panel_info.logo_height) >> 1);
		y += panel_info.logo_y_offset; /* For Y center align */
	} else {
		y = 0;
		printf("Warning: image height is bigger than display height\n");
	}

	lcd_clear();

	bmp_display(addr, x, y);
}
#endif /* CONFIG_CMD_BMP */
