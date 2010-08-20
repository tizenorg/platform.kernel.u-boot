/*
 *  Copyright (C) 2010 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

static struct s5pc210_gpio_part1 *gpio1;
static struct s5pc210_gpio_part2 *gpio2;

int board_init(void)
{
	gpio1 = (struct s5pc210_gpio_part1 *) S5PC210_GPIO_PART1_BASE;
	gpio2 = (struct s5pc210_gpio_part2 *) S5PC210_GPIO_PART2_BASE;

	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;

	return 0;
}

#ifdef CONFIG_GENERIC_MMC
int s5p_no_mmc_support(void)
{
	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int i;

	/* Workaround: set the low to enable LDO_EN */
	/* MASSMEMORY_EN: XMDMDATA_6: GPE3[6] */
	gpio_direction_output(&gpio1->gpio_e3, 6, 0);

	/*
	 * eMMC GPIO:
	 * SDR 8-bit@48MHz at MMC0
	 * GPK0[0]	SD_0_CLK(2)
	 * GPK0[1]	SD_0_CMD(2)
	 * GPK0[2]	SD_0_CDn	-> Not used
	 * GPK0[3:6]	SD_0_DATA[0:3](2)
	 * GPK1[3:6]	SD_0_DATA[0:3](3)
	 *
	 * DDR 4-bit@26MHz at MMC4
	 * GPK0[0]	SD_4_CLK(3)
	 * GPK0[1]	SD_4_CMD(3)
	 * GPK0[2]	SD_4_CDn	-> Not used
	 * GPK0[3:6]	SD_4_DATA[0:3](3)
	 * GPK1[3:6]	SD_4_DATA[4:7](4)
	 */
	for (i = 0; i < 7; i++) {
		if (i == 2)
			continue;
		/* GPK0[0:6] special function 2 */
		gpio_cfg_pin(&gpio2->gpio_k0, i, 0x2);
		/* GPK0[0:6] pull disable */
		gpio_set_pull(&gpio2->gpio_k0, i, GPIO_PULL_NONE);
		/* GPK0[0:6] drv 4x */
		gpio_set_drv(&gpio2->gpio_k0, i, GPIO_DRV_4X);
	}

	for (i = 3; i < 7; i++) {
		/* GPK1[3:6] special function 3 */
		gpio_cfg_pin(&gpio2->gpio_k1, i, 0x3);
		/* GPK1[3:6] pull disable */
		gpio_set_pull(&gpio2->gpio_k1, i, GPIO_PULL_NONE);
		/* GPK1[3:6] drv 4x */
		gpio_set_drv(&gpio2->gpio_k1, i, GPIO_DRV_4X);
	}

	return s5p_mmc_init(0);
}
#endif
