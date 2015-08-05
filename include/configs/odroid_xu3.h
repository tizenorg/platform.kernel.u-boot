/*
 * Copyright (C) 2013 Samsung Electronics
 * Hyungwon Hwang <human.hwang@samsung.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_ODROID_XU3_H
#define __CONFIG_ODROID_XU3_H

#include "exynos5420-common.h"

#define CONFIG_SYS_PROMPT		"ODROID-XU3 # "
#define CONFIG_IDENT_STRING		" for ODROID-XU3"

#define CONFIG_SIG
#define CONFIG_BOARD_COMMON

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_TEXT_BASE		0x43E00000

/* select serial console configuration */
#define CONFIG_SERIAL2			/* use SERIAL 2 */

#define TZPC_BASE_OFFSET		0x10000

/* FLASH and environment organization */
#define CONFIG_MMC_DEFAULT_DEV	0

#define CONFIG_CMD_MMC
#define CONFIG_CMD_DFU

#define CONFIG_NR_DRAM_BANKS	8
#define SDRAM_BANK_SIZE		(256UL << 20UL)	/* 256 MB */
/* Reserve the last 22 MiB for the secure firmware */
#define CONFIG_SYS_MEM_TOP_HIDE		(22UL << 20UL)
#define CONFIG_TZSW_RESERVED_DRAM_SIZE	CONFIG_SYS_MEM_TOP_HIDE

#define CONFIG_ENV_IS_IN_MMC

#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_SIZE			(SZ_1K * 16)
#define CONFIG_ENV_OFFSET		(SZ_1K * 3136)

#define CONFIG_SYS_INIT_SP_ADDR        (CONFIG_SYS_LOAD_ADDR - 0x1000000)

#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC2,115200n8\0"

/* USB */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_EXYNOS

#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_USB_DWC3_PHY_SAMSUNG

/* USB Composite download gadget - g_dnl */
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	2
#define CONFIG_USBDOWNLOAD_GADGET

/* TIZEN THOR downloader support */
#define CONFIG_CMD_THOR_DOWNLOAD
#define CONFIG_THOR_FUNCTION

#define CONFIG_DFU_FUNCTION
#define CONFIG_DFU_MMC
#define CONFIG_SYS_DFU_DATA_BUF_SIZE SZ_32M
#define DFU_DEFAULT_POLL_TIMEOUT 300

/* USB Samsung's IDs */
#define CONFIG_G_DNL_VENDOR_NUM 0x04E8
#define CONFIG_G_DNL_PRODUCT_NUM 0x6601
#define CONFIG_G_DNL_THOR_VENDOR_NUM CONFIG_G_DNL_VENDOR_NUM
#define CONFIG_G_DNL_THOR_PRODUCT_NUM 0x685D
#define CONFIG_G_DNL_UMS_VENDOR_NUM 0x0525
#define CONFIG_G_DNL_UMS_PRODUCT_NUM 0xA4A5
#define CONFIG_G_DNL_MANUFACTURER "Samsung"

#define CONFIG_CMD_USB_MASS_STORAGE
#define CONFIG_USB_DWC3
#define CONFIG_USB_DWC3_GADGET
#define CONFIG_USB_GADGET_MASS_STORAGE

#define CONFIG_MISC_COMMON
#define CONFIG_MISC_INIT_R
#define CONFIG_REBOOT_MODE

#include <samsung/platform_boot.h>

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND	"run autoboot"

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	PLATFORM_BOOT_INFO \
	"checkboard=\0" \
	"bootdelay=0\0" \
	"rootfstype=ext4\0" \
	"console=" CONFIG_DEFAULT_CONSOLE \
	"fdtfile=exynos5422-odroidxu3.dtb\0" \
	"mmcbootdev=0\0" \
	"mmcbootpart=1\0" \
	"mmcrootdev=0\0" \
	"mmcrootpart=2\0" \
	"dfu_usb_con=0\0" \
	"dfu_interface=mmc\0" \
	"dfu_device=" __stringify(CONFIG_MMC_DEFAULT_DEV) "\0" \
	"dfu_alt_system="CONFIG_DFU_ALT \
	"dfu_alt_info=Autoset by THOR/DFU command run.\0"

/* Partitions name for Tizen 3.0 */
#define PARTS_BOOT		"boot"
#define PARTS_ROOT		"rootfs"
#define PARTS_DATA		"system-data"
#define PARTS_USER		"user"
#define PARTS_MODULES		"modules"

#define CONFIG_DFU_ALT \
	"uImage fat 0 1;" \
	"zImage fat 0 1;" \
	"Image.itb fat 0 1;" \
	"uInitrd fat 0 1;" \
	"exynos5422-odroidxu3.dtb fat 0 1;" \
	""PARTS_BOOT" part 0 1;" \
	""PARTS_ROOT" part 0 2;" \
	""PARTS_DATA" part 0 3;" \
	""PARTS_USER" part 0 5;" \
	""PARTS_MODULES" part 0 6\0"

#define CONFIG_SET_DFU_ALT_INFO
#define CONFIG_SET_DFU_ALT_BUF_LEN	(SZ_1K)

#define CONFIG_DFU_ALT_BOOT_EMMC \
	"u-boot raw 0x3e 0x800 mmcpart 1;" \
	"bl1 raw 0x0 0x1e mmcpart 1;" \
	"bl2 raw 0x1e 0x1d mmcpart 1;" \
	"tzsw raw 0x83e 0x200 mmcpart 1;" \
	"params.bin raw 0x1880 0x20\0"

#define CONFIG_DFU_ALT_BOOT_SD \
	"u-boot raw 0x3f 0x800;" \
	"bl1 raw 0x1 0x1e;" \
	"bl2 raw 0x1f 0x1d;" \
	"tzsw raw 0x83f 0x200;" \
	"params.bin raw 0x1880 0x20\0"

/* FIXME: MUST BE REMOVED AFTER TMU IS TURNED ON */
#undef CONFIG_EXYNOS_TMU
#undef CONFIG_TMU_CMD_DTT

#endif	/* __CONFIG_H */
