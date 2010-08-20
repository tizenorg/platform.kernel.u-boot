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

#define LP3974_REG_ONOFF1	0x11
#define LP3974_REG_ONOFF2	0x12
#define LP3974_REG_ONOFF3	0x13
#define LP3974_REG_ONOFF4	0x14
#define LP3974_REG_LDO7	0x21
#define LP3974_REG_LDO17	0x29
/* ONOFF1 */
#define LP3974_LDO3		(1 << 2)
/* ONOFF2 */
#define LP3974_LDO6		(1 << 7)
#define LP3974_LDO7		(1 << 6)
#define LP3974_LDO8		(1 << 5)
#define LP3974_LDO9		(1 << 4)
#define LP3974_LDO10		(1 << 3)
#define LP3974_LDO11		(1 << 2)
#define LP3974_LDO12		(1 << 1)
#define LP3974_LDO13		(1 << 0)
/* ONOFF3 */
#define LP3974_LDO14		(1 << 7)
#define LP3974_LDO15		(1 << 6)
#define LP3974_LDO16		(1 << 5)
#define LP3974_LDO17		(1 << 4)

static int lp3974_probe(void)
{
	unsigned char addr = 0xCC >> 1;

	i2c_set_bus_num(I2C_5);

	if (i2c_probe(addr)) {
		puts("Can't found lp3974\n");
		return 1;
	}

	return 0;
}

static int max8952_probe(void)
{
	unsigned char addr = 0xC0 >> 1;

	i2c_set_bus_num(I2C_5);

	if (i2c_probe(addr)) {
		puts("Cannot find MAX8952\n");
		return 1;
	}

	return 0;
}

static void init_pmic_lp3974(void)
{
	unsigned char addr;
	unsigned char val[2];

	addr = 0xCC >> 1;	/* lp3974 */
	if (lp3974_probe())
		return;

	/* ONOFF1 */
	i2c_read(addr, LP3974_REG_ONOFF1, 1, val, 1);
	val[0] &= ~LP3974_LDO3;
	i2c_write(addr, LP3974_REG_ONOFF1, 1, val, 1);

	/* ONOFF2 */
	i2c_read(addr, LP3974_REG_ONOFF2, 1, val, 1);
	/*
	 * Disable LDO10(VPLL_1.1V), LDO11(CAM_IO_2.8V),
	 * LDO12(CAM_ISP_1.2V), LDO13(CAM_A_2.8V)
	 */
	val[0] &= ~(LP3974_LDO10 | LP3974_LDO11 |
			LP3974_LDO12 | LP3974_LDO13);

	val[0] |= LP3974_LDO7;		/* LDO7: VLCD_1.8V */

	i2c_write(addr, LP3974_REG_ONOFF2, 1, val, 1);
	i2c_read(addr, LP3974_REG_ONOFF2, 1, val, 1);

	/* ONOFF3 */
	i2c_read(addr, LP3974_REG_ONOFF3, 1, val, 1);
	/*
	 * Disable LDO14(CAM_CIF_1.8), LDO15(CAM_AF_3.3V),
	 * LDO16(VMIPI_1.8V), LDO17(CAM_8M_1.8V)
	 */
	val[0] &= ~(LP3974_LDO14 | LP3974_LDO15 |
			LP3974_LDO16 | LP3974_LDO17);

	val[0] |= LP3974_LDO17;	/* LDO17: VCC_3.0V_LCD */

	i2c_write(addr, LP3974_REG_ONOFF3, 1, val, 1);
	i2c_read(addr, LP3974_REG_ONOFF3, 1, val, 1);
}

static void init_pmic_max8952(void)
{
	unsigned char addr;
	unsigned char val[2];
	char buf[4];

	addr = 0xC0 >> 1; /* MAX8952 */
	if (max8952_probe())
		return;

	/* VARM_OUTPUT_SEL_A / VID_0 / XEINT_3 (GPX0[3]) = default 0 */
	gpio_direction_output(&gpio2->gpio_x0, 3, 0);
	/* VARM_OUTPUT_SEL_B / VID_1 / XEINT_4 (GPX0[4]) = default 0 */
	gpio_direction_output(&gpio2->gpio_x0, 4, 0);

	/* MODE0: 1.25V */
	val[0] = 48;
	i2c_write(addr, 0x00, 1, val, 1);
	/* MODE1: 1.20V */
	val[0] = 32;
	i2c_write(addr, 0x01, 1, val, 1);
	/* MODE2: 1.05V */
	val[0] = 28;
	i2c_write(addr, 0x02, 1, val, 1);
	/* MODE3: 0.95V */
	val[0] = 18;
	i2c_write(addr, 0x03, 1, val, 1);

	/* CONTROL: Disable PULL_DOWN */
	val[0] = 0;
	i2c_write(addr, 0x04, 1, val, 1);

	/* SYNC: Do Nothing */
	/* RAMP: As Fast As Possible: Default: Do Nothing */
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	check_auto_burn();

	init_pmic_lp3974();
	init_pmic_max8952();

	return 0;
}
#endif

