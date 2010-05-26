/*
 * Copyright (C) 2009-2010 Samsung Electronics
 */

#include <common.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/keypad.h>
#include <asm/arch/power.h>
#include "recovery.h"
#include "onenand.h"

#ifdef RECOVERY_DEBUG
#define	PUTS(s)	serial_puts(DEBUG_MARK""s)
#else
#define PUTS(s)
#endif

#define C110_MACH_START		3100
#define C100_MACH_START		3000

/* board is MACH_AQUILA and board is like below. */
#define J1_B2_BOARD		0x0200
#define LIMO_UNIVERSAL_BOARD	0x0400
#define LIMO_REAL_BOARD		0x0800
#define MEDIA_BOARD		0x1000
#define BAMBOO_BOARD		0x2000

/* board is MACH_GONI and board is like below */
#define S1_BOARD		0x1000
#define KESSLER_BOARD		0x4000
#define SDK_BOARD		0x8000

#define BOARD_MASK		0xFF00

enum {
	MACH_UNIVERSAL,
	MACH_TICKERTAPE,
	MACH_CHANGED,
	MACH_P1P2,	/* Don't remove it */
	MACH_GEMINUS,
	MACH_CYPRESS,

	MACH_WMG160 = 160,

	MACH_PSEUDO_END,

	MACH_GONI = 3102,
};

typedef int (init_fnc_t) (void);

static bd_t bd;

static unsigned int board_rev;

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

static int hwrevision(int rev)
{
	return (board_rev & 0xf) == rev;
}

static int c110_machine_id(void)
{
	return bd.bi_arch_number - C110_MACH_START;
}

static int mach_is_aquila(void)
{
	return bd.bi_arch_number == MACH_TYPE_AQUILA;
}

static int mach_is_geminus(void)
{
	return c110_machine_id() == MACH_GEMINUS;
}

static int board_is_limo_universal(void)
{
	return mach_is_aquila() && (board_rev & LIMO_UNIVERSAL_BOARD);
}

static int board_is_limo_real(void)
{
	return mach_is_aquila() && (board_rev & LIMO_REAL_BOARD);
}

/* Kessler */
static int mach_is_goni(void)
{
	return bd.bi_arch_number == MACH_GONI;
}

static int board_is_sdk(void)
{
	return mach_is_goni() && (board_rev & SDK_BOARD);
}

static void check_board_revision(int board, int rev)
{
	if (board == MACH_TYPE_AQUILA) {
		/* Limo Real or Universal */
		if (rev & LIMO_UNIVERSAL_BOARD)
			board_rev &= ~J1_B2_BOARD;
		if (rev & LIMO_REAL_BOARD)
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD);
		if (rev & MEDIA_BOARD)
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD);
		if (rev & BAMBOO_BOARD)
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD |
					LIMO_REAL_BOARD |
					MEDIA_BOARD);
	} else if (board == MACH_GONI) {
		if (rev & KESSLER_BOARD)
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD);
		if (rev & SDK_BOARD)
			board_rev &= ~(J1_B2_BOARD |
					LIMO_UNIVERSAL_BOARD);
		if (rev & S1_BOARD)
			board_rev &= ~(J1_B2_BOARD | LIMO_UNIVERSAL_BOARD |
					LIMO_REAL_BOARD);
	} else {
		board_rev &= ~BOARD_MASK;
	}
}

static unsigned int get_hw_revision(struct s5p_gpio_bank *bank, int hwrev3)
{
	unsigned int rev;

	gpio_direction_input(bank, 2);
	gpio_direction_input(bank, 3);
	gpio_direction_input(bank, 4);
	gpio_direction_input(bank, hwrev3);

	gpio_set_pull(bank, 2, GPIO_PULL_NONE);		/* HWREV_MODE0 */
	gpio_set_pull(bank, 3, GPIO_PULL_NONE);		/* HWREV_MODE1 */
	gpio_set_pull(bank, 4, GPIO_PULL_NONE);		/* HWREV_MODE2 */
	gpio_set_pull(bank, hwrev3, GPIO_PULL_NONE);	/* HWREV_MODE3 */

	rev = gpio_get_value(bank, 2);
	rev |= (gpio_get_value(bank, 3) << 1);
	rev |= (gpio_get_value(bank, 4) << 2);
	rev |= (gpio_get_value(bank, hwrev3) << 3);

	return rev;
}

