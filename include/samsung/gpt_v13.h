#ifndef _TIZEN_GPT_V13_H_
#define _TIZEN_GPT_V13_H_

/**
 * Platform: trats
 * Boards: trats/trats2
 * version: Pit v13
*/

/* Tizen - partitions definitions */
#define PARTS_V13_CSA		"csa"
#define PARTS_V13_BOOT		"boot"
#define PARTS_V13_QBOOT		"qboot"
#define PARTS_V13_CSC		"csc"
#define PARTS_V13_ROOT		"platform"
#define PARTS_V13_DATA		"data"
#define PARTS_V13_UMS		"ums"
#define PARTS_V13_EFS		"EFS"
#define PARTS_V13_RECOVERY	"recovery"
#define PARTS_V13_MODEM		"modem"
#define PARTS_V13_RESERVED3	"reserved3"
#define PARTS_V13_RAMDISK1	"ramdisk1"
#define PARTS_V13_RAMDISK2	"ramdisk2"
#define PARTS_V13_MODULE	"module"
#define PARTS_V13_FOTA		"fota"
#define PARTS_V13_ROOTFS	"rootfs"
#define PARTS_V13_SYSDATA	"system-data"
#define PARTS_V13_USER		"user"

#define PARTS_TRATS2_GPT_V13 \
	"uuid_disk=${uuid_gpt_disk};" \
	"name="PARTS_V13_EFS",start=12MiB,size=20MiB,uuid=${uuid_gpt_"PARTS_V13_EFS"};" \
	"name="PARTS_V13_BOOT",size=14MiB,uuid=${uuid_gpt_"PARTS_V13_BOOT"};" \
	"name="PARTS_V13_RECOVERY",size=37MiB,uuid=${uuid_gpt_"PARTS_V13_RECOVERY"};" \
	"name="PARTS_V13_CSC",size=14MiB,uuid=${uuid_gpt_"PARTS_V13_CSC"};" \
	"name="PARTS_V13_MODEM",size=86MiB,uuid=${uuid_gpt_"PARTS_V13_MODEM"};" \
	"name="PARTS_V13_RESERVED3",size=14MiB,uuid=${uuid_gpt_"PARTS_V13_RESERVED3"};" \
	"name="PARTS_V13_RAMDISK1",size=20MiB,uuid=${uuid_gpt_"PARTS_V13_RAMDISK1"};" \
	"name="PARTS_V13_RAMDISK2",size=20MiB,uuid=${uuid_gpt_"PARTS_V13_RAMDISK2"};" \
	"name="PARTS_V13_MODULE",size=20MiB,uuid=${uuid_gpt_"PARTS_V13_MODULE"};" \
	"name="PARTS_V13_FOTA",size=10MiB,uuid=${uuid_gpt_"PARTS_V13_FOTA"};" \
	"name="PARTS_V13_ROOTFS",size=3000MiB,uuid=${uuid_gpt_"PARTS_V13_ROOTFS"};" \
	"name="PARTS_V13_SYSDATA",size=512MiB,uuid=${uuid_gpt_"PARTS_V13_SYSDATA"};" \
	"name="PARTS_V13_CSC",size=150MiB,uuid=${uuid_gpt_"PARTS_V13_CSC"};" \
	"name="PARTS_V13_USER",size=-,uuid=${uuid_gpt_"PARTS_V13_USER"}\0" \

#define DFU_ALT_SYSTEM_TRATS2_GPT_V13 \
	"/uImage ext4 0 2;" \
	"/zImage ext4 0 2;" \
	"/Image.itb ext4 0 2;" \
	"/exynos4412-trats2.dtb ext4 0 2;" \
	"/modem.bin ext4 0 5;" \
	"/modem_cdma.bin ext4 0 5;" \
	""PARTS_V13_EFS" part 0 1;" \
	""PARTS_V13_BOOT" part 0 2;" \
	""PARTS_V13_RECOVERY" part 0 3;" \
	"csa-mmc part 0 4;" \
	""PARTS_V13_MODEM" part 0 5;" \
	""PARTS_V13_RESERVED3" part 0 6;" \
	""PARTS_V13_RAMDISK1" part 0 7;" \
	"ramdisk-recovery part 0 8;" \
	""PARTS_V13_MODULE" part 0 9;" \
	""PARTS_V13_FOTA" part 0 10;" \
	""PARTS_V13_ROOTFS" part 0 11;" \
	""PARTS_V13_SYSDATA" part 0 12;" \
	""PARTS_V13_CSC" part 0 13;" \
	""PARTS_V13_USER" part 0 14\0"

#endif /* _TIZEN_GPT_V13_H_ */
