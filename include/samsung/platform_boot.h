#ifndef _INCLUDE_PLATFORM_BOOT_
#define _INCLUDE_PLATFORM_BOOT_

/*
 * Default boot environment for Samsung Platform
 * For board config file:
 * - add: #include <samsung/platform_boot.h>
 * - add PLATFORM_BOOT_INFO to CONFIG_EXTRA_ENV_SETTINGS
 * - add checkboard macro to CONFIG_EXTRA_ENV_SETTINGS
 *   e.g.:
 *   #define CONFIG_EXTRA_ENV_SETTINGS \
 *           PLATFORM_BOOT_INFO \
 *           "checkboard=\0" \
 *            ...
 * - add: #define CONFIG_BOOTCOMMAND        "run autoboot"
 *
 * The above changes may need undef the existing configs.
 * #undef CONFIG_BOOTCOMMAND
 * #undef CONFIG_EXTRA_ENV_SETTINGS
 * Note: Add the #undef before the new definitions.
 */

#define PLATFORM_BOOT_INFO \
	"initrdname=uInitrd\0" \
	"initrdaddr=42000000\0" \
	"fdtaddr=40800000\0" \
	"loadkernel=load mmc ${mmcbootdev}:${mmcbootpart} ${kerneladdr} " \
		"${kernelname}\0" \
	"loadinitrd=load mmc ${mmcbootdev}:${mmcbootpart} ${initrdaddr} " \
		"${initrdname}\0" \
	"loaddtb=load mmc ${mmcbootdev}:${mmcbootpart} ${fdtaddr} " \
		"${fdtfile}\0" \
	"check_ramdisk=" \
		"if run loadinitrd; then " \
			"setenv initrd_addr ${initrdaddr};" \
		"else " \
			"setenv initrd_addr -;" \
		"fi;\0" \
	"check_dtb=" \
		"if run loaddtb; then " \
			"setenv fdt_addr ${fdtaddr};" \
		"else " \
			"setenv fdt_addr;" \
		"fi;\0" \
	"kernel_args=" \
		"setenv bootargs root=/dev/mmcblk${mmcrootdev}p${mmcrootpart}" \
		" rootfstype=${rootfstype} rootwait ${console} ${opts}\0" \
	"boot_fit=" \
		"setenv kerneladdr 0x42000000;" \
		"setenv kernelname Image.itb;" \
		"run loadkernel;" \
		"run kernel_args;" \
		"bootm ${kerneladdr}#${boardname}\0" \
	"boot_uimg=" \
		"setenv kerneladdr 0x40007FC0;" \
		"setenv kernelname uImage;" \
		"run check_dtb;" \
		"run check_ramdisk;" \
		"run loadkernel;" \
		"run kernel_args;" \
		"bootm ${kerneladdr} ${initrd_addr} ${fdt_addr};\0" \
	"boot_zimg=" \
		"setenv kerneladdr 0x40007FC0;" \
		"setenv kernelname zImage;" \
		"run check_dtb;" \
		"run check_ramdisk;" \
		"run loadkernel;" \
		"run kernel_args;" \
		"bootz ${kerneladdr} ${initrd_addr} ${fdt_addr};\0" \
	"autoboot=" \
		"run checkboard;" \
		"if test -e mmc 0:${mmcbootpart} Image.itb; then; " \
			"run boot_fit;" \
		"elif test -e mmc 0:${mmcbootpart} zImage; then; " \
			"run boot_zimg;" \
		"elif test -e mmc 0:${mmcbootpart} uImage; then; " \
			"run boot_uimg;" \
		"fi;\0"

#endif