static void check_hw_revision(void)
{
	unsigned int board = MACH_UNIVERSAL;	/* Default is Universal */

	if (cpu_is_s5pc100()) {
		struct s5pc100_gpio *gpio =
			(struct s5pc100_gpio *)S5PC100_GPIO_BASE;

		board_rev = get_hw_revision(&gpio->gpio_j0, 0);

		/* C100 TickerTape */
		if (board_rev == 3)
			board = MACH_TICKERTAPE;
	} else {
		struct s5pc110_gpio *gpio =
			(struct s5pc110_gpio *)S5PC110_GPIO_BASE;
		int hwrev3 = 1;

		board_rev = 0;

		/*
		 * Note Check 'Aquila' board first
		 *
		 * TT: TickerTape
		 * SS: SplitScreen
		 * LRA: Limo Real Aquila
		 * LUA: Limo Universal Aquila
		 * OA: Old Aquila
		 * CYP: Cypress
		 * BB: Bamboo
		 *
		 * ADDR = 0xE0200000 + OFF
		 *
		 * 	 OFF	Universal BB   LRA  LUA  OA   TT   SS	    CYP
		 *   J1: 0x0264	0x10      0x10 0x00 0x00 0x00 0x00 0x00
		 *   J2: 0x0284	          0x01 0x10 0x00
		 *   H1: 0x0C24	   W           0x28 0xA8 0x1C		    0x0F
		 *   H3: 0x0C64	               0x03 0x07 0x0F
		 *   D1: 0x00C4	0x0F	       0x3F 0x3F 0x0F 0xXC 0x3F
		 *    I: 0x0224	                         0x02 0x00 0x08
		 * MP03: 0x0324	                         0x9x      0xbx 0x9x
		 * MP05: 0x0364	                         0x80      0x88
		 */

		/* C110 Aquila */
		if (gpio_get_value(&gpio->gpio_j1, 4) == 0) {
			board = MACH_TYPE_AQUILA;
			board_rev |= J1_B2_BOARD;

			gpio_set_pull(&gpio->gpio_j2, 6, GPIO_PULL_NONE);
			gpio_direction_input(&gpio->gpio_j2, 6);

			/* Check board */
			if (gpio_get_value(&gpio->gpio_h1, 2) == 0)
				board_rev |= LIMO_UNIVERSAL_BOARD;

			if (gpio_get_value(&gpio->gpio_h3, 2) == 0)
				board_rev |= LIMO_REAL_BOARD;

			if (gpio_get_value(&gpio->gpio_j2, 6) == 1)
				board_rev |= MEDIA_BOARD;

			/* set gpio to default value. */
			gpio_set_pull(&gpio->gpio_j2, 6, GPIO_PULL_DOWN);
			gpio_direction_output(&gpio->gpio_j2, 6, 0);
		}

		/* Workaround: C110 Aquila Rev0.6 */
		if (board_rev == 6) {
			board = MACH_TYPE_AQUILA;
			board_rev |= LIMO_REAL_BOARD;
		}

		/* C110 Aquila Bamboo */
		if (gpio_get_value(&gpio->gpio_j2, 0) == 1) {
			board = MACH_TYPE_AQUILA;
			board_rev |= BAMBOO_BOARD;
		}

		/* C110 TickerTape */
		if (gpio_get_value(&gpio->gpio_d1, 0) == 0 &&
				gpio_get_value(&gpio->gpio_d1, 1) == 0)
			board = MACH_TICKERTAPE;

		/* WMG160 - GPH3[0:4] = 0x00 */
		if (board == MACH_TICKERTAPE) {
			int i, wmg160 = 1;

			for (i = 0; i < 4; i++) {
				if (gpio_get_value(&gpio->gpio_h3, i) != 0) {
					wmg160 = 0;
					break;
				}
			}
			if (wmg160) {
				board = MACH_WMG160;
				hwrev3 = 7;
			}
		}

		/* C110 Geminus for rev0.0 */
		gpio_set_pull(&gpio->gpio_j1, 2, GPIO_PULL_NONE);
		gpio_direction_input(&gpio->gpio_j1, 2);
		if (gpio_get_value(&gpio->gpio_j1, 2) == 1) {
			board = MACH_GEMINUS;
			if ((board_rev & ~BOARD_MASK) == 3)
				board_rev &= ~0xff;
		}
		gpio_set_pull(&gpio->gpio_j1, 2, GPIO_PULL_DOWN);
		gpio_direction_output(&gpio->gpio_j1, 2, 0);

		/* C110 Geminus for rev0.1 ~ */
		gpio_set_pull(&gpio->gpio_j0, 6, GPIO_PULL_NONE);
		gpio_direction_input(&gpio->gpio_j0, 6);
		if (gpio_get_value(&gpio->gpio_j0, 6) == 1) {
			board = MACH_GEMINUS;
			hwrev3 = 7;
		}
		gpio_set_pull(&gpio->gpio_j0, 6, GPIO_PULL_DOWN);

		/* Kessler MP0_5[6] == 1 */
		gpio_direction_input(&gpio->gpio_mp0_5, 6);
		if (gpio_get_value(&gpio->gpio_mp0_5, 6) == 1) {
			/* Cypress: Do this for cypress */
			gpio_set_pull(&gpio->gpio_j2, 2, GPIO_PULL_NONE);
			gpio_direction_input(&gpio->gpio_j2, 2);
			if (gpio_get_value(&gpio->gpio_j2, 2) == 1) {
				board = MACH_CYPRESS;
				gpio_direction_output(&gpio->gpio_mp0_5, 6, 0);
			} else {
				board = MACH_GONI;
				board_rev |= KESSLER_BOARD;

				/* Limo SDK MP0_5[4] == 1 */
				gpio_direction_input(&gpio->gpio_mp0_5, 4);
				if (gpio_get_value(&gpio->gpio_mp0_5, 4) == 1) {
					board_rev &= ~KESSLER_BOARD;
					board_rev |= SDK_BOARD;
				}
			}
			gpio_set_pull(&gpio->gpio_j2, 2, GPIO_PULL_DOWN);
			hwrev3 = 7;
		} else {
			gpio_direction_output(&gpio->gpio_mp0_5, 6, 0);
			/* Goni S1 board detection */
			if (board == MACH_TICKERTAPE) {
				board = MACH_GONI;
				board_rev |= S1_BOARD;
				hwrev3 = 7;
			}
		}

		board_rev |= get_hw_revision(&gpio->gpio_j0, hwrev3);
	}

	/* Set machine id */
	if (board < MACH_PSEUDO_END) {
		if (cpu_is_s5pc110())
			bd.bi_arch_number = C110_MACH_START + board;
		else
			bd.bi_arch_number = C100_MACH_START + board;
	} else {
		bd.bi_arch_number = board;
	}
}

