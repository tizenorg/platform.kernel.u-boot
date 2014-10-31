#ifndef _TIZEN_ENVIRONMENT_H_
#define _TIZEN_ENVIRONMENT_H_

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
#define PARTS_CSA		"csa-mmc"
#define PARTS_BOOT		"boot"
#define PARTS_QBOOT		"qboot"
#define PARTS_CSC		"csc"
#define PARTS_ROOT		"platform"
#define PARTS_DATA		"data"
#define PARTS_UMS		"ums"
#define PARTS_EFS		"EFS"
#define PARTS_RECOVERY		"recovery"
#define PARTS_MODEM		"modem"
#define PARTS_RESERVED3		"reserved3"
#define PARTS_RAMDISK1		"ramdisk1"
#define PARTS_RAMDISK2		"ramdisk2"
#define PARTS_MODULE		"module"
#define PARTS_FOTA		"fota"
#define PARTS_ROOTFS		"rootfs"
#define PARTS_SYSDATA		"system-data"
#define PARTS_USER		"user"

#define PARTS_ODROID \
	"This board uses MSDOS partition table."

#define PARTS_TRATS2 \
	"uuid_disk=${uuid_gpt_disk};" \
	"name="PARTS_EFS",size=20MiB,uuid=${uuid_gpt_"PARTS_EFS"};" \
	"name="PARTS_BOOT",size=14MiB,uuid=${uuid_gpt_"PARTS_BOOT"};" \
	"name="PARTS_RECOVERY",size=37MiB,uuid=${uuid_gpt_"PARTS_RECOVERY"};" \
	"name=csa,size=14MiB,uuid=${uuid_gpt_csa};" \
	"name="PARTS_MODEM",size=86MiB,uuid=${uuid_gpt_"PARTS_MODEM"};" \
	"name="PARTS_RESERVED3",size=14MiB,uuid=${uuid_gpt_"PARTS_RESERVED3"};" \
	"name="PARTS_RAMDISK1",size=20MiB,uuid=${uuid_gpt_"PARTS_RAMDISK1"};" \
	"name="PARTS_RAMDISK2",size=20MiB,uuid=${uuid_gpt_"PARTS_RAMDISK2"};" \
	"name="PARTS_MODULE",size=20MiB,uuid=${uuid_gpt_"PARTS_MODULE"};" \
	"name="PARTS_FOTA",size=10MiB,uuid=${uuid_gpt_"PARTS_FOTA"};" \
	"name="PARTS_ROOTFS",size=3000MiB,uuid=${uuid_gpt_"PARTS_ROOTFS"};" \
	"name="PARTS_SYSDATA",size=512MiB,uuid=${uuid_gpt_"PARTS_SYSDATA"};" \
	"name="PARTS_CSC",size=150MiB,uuid=${uuid_gpt_"PARTS_CSC"};" \
	"name="PARTS_USER",size=-,uuid=${uuid_gpt_"PARTS_USER"}\0" \

#define DFU_ALT_SYSTEM_TRATS2 \
	"s-boot-mmc.bin raw 0x0 0x400 mmcpart 1;" \
	"u-boot-mmc.bin raw 0x80 0x800;" \
	"/uImage ext4 0 2;" \
	"/zImage ext4 0 2;" \
	"/Image.itb ext4 0 2;" \
	"/exynos4412-trats2.dtb ext4 0 2;" \
	"/modem.bin ext4 0 5" \
	"/modem_cdma.bin ext4 0 5" \
	""PARTS_EFS" part 0 1;" \
	""PARTS_BOOT" part 0 2;" \
	""PARTS_RECOVERY" part 0 3;" \
	""PARTS_CSA" part 0 4;" \
	""PARTS_MODEM" part 0 5;" \
	""PARTS_RESERVED3" part 0 6;" \
	""PARTS_RAMDISK1" part 0 7;" \
	"ramdisk-recovery part 0 8;" \
	""PARTS_MODULE" part 0 9;" \
	""PARTS_FOTA" part 0 10;" \
	""PARTS_ROOTFS" part 0 11;" \
	""PARTS_SYSDATA" part 0 12;" \
	""PARTS_CSC" part 0 13;" \
	""PARTS_USER" part 0 14;" \
	"params.bin raw 0x1880 0x200\0"

#define DFU_ALT_SYSTEM_ODROID \
	"uImage fat 0 1;" \
	"zImage fat 0 1;" \
	"Image.itb fat 0 1;" \
	"exynos4412-odroidu3.dtb fat 0 1;" \
	"exynos4412-odroidx2.dtb fat 0 1;" \
	""PARTS_BOOT" part 0 1;" \
	""PARTS_ROOT" part 0 2;" \
	""PARTS_DATA" part 0 3;" \
	""PARTS_UMS" part 0 4;" \
	"params.bin raw 2560 8\0"

#define DFU_ALT_BOOT_EMMC_TRATS2 \
	"u-boot raw 0x80 0x800\0"

#define DFU_ALT_BOOT_SD_TRATS2 \
	"This boot mode is not used\0"

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

#endif /* _TIZEN_ENVIRONMENT_H_ */
