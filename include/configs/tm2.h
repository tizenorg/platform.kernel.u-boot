/*
 * (C) Copyright 2016 Samsung Electronics
 *
 * Marek Szyprowski <m.szyprowski@samsung.com>
 *
 * Configuration for Samsung Exynos 5433-based TM2/TM2E boards.
 * Parts were derived from Hikey board configuration.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SAMSUNG_TM2_H
#define __SAMSUNG_TM2_H

#include <linux/sizes.h>

#define CONFIG_IDENT_STRING		"\nSamsung Exynos5433 TM2"

#define CONFIG_OF_LIBFDT

#define CONFIG_TPL_TM2

/* MMU Definitions */
#define CONFIG_SYS_CACHELINE_SIZE 64

#define CONFIG_PWM
#define CONFIG_ARCH_EXYNOS

#define CONFIG_EXYNOS7
#define CONFIG_CMD_BDI
#define CONFIG_CMD_UNZIP

/* Physical Memory Map */

/* MMC definition */
#define CONFIG_CMD_MMC
#define CONFIG_CMD_PART

/* SD/MMC configuration */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_DWMMC
#define CONFIG_EXYNOS_DWMMC
#define CONFIG_BOUNCE_BUFFER

/* GPT */
#define CONFIG_CMD_GPT
#define CONFIG_EFI_PARTITION
#define CONFIG_PARTITION_UUIDS

#define CONFIG_CMD_FS_GENERIC

/* DWC3 */
#define CONFIG_USB_DWC3
#define CONFIG_USB_DWC3_GADGET
#define CONFIG_USB_DWC3_PHY_SAMSUNG

/* USB gadget */
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	2

/* Downloader */
#define CONFIG_G_DNL_VENDOR_NUM		0x04E8
#define CONFIG_G_DNL_PRODUCT_NUM	0x6601
#define CONFIG_G_DNL_MANUFACTURER	"Samsung"
#define CONFIG_USB_GADGET_DOWNLOAD

/* DFU */
#define CONFIG_USB_FUNCTION_DFU
#define CONFIG_DFU_MMC
#define CONFIG_CMD_DFU
#define CONFIG_SYS_DFU_DATA_BUF_SIZE	SZ_32M
#define DFU_DEFAULT_POLL_TIMEOUT	300
#define DFU_MANIFEST_POLL_TIMEOUT       25000

/* THOR */
#define CONFIG_G_DNL_THOR_VENDOR_NUM	CONFIG_G_DNL_VENDOR_NUM
#define CONFIG_G_DNL_THOR_PRODUCT_NUM	0x685D
#define CONFIG_USB_FUNCTION_THOR
#define CONFIG_CMD_THOR_DOWNLOAD

/* require to avoid build break */
#define CONFIG_G_DNL_UMS_VENDOR_NUM	CONFIG_G_DNL_VENDOR_NUM
#define CONFIG_G_DNL_UMS_PRODUCT_NUM	0xA4A5

/* CONFIG_SYS_TEXT_BASE needs to align with where sboot loads kernel */
#define CONFIG_SYS_TEXT_BASE		0x20080000

#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x20000000
#define PHYS_SDRAM_1_SIZE		0xbf700000
#define GICD_BASE			0x11001000

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x7fff0)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x80000)
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_CONSOLE_INFO_QUIET

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (80 << 20))
#define CONFIG_SYS_BOOTM_LEN		(48 << 20)

/* Serial port Exynos S5P through the device model */
#define CONFIG_S5P_SERIAL
#define CONFIG_BAUDRATE			115200

/* Initial environment variables */
#define CONFIG_BOOTCOMMAND		"run modedetect"
#define CONFIG_EXTRA_ENV_SETTINGS	"dfu_alt_info=kernel part 0 9 offset 0x400;rootfs part 0 18;system-data part 0 19;user part 0 21\0" \
					"modedetect=if itest.l *0x10580044 == 0x81 || itest.l *0x10580044 == 0x1; then echo Thor mode enabled; run displayimg; thor 0 mmc 0; reset; else echo Booting kernel; run boarddetect; run loadkernel; bootm 0x30080000#$board; reset; fi\0" \
					"fdt_high=0xffffffffffffffff\0" \
					"bootargs=console=ttySAC1,115200 earlycon=exynos4210,0x14C20000 loglevel=7 root=/dev/mmcblk0p18 rootfstype=ext4 rootwait\0" \
					"boarddetect=if itest.l *0x138000b4 == 0x0063f9ff; then setenv board tm2e; elif itest.l *0x138000b4 == 0x0059f9ff; then setenv board tm2; else setenv board unknown; fi; echo Detected $board board\0" \
					"loadkernel=part start mmc 0 9 kernel_sect; part size mmc 0 9 kernel_size; mmc read 0x30000000 $kernel_sect $kernel_size\0" \
					"displayimg=unzip 200d0000 67000000; mw.l 138000b4 0059f9ff; mw.l 138001a0 67e10000; mw.l 13800200 00001680; mw.l 13801410 1; mw.l 13802040 e0000018; sleep 1; mw.l 13802040 e0000008\0"
#define CONFIG_BOOTDELAY		-2	/* force autoboot */
#define CONFIG_MENUPROMPT		"Loading, please wait..."

#define CONFIG_ENV_SIZE			0x1000
#define CONFIG_ENV_IS_NOWHERE

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_MAXARGS		16	/* max command args */

#endif /* __SAMSUNG_TM2_H */
