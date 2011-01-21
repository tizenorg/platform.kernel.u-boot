/*
 *  Copyright (C) 2011 Samsung Electronics
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
#include <lcd.h>
#include <spi.h>
#include <swi.h>
#include <nt39411.h>
#include <asm/io.h>
#include <asm/arch/adc.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/power.h>
#include <asm/arch/clk.h>
#include <ramoops.h>
#include <info_action.h>
#include <pmic.h>

DECLARE_GLOBAL_DATA_PTR;

static struct s5pc210_gpio_part1 *gpio1;
static struct s5pc210_gpio_part2 *gpio2;

static unsigned int battery_soc;
static unsigned int board_rev;

u32 get_board_rev(void)
{
	return board_rev;
}

static int get_hwrev(void)
{
	return board_rev & 0xFF;
}

enum {
	I2C_0, I2C_1, I2C_2, I2C_3,
	I2C_4, I2C_5, I2C_6, I2C_7,
	I2C_8, I2C_9, I2C_NUM,
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

/* i2c3 (TSP)	SDA: GPA1[2] SCL: GPA1[3] */
static struct i2c_gpio_bus_data i2c_3 = {
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

/* i2c9 (Fuel Gauge)	SDA: SPY4[0] SCL: SPY4[1] */
static struct i2c_gpio_bus_data i2c_9 = {
	.sda_pin	= 0,
	.scl_pin	= 1,
};

static struct i2c_gpio_bus i2c_gpio[I2C_NUM];

static void check_battery(int mode);
static void init_pmic_max8997(void);

void i2c_init_board(void)
{
	gpio1 = (struct s5pc210_gpio_part1 *) S5PC210_GPIO_PART1_BASE;
	gpio2 = (struct s5pc210_gpio_part2 *) S5PC210_GPIO_PART2_BASE;

	i2c_gpio[I2C_0].bus = &i2c_0;
	i2c_gpio[I2C_1].bus = &i2c_1;
	i2c_gpio[I2C_2].bus = NULL;
	i2c_gpio[I2C_3].bus = &i2c_3;
	i2c_gpio[I2C_4].bus = NULL;
	i2c_gpio[I2C_5].bus = &i2c_5;
	i2c_gpio[I2C_6].bus = &i2c_6;
	i2c_gpio[I2C_7].bus = &i2c_7;
	i2c_gpio[I2C_8].bus = NULL;
	i2c_gpio[I2C_9].bus = &i2c_9;

	i2c_gpio[I2C_0].bus->gpio_base = (unsigned int)&gpio1->d1;
	i2c_gpio[I2C_1].bus->gpio_base = (unsigned int)&gpio1->d1;
	i2c_gpio[I2C_3].bus->gpio_base = (unsigned int)&gpio1->a1;
	i2c_gpio[I2C_5].bus->gpio_base = (unsigned int)&gpio1->b;
	i2c_gpio[I2C_6].bus->gpio_base = (unsigned int)&gpio1->c1;
	i2c_gpio[I2C_7].bus->gpio_base = (unsigned int)&gpio1->d0;
	i2c_gpio[I2C_9].bus->gpio_base = (unsigned int)&gpio2->y4;

	i2c_gpio_init(i2c_gpio, I2C_NUM, I2C_5);
}

static void check_hw_revision(void);

int board_init(void)
{
	gpio1 = (struct s5pc210_gpio_part1 *) S5PC210_GPIO_PART1_BASE;
	gpio2 = (struct s5pc210_gpio_part2 *) S5PC210_GPIO_PART2_BASE;

	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	check_hw_revision();

	return 0;
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE + PHYS_SDRAM_2_SIZE +
			PHYS_SDRAM_3_SIZE + PHYS_SDRAM_4_SIZE;

	/* Early init for i2c devices - Where these funcions should go?? */

	/* Reset on fuel gauge */
	check_battery(1);

	/* pmic init */
	init_pmic_max8997();

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = PHYS_SDRAM_4_SIZE;

	gd->ram_size = gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size +
			gd->bd->bi_dram[2].size + gd->bd->bi_dram[3].size;
}

static void check_auto_burn(void)
{
	unsigned int magic_base = CONFIG_SYS_SDRAM_BASE + 0x02000000;
	unsigned int count = 0;
	char buf[64];

	/* Initial Setting */
	if (readl(magic_base) == 0x534E5344) {	/* ASICC: SNSD */
		puts("Auto buring intiail Setting (boot image)\n");
		count += sprintf(buf + count, "run setupboot; ");
		goto done;
	}
	/* OneNAND */
	if (readl(magic_base) == 0x426f6f74) {		/* ASICC: Boot */
		puts("Auto buring bootloader\n");
		count += sprintf(buf + count, "run updateb; ");
	}
	/* MMC */
	if (readl(magic_base) == 0x654D4D43) {		/* ASICC: eMMC */
		puts("Auto buring bootloader (eMMC)\n");
		count += sprintf(buf + count, "run updatemmc; ");
	}
	if (readl(magic_base + 0x4) == 0x4b65726e) {	/* ASICC: Kern */
		puts("Auto buring kernel\n");
		count += sprintf(buf + count, "run updatek; ");
	}
	/* Backup u-boot in eMMC */
	if (readl(magic_base + 0x8) == 0x4261636B) {	/* ASICC: Back */
		puts("Auto buring u-boot image (boot partition2 in eMMC)\n");
		count += sprintf(buf + count, "run updatebackup; ");
	}

done:
	if (count) {
		count += sprintf(buf + count, "reset");
		setenv("bootcmd", buf);
	}

	/* Clear the magic value */
	memset((void *)magic_base, 0, 2);
}

static void check_battery(int mode)
{
	unsigned char val[2];
	unsigned char addr = 0x36;	/* max17042 fuel gauge */

	i2c_set_bus_num(I2C_9);

	if (i2c_probe(addr)) {
		puts("Can't found max17042 fuel gauge\n");
		return;
	}

	/* mode 0: check mode / 1: enable mode */
	if (mode) {
		val[0] = 0x00;
		val[1] = 0x00;
		i2c_write(addr, 0x2e, 1, val, 2); /* CGAIN */
		val[0] = 0x03;
		val[1] = 0x00;
		i2c_write(addr, 0x2b, 1, val, 2); /* MiscCFG */
		val[0] = 0x07;
		val[1] = 0x00;
		i2c_write(addr, 0x28, 1, val, 2); /* LearnCFG */
	} else {
		i2c_read(addr, 0x0d, 1, val, 2);
		battery_soc = val[0] + val[1] * 256;
		battery_soc /= 256;
		printf("battery:\t%d%%\n", battery_soc);
	}
}

static int max8997_probe(void)
{
	unsigned char addr = 0xCC >> 1;

	i2c_set_bus_num(I2C_5);

	if (i2c_probe(addr)) {
		puts("Can't found max8997\n");
		return 1;
	}

	return 0;
}

static unsigned char inline max8997_reg_ldo(int uV)
{
	unsigned char ret;
	if (uV <= 800000)
		return 0;
	if (uV >= 3950000)
		return 0x3f;
	ret = (uV - 800000) / 50000;
	if (ret > 0x3f) {
		printf("MAX8997 LDO SETTING ERROR (%duV) -> %u\n", uV, ret);
		ret = 0x3f;
	}

	return ret;
}

static void init_pmic_max8997(void)
{
	/* TURN ON EVERYTHING!!! */

	unsigned char addr;
	unsigned char val[2];

	addr = 0xCC >> 1; /* MAX8997 */
	if (max8997_probe())
		return;

	/* BUCK1 VARM: 1.2V */
	val[0] = (1200000 - 650000) / 25000;
	i2c_write(addr, 0x19, 1, val, 1);
	val[0] = 0x09; /* DVS OFF */
	i2c_write(addr, 0x18, 1, val, 1);

	/* BUCK2 VINT: 1.1V */
	val[0] = (1100000 - 650000) / 25000;
	i2c_write(addr, 0x22, 1, val, 1);
	val[0] = 0x09; /* DVS OFF */
	i2c_write(addr, 0x21, 1, val, 1);

	/* BUCK3 G3D: 1.1V */
	val[0] = (1100000 - 750000) / 50000;
	i2c_write(addr, 0x2b, 1, val, 1);

	/* BUCK4 CAMISP: 1.2V */
	val[0] = (1200000 - 650000) / 25000;
	i2c_write(addr, 0x2d, 1, val, 1);

	/* BUCK5 VMEM: 1.2V */
	val[0] = (1200000 - 650000) / 25000;
	i2c_write(addr, 0x2f, 1, val, 1);
	val[0] = 0x09; /* DVS OFF */
	i2c_write(addr, 0x2e, 1, val, 1);

	/* BUCK6 CAM AF: 2.8V */
	/* No Voltage Setting Register */

	/* BUCK7 VCC_SUB: 2.0V */
	val[0] = (2000000 - 750000) / 50000;
	i2c_write(addr, 0x3a, 1, val, 1);

	/* LDO1 VADC: 3.3V */
	val[0] = max8997_reg_ldo(3300000) | 0xC0;
	i2c_write(addr, 0x3b, 1, val, 1);

	/* LDO2 VALIVE: 1.1V */
	val[0] = max8997_reg_ldo(1100000) | 0xC0;
	i2c_write(addr, 0x3c, 1, val, 1);

	/* LDO3 VUSB/MIPI: 1.1V */
	val[0] = max8997_reg_ldo(1100000) | 0xC0;
	i2c_write(addr, 0x3d, 1, val, 1);

	/* LDO4 VMIPI: 1.8V */
	val[0] = max8997_reg_ldo(1800000) | 0xC0;
	i2c_write(addr, 0x3e, 1, val, 1);

	/* LDO5 VHSIC: 1.2V */
	val[0] = max8997_reg_ldo(1200000) | 0xC0;
	i2c_write(addr, 0x3f, 1, val, 1);

	/* LDO6 VCC_1.8V_PDA: 1.8V */
	val[0] = max8997_reg_ldo(1800000) | 0xC0;
	i2c_write(addr, 0x40, 1, val, 1);

	/* LDO7 CAM_ISP: 1.8V */
	val[0] = max8997_reg_ldo(1800000) | 0xC0;
	i2c_write(addr, 0x41, 1, val, 1);

	/* LDO8 VDAC/VUSB: 3.3V */
	val[0] = max8997_reg_ldo(3300000) | 0xC0;
	i2c_write(addr, 0x42, 1, val, 1);

	/* LDO9 VCC_2.8V_PDA: 2.8V */
	val[0] = max8997_reg_ldo(2800000) | 0xC0;
	i2c_write(addr, 0x43, 1, val, 1);

	/* LDO10 VPLL: 1.1V */
	val[0] = max8997_reg_ldo(1100000) | 0xC0;
	i2c_write(addr, 0x44, 1, val, 1);

	/* LDO11 LVDS: 3.3V */
	val[0] = max8997_reg_ldo(3300000) | 0xC0;
	i2c_write(addr, 0x45, 1, val, 1);

	/* LDO12 VTCAM: 1.8V */
	val[0] = max8997_reg_ldo(1800000) | 0xC0;
	i2c_write(addr, 0x46, 1, val, 1);

	/* LDO13 VTF: 2.8V */
	val[0] = max8997_reg_ldo(2800000) | 0xC0;
	i2c_write(addr, 0x47, 1, val, 1);

	/* LDO14 MOTOR: 3.0V */
	val[0] = max8997_reg_ldo(3000000) | 0xC0;
	i2c_write(addr, 0x48, 1, val, 1);

	/* LDO15 VTOUCH: 2.8V */
	val[0] = max8997_reg_ldo(2800000) | 0xC0;
	i2c_write(addr, 0x49, 1, val, 1);

	/* LDO16 CAM_SENSOR: 1.8V */
	val[0] = max8997_reg_ldo(1800000) | 0xC0;
	i2c_write(addr, 0x4a, 1, val, 1);


	/* LDO18 VTOUCH 2.8V */
	val[0] = max8997_reg_ldo(2800000) | 0xC0;
	i2c_write(addr, 0x4c, 1, val, 1);

	/* LDO21 VDDQ: 1.2V */
	val[0] = max8997_reg_ldo(1200000) | 0xC0;
	i2c_write(addr, 0x4d, 1, val, 1);

	/* SAFEOUT for both 1 and 2: 4.9V, Active discharge, Enable */
	val[0] = (0x1 << 0) | (0x1 << 2) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);
	i2c_write(addr, 0x5a, 1, val, 1);
}