#ifdef CONFIG_CMD_USBDOWN
int usb_board_init(void)
{
#ifdef CONFIG_CMD_PMIC
	run_command("pmic ldo 3 on", 0);
#endif
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

#ifdef CONFIG_CMD_PMIC
static int pmic_status(void)
{
	unsigned char addr, val[2];
	int reg, i;

	addr = 0xCC >> 1;

	if (lp3974_probe())
		return -1;

	reg = 0x11;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 4; i--)
		printf("BUCK%d %s\n", 7 - i + 1,
			val[0] & (1 << i) ? "on" : "off");
	for (; i >= 0; i--)
		printf("LDO%d %s\n", 5 - i,
			val[0] & (1 << i) ? "on" : "off");
	reg = 0x12;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 0; i--)
		printf("LDO%d %s\n", 7 - i + 6,
			val[0] & (1 << i) ? "on" : "off");
	reg = 0x13;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 4; i--)
		printf("LDO%d %s\n", 7 - i + 14,
			val[0] & (1 << i) ? "on" : "off");

	reg = 0xd;
	i2c_read(addr, reg, 1, val, 1);
	for (i = 7; i >= 6; i--)
		printf("SAFEOUT%d %s\n", 7 - i + 1,
			val[0] & (1 << i) ? "on" : "off");
	return 0;
}

static int pmic_ldo_control(int buck, int ldo, int safeout, int on)
{
	unsigned char addr, val[2];
	unsigned int reg, shift;

	if (ldo) {
		if (ldo < 2)
			return -1;
		if (ldo <= 5) {
			reg = 0x11;
			shift = 5 - ldo;
		} else if (ldo <= 13) {
			reg = 0x12;
			shift = 13 - ldo;
		} else if (ldo <= 17) {
			reg = 0x13;
			shift = 17 - ldo + 4;
		} else
			return -1;
	} else if (buck) {
		if (buck > 4)
			return -1;
		reg = 0x11;
		shift = 4 - buck + 4;
	} else if (safeout) {
		if (safeout > 3)
			return -1;
		reg = 0xd;
		shift = 8 - safeout;
	} else
		return -1;

	addr = 0xCC >> 1;

	if (lp3974_probe())
		return -1;

	i2c_read(addr, reg, 1, val, 1);
	if (on)
		val[0] |= (1 << shift);
	else
		val[0] &= ~(1 << shift);
	i2c_write(addr, reg, 1, val, 1);
	i2c_read(addr, reg, 1, val, 1);

	if (ldo)
		printf("ldo %d value 0x%x, %s\n", ldo, val[0],
			val[0] & (1 << shift) ? "on" : "off");
	else if (buck)
		printf("buck %d value 0x%x, %s\n", buck, val[0],
			val[0] & (1 << shift) ? "on" : "off");
	else if (safeout)
		printf("safeout %d value 0x%x, %s\n", safeout, val[0],
			val[0] & (1 << shift) ? "on" : "off");

	return 0;
}

static int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int buck = 0, ldo = 0, safeout = 0, on = -1;

	switch (argc) {
	case 2:
		if (strncmp(argv[1], "status", 6) == 0)
			return pmic_status();
		break;
	case 4:
		if (strncmp(argv[1], "ldo", 3) == 0)
			ldo = simple_strtoul(argv[2], NULL, 10);
		else if (strncmp(argv[1], "buck", 4) == 0)
			buck = simple_strtoul(argv[2], NULL, 10);
		else if (strncmp(argv[1], "safeout", 7) == 0)
			safeout = simple_strtoul(argv[2], NULL, 10);
		else
			break;

		if (strncmp(argv[3], "on", 2) == 0)
			on = 1;
		else if (strncmp(argv[3], "off", 3) == 0)
			on = 0;
		else
			break;

		return pmic_ldo_control(buck, ldo, safeout, on);

	default:
		break;
	}

	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	pmic,		4,	1, do_pmic,
	"PMIC LDO & BUCK control",
	"status - Display PMIC LDO & BUCK status\n"
	"pmic ldo num on/off - Turn on/off the LDO\n"
	"pmic buck num on/off - Turn on/off the BUCK\n"
	"pmic safeout num on/off - Turn on/off the SAFEOUT\n"
);
#endif
