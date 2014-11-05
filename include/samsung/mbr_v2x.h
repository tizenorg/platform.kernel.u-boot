#ifndef _TIZEN_MBR_V2X_H_
#define _TIZEN_MBR_V2X_H_

/**
 * Platform: odroid
 * Boards: odroidu3/odroidx2
 * version: Tizen v2.x
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
 * 1.  BOOT:   100MiB 2MiB
 * 2.  ROOT:   3GiB
 * 3.  DATA:   3GiB
 * 4.  UMS:    -
*/

/* Tizen 2.0 - MBR partition definitions for Odroid */
#define PARTS_V2X_BOOT		"boot"
#define PARTS_V2X_ROOT		"platform"
#define PARTS_V2X_DATA		"data"
#define PARTS_V2X_UMS		"ums"

#define DFU_ALT_SYSTEM_ODROID_MBR_V2X \
	"uImage fat 0 1;" \
	"zImage fat 0 1;" \
	"Image.itb fat 0 1;" \
	"exynos4412-odroidu3.dtb fat 0 1;" \
	"exynos4412-odroidx2.dtb fat 0 1;" \
	""PARTS_V2X_BOOT" part 0 1;" \
	""PARTS_V2X_ROOT" part 0 2;" \
	""PARTS_V2X_DATA" part 0 3;" \
	""PARTS_V2X_UMS" part 0 4\0"

#endif /* _TIZEN_MBR_V2X_H_ */