int check_exit_key(void)
{
	return pmic_get_irq(PWRON1S);
}

static void check_keypad(void)
{
	unsigned int val = 0;
	unsigned int power_key, auto_download = 0;

	/* volume down */
	val = !(gpio_get_value(&gpio2->x2, 1));

	power_key = pmic_get_irq(PWRONR);

	if (power_key && (val & 0x1))
		auto_download = 1;

	if (auto_download)
		setenv("bootcmd", "usbdown");
}

/*
 * charger_en(): set max8997 pmic's charger mode
 * enable 0: disable charger
 * 600: 600mA
 * 500: 500mA
 */
static void charger_en(int enable)
{
	unsigned char addr = 0xCC >> 1;
	unsigned char val[2];

	if (max8997_probe())
		return;

	switch (enable) {
	case 0:
		puts("Disable the charger.\n");
		i2c_read(addr, 0x50, 1, val, 1);
		val[0] &= ~0x1;
		i2c_write(addr, 0x50, 1, val, 1);
		break;
	case 500:
		puts("Enable the charger @ 500mA\n");
		/*
		 * CHGCNTL1
		 * ICHG: 500mA (0x3) / 600mA (0x5)
		 * RESTART LEVEL: 100mA (0x1)
		 * EOC LEVEL: 30% (0x4) / 25% (0x3) : both 150mA of ICHG
		 * Let's start with slower charging mode and
		 * let micro usb driver determine whether we can do it
		 * fast or not. Thus, using the slower setting...
		 */
		val[0] = 0x16;
		i2c_write(addr, 0x53, 1, val, 1);
		i2c_read(addr, 0x50, 1, val, 1);
		val[0] |= 0x1;
		i2c_write(addr, 0x50, 1, val, 1);
		break;
	case 600:
		puts("Enable the charger @ 600mA\n");
		val[0] = 0x18;
		i2c_write(addr, 0x53, 1, val, 1);
		i2c_read(addr, 0x50, 1, val, 1);
		val[0] |= 0x1;
		i2c_write(addr, 0x50, 1, val, 1);
		break;
	default:
		puts("Incorrect charger setting.\n");
	}
}

