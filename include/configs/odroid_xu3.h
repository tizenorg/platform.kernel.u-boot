/*
 * Copyright (C) 2013 Samsung Electronics
 * Hyungwon Hwang <human.hwang@samsung.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_ODROID_XU3_H
#define __CONFIG_ODROID_XU3_H

#include "exynos5-dt.h"
#define CONFIG_DEFAULT_DEVICE_TREE	exynos5422-odroidxu3

#define CONFIG_SYS_PROMPT		"ODROID-XU3 # "

/* High Level Configuration Options */
#define CONFIG_SAMSUNG			/* in a SAMSUNG core */
#define CONFIG_S5P			/* S5P Family */
#define CONFIG_EXYNOS5			/* which is in a Exynos5 Family */
#define CONFIG_ARCH_EXYNOS5	        /* which is in a Exynos5 Family */
#define CONFIG_CPU_EXYNOS5420		/* which is in a Exynos5420 */
#define CONFIG_CPU_EXYNOS5422		/* which is in a Exynos5422 */
#include <asm/arch/cpu.h>		/* get chip and board defs */

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Power Management is enabled */
#define CONFIG_PM
#define CONFIG_PM_VDD_ARM	1.00
#define CONFIG_PM_VDD_KFC	1.0250
#define CONFIG_PM_VDD_INT	1.00
#define CONFIG_PM_VDD_G3D	1.00
#define CONFIG_PM_VDD_MIF	1.10

/* Keep L2 Cache Disabled */
#define CONFIG_SYS_DCACHE_OFF

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_TEXT_BASE		0x43E00000
#define CONFIG_SPL_TEXT_BASE		0x02027000

/* input clock of PLL: SMDK5422 has 24MHz input clock */
#define CONFIG_SYS_CLK_FREQ		24000000

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_CMDLINE_EDITING

/* iRAM information */
#define CONFIG_PHY_IRAM_BASE            (0x02020000)
#define CONFIG_PHY_IRAM_NS_BASE         (CONFIG_PHY_IRAM_BASE + 0x53000)

/* Power Down Modes */
#define S5P_CHECK_SLEEP			0x00000BAD
#define S5P_CHECK_DIDLE			0xBAD00000
#define S5P_CHECK_LPA			0xABAD0000

/* Offset for OM status registers */
#define OM_STATUS_OFFSET                0x0

/* select serial console configuration */
#define CONFIG_SERIAL_MULTI
#define CONFIG_SERIAL2			/* use SERIAL 2 */
#define CONFIG_BAUDRATE			115200
#define EXYNOS5_DEFAULT_UART_OFFSET	0x010000

#define TZPC_BASE_OFFSET		0x10000

/* SD/MMC configuration */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_SDHCI
#define CONFIG_S5P_MSHC
#define CONFIG_S5P_SDHCI
#define CONFIG_MMC_64BIT_BUS

#if defined(CONFIG_S5P_MSHC)
#define CONFIG_MMC_EARLY_INIT
#define MMC_MAX_CHANNEL		4
#define USE_MMC0
#define USE_MMC2

#define PHASE_DEVIDER		4

#define SDR_CH0			0x03030002
#define DDR_CH0			0x03020001

#define SDR_CH2			0x03020001
#define DDR_CH2			0x03030002

#define SDR_CH4			0x0
#define DDR_CH4			0x0
#endif

/*
 * Boot configuration
 */
#define BOOT_MMCSD		0x3
#define BOOT_EMMC		0x6
#define BOOT_EMMC_4_4		0x7

/*
 *  Boot device
 */
#define SDMMC_CH2		0x0
#define SDMMC_CH0		0x4
#define EMMC			0x14

#define CONFIG_BOARD_EARLY_INIT_F

/* PWM */
#define CONFIG_PWM

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Command definition*/
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_ELF
#define CONFIG_CMD_MMC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_BOOTZ

#define CONFIG_BOOTDELAY		3
#define CONFIG_ZERO_BOOTDELAY_CHECK

/* USB */
#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_EXYNOS
#define CONFIG_USB_STORAGE

/* OHCI : Host 1.0 */
#define CONFIG_USB_OHCI

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */

#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		384	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x5E00000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x3E00000)

#define CONFIG_SYS_HZ			1000

#define CONFIG_RD_LVL

/* Stack sizes */
#define CONFIG_STACKSIZE		(256 << 10)	/* 256KB */

#define CONFIG_NR_DRAM_BANKS	8

#define SDRAM_BANK_SIZE		(256UL << 20UL)	/* 256 MB */

#define CONFIG_SYS_MONITOR_BASE	0x00000000

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH
#undef CONFIG_CMD_IMLS
#define CONFIG_IDENT_STRING		" for ODROID-XU3"

#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0

/* Configuration of ENV size on mmc */
#define CONFIG_ENV_SIZE		(16 << 10)	/* 16 KB */

/* Configuration of ROOTFS_ATAGS */
#define CONFIG_ROOTFS_ATAGS
#ifdef CONFIG_ROOTFS_ATAGS
#ifdef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_EXTRA_ENV_SETTINGS
#endif
#define CONFIG_EXTRA_ENV_SETTINGS       "rootfslen= 100000"
#endif

/* U-boot copy size from boot Media to DRAM.*/
#define CONFIG_DOS_PARTITION
#define CFG_PARTITION_START	0x4000000
#define CONFIG_IRAM_STACK	0x02074000

#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_LOAD_ADDR - 0x1000000)

/* Ethernet Controllor Driver */
#ifdef CONFIG_CMD_NET
#define CONFIG_NET_MULTI
#endif /*CONFIG_CMD_NET*/

/* Base address for secondary boot information */
#define CONFIG_SECONDARY_BOOT_INFORM_BASE	(CONFIG_SYS_TEXT_BASE - 0x8)

/* Offset for pmu reset status */
#define RST_STAT_OFFSET			0x404

/* RST_STAT */
#define WRESET			(1 << 10)
#define SYS_WDTRESET		(1 << 9)
#define CONFIG_SKIP_LOWLEVEL_INIT

/* FIXME: MUST BE REMOVED AFTER TMU IS TURNED ON */
#undef CONFIG_EXYNOS_TMU
#undef CONFIG_TMU_CMD_DTT

#define CONFIG_FDTDEC_MEMORY
#define CONFIG_SYS_MALLOC_F_LEN (256)

#undef CONFIG_ARCH_EARLY_INIT_R
#undef CONFIG_EXYNOS_SPL
#undef CONFIG_SILENT_CONSOLE
#undef CONFIG_CROS_EC
#undef CONFIG_CROS_EC_SPI
#undef CONFIG_CROS_EC_I2C
#undef CONFIG_CROS_EC_KEYB
#undef CONFIG_CMD_CROS_EC
#undef CONFIG_KEYBOARD
#undef CONFIG_SPI_BOOTING
#undef CONFIG_ENV_IS_IN_SPI_FLASH
#undef CONFIG_SPI_FLASH
#undef CONFIG_EXYNOS_SPI
#undef CONFIG_CMD_SF
#undef CONFIG_CMD_SPI
#undef CONFIG_SPI_FLASH_WINBOND
#undef CONFIG_SPI_FLASH_GIGADEVICE
#undef CONFIG_OF_SPI

#endif	/* __CONFIG_H */
