/*
 * OneNAND initialization at U-Boot
 */

#include <common.h>
#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#include <onenand_uboot.h>

#include <s5pc1xx-onenand.h>

#include <asm/io.h>

#define DPRINTK(format, args...)					\
do {									\
	printk("%s[%d]: " format "\n", __func__, __LINE__, ##args);	\
} while (0)

void onenand_board_init(struct mtd_info *mtd)
{
        struct onenand_chip *this = mtd->priv;
        int value;

	this->base = (void *)CONFIG_SYS_ONENAND_BASE;

	s3c_onenand_init(mtd);

	/* D0 Domain clock gating */
	value = S5P_CLK_GATE_D00_REG;
	value &= ~(1 << 2);
	value |= (1 << 2);
	S5P_CLK_GATE_D00_REG = value;

	value = S5P_CLK_SRC0_REG;
	value &= ~(1 << 24);
	value &= ~(1 << 20);
	S5P_CLK_SRC0_REG = value;

	/* SYSCON */
	value = S5P_CLK_DIV1_REG;
	value &= ~(3 << 16);
	value |= (1 << 16);
	S5P_CLK_DIV1_REG = value;

	INT_ERR_MASK0_REG = 0x17ff;
	INT_PIN_ENABLE0_REG = (1 << 0); /* Enable */
	INT_ERR_MASK0_REG &= ~(1 << 11);        /* ONENAND_INT_ERR_RDY_ACT */

	MEM_RESET0_REG = ONENAND_MEM_RESET_COLD;

	while (!(INT_ERR_STAT0_REG & (RST_CMP|INT_ACT)))
		continue;

	INT_ERR_ACK0_REG |= RST_CMP | INT_ACT;

	ACC_CLOCK0_REG = 0x2;

	/* MEM_CFG0_REG = 0x46e0; */
	MEM_CFG0_REG =
#ifdef CONFIG_SYNC_MODE
		ONENAND_SYS_CFG1_SYNC_READ |
#endif
		ONENAND_SYS_CFG1_BRL_4 |
		ONENAND_SYS_CFG1_BL_16 |
		ONENAND_SYS_CFG1_RDY |
		ONENAND_SYS_CFG1_INT |
		ONENAND_SYS_CFG1_IOBE
		;

	BURST_LEN0_REG = 16;

	s3c_set_width_regs(this);
}
