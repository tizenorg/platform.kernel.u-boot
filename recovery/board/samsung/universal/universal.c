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
#define	PUTS(s)	serial_puts(DEBUG_MARK""s)
#else
#define PUTS(s)
#endif

typedef int (init_fnc_t) (void);

DECLARE_GLOBAL_DATA_PTR;

static struct s5pc110_gpio *gpio_base =
	(struct s5pc110_gpio *) S5PC110_GPIO_BASE;

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
	int retlen = 0;
	u32 from = CONFIG_RECOVERY_UBOOT_BLOCK << this->erase_shift;
	u32 len = 1 << this->erase_shift;
	u32 *buf = (u32 *)CONFIG_SYS_DOWN_ADDR;

	/* check first page of bootloader*/
	onenand_ops->read(from, len, (ssize_t *)&retlen, (u_char *)buf, 0);
	if (retlen != len)
		return 1;

	for (i = 0; i < (this->writesize / sizeof(this->writesize)); i++)
		if (*(buf + i) != 0xffffffff)
			return 0;

	return 1;
}

int board_check_condition(void)
{
	if (check_keypad()) {
		PUTS("check: manual\n");
		return 1;
	}

	if (check_block()) {
		PUTS("check: bootloader broken\n");
		return 2;
	}

	return 0;
}

int board_load_bootloader(unsigned char *buf)
{
	struct mtd_info *mtd = &onenand_mtd;
	struct onenand_chip *this = mtd->priv;
	u32 ofs, len, retlen;

	ofs = CONFIG_RECOVERY_UBOOT_BLOCK << this->erase_shift;
	len = CONFIG_SYS_MONITOR_LEN;

	mtd->read(mtd, ofs, len, &retlen, buf);

	if (len != retlen)
		return -1;

	return 0;
}

int board_lock_recoveryblock(void)
{
	struct mtd_info *mtd = &onenand_mtd;
	struct onenand_chip *this = mtd->priv;
	u32 ofs = 0;
	u32 len = CONFIG_RECOVERY_UBOOT_BLOCK << this->erase_shift;
	int ret = 0;

	/* lock-tight the recovery block */
	if (this->lock_tight != NULL)
		ret = this->lock_tight(mtd, ofs, len);

	return ret;
}

void board_update_init(void)
{
	struct mtd_info *mtd = &onenand_mtd;
	struct onenand_chip *this = mtd->priv;

	/* Unlock whole block */
	this->unlock_all(mtd);
}

int board_update_image(u32 *buf, u32 len)
{
	struct onenand_op *onenand_ops = onenand_get_interface();
	struct mtd_info *mtd = &onenand_mtd;
	struct onenand_chip *this = mtd->priv;
	u32 ofs;
	u32 ipl_addr = 0;
	u32 ipl_edge = CONFIG_ONENAND_START_PAGE << this->page_shift;
	u32 recovery_addr = ipl_edge;
	u32 recovery_edge = CONFIG_RECOVERY_UBOOT_BLOCK << this->erase_shift;
	u32 bootloader_addr = recovery_edge;
	u32 bootloader_edge = CONFIG_RECOVERY_BOOT_BLOCKS << this->erase_shift;
	int ret, retlen;

	if (len > bootloader_addr) {
		if (*(buf + bootloader_addr/sizeof(buf)) == 0xea000012) {
			/* case: IPL + Recovery + bootloader */
			PUTS("target: ipl + recovery + bootloader\n");
			ofs = ipl_addr;
			/* len = bootloader_edge; */
		} else {
			/* case: unknown format */
			PUTS("target: unknown\n");
			return 1;
		}
	} else {
		if (*(buf + recovery_addr/sizeof(buf)) == 0xea000012 &&
			*(buf + recovery_addr/sizeof(buf) - 1) == 0x00000000) {
			/* case: old image (IPL + bootloader) */
			PUTS("target: ipl + bootloader (old type)\n");
			ofs = ipl_addr;
			/* len = recovery_edge; */
		} else {
			/* case: bootloader only */
			PUTS("target: bootloader\n");
			ofs = bootloader_addr;
			/* len = bootloader_edge - recovery_edge; */
		}
	}

	/* Erase */
	ret = onenand_ops->erase(ofs, len, 0);
	if (ret)
		return ret;

	/* Write */
	onenand_ops->write(ofs, len, (ssize_t *)&retlen, (u_char *)buf);
	if (ret)
		return ret;

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
