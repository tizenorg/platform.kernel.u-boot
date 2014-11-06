#ifndef _TIZEN_MBR_V30_H_
#define _TIZEN_MBR_V30_H_

/**
 * Platform: odroid
 * Boards: odroidu3/odroidx2
 * version: Tizen v3.0
 *
 * Bootable media layout:
 * dev:    SD   eMMC(part boot)
 * BL1      1    0
 * BL2     31   30
 * UBOOT   63   62
 * TZSW  2111 2110
 * ENV   2560 2560(part user)
 *
 * MBR Primary partiions:
 * Num Name    Size  Offset
 * 1.  BOOT:   32MiB 4MiB
 * 2.  ROOT:   3GiB
 * 3.  DATA:   512MiB
 * 4.  Extd:
 * 5.  USER:   -
 * 6.  MODULE: 20MiB
*/

/* Tizen 3.0 - MBR partition definitions for Odroid */
#define PARTS_V30_BOOT		"boot"
#define PARTS_V30_ROOT		"rootfs"
#define PARTS_V30_DATA		"system-data"
#define PARTS_V30_USER		"user"
#define PARTS_V30_MODULES	"modules"

#define DFU_ALT_SYSTEM_ODROID_MBR_V30 \
	"uImage fat 0 1;" \
	"zImage fat 0 1;" \
	"Image.itb fat 0 1;" \
	"exynos4412-odroidu3.dtb fat 0 1;" \
	"exynos4412-odroidx2.dtb fat 0 1;" \
	""PARTS_V30_BOOT" part 0 1;" \
	""PARTS_V30_ROOT" part 0 2;" \
	""PARTS_V30_DATA" part 0 3;" \
	""PARTS_V30_USER" part 0 5;" \
	""PARTS_V30_MODULES" part 0 6\0"

#endif /* _TIZEN_MBR_V30_H_ */