struct thermister_stat {
	short centigrade;
	unsigned short adc;
};

static struct thermister_stat adc_to_temperature_data[] = {
	{ .centigrade = -20,    .adc = 1856, },
	{ .centigrade = -15,    .adc = 1799, },
	{ .centigrade = -10,    .adc = 1730, },
	{ .centigrade = -5,     .adc = 1649, },
	{ .centigrade = 0,      .adc = 1556, },
	{ .centigrade = 5,      .adc = 1454, },
	{ .centigrade = 10,     .adc = 1343, },
	{ .centigrade = 15,     .adc = 1227, },
	{ .centigrade = 20,     .adc = 1109, },
	{ .centigrade = 25,     .adc = 992, },
	{ .centigrade = 30,     .adc = 880, },
	{ .centigrade = 35,     .adc = 773, },
	{ .centigrade = 40,     .adc = 675, },
	{ .centigrade = 45,     .adc = 586, },
	{ .centigrade = 50,     .adc = 507, },
	{ .centigrade = 55,     .adc = 436, },
	{ .centigrade = 58,     .adc = 399, },
	{ .centigrade = 63,     .adc = 343, },
	{ .centigrade = 65,     .adc = 322, },
};

#ifndef USHRT_MAX
#define USHRT_MAX	0xFFFFU
#endif

