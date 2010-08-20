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
#include <i2c.h>
#include <asm/arch/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

static struct s5pc210_gpio_part1 *gpio1;
static struct s5pc210_gpio_part2 *gpio2;

enum {
	I2C_0,
	I2C_1,
	I2C_2,
	I2C_3,
	I2C_4,
	I2C_5,
	I2C_6,
	I2C_7,
};

/* i2c0 (CAM)	SDA: GPD1[0] SCL: GPD1[1] */
static struct i2c_gpio_bus_data i2c_0 = {
	.sda_pin	= 0,
	.scl_pin	= 1,
};

/* i2c1 (Gryo)	SDA: GPD1[2] SCL: GPD1[3] */
static struct i2c_gpio_bus_data i2c_1 = {
	.sda_pin	= 2,
	.scl_pin	= 3,
};

/* i2c2		SDA: GPA0[6] SCL: GPA0[7] */
static struct i2c_gpio_bus_data i2c_2 = {
	.sda_pin	= 6,
	.scl_pin	= 7,
};

/* i2c3 (TSP)	SDA: GPA1[2] SCL: GPA1[3] */
static struct i2c_gpio_bus_data i2c_3 = {
	.sda_pin	= 2,
	.scl_pin	= 3,
};

/* i2c4		SDA: GPB[2] SCL: GPB[3] */
static struct i2c_gpio_bus_data i2c_4 = {
	.sda_pin	= 2,
	.scl_pin	= 3,
};

/* i2c5 (PMIC)	SDA: GPB[6] SCL: GPB[7] */
static struct i2c_gpio_bus_data i2c_5 = {
	.sda_pin	= 6,
	.scl_pin	= 7,
};

/* i2c6 (CODEC)	SDA: GPC1[3] SCL: GPC1[4] */
static struct i2c_gpio_bus_data i2c_6 = {
	.sda_pin	= 3,
	.scl_pin	= 4,
};

/* i2c7		SDA: GPD0[2] SCL: GPD0[3] */
static struct i2c_gpio_bus_data i2c_7 = {
	.sda_pin	= 2,
	.scl_pin	= 3,
};

static struct i2c_gpio_bus i2c_gpio[] = {
	{ .bus	= &i2c_0, },
	{ .bus	= &i2c_1, },
	{ .bus	= &i2c_2, },
	{ .bus	= &i2c_3, },
	{ .bus	= &i2c_4, },
	{ .bus	= &i2c_5, },
	{ .bus	= &i2c_6, },
	{ .bus	= &i2c_7, },
};

void i2c_init_board(void)
{
	int num_bus;

	num_bus = ARRAY_SIZE(i2c_gpio);

	i2c_gpio[I2C_0].bus->gpio_base = (unsigned int)&gpio1->gpio_d1;
	i2c_gpio[I2C_1].bus->gpio_base = (unsigned int)&gpio1->gpio_d1;
	i2c_gpio[I2C_2].bus->gpio_base = (unsigned int)&gpio1->gpio_a0;
	i2c_gpio[I2C_3].bus->gpio_base = (unsigned int)&gpio1->gpio_a1;
	i2c_gpio[I2C_4].bus->gpio_base = (unsigned int)&gpio1->gpio_b;
	i2c_gpio[I2C_5].bus->gpio_base = (unsigned int)&gpio1->gpio_b;
	i2c_gpio[I2C_6].bus->gpio_base = (unsigned int)&gpio1->gpio_c1;
	i2c_gpio[I2C_7].bus->gpio_base = (unsigned int)&gpio1->gpio_d0;

	i2c_gpio_init(i2c_gpio, num_bus, I2C_5);
}

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

static void check_auto_burn(void)
{
	unsigned long magic_base = CONFIG_SYS_SDRAM_BASE + 0x02000000;
	unsigned int count = 0;
	char buf[64];

	if (readl(magic_base) == 0x426f6f74) {		/* ASICC: Boot */
		puts("Auto buring bootloader\n");
		count += sprintf(buf + count, "run updateb; ");
	}
	if (readl(magic_base + 0x4) == 0x4b65726e) {	/* ASICC: Kern */
		puts("Auto buring kernel\n");
		count += sprintf(buf + count, "run updatek; ");
	}

	if (count) {
		count += sprintf(buf + count, "reset");
		setenv("bootcmd", buf);
	}

	/* Clear the magic value */
	memset(magic_base, 0, 2);
}

#define MAX8998_REG_ONOFF1	0x11
#define MAX8998_REG_ONOFF2	0x12
#define MAX8998_REG_ONOFF3	0x13
#define MAX8998_REG_ONOFF4	0x14
#define MAX8998_REG_LDO7	0x21
#define MAX8998_REG_LDO17	0x29
/* ONOFF1 */
#define MAX8998_LDO3		(1 << 2)
/* ONOFF2 */
#define MAX8998_LDO6		(1 << 7)
#define MAX8998_LDO7		(1 << 6)
#define MAX8998_LDO8		(1 << 5)
#define MAX8998_LDO9		(1 << 4)
#define MAX8998_LDO10		(1 << 3)
#define MAX8998_LDO11		(1 << 2)
#define MAX8998_LDO12		(1 << 1)
#define MAX8998_LDO13		(1 << 0)
/* ONOFF3 */
#define MAX8998_LDO14		(1 << 7)
#define MAX8998_LDO15		(1 << 6)
#define MAX8998_LDO16		(1 << 5)
#define MAX8998_LDO17		(1 << 4)

static int max8998_probe(void)
{
	unsigned char addr = 0xCC >> 1;

	i2c_set_bus_num(I2C_5);

	if (i2c_probe(addr)) {
		puts("Can't found max8998\n");
		return 1;
	}

	return 0;
}

static void init_pmic(void)
{
	unsigned char addr;
	unsigned char val[2];

	addr = 0xCC >> 1;	/* max8998 */
	if (max8998_probe())
		return;

	/* ONOFF1 */
#if 0
	/* disable for usb.. will be enabled */
	i2c_read(addr, MAX8998_REG_ONOFF1, 1, val, 1);
	val[0] &= ~MAX8998_LDO3;
	i2c_write(addr, MAX8998_REG_ONOFF1, 1, val, 1);
#endif

	/* ONOFF2 */
	i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	/*
	 * Disable LDO10(VPLL_1.1V), LDO11(CAM_IO_2.8V),
	 * LDO12(CAM_ISP_1.2V), LDO13(CAM_A_2.8V)
	 */
	val[0] &= ~(MAX8998_LDO10 | MAX8998_LDO11 |
			MAX8998_LDO12 | MAX8998_LDO13);

	val[0] |= MAX8998_LDO7;		/* LDO7: VLCD_1.8V */

	i2c_write(addr, MAX8998_REG_ONOFF2, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF2, 1, val, 1);

	/* ONOFF3 */
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	/*
	 * Disable LDO14(CAM_CIF_1.8), LDO15(CAM_AF_3.3V),
	 * LDO16(VMIPI_1.8V), LDO17(CAM_8M_1.8V)
	 */
	val[0] &= ~(MAX8998_LDO14 | MAX8998_LDO15 |
			MAX8998_LDO16 | MAX8998_LDO17);

	val[0] |= MAX8998_LDO17;	/* LDO17: VCC_3.0V_LCD */

	i2c_write(addr, MAX8998_REG_ONOFF3, 1, val, 1);
	i2c_read(addr, MAX8998_REG_ONOFF3, 1, val, 1);
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	check_auto_burn();

	/* check max8998 */
	init_pmic();

	return 0;
}
#endif

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

	return s5p_mmc_init(0, 8);
}
#endif
