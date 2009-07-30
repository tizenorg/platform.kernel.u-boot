/*
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * Configuation settings for the SAMSUNG Universal (s5pc100) board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H
#include <asm/sizes.h>

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARMCORTEXA8	1	/* This is an ARM V7 CPU core */
#define CONFIG_SAMSUNG		1	/* in a SAMSUNG core */
#define CONFIG_S5PC1XX		1	/* which is in a S5PC1XX Family */
#define CONFIG_S5PC100		1	/* which is in a S5PC100 */
#define CONFIG_S5PC110		1	/* which is in a S5PC100 */
#define CONFIG_UNIVERSAL	1	/* working with Universal */

#include <asm/arch/cpu.h>		/* get chip and board defs */

#define CONFIG_ARCH_CPU_INIT

#define CONFIG_SYS_SDRAM_BASE	0x20000000

/* input clock of PLL: Universal has 12MHz/24MHz input clock at S5PC100/C110 */
#define CONFIG_SYS_CLK_FREQ_C100	12000000
#define CONFIG_SYS_CLK_FREQ_C110	24000000

#define CONFIG_MEMORY_UPPER_CODE

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* Clock Defines */
#define V_OSCK		26000000	/* Clock output from T2 */
#define V_SCLK		(V_OSCK >> 1)

/*
 * Architecture magic and machine type
 */
#define MACH_TYPE	3000

#define CONFIG_DISPLAY_CPUINFO

#undef CONFIG_SKIP_RELOCATE_UBOOT

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 1024 * 1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes for initial data */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL2          1	/* we use SERIAL 2 on S5PC100 */

#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser	*/
#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif

#define CONFIG_CMDLINE_EDITING

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200
#define CONFIG_L2_OFF

/* It should define before config_cmd_default.h */
#define CONFIG_SYS_NO_FLASH		1
/***********************************************************
 * Command definition
 ***********************************************************/
#include <config_cmd_default.h>

#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_BOOTD
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_XIMG
#undef CONFIG_CMD_NAND
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_NET
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_ONENAND
#define CONFIG_CMD_MTDPARTS

#define CONFIG_SYS_64BIT_VSPRINTF	1

#define CONFIG_BOOTDELAY	1

#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS

/* Actual modem binary size is 16MiB. Add 2MiB for bad block handling */
#define MTDIDS_DEFAULT 		"onenand0=samsung-onenand"
#define MTDPARTS_DEFAULT	"mtdparts=samsung-onenand:256k(bootloader)"\
				",128k@0x40000(params)"\
				",3m@0x60000(kernel)"\
				",18m@0x360000(modem)"\
				",-(UBI)"
#define MTDPARTS_DEFAULT_4KB	"mtdparts=samsung-onenand:256k(bootloader)"\
				",256k@0x40000(params)"\
				",3m@0x80000(kernel)"\
				",18m@0x380000(modem)"\
				",-(UBI)"

#define NORMAL_MTDPARTS_DEFAULT MTDPARTS_DEFAULT

#if 1
#define CONFIG_BOOTCOMMAND	"run ubifsboot"
#else
#define CONFIG_BOOTCOMMAND	"bootm 0x21008000"
#endif

#define CONFIG_RAMDISK_BOOT	"root=/dev/ram0 rw rootfstype=ext2" \
		" console=ttySAC2,115200n8" \
		" ${meminfo}"

#define CONFIG_COMMON_BOOT	"console=ttySAC2,115200n8" \
		" ${meminfo} " \
		" ${mtdparts}"

#define CONFIG_BOOTARGS	"root=/dev/mtdblock5 ubi.mtd=4" \
		" rootfstype=cramfs " CONFIG_COMMON_BOOT

#define CONFIG_UPDATEB	"updateb=onenand erase 0x0 0x40000;" \
			" onenand write 0x22008000 0x0 0x40000\0"