static void show_hw_revision(void)
{
	int board;

	/*
	 * Workaround for Rev 0.3 + CP Ver ES 3.1
	 * it's Rev 0.4
	 */
	if (board_is_limo_real()) {
		if (hwrevision(0)) {
			/* default is Rev 0.4 */
			board_rev &= ~0xf;
			board_rev |= 0x4;
		}
	}

	if (mach_is_goni() || mach_is_aquila())
		board = bd.bi_arch_number;
	else if (cpu_is_s5pc110())
		board = bd.bi_arch_number - C110_MACH_START;
	else
		board = bd.bi_arch_number - C100_MACH_START;

	check_board_revision(board, board_rev);

	/* Set CPU Revision */
	if (mach_is_aquila()) {
		if (board_is_limo_real()) {
			if ((board_rev & 0xf) < 8)
				s5pc1xx_set_cpu_rev(0);
		}
	} else if (mach_is_goni()) {
		if (board_is_sdk() && hwrevision(2))
			s5pc1xx_set_cpu_rev(2);	/* EVT1-Fused */
		else
			s5pc1xx_set_cpu_rev(1);
	} else if (mach_is_geminus()) {
		if ((board_rev & 0xf) < 1)
			s5pc1xx_set_cpu_rev(0);
	} else {
		s5pc1xx_set_cpu_rev(0);
	}

	if (cpu_is_s5pc110())
		writel(0xc1100000 | (0xffff & (s5pc1xx_get_cpu_rev() ? 1 : 0)),
				S5PC110_INFORM3);
}