static int adc_to_temperature_centigrade(unsigned short adc)
{
	int i;
	int approximation;
	/* low_*: Greatest Lower Bound,
	 *          *          *          * high_*: Smallest Upper Bound */
	int low_temp = 0, high_temp = 0;
	unsigned short low_adc = 0, high_adc = USHRT_MAX;
	for (i = 0; i < ARRAY_SIZE(adc_to_temperature_data); i++) {
		if (adc_to_temperature_data[i].adc <= adc &&
				adc_to_temperature_data[i].adc >= low_adc) {
			low_temp = adc_to_temperature_data[i].centigrade;
			low_adc = adc_to_temperature_data[i].adc;
		}
		if (adc_to_temperature_data[i].adc >= adc &&
				adc_to_temperature_data[i].adc <= high_adc) {
			high_temp = adc_to_temperature_data[i].centigrade;
			high_adc = adc_to_temperature_data[i].adc;
		}
	}

	/* Linear approximation between cloest low and high,
	 * which is the weighted average of the two. */

	/* The following equation is correct only when the two are different */
	if (low_adc == high_adc)
		return low_temp;
	if (ARRAY_SIZE(adc_to_temperature_data) < 2)
		return 20; /* The room temperature */
	if (low_adc == 0)
		return high_temp;
	if (high_adc == USHRT_MAX)
		return low_temp;

	approximation = low_temp * (adc - low_adc) +
		high_temp * (high_adc - adc);
	approximation /= high_adc - low_adc;

	return approximation;
}

