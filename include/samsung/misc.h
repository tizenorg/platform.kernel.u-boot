#ifndef __SAMSUNG_MISC_COMMON_H__
#define __SAMSUNG_MISC_COMMON_H__

#include <linux/input.h>

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void);
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
void set_board_info(void);
#endif

#if defined(CONFIG_LCD_MENU) || defined(CONFIG_INTERACTIVE_CHARGER)
void keys_init(void);
int check_keys(void);
int key_pressed(int key);
void check_boot_mode(void);
#endif /* CONFIG_LCD_MENU || CONFIG_INTERACTIVE_CHARGER */

#ifdef CONFIG_LCD_MENU
enum {
	BOOT_MODE_INFO,
	BOOT_MODE_BATT,
	BOOT_MODE_THOR,
	BOOT_MODE_UMS,
	BOOT_MODE_DFU,
	BOOT_MODE_GPT,
	BOOT_MODE_ENV,
	BOOT_MODE_EXIT,
};
#endif /* CONFIG_LCD_MENU */

#ifdef CONFIG_INTERACTIVE_CHARGER
enum {
	CMD_BATTERY_BOOT_CHECK,
	CMD_BATTERY_STATE,
	CMD_BATTERY_CHARGE,
};

#define CHARGE_TRESHOLD_BOOT			20
#define CHARGE_DISPLAY_TIMEOUT_SEC		10
#define NO_CHARGER_ANIM_TIMEOUT_SEC		10
#define NO_CHARGER_TIMEOUT_SEC			30
#define CHARGE_PWR_KEY_RESET_TIMEOUT		3
#define CHARGE_KEYS_EXIT_TIMEOUT		3

#define CHARGE_KEYS_EXIT		(KEY_VOLUMEDOWN + KEY_VOLUMEUP)

/* Should be implemented by board */
int charger_enable(void);
int charger_type(void);
int battery_present(void);
int battery_state(unsigned int *soc);
void board_low_power_mode(void);
#endif /* CONFIG_INTERACTIVE_CHARGER */

#ifdef CONFIG_CMD_BMP
void draw_logo(void);
#endif

#ifdef CONFIG_SET_DFU_ALT_INFO
char *get_dfu_alt_system(void);
char *get_dfu_alt_boot(void);
void set_dfu_alt_info(void);
#endif
#ifdef CONFIG_BOARD_TYPES
void set_board_type(void);
const char *get_board_type(void);
#ifdef CONFIG_OF_MULTI
const char *get_board_name(void);
#endif
#endif
#ifdef CONFIG_OF_MULTI
int board_is_trats2(void);
int board_is_odroid_x2(void);
int board_is_odroid_u3(void);
#endif

enum {
	GPT_ENV_PARTS,
	GPT_ENV_BOOT_EMMC,
	GPT_ENV_BOOT_SD,
	GPT_ENV_SYSTEM,
	GPT_ENV_BOOT_PART,
	GPT_ENV_ROOT_PART,
	GPT_ENV_EXIT,
};

enum {
	GPT_VERSION_V08,
	GPT_VERSION_V13,
	GPT_VERSION_EXIT,
};

char *get_gpt_environment(int id);
#endif /* __SAMSUNG_MISC_COMMON_H__ */