static int check_keypad(void)
{
	uint condition = 0;
	uint reg, value;
	uint col_num, row_num;
	uint col_mask;
	uint col_mask_shift;
	uint row_state[4] = {0, 0, 0, 0};
	uint i;
	int cpu_rev;

	if (cpu_is_s5pc100()) {
		struct s5pc100_gpio *gpio =
			(struct s5pc100_gpio *)S5PC100_GPIO_BASE;

		row_num = 3;
		col_num = 3;

		/* Set GPH2[2:0] to KP_COL[2:0] */
		gpio_cfg_pin(&gpio->gpio_h2, 0, 0x3);
		gpio_cfg_pin(&gpio->gpio_h2, 1, 0x3);
		gpio_cfg_pin(&gpio->gpio_h2, 2, 0x3);

		/* Set GPH3[2:0] to KP_ROW[2:0] */
		gpio_cfg_pin(&gpio->gpio_h3, 0, 0x3);
		gpio_cfg_pin(&gpio->gpio_h3, 1, 0x3);
		gpio_cfg_pin(&gpio->gpio_h3, 2, 0x3);

		reg = S5PC100_KEYPAD_BASE;
		col_mask = S5PC1XX_KEYIFCOL_MASK;
		col_mask_shift = 0;
	} else {
		struct s5pc110_gpio *gpio =
			(struct s5pc110_gpio *) S5PC110_GPIO_BASE;

		if (board_is_limo_real() || board_is_limo_universal()) {
			row_num = 2;
			col_num = 3;
		} else {
			row_num = 4;
			col_num = 4;
		}

		for (i = 0; i < row_num; i++) {
			/* Set GPH3[3:0] to KP_ROW[3:0] */
			gpio_cfg_pin(&gpio->gpio_h3, i, 0x3);
			gpio_set_pull(&gpio->gpio_h3, i, GPIO_PULL_UP);
		}

		for (i = 0; i < col_num; i++)
			/* Set GPH2[3:0] to KP_COL[3:0] */
			gpio_cfg_pin(&gpio->gpio_h2, i, 0x3);

		reg = S5PC110_KEYPAD_BASE;
		col_mask = S5PC110_KEYIFCOLEN_MASK;
		col_mask_shift = 8;
	}

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

	cpu_rev = s5pc1xx_get_cpu_rev();
	if (cpu_rev == 1) {
		if ((row_state[1] & 0x6) == 0x6)
			condition = 1;
	} else if (cpu_rev == 2) {
		if ((row_state[2] & 0x6) == 0x6)
			condition = 1;
	} else {
		if ((row_state[1] & 0x3) == 0x3)
			condition = 1;
	}

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

#ifdef CONFIG_S5PC1XX
	/* Workaround: for prevent revision mismatch */
	if (cpu_is_s5pc110() && (ofs == 0)) {
		int img_rev;

		if (*buf == 0xea000012)
			img_rev = 0;
		else if (*(buf + 0x400) == 0xea000012)
			img_rev = 2;
		else
			img_rev = 1;

		if (img_rev != s5pc1xx_get_cpu_rev()) {
			PUTS("target check: CPU revision mismatch!\n");
			PUTS("target check: system is ");
			if (s5pc1xx_get_cpu_rev() == 1)
				serial_puts("EVT1\n");
			else if (s5pc1xx_get_cpu_rev() == 2)
				serial_puts("EVT1-Fused\n");
			else
				serial_puts("EVT0\n");
			PUTS("target check: download image is ");
			if (img_rev == 1)
				serial_puts("EVT1\n");
			else if (img_rev == 2)
				serial_puts("EVT1-Fused\n");
			else
				serial_puts("EVT0\n");
			PUTS("try again.");
			return 1;
		}
	}
#endif

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
	struct s5pc110_gpio *gpio_base =
		(struct s5pc110_gpio *) S5PC110_GPIO_BASE;

	/* basic arch cpu dependent setup */
	arch_cpu_init();

	/* Check H/W Revision */
	check_hw_revision();

	show_hw_revision();

	/* set GPIO to enable UART2 */
	gpio_cfg_pin(&gpio_base->gpio_a1, 0, 0x2);
	gpio_cfg_pin(&gpio_base->gpio_a1, 1, 0x2);

	/* UART_SEL MP0_5[7] at S5PC110 */
	gpio_direction_output(&gpio_base->gpio_mp0_5, 7, 0x1);
	gpio_set_pull(&gpio_base->gpio_mp0_5, 7, GPIO_PULL_DOWN);
}