static int adc_get_average_ambient_temperature(void)
{
	unsigned short min = USHRT_MAX;
	unsigned short max = 0;
	unsigned int sum = 0;
	unsigned int measured = 0;
	int i;

	for (i = 0; i < 7; i++) {
		/* XADCAIN6 */
		unsigned short measurement = get_adc_value(6);
		sum += measurement;
		measured++;
		if (min > measurement)
			min = measurement;
		if (max < measurement)
			max = measurement;
	}
	if (measured >= 3) {
		measured -= 2;
		sum -= min;
		sum -= max;
	}
	sum /= measured;
	printf("Average Ambient Temperature = %d(ADC=%d)\n",
			adc_to_temperature_centigrade(sum), sum);
	return adc_to_temperature_centigrade(sum);
}

enum temperature_level {
	_TEMP_OK,
	_TEMP_OK_HIGH,
	_TEMP_OK_LOW,
	_TEMP_TOO_HIGH,
	_TEMP_TOO_LOW,
};

static enum temperature_level temperature_check(void)
{
	int temp = adc_get_average_ambient_temperature();
	if (temp < -5)
		return _TEMP_TOO_LOW;
	if (temp < 0)
		return _TEMP_OK_LOW;
	if (temp > 63)
		return _TEMP_TOO_HIGH;
	if (temp > 58)
		return _TEMP_OK_HIGH;
	return _TEMP_OK;
}

