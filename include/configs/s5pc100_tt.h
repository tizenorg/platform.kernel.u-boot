/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <gj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * Configuation settings for the SAMSUNG SMDK6400(mDirac-III) board.
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
#define CONFIG_S5PC100		1	/* which is in a S5PC100 */
#define CONFIG_S5PC1XX		1	/* which is in a S5PC1XX Family */
#define CONFIG_S5PC100_TT	1	/* working with TT */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/tt.h>

#define CONFIG_SYS_SDRAM_BASE	0x20000000

/* input clock of PLL: TickerTape has 12MHz input clock */
#define CONFIG_SYS_CLK_FREQ	12000000

#if !defined(CONFIG_NAND_SPL) && (TEXT_BASE >= 0xc0000000)
#define CONFIG_ENABLE_MMU
#endif

#define CONFIG_MEMORY_UPPER_CODE

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG


/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

/*
 * Architecture magic and machine type
 */
#define MACH_TYPE		1826

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

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

#define CONFIG_SYS_HUSH_PARSER			/* use "hush" command parser	*/
#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#endif

#define CONFIG_CMDLINE_EDITING

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

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
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_MTDPARTS

#define CONFIG_BOOTDELAY	10	

#define CONFIG_ZERO_BOOTDELAY_CHECK

#define MTDIDS_DEFAULT "onenand0=s3c-onenand"
#define MTDPARTS_DEFAULT	"mtdparts=s3c-onenand:256k(bootloader)"\
				",128k@0x40000(params)"\
				",2m@0x60000(kernel)"\
				",16m@0x260000(test)"\
				",-(UBI)"

#define NORMAL_MTDPARTS_DEFAULT MTDPARTS_DEFAULT

#if 1
#define CONFIG_BOOTCOMMAND	"run ubifsboot"
#else
#define CONFIG_BOOTCOMMAND	"bootm 0x21008000"
#endif

#define CONFIG_COMMON_BOOT	"console=ttySA,115200n8" \
		" ${meminfo}" \
		" " MTDPARTS_DEFAULT

#define CONFIG_BOOTARGS	"root=/dev/mtdblock5 ubi.mtd=4" \
		" rootfstype=cramfs " CONFIG_COMMON_BOOT

#ifdef CONFIG_USE_BIG_UBOOT
#define CONFIG_UPDATEB	"updateb=onenand erase block 0-1;" \
			" onenand write 0x22008000 0x0 0x40000\0"
#else
#define CONFIG_UPDATEB	"updateb=onenand erase block 0-1;" \
			" onenand write 0x22008000 0x0 0x20000;" \
			" onenand write 0x22008000 0x20000 0x20000\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	CONFIG_UPDATEB \
	"updatek=onenand erase 0x60000 0x200000;" \
	" onenand write 0x21008000 0x60000 0x200000\0" \
	"updateu=onenand erase block 147-4095;" \
	" onenand write 0x22000000 0x1260000 0x8C0000\0" \
	"bootk=onenand read 0x20007FC0 0x60000 0x200000;" \
	" bootm 0x20007FC0\0" \
	"flashboot=set bootargs root=/dev/mtdblock${bootblock}" \
	 " rootfstype=${rootfstype}" \
	 " ubi.mtd=${ubiblock} ${opts} " CONFIG_COMMON_BOOT "; run bootk\0" \
	"ubifsboot=set bootargs root=ubi0!initrd.ubifs rootfstype=ubifs" \
	 " ubi.mtd=${ubiblock} ${opts} " CONFIG_COMMON_BOOT "; run bootk\0" \
	"boottrace=setenv opts initcall_debug; run bootcmd\0" \
	"android=set bootargs root=ubi0!ramdisk ubi.mtd=${ubiblock}" \
	 " rootfstype=ubifs init=/init.sh " CONFIG_COMMON_BOOT "; run bootk\0" \
	"nfsboot=set bootargs root=/dev/nfs ubi.mtd=${ubiblock}" \
	 " nfsroot=${nfsroot},nolock ip=${ipaddr}:${serverip}:${gatewayip}:" \
	 "${netmask}:nowplus:usb0:off " CONFIG_COMMON_BOOT "; run bootk\0" \
	"rootfstype=cramfs\0" \
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"meminfo=mem=128M\0" \
	"nfsroot=/nfsroot/arm\0" \
	"bootblock=5\0" \
	"ubiblock=4\0" \
	"ubi=enabled"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory	      */
