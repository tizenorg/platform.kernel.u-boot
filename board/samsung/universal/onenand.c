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
#include <asm/arch/cpu.h>

extern void s3c_onenand_init(struct mtd_info *);

static inline int onenand_read_reg(int offset)
{
	return readl(S5PC100_ONENAND_BASE + offset);
}

static inline void onenand_write_reg(int value, int offset)
{
	writel(value, S5PC100_ONENAND_BASE + offset);
}

static int s5pc1xx_clock_read(int offset)
{
	return readl(S5PC1XX_CLOCK_BASE + offset);
}

static void s5pc1xx_clock_write(int value, int offset)
{
	writel(value, S5PC1XX_CLOCK_BASE + offset);
}

void onenand_board_init(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;
	int value;

	if (cpu_is_s5pc110()) {
		this->base = (void *) 0xB0000000;
		this->options |= ONENAND_RUNTIME_BADBLOCK_CHECK;
		this->options |= ONENAND_HAS_4KB_PAGE;
	} else {
		this->base = (void *) S5PC100_ONENAND_BASE;

		/* D0 Domain system 1 clock gating */
		value = s5pc1xx_clock_read(S5P_CLK_GATE_D00_OFFSET);
		value &= ~(1 << 2);		/* CFCON */
		value |= (1 << 2);
		s5pc1xx_clock_write(value, S5P_CLK_GATE_D00_OFFSET);

		/* D0 Domain memory clock gating */
		value = s5pc1xx_clock_read(S5P_CLK_GATE_D01_OFFSET);
		value &= ~(1 << 2);		/* CLK_ONENANDC */
		value |= (1 << 2);
		s5pc1xx_clock_write(value, S5P_CLK_GATE_D01_OFFSET);

		/* System Special clock gating */
		value = s5pc1xx_clock_read(S5P_CLK_GATE_SCLK0_OFFSET);
		value &= ~(1 << 2);		/* OneNAND */
		value |= (1 << 2);
		s5pc1xx_clock_write(value, S5P_CLK_GATE_SCLK0_OFFSET);

		value = s5pc1xx_clock_read(S5P_CLK_SRC0_OFFSET);
		value &= ~(1 << 24);		/* MUX_1nand: 0 from HCLKD0 */
	//	value |= (1 << 24);		/* MUX_1nand: 1 from DIV_D1_BUS */
		value &= ~(1 << 20);		/* MUX_HREF: 0 from FIN_27M */
		s5pc1xx_clock_write(value, S5P_CLK_SRC0_OFFSET);

		value = s5pc1xx_clock_read(S5P_CLK_DIV1_OFFSET);
	//	value &= ~(3 << 20);		/* DIV_1nand: 1 / (ratio+1) */
	//	value |= (0 << 20);		/* ratio = 1 */
		value &= ~(3 << 16);
		value |= (1 << 16);
		s5pc1xx_clock_write(value, S5P_CLK_DIV1_OFFSET);

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

		s3c_onenand_init(mtd);
	}
}
