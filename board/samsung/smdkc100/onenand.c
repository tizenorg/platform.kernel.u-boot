/*
 * OneNAND initialization at U-Boot
 */

#include <common.h>
#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#include <onenand_uboot.h>

#include <samsung_onenand.h>

#include <asm/io.h>
#include <asm/arch/clock.h>

extern void s3c_onenand_init(struct mtd_info *);

static inline int onenand_read_reg(int offset)
{
	return readl(CONFIG_SYS_ONENAND_BASE + offset);
}

static inline void onenand_write_reg(int value, int offset)
{
	writel(value, CONFIG_SYS_ONENAND_BASE + offset);
}

void onenand_board_init(struct mtd_info *mtd)
{
        struct onenand_chip *this = mtd->priv;
        int value;

	this->base = (void *)CONFIG_SYS_ONENAND_BASE;

#if 0
	/* D0 Domain system 1 clock gating */
	value = readl(S5P_CLK_GATE_D00);
	value &= ~(1 << 2);		/* CFCON */
	value |= (1 << 2);
	writel(value, S5P_CLK_GATE_D00);

	/* D0 Domain memory clock gating */
	value = readl(S5P_CLK_GATE_D01);
	value &= ~(1 << 2);		/* CLK_ONENANDC */
	value |= (1 << 2);
	writel(value, S5P_CLK_GATE_D01);
#endif

	/* System Special clock gating */
	value = readl(S5P_CLK_GATE_SCLK0);
	value &= ~(1 << 2);		/* OneNAND */
	value |= (1 << 2);
	writel(value, S5P_CLK_GATE_SCLK0);

	value = readl(S5P_CLK_SRC0);
	value &= ~(1 << 24);		/* MUX_1nand: 0 from HCLKD0 */
//	value |= (1 << 24);		/* MUX_1nand: 1 from DIV_D1_BUS */
	value &= ~(1 << 20);		/* MUX_HREF: 0 from FIN_27M */
	writel(value, S5P_CLK_SRC0);

	value = readl(S5P_CLK_DIV1);
//	value &= ~(3 << 20);		/* DIV_1nand: 1 / (ratio+1) */
//	value |= (0 << 20);		/* ratio = 1 */
	value &= ~(3 << 16);
	value |= (1 << 16);
	writel(value, S5P_CLK_DIV1);

	onenand_write_reg(ONENAND_MEM_RESET_COLD, MEM_RESET_OFFSET);

	while (!(onenand_read_reg(INT_ERR_STAT_OFFSET) & RST_CMP))
		continue;

	onenand_write_reg(RST_CMP, INT_ERR_ACK_OFFSET);

	onenand_write_reg(0x3, ACC_CLOCK_OFFSET);

	onenand_write_reg(0x3fff, INT_ERR_MASK_OFFSET);
	onenand_write_reg(1 << 0, INT_PIN_ENABLE_OFFSET); /* Enable */

	value = onenand_read_reg(INT_ERR_MASK_OFFSET);
	value &= ~RDY_ACT;
	onenand_write_reg(value, INT_ERR_MASK_OFFSET);

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

	s3c_onenand_init(mtd);
}