#define CONFIG_SYS_PROMPT		"TT # "		/* Monitor Command Prompt     */
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size    */
#define CONFIG_SYS_PBSIZE		384		/* Print Buffer Size          */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size  */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE	/* memtest works on	      */
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x7e00000) /* 126MB in DRAM */

#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE	/* default load address	*/

#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2       /* Divisor: 2^(PTV+1) => 8 */

#define CONFIG_SYS_HZ			1000

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	0x40000		/* regular stack 256KB */

/**********************************
 Support Clock Settings
 **********************************
 Setting	SYNC	ASYNC
 ----------------------------------
 667_133_66	 X	  O
 533_133_66	 O	  O
 400_133_66	 X	  O
 400_100_50	 O	  O
 **********************************/

/*#define CONFIG_CLK_667_133_66*/
#define CONFIG_CLK_533_133_66
/*
#define CONFIG_CLK_400_100_50
#define CONFIG_CLK_400_133_66
#define CONFIG_SYNC_MODE
*/

/* SMDK6400 has 2 banks of DRAM, but we use only one in U-Boot */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CONFIG_SYS_SDRAM_BASE	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x08000000		/* 128 MB in Bank #1 */

#define CONFIG_SYS_MONITOR_BASE	0x00000000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_NO_FLASH		1

/* **** PISMO SUPPORT *** */

/* Configure the PISMO */
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M
#define PISMO1_ONEN_SIZE		GPMC_SIZE_128M

#define CONFIG_SYS_MONITOR_LEN		SZ_256K	/* Reserve 2 sectors */

#define ONENAND_ENV_OFFSET		0x260000 /* environment starts here */
#define SMNAND_ENV_OFFSET		0x260000 /* environment starts here */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(5 * CONFIG_SYS_HZ) /* Timeout for Flash Erase	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	(5 * CONFIG_SYS_HZ) /* Timeout for Flash Write	*/

#define CONFIG_ENV_SIZE		0x20000	/* Total Size of Environment Sector */
#define CONFIG_ENV_ADDR		0x40000

/*
 * SMDK6400 board specific data
 */

#define CONFIG_IDENT_STRING	" for TT"

/* base address for uboot */
#define CONFIG_SYS_PHY_UBOOT_BASE	(CONFIG_SYS_SDRAM_BASE + 0x07e00000)
/* total memory available to uboot */
#define CONFIG_SYS_UBOOT_SIZE		(1024 * 1024)

#ifdef CONFIG_ENABLE_MMU
#define CONFIG_SYS_MAPPED_RAM_BASE	0xc0000000
#else
#define CONFIG_SYS_MAPPED_RAM_BASE	CONFIG_SYS_SDRAM_BASE
#endif

/* NAND U-Boot load and start address */
#define CONFIG_SYS_UBOOT_BASE		(CONFIG_SYS_MAPPED_RAM_BASE + 0x07e00000)

#define CONFIG_ENV_OFFSET		0x0040000

/* Boot configuration (define only one of next 3) */
#define CONFIG_BOOT_ONENAND
/* None of these are currently implemented. Left from the original Samsung
 * version for reference
#define CONFIG_BOOT_NAND
#define CONFIG_BOOT_NOR
#define CONFIG_BOOT_MOVINAND
*/

#define CONFIG_ONENAND
/* Unimplemented or unsupported. See comment above.
#define CONFIG_NAND
#define CONFIG_NAND_S3C64XX
#define CONFIG_MOVINAND
*/

#define CONFIG_USE_ONENAND_BOARD_INIT
#define CONFIG_SYS_ONENAND_BASE		0x00000000
/* Settings as above boot configuration */
#define CONFIG_ENV_IS_IN_ONENAND

#define CONFIG_DOS_PARTITION	1

#endif	/* __CONFIG_H */
