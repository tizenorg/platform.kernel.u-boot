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

	/* D0 Domain system 1 clock gating */
	value = S5P_CLK_GATE_D00_REG;
	value &= ~(1 << 2);		/* CFCON */
	value |= (1 << 2);
	S5P_CLK_GATE_D00_REG = value;

	/* D0 Domain memory clock gating */
	value = S5P_CLK_GATE_D01_REG;
	value &= ~(1 << 2);		/* CLK_ONENANDC */
	value |= (1 << 2);
	S5P_CLK_GATE_D01_REG = value;

	/* System Special clock gating */
	value = S5P_CLK_GATE_SCLK0_REG;
	value &= ~(1 << 2);		/* OneNAND */
	value |= (1 << 2);
	S5P_CLK_GATE_SCLK0_REG = value;

	value = S5P_CLK_SRC0_REG;
	value &= ~(1 << 24);		/* MUX_1nand: 0 from HCLKD0 */
//	value |= (1 << 24);		/* MUX_1nand: 1 from DIV_D1_BUS */
	value &= ~(1 << 20);		/* MUX_HREF: 0 from FIN_27M */
	S5P_CLK_SRC0_REG = value;

	value = S5P_CLK_DIV1_REG;
//	value &= ~(3 << 20);		/* DIV_1nand: 1 / (ratio+1) */
//	value |= (0 << 20);		/* ratio = 1 */
	value &= ~(3 << 16);
	value |= (1 << 16);
	S5P_CLK_DIV1_REG = value;

	MEM_RESET0_REG = ONENAND_MEM_RESET_COLD;

	while (!(INT_ERR_STAT0_REG & RST_CMP))
		continue;

	INT_ERR_ACK0_REG = RST_CMP;

	ACC_CLOCK0_REG = 0x3;

	INT_ERR_MASK0_REG = 0x3fff;
	INT_PIN_ENABLE0_REG = (1 << 0); /* Enable */

	value = INT_ERR_MASK0_REG;
	value &= ~RDY_ACT;
	INT_ERR_MASK0_REG = value;

#if 0
	MEM_CFG0_REG |=
		ONENAND_SYS_CFG1_SYNC_READ |
		ONENAND_SYS_CFG1_BRL_4 |
		ONENAND_SYS_CFG1_BL_16 |
		ONENAND_SYS_CFG1_RDY |
		ONENAND_SYS_CFG1_INT |
		ONENAND_SYS_CFG1_IOBE
		;
	MEM_CFG0_REG |= ONENAND_SYS_CFG1_RDY;
	MEM_CFG0_REG |=	ONENAND_SYS_CFG1_INT; 
	MEM_CFG0_REG |= ONENAND_SYS_CFG1_IOBE;
#endif
//	MEM_CFG0_REG |= ONENAND_SYS_CFG1_VHF;
//	MEM_CFG0_REG |= ONENAND_SYS_CFG1_HF;

	this->base = (void *) 0xe7100000;
	this->base = (void *)CONFIG_SYS_ONENAND_BASE;

	s3c_onenand_init(mtd);
}