#define CONFIG_ENV_OVERWRITE
#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_UPDATEB \
	"updatek=onenand erase 0x60000 0x300000;" \
	" onenand write 0x21008000 0x60000 0x300000\0" \
	"updateu=onenand erase 0x01560000 0x1eaa0000;" \
	" onenand write 0x22000000 0x1260000 0x8C0000\0" \
	"bootk=onenand read 0x20007FC0 0x80000 0x300000;" \
	" bootm 0x20007FC0\0" \
	"flashboot=set bootargs root=/dev/mtdblock${bootblock}" \
	 " rootfstype=${rootfstype}" \
	 " ubi.mtd=${ubiblock} ${opts} " CONFIG_COMMON_BOOT "; run bootk\0" \
	"ubifsboot=set bootargs root=ubi0!rootfs rootfstype=ubifs" \
	 " ubi.mtd=${ubiblock} ${opts} " CONFIG_COMMON_BOOT "; run bootk\0" \
	"boottrace=setenv opts initcall_debug; run bootcmd\0" \
	"android=set bootargs root=ubi0!ramdisk ubi.mtd=${ubiblock}" \
	 " rootfstype=ubifs init=/init.sh " CONFIG_COMMON_BOOT "; run bootk\0" \
	"nfsboot=set bootargs root=/dev/nfs ubi.mtd=${ubiblock}" \
	 " nfsroot=${nfsroot},nolock ip=${ipaddr}:${serverip}:${gatewayip}:" \
	 " ${netmask}:nowplus:usb0:off " CONFIG_COMMON_BOOT "; run bootk\0" \
	"ramboot=set bootargs " CONFIG_RAMDISK_BOOT \
	 " initrd=0x23000000,8M ramdisk=8192\0" \
	"rootfstype=cramfs\0" \
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"meminfo=mem=208M\0" \
	"nfsroot=/nfsroot/arm\0" \
	"bootblock=5\0" \
	"ubiblock=4\0" \
	"ubi=enabled"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory */
#define CONFIG_SYS_PROMPT	"Universal # "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE	256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE	384	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE	/* memtest works on	      */
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x4e00000)

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x4800000)

#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2       /* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			2085900		/* at PCLK 66.75MHz */

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	0x40000		/* regular stack 256KB */

/*******************************
 Support Clock Settings(APLL)
 *******************************
 ARMCLK		HCLKD0		PCLKD0
 -------------------------------
 667		166			83
 600		150			75
 533		133			66
 500		166			66
 467		117			59
 400		100			50
 *******************************/

#define CONFIG_CLK_667_166_83
/*#define CONFIG_CLK_666_166_66*/
/*#define CONFIG_CLK_600_150_75*/
/*#define CONFIG_CLK_533_133_66*/
/*#define CONFIG_CLK_500_166_66*/
/*#define CONFIG_CLK_467_117_59*/
/*#define CONFIG_CLK_400_100_50*/

/* Universal has 2 banks of DRAM, but swap the bank */
#define CONFIG_NR_DRAM_BANKS	2
#define CONFIG_S5PC1XX_SWAP_MEMORY_BANK		1
#define PHYS_SDRAM_1		CONFIG_SYS_SDRAM_BASE	/* OneDRAM Bank #0 */
#define PHYS_SDRAM_1_SIZE	0x05000000		/* 80 MB in Bank #0 */
#define PHYS_SDRAM_2		0x28000000		/* MobileDDR Bank #1 */
#define PHYS_SDRAM_2_SIZE	0x08000000		/* 128 MB in Bank #1 */

#define CONFIG_SYS_MONITOR_BASE	0x00000000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_SYS_MONITOR_LEN		SZ_256K	/* Reserve 2 sectors */

/* OneNAND IPL uses 8KiB */
#define CONFIG_ONENAND_START_PAGE	4

#define CONFIG_ENV_IS_IN_ONENAND	1
#define CONFIG_ENV_SIZE			0x20000
#define CONFIG_ENV_ADDR			0x40000
#define CONFIG_ENV_OFFSET		0x40000

#define CONFIG_USE_ONENAND_BOARD_INIT
#define CONFIG_SYS_ONENAND_BASE		0xB0000000

#define CONFIG_DOS_PARTITION	1

#define CONFIG_MISC_INIT_R

/* I2C */
#define CONFIG_DRIVER_S5PC1XX_I2C
#define CONFIG_HARD_I2C		1
#define CONFIG_SYS_I2C_SPEED	50000
#define CONFIG_SYS_I2C_SLAVE	0xFE
#define CONFIG_SYS_I2C_0	1	

/* USB Downloader */
#define CONFIG_CMD_USBDOWN
#define CONFIG_SAMSUNG_USB
#define CONFIG_OTG_CLK_OSCC
#define CONFIG_SYS_DOWN_ADDR	CONFIG_SYS_SDRAM_BASE
#define CONFIG_RAMDISK_ADDR	0x23000000

/* LCD */
#if 0		/* For LCD test */
#define CONFIG_LCD		1
#define CONFIG_S5PC1XXFB	1
#define BOARD_LATE_INIT
#endif

#endif	/* __CONFIG_H */
