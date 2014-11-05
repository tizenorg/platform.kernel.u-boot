#ifndef _TIZEN_GPT_V08_H_
#define _TIZEN_GPT_V08_H_

/*
 * Bootable media layout:
 * dev:    SD   eMMC(part boot)
 * BL1      1    0
 * BL2     31   30
 * UBOOT   63   62
 * TZSW  2111 2110
 * ENV   2560 2560(part user)
 *
 * MBR Primary partiions:
 * Num Name   Size  Offset
 * 1.  BOOT:  100MiB 2MiB
 * 2.  ROOT:  3GiB
 * 3.  DATA:  3GiB
 * 4.  UMS:   -
*/

/* Tizen - partitions definitions */
#define PARTS_V8_CSA		"csa-mmc"
#define PARTS_V8_BOOT		"boot"
#define PARTS_V8_QBOOT		"qboot"
#define PARTS_V8_CSC		"csc"
#define PARTS_V8_ROOT		"platform"
#define PARTS_V8_DATA		"data"
#define PARTS_V8_UMS		"ums"

#define PARTS_TRATS2_GPT_V08 \
	"uuid_disk=${uuid_gpt_disk};" \
	"name="PARTS_V8_CSA",start=5MiB,size=8MiB,uuid=${uuid_gpt_"PARTS_V8_CSA"};" \
	"name="PARTS_V8_BOOT",size=60MiB,uuid=${uuid_gpt_"PARTS_V8_BOOT"};" \
	"name="PARTS_V8_QBOOT",size=100MiB,uuid=${uuid_gpt_"PARTS_V8_QBOOT"};" \
	"name="PARTS_V8_CSC",size=150MiB,uuid=${uuid_gpt_"PARTS_V8_CSC"};" \
	"name="PARTS_V8_ROOT",size=1536MiB,uuid=${uuid_gpt_"PARTS_V8_ROOT"};" \
	"name="PARTS_V8_DATA",size=3000MiB,uuid=${uuid_gpt_"PARTS_V8_DATA"};" \
	"name="PARTS_V8_UMS",size=-,uuid=${uuid_gpt_"PARTS_V8_UMS"}\0" \

#define DFU_ALT_SYSTEM_TRATS2_GPT_V08 \
	"/uImage ext4 0 2;" \
	"/zImage ext4 0 2;" \
	"/Image.itb ext4 0 2;" \
	"/modem.bin ext4 0 2;" \
	"/exynos4412-trats2.dtb ext4 0 2;" \
	""PARTS_V8_CSA" part 0 1;" \
	""PARTS_V8_BOOT" part 0 2;" \
	""PARTS_V8_QBOOT" part 0 3;" \
	""PARTS_V8_CSC" part 0 4;" \
	""PARTS_V8_ROOT" part 0 5;" \
	""PARTS_V8_DATA" part 0 6;" \
	""PARTS_V8_UMS" part 0 7;" \
	"params.bin raw 2560 8\0"

#define DFU_ALT_SYSTEM_ODROID \
	"uImage fat 0 1;" \
	"zImage fat 0 1;" \
	"Image.itb fat 0 1;" \
	"exynos4412-odroidu3.dtb fat 0 1;" \
	"exynos4412-odroidx2.dtb fat 0 1;" \
	""PARTS_V8_BOOT" part 0 1;" \
	""PARTS_V8_ROOT" part 0 2;" \
	""PARTS_V8_DATA" part 0 3;" \
	""PARTS_V8_UMS" part 0 4;" \
	"params.bin raw 2560 8\0"

#define DFU_ALT_BOOT_SD_TRATS2_GPT_V08 \
	"This boot mode is not used\0"

#define DFU_ALT_BOOT_EMMC_TRATS2_GPT_V08 \
	"u-boot raw 0x80 0x800\0"

#define PARTS_ODROID \
	"This board uses MSDOS partition table."

#define DFU_ALT_BOOT_EMMC_ODROID \
	"u-boot raw 0x3e 0x800 mmcpart 1;" \
	"bl1 raw 0x0 0x1e mmcpart 1;" \
	"bl2 raw 0x1e 0x1d mmcpart 1;" \
	"tzsw raw 0x83e 0x138 mmcpart 1\0"

#define DFU_ALT_BOOT_SD_ODROID \
	"u-boot raw 0x3f 0x800;" \
	"bl1 raw 0x1 0x1e;" \
	"bl2 raw 0x1f 0x1d;" \
	"tzsw raw 0x83f 0x138\0"

#endif /* _TIZEN_GPT_V08_H_ */