/*
 * into_charge_mode()
 * Run a charge loop with animation and temperature check with sleep
**/
static void into_charge_mode(int charger_speed)
{
	int i, j, delay;
	enum temperature_level previous_state = _TEMP_OK;
	unsigned int wakeup_stat = 0;

	/* 1. Show Animation */
	for (i = 0; i < 5; i++) {
		for (j = 0; j < 5; j++) {
			printf(".");
			for (delay = 0; delay < 1000; delay++)
				udelay(1000);
		}
		printf("\n");
	}

	/* 2. Loop with temperature check and sleep */
	do {
		/* TODO: 2.A. Setup wakeup source and rtc tick */

		/* TODO: 2.B. Go to sleep */
		for (delay = 0; delay < 4000; delay++)
			udelay(1000);

		/* 2.C. Check the temperature */
		switch (temperature_check()) {
		case _TEMP_OK:
			charger_en(charger_speed);
			previous_state = _TEMP_OK;
			break;
		case _TEMP_TOO_LOW:
			charger_en(0);
			previous_state = _TEMP_TOO_LOW;
			break;
		case _TEMP_TOO_HIGH:
			charger_en(0);
			previous_state = _TEMP_TOO_HIGH;
			break;
		case _TEMP_OK_LOW:
			if (previous_state == _TEMP_TOO_LOW) {
				charger_en(0);
			} else {
				charger_en(charger_speed);
				previous_state = _TEMP_OK;
			}
			break;
		case _TEMP_OK_HIGH:
			if (previous_state == _TEMP_TOO_HIGH) {
				charger_en(0);
			} else {
				charger_en(charger_speed);
				previous_state = _TEMP_OK;
			}
			break;
		}
	} while (wakeup_stat == 0x04);
}

#ifdef CONFIG_LCD
void fimd_clk_set(void)
{
	struct s5pc210_clock *clk =
		(struct s5pc210_clock *)samsung_get_base_clock();

	/* workaround */
	unsigned long display_ctrl = 0x10010210;
	unsigned int cfg = 0;

	/* LCD0_BLK FIFO S/W reset */
	cfg = readl(display_ctrl);
	cfg |= (1 << 9);
	writel(cfg, display_ctrl);

	cfg = 0;

	/* FIMD of LBLK0 Bypass Selection */
	cfg = readl(display_ctrl);
	cfg &= ~(1 << 9);
	cfg |= (1 << 1);
	writel(cfg, display_ctrl);

	/* set lcd src clock */
	cfg = readl(&clk->src_lcd0);
	cfg &= ~(0xf);
	cfg |= 0x6;
	writel(cfg, &clk->src_lcd0);

	/* set fimd ratio */
	cfg = readl(&clk->div_lcd0);
	cfg &= ~(0xf);
	cfg |= 0x1;
	writel(cfg, &clk->div_lcd0);
}

struct nt39411_platform_data nt39411_pd;

static void lcd_cfg_gpio(void)
{
	unsigned int i;

	for (i = 0; i < 8; i++) {
		/* set GPF0,1,2[0:7] for RGB Interface and Data lines (32bit) */
		gpio_cfg_pin(&gpio1->f0, i, GPIO_FUNC(2));
		gpio_cfg_pin(&gpio1->f1, i, GPIO_FUNC(2));
		gpio_cfg_pin(&gpio1->f2, i, GPIO_FUNC(2));
		/* pull-up/down disable */
		gpio_set_pull(&gpio1->f0, i, GPIO_PULL_NONE);
		gpio_set_pull(&gpio1->f1, i, GPIO_PULL_NONE);
		gpio_set_pull(&gpio1->f2, i, GPIO_PULL_NONE);

		/* drive strength to max (24bit) */
		gpio_set_drv(&gpio1->f0, i, GPIO_DRV_4X);
		gpio_set_rate(&gpio1->f0, i, GPIO_DRV_SLOW);
		gpio_set_drv(&gpio1->f1, i, GPIO_DRV_4X);
		gpio_set_rate(&gpio1->f1, i, GPIO_DRV_SLOW);
		gpio_set_drv(&gpio1->f2, i, GPIO_DRV_4X);
		gpio_set_rate(&gpio1->f2, i, GPIO_DRV_SLOW);
	}

	for (i = 0; i < 4; i++) {
		/* set GPF3[0:3] for RGB Interface and Data lines (32bit) */
		gpio_cfg_pin(&gpio1->f3, i, GPIO_PULL_UP);
		/* pull-up/down disable */
		gpio_set_pull(&gpio1->f3, i, GPIO_PULL_NONE);
		/* drive strength to max (24bit) */
		gpio_set_drv(&gpio1->f3, i, GPIO_DRV_4X);
		gpio_set_rate(&gpio1->f3, i, GPIO_DRV_SLOW);
	}

	gpio_cfg_pin(&gpio1->f2, i, GPIO_FUNC(2));
	/* pull-up/down disable */
	gpio_set_pull(&gpio1->f0, i, GPIO_PULL_NONE);

	/* LED_BACKLIGHT_PWM */
	nt39411_pd.swi.swi_bank = &gpio1->d0;
	nt39411_pd.swi.controller_data = 0;
	gpio_cfg_pin(&gpio1->d0, nt39411_pd.swi.controller_data, GPIO_FUNC(2));

	/* LVDS_nSHDN */
	gpio_direction_output(&gpio1->e1, 5, 1);

	/* LCD_LDO_EN */
	gpio_direction_output(&gpio1->e2, 3, 1);

	nt39411_pd.swi.low_period = 30;
	nt39411_pd.swi.high_period = 30;
	return;
}

