#ifndef _PLATFORM_SETUP_H_
#define _PLATFORM_SETUP_H_

#include <samsung/gpt_v08.h>
#include <samsung/gpt_v13.h>
#include <samsung/mbr_v2x.h>
#include <samsung/mbr_v30.h>

/**
 * Each device has predefined bootloader area which should stay unchanged
 * to prevent bootloader failure.
 */
#define DFU_ALT_BOOT_EMMC_TRATS2 \
	"s-boot-mmc.bin raw 0x0 0x400 mmcpart 1;" \
	"u-boot-mmc.bin raw 0x80 0x800;" \
	"params.bin raw 0x1880 0x20\0"

#define DFU_ALT_BOOT_SD_TRATS2 \
	"This boot mode is not used\0"

#define DFU_ALT_BOOT_EMMC_ODROID \
	"u-boot raw 0x3e 0x800 mmcpart 1;" \
	"bl1 raw 0x0 0x1e mmcpart 1;" \
	"bl2 raw 0x1e 0x1d mmcpart 1;" \
	"tzsw raw 0x83e 0x138 mmcpart 1;" \
	"params.bin raw 0x1880 0x20\0"

#define DFU_ALT_BOOT_SD_ODROID \
	"u-boot raw 0x3f 0x800;" \
	"bl1 raw 0x1 0x1e;" \
	"bl2 raw 0x1f 0x1d;" \
	"tzsw raw 0x83f 0x138;" \
	"params.bin raw 0x1880 0x20\0"

/**
 * Each platform (${platname}) can provide N configs.
 * Each config can provide a number of partitions.
 *
 * How to add new setup:
 * - ${platname}_setup_N_name
 * - ${platname}_setup_N_partitions
 * - ${platname}_setup_N_alt_system
 * - ${platname}_setup_N_bootpart
 * - ${platname}_setup_N_rootpart
 *
 * Update PLATFORM_SETUP_INFO with:
 * - ${platname}_setup_N_name/partitions/alt_system - new setup data
 * - ${platname}_setup_cnt    - number of configs for board
 * - ${platname}_setup_chosen - number of chosen board config
 * - ${platname}_setup_active - number of active board config (autoset)
 */

/**
 * TRATS_SETUP_1 - data defined in include/samsung/gpt_v08.h
 * TRATS_SETUP_2 - data defined in include/samsung/gpt_v13.h
 */
#define TRATS_SETUP_1 \
	"trats_setup_1_name=PIT v08\0" \
	"trats_setup_1_partitions="PARTS_TRATS2_GPT_V08 \
	"trats_setup_1_alt_system="DFU_ALT_SYSTEM_TRATS2_GPT_V08 \
	"trats_setup_1_bootpart=2\0" \
	"trats_setup_1_rootpart=5\0"

#define TRATS_SETUP_2 \
	"trats_setup_2_name=PIT v13\0" \
	"trats_setup_2_partitions="PARTS_TRATS2_GPT_V13 \
	"trats_setup_2_alt_system="DFU_ALT_SYSTEM_TRATS2_GPT_V13 \
	"trats_setup_2_bootpart=2\0" \
	"trats_setup_2_rootpart=11\0"

/**
 * ${platname}: trats
 * Supported by: trats/trats2
 * setup 1: Pit v08
 * setup 2: Pit v13 (chosen)
 */
#define PLATFORM_SETUP_TRATS \
	"trats_setup_cnt=2\0" \
	"trats_setup_chosen=2\0" \
	"trats_setup_active=\0" \
	TRATS_SETUP_1 \
	TRATS_SETUP_2


/* $partitions - not defined(MSDOS) */
#define ODROID_SETUP_1 \
	"odroid_setup_1_name=Tizen v2.x\0" \
	"odroid_setup_1_partitions=\0" \
	"odroid_setup_1_alt_system="DFU_ALT_SYSTEM_ODROID_MBR_V2X \
	"odroid_setup_1_bootpart=1\0" \
	"odroid_setup_1_rootpart=2\0"

/* $partitions - not defined(MSDOS) */
#define ODROID_SETUP_2 \
	"odroid_setup_2_name=Tizen v3.0\0" \
	"odroid_setup_2_partitions=\0" \
	"odroid_setup_2_alt_system="DFU_ALT_SYSTEM_ODROID_MBR_V30 \
	"odroid_setup_2_bootpart=1\0" \
	"odroid_setup_2_rootpart=2\0"

/**
 * ${platname}: odroid
 * Supported by: odroidu3/odroidx2
 * setup 1: Tizen v2.x
 * setup 2: Tizen v3.0 (chosen)
 */
#define PLATFORM_SETUP_ODROID \
	"odroid_setup_cnt=2\0" \
	"odroid_setup_chosen=2\0" \
	"odroid_setup_active=\0" \
	ODROID_SETUP_1 \
	ODROID_SETUP_2

/**
 * PLATFORM_SETUP_INFO - this should be a part of CONFIG_EXTRA_ENV_SETTINGS
 * It provides environment variables for each board multiple platform setup.
 * !!!Update this when adding new config!!!
 * This is default setup - can be overwritten by params.bin
 */
#define PLATFORM_SETUP_INFO \
	PLATFORM_SETUP_TRATS \
	PLATFORM_SETUP_ODROID

#endif /* _PLATFORM_SETUP_H_ */
