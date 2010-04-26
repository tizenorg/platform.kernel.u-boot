/*
 * Copyright (C) 2009-2010 Samsung Electronics
 */

#include <common.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/keypad.h>
#include "recovery.h"
#include "onenand.h"

#ifdef RECOVERY_DEBUG
#define	PUTS(s)	serial_puts (DEBUG_MARK""s)
#else
#define PUTS(s)
#endif

typedef int (init_fnc_t) (void);

DECLARE_GLOBAL_DATA_PTR;

static struct s5pc110_gpio *gpio_base = (struct s5pc110_gpio *) S5PC110_GPIO_BASE;

static void sdelay(unsigned long usec)
{
	ulong kv;

	do {
		kv = 0x100000;
		while (kv)
			kv--;
		usec--;
	} while (usec);
}

static int check_keypad(void)
{
	unsigned int condition = 0;
	unsigned int reg, value;
	unsigned int col_num, row_num;
	unsigned int col_mask;
	unsigned int col_mask_shift;
	unsigned int row_state[4] = {0, 0, 0, 0};
	unsigned int i;

	/* board is limo universal */

	row_num = 2;
	col_num = 3;

	for (i = 0; i < sizeof(row_state)/sizeof(int); i++)
		row_state[i] = 0;

	for (i = 0; i < row_num; i++) {
		/* Set GPH3[3:0] to KP_ROW[3:0] */
		gpio_cfg_pin(&gpio_base->gpio_h3, i, 0x3);
		gpio_set_pull(&gpio_base->gpio_h3, i, GPIO_PULL_UP);
	}

	for (i = 0; i < col_num; i++)
		/* Set GPH2[3:0] to KP_COL[3:0] */
		gpio_cfg_pin(&gpio_base->gpio_h2, i, 0x3);

	reg = S5PC110_KEYPAD_BASE;
	col_mask = S5PC110_KEYIFCOLEN_MASK;
	col_mask_shift = 8;

	/* KEYIFCOL reg clear */
	writel(0, reg + S5PC1XX_KEYIFCOL_OFFSET);

	/* key_scan */
	for (i = 0; i < col_num; i++) {
		value = col_mask;
		value &= ~(1 << i) << col_mask_shift;

		writel(value, reg + S5PC1XX_KEYIFCOL_OFFSET);
		sdelay(1000);

		value = readl(reg + S5PC1XX_KEYIFROW_OFFSET);
		row_state[i] = ~value & ((1 << row_num) - 1);
	}

	/* KEYIFCOL reg clear */
	writel(0, reg + S5PC1XX_KEYIFCOL_OFFSET);

	/* check volume up/down */
	if ((row_state[1] & 0x3) == 0x3)
		condition = 1;

	return condition;
}

static int check_block(void)
{
	struct onenand_op *onenand_ops = onenand_get_interface();
	struct mtd_info *mtd = onenand_ops->mtd;
	struct onenand_chip *this = mtd->priv;
	int i;
	int page_to_check = 4;
	int ret, retlen = 0;
	ulong blocksize = 1 << this->erase_shift;
	ulong pagesize = 1 << this->page_shift;
	u_char *down_ram_addr;
	ulong uboot_addr;
	u_char verify_buf[0x10];

	down_ram_addr = (unsigned char *)CONFIG_SYS_DOWN_ADDR;
	uboot_addr = CONFIG_RECOVERY_UBOOT_BLOCK << this->erase_shift;

	onenand_ops->read(uboot_addr, blocksize, &retlen, down_ram_addr, 0);
	if (retlen != blocksize)
		return 1;

	memset(verify_buf, 0xFF, sizeof(verify_buf));

	for (i = 0; i < page_to_check; i++) {
		ret = memcmp(down_ram_addr + pagesize*i, verify_buf,
				sizeof(verify_buf));
		if (ret)
			break;
	}

	if (i == page_to_check)
		return 1;

	return 0;
}

int board_check_condition(void)
{
	if (check_keypad()) {
		PUTS("manual mode\n");
		return 1;
	}

	if (check_block()) {
		PUTS("bootloader image broken\n");
		return 1;
	}

	return 0;
}

int board_load_uboot(unsigned char *buf)
{
	struct mtd_info *mtd = &onenand_mtd;
	struct onenand_chip *this = mtd->priv;
	int offset;
	size_t size;
	size_t ret;

	offset = CONFIG_RECOVERY_UBOOT_BLOCK << this->erase_shift;
	size = CONFIG_SYS_MONITOR_LEN;

	mtd->read(mtd, offset, size, &ret, buf);

	if (size != ret)
		return -1;

	return 0;
}

void board_recovery_init(void)
{
	/* set GPIO to enable UART2 */
	gpio_cfg_pin(&gpio_base->gpio_a1, 0, 0x2);
	gpio_cfg_pin(&gpio_base->gpio_a1, 1, 0x2);

	/* UART_SEL MP0_5[7] at S5PC110 */
	gpio_direction_output(&gpio_base->gpio_mp0_5, 7, 0x1);
	gpio_set_pull(&gpio_base->gpio_mp0_5, 7, GPIO_PULL_DOWN);
}