int s5p_no_lcd_support(void)
{
	return 0;
}

void init_panel_info(vidinfo_t *vid)
{
	vid->vl_freq	= 60;
	vid->vl_col	= 1024;
	vid->vl_row	= 600;
	vid->vl_width	= 1024;
	vid->vl_height	= 600;
	vid->vl_clkp	= CONFIG_SYS_LOW;
	vid->vl_hsp	= CONFIG_SYS_HIGH;
	vid->vl_vsp	= CONFIG_SYS_HIGH;
	vid->vl_dp	= CONFIG_SYS_LOW;
	vid->vl_bpix	= 32;
	vid->enable_ldo = nt39411_send_intensity;

	/* NT39411 LCD Panel */
	vid->vl_hspw	= 50;
	vid->vl_hbpd	= 142;
	vid->vl_hfpd	= 210;

	vid->vl_vspw	= 10;
	vid->vl_vbpd	= 11;
	vid->vl_vfpd	= 10;

	vid->cfg_gpio = lcd_cfg_gpio;
	vid->backlight_on = NULL;
	/* vid->lcd_power_on = lcd_power_on; */	/* Don't need the poweron squence */
	/* vid->reset_lcd = reset_lcd; */	/* Don't need the reset squence */

	vid->cfg_ldo = NULL;
	vid->enable_ldo = nt39411_send_intensity;

	vid->init_delay = 0;
	vid->power_on_delay = 0;
	vid->reset_delay = 0;
	vid->interface_mode = FIMD_RGB_INTERFACE;
	nt39411_pd.brightness = NT39411_DEFAULT_INTENSITY;
	nt39411_pd.a_onoff = A1_ON;
	nt39411_pd.b_onoff = B1_ON;
	nt39411_set_platform_data(&nt39411_pd);
	setenv("lcdinfo", "lcd=nt39411");
}
#endif

static unsigned int get_hw_revision(void)
{
	int hwrev = 0;
	int i;

	/* HW_REV[0:3]: GPE1[0:3] */
	for (i = 0; i < 4; i++) {
		gpio_direction_input(&gpio1->e1, i);
		gpio_set_pull(&gpio1->e1, i, GPIO_PULL_NONE);
	}

	/* Workaround: don't support hwrev 0 */
	while (!hwrev) {
		for (i = 0; i < 4; i++)
			hwrev |= (gpio_get_value(&gpio1->e1, i) << i);
	}

	return hwrev;
}

static const char * const pcb_rev[] = {
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"SLP_MAIN_7INCH",
};

static void check_hw_revision(void)
{
	int hwrev;

	hwrev = get_hw_revision();

	board_rev |= hwrev;
}

static void show_hw_revision(void)
{
	printf("HW Revision:\t0x%x\n", board_rev);
	printf("PCB Revision:\t%s\n", pcb_rev[board_rev & 0xf]);
}

void get_rev_info(char *rev_info)
{
	sprintf(rev_info, "HW Revision: 0x%x (%s)\n",
			board_rev, pcb_rev[board_rev & 0xf]);
}

