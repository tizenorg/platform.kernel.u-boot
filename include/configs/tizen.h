/*
 * Copyright (C) 2014 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TIZEN_H
#define __CONFIG_TIZEN_H

#include <configs/exynos4-common.h>
#include <samsung/platform_setup.h>
#include <samsung/platform_boot.h>

#define CONFIG_SYS_PROMPT	"Exynos4412 # "	/* Monitor Command Prompt */

#undef CONFIG_DEFAULT_DEVICE_TREE
#define CONFIG_DEFAULT_DEVICE_TREE	exynos4412-odroid

#define CONFIG_OF_MULTI
#define CONFIG_FDTDEC_MEMORY

/* Arch number for Trats2 */
#define MACH_TYPE_TRATS2	3766
/* Arch number for Odroid */
#define MACH_TYPE_ODROIDX	4289

#define CONFIG_SYS_L2CACHE_OFF
#ifndef CONFIG_SYS_L2CACHE_OFF
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE	0x10502000
#endif

#define CONFIG_NR_DRAM_BANKS	8
#define CONFIG_SYS_SDRAM_BASE	0x40000000
#define SDRAM_BANK_SIZE		(256 << 20)	/* 256 MB */

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x3E00000)
#define CONFIG_SYS_TEXT_BASE		0x43e00000


/* set serial baudrate */
#define CONFIG_BAUDRATE			115200

/* Console configuration */
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define CONFIG_CMD_BOOTZ
#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE
#define CONFIG_BOOTARGS			"Please use defined boot"
#define CONFIG_BOOTCOMMAND		"run autoboot"
#define CONFIG_CONSOLE_TTY1		" console=ttySAC1,115200n8"
#define CONFIG_CONSOLE_TTY2		" console=ttySAC2,115200n8"

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_LOAD_ADDR \
					- GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MEM_TOP_HIDE	(SZ_1M)	/* ram console */

#define CONFIG_SYS_MONITOR_BASE	0x00000000

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		CONFIG_MMC_DEFAULT_DEV
#define CONFIG_ENV_SIZE			(SZ_1K * 16)
#define CONFIG_ENV_OFFSET		(SZ_1K * 3136)
#define CONFIG_ENV_OVERWRITE

#define CONFIG_SET_DFU_ALT_INFO
#define CONFIG_SET_DFU_ALT_BUF_LEN		SZ_1K

#define CONFIG_EXTRA_ENV_SETTINGS \
	PLATFORM_SETUP_INFO \
	PLATFORM_BOOT_INFO \
	"checkboard=" \
		"if test ${boardname} = trats2; then " \
			"setenv console" CONFIG_CONSOLE_TTY2";" \
		"else " \
			"setenv console" CONFIG_CONSOLE_TTY1";" \
		"fi;\0" \
	"mmcbootdev=0\0" \
	"mmcrootdev=0\0" \
	"rootfstype=ext4\0" \
	"bootdelay=0\0" \
	"dfu_alt_info=Autoset by THOR/DFU command run.\0" \
	"dfu_usb_con=0\0" \
	"dfu_interface=mmc\0" \
	"dfu_device=" __stringify(CONFIG_MMC_DEFAULT_DEV) "\0" \
	"consoleon=run checkboard; saveenv; reset\0" \
	"consoleoff=set console console=ram; saveenv; reset\0"

/* I2C */
#include <asm/arch/gpio.h>

#define CONFIG_CMD_I2C

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_S3C24X0
#define CONFIG_SYS_I2C_S3C24X0_SPEED	100000
#define CONFIG_SYS_I2C_S3C24X0_SLAVE	0
#define CONFIG_MAX_I2C_NUM		8
#define CONFIG_SYS_I2C_SOFT
#define CONFIG_SYS_I2C_SOFT_SPEED	50000
#define CONFIG_SYS_I2C_SOFT_SLAVE	0x00
#define I2C_SOFT_DECLARATIONS2
#define CONFIG_SYS_I2C_SOFT_SPEED_2     50000
#define CONFIG_SYS_I2C_SOFT_SLAVE_2     0x00
#define CONFIG_SOFT_I2C_READ_REPEATED_START
#define CONFIG_SYS_I2C_INIT_BOARD

#ifndef __ASSEMBLY__
int get_soft_i2c_scl_pin(void);
int get_soft_i2c_sda_pin(void);
#endif
#define CONFIG_SOFT_I2C_GPIO_SCL	get_soft_i2c_scl_pin()
#define CONFIG_SOFT_I2C_GPIO_SDA	get_soft_i2c_sda_pin()

/* POWER */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_MAX77686
#define CONFIG_POWER_PMIC_MAX77693
#define CONFIG_POWER_MUIC_MAX77693
#define CONFIG_POWER_FG_MAX77693
#define CONFIG_POWER_BATTERY_TRATS2
#define CONFIG_CMD_POWEROFF

/**
 * Platform setup command (GPT and DFU)
 * Define setup and part num for some static data
 * should be changed to linked list in the future.
 */
#define CONFIG_PLATFORM_SETUP
#define CONFIG_PLATFORM_MAX_PART_NUM	32
#define CONFIG_PLATFORM_MAX_SETUP_NUM	6

/* GPT */
#define CONFIG_RANDOM_UUID

/* Security subsystem - enable hw_rand() */
#define CONFIG_EXYNOS_ACE_SHA
#define CONFIG_LIB_HW_RAND

#define CONFIG_CMD_GPIO

/*
 * Supported Odroid boards: X3, U3
 * TODO: Add Odroid X support
 */
#define CONFIG_MISC_COMMON
#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
#define CONFIG_BOARD_TYPES
#define CONFIG_MISC_INIT_R
#undef CONFIG_REVISION_TAG
#define CONFIG_REBOOT_MODE

#define CONFIG_TIZEN
#define CONFIG_SIG

/* Charge battery with interactive LCD animation */
#define CONFIG_INTERACTIVE_CHARGER
#define CONFIG_CMD_BATTERY

/* Download menu - Samsung common */
#define CONFIG_LCD_MENU
#define CONFIG_LCD_MENU_BOARD

/*
 * Download menu - definitions for check keys
 * This is valid only for Trats2 and will be reworked
 */
#ifndef __ASSEMBLY__
#include <power/max77686_pmic.h>

#define KEY_PWR_PMIC_NAME		"MAX77686_PMIC"
#define KEY_PWR_STATUS_REG		MAX77686_REG_PMIC_STATUS1
#define KEY_PWR_STATUS_MASK		(1 << 0)
#define KEY_PWR_INTERRUPT_REG		MAX77686_REG_PMIC_INT1
#define KEY_PWR_INTERRUPT_MASK		(1 << 1)

#define KEY_VOL_UP_GPIO			EXYNOS4X12_GPIO_X22
#define KEY_VOL_DOWN_GPIO		EXYNOS4X12_GPIO_X33
#endif /* __ASSEMBLY__ */

/* LCD console */
#define LCD_BPP				LCD_COLOR16
#define CONFIG_SYS_WHITE_ON_BLACK

/* LCD */
#define CONFIG_EXYNOS_FB
#define CONFIG_LCD
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP
#define CONFIG_FB_ADDR		0x52504000
#define CONFIG_S6E8AX0
#define CONFIG_EXYNOS_MIPI_DSIM
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE ((500 * 160 * 4) + 54)

#define LCD_XRES	720
#define LCD_YRES	1280

#endif	/* __CONFIG_H */