static void check_reset_status(void)
{
	int status = get_reset_status();

	puts("Reset Status: ");

	switch (status) {
	case EXTRESET:
		puts("Pin(Ext) Reset\n");
		break;
	case WARMRESET:
		puts("Warm Reset\n");
		break;
	case WDTRESET:
		puts("Watchdog Reset\n");
		break;
	case SWRESET:
		puts("S/W Reset\n");
		break;
	default:
		printf("Unknown (0x%x)\n", status);
	}
}

#ifdef CONFIG_CMD_RAMOOPS
static void show_dump_msg(void)
{
	int ret;

	ret = ramoops_init(samsung_get_base_modem());

	if (!ret)
		setenv("bootdelay", "-1");
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	check_reset_status();
#ifdef CONFIG_CMD_RAMOOPS
	show_dump_msg();
#endif

	show_hw_revision();

#ifdef CONFIG_CMD_PMIC
	pmic_bus_init(I2C_5);
#endif

	check_keypad();

	check_auto_burn();

	/* check max17040 */
	check_battery(0);

#ifdef CONFIG_INFO_ACTION
	info_action_check();
#endif

	return 0;
}
#endif

#ifdef CONFIG_CMD_USBDOWN
int usb_board_init(void)
{
	/* interrupt clear */
	pmic_get_irq(PWRON1S);

#ifdef CONFIG_CMD_PMIC
	run_command("pmic ldo 8 on", 0);
	run_command("pmic ldo 3 on", 0);
	run_command("pmic safeout 1 on", 0);
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
	int i, err;

	/*
	 * MASSMEMORY_EN: XGNSS_SDA: GPL1[1] - Output Low
	 * MASSMEMORY_EN is connected to invertor.
	 * If want to enable eMMC, must set to the low.
	 */
	gpio_direction_output(&gpio2->l1, 1, 0);
	gpio_set_pull(&gpio2->l1, 1, GPIO_PULL_NONE);

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
		gpio_cfg_pin(&gpio2->k0, i, 0x2);
		/* GPK0[0:6] pull disable */
		gpio_set_pull(&gpio2->k0, i, GPIO_PULL_NONE);
		/* GPK0[0:6] drv 4x */
		gpio_set_drv(&gpio2->k0, i, GPIO_DRV_4X);
	}

	for (i = 3; i < 7; i++) {
		/* GPK1[3:6] special function 3 */
		gpio_cfg_pin(&gpio2->k1, i, 0x3);
		/* GPK1[3:6] pull disable */
		gpio_set_pull(&gpio2->k1, i, GPIO_PULL_NONE);
		/* GPK1[3:6] drv 4x */
		gpio_set_drv(&gpio2->k1, i, GPIO_DRV_4X);
	}

	/* T-flash detect */
	gpio_cfg_pin(&gpio2->x3, 3, 0xf);
	gpio_set_pull(&gpio2->x3, 3, GPIO_PULL_UP);

	/*
	 * MMC device init
	 * mmc0	 : eMMC (8-bit buswidth)
	 * mmc2	 : SD card (4-bit buswidth)
	 */
	err = s5p_mmc_init(0, 8);

	/*
	 * Check the T-flash  detect pin
	 * GPX3[3] T-flash detect pin
	 */
	if (!gpio_get_value(&gpio2->x3, 3)) {
		/*
		 * SD card GPIO:
		 * GPK2[0]	SD_2_CLK(2)
		 * GPK2[1]	SD_2_CMD(2)
		 * GPK2[2]	SD_2_CDn	-> Not used
		 * GPK2[3:6]	SD_2_DATA[0:3](2)
		 */
		for (i = 0; i < 7; i++) {
			if (i == 2)
				continue;
			/* GPK2[0:6] special function 2 */
			gpio_cfg_pin(&gpio2->k2, i, 0x2);
			/* GPK2[0:6] pull disable */
			gpio_set_pull(&gpio2->k2, i, GPIO_PULL_NONE);
			/* GPK2[0:6] drv 4x */
			gpio_set_drv(&gpio2->k2, i, GPIO_DRV_4X);
		}
		err = s5p_mmc_init(2, 4);
	}

	return err;

}
#endif
