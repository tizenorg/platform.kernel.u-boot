/*
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
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
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <i2c-gpio.h>

static unsigned int bus_index;

/*
 * i2c gpio3
 * SDA: GPJ3[6]
 * SCL: GPJ3[7]
 */
static struct i2c_gpio_bus_data i2c_gpio3 = {
	.gpio_base	= S5PC110_GPIO_BASE(S5PC110_GPIO_J3_OFFSET),
	.sda_pin	= 6,
	.scl_pin	= 7,
};

/*
 * i2c pmic
 * SDA: GPJ4[0]
 * SCL: GPJ4[3]
 */
static struct i2c_gpio_bus_data i2c_pmic = {
	.gpio_base	= S5PC110_GPIO_BASE(S5PC110_GPIO_J4_OFFSET),
	.sda_pin	= 0,
	.scl_pin	= 3,
};

/*
 * i2c gpio5
 * SDA: MP05[3]
 * SCL: MP05[2]
 */
static struct i2c_gpio_bus_data i2c_gpio5 = {
	.gpio_base	= S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_5_OFFSET),
	.sda_pin	= 3,
	.scl_pin	= 2,
};

static struct i2c_gpio_bus i2c_gpio[] = {
	{
		.bus	= &i2c_gpio3,
	}, {
		.bus	= &i2c_pmic,
	}, {
		.bus	= &i2c_gpio5,
	},
};

inline void i2c_gpio_set_bus(int bus)
{
	bus_index = bus;
}

void i2c_gpio_init(void)
{
	int i;
	unsigned int pin, reg;

	/* set to output */
	for (i = 0; i < ARRAY_SIZE(i2c_gpio); i++) {
		pin = i2c_gpio[i].bus->gpio_base;

		/* SDA */
		reg = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
		reg &= ~(0xf << (i2c_gpio[i].bus->sda_pin * 4));
		reg |= (1 << (i2c_gpio[i].bus->sda_pin * 4));
		writel(reg, pin + S5PC1XX_GPIO_CON_OFFSET);

		/* SCL */
		reg = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
		reg &= ~(0xf << (i2c_gpio[i].bus->scl_pin * 4));
		reg |= (1 << (i2c_gpio[i].bus->scl_pin * 4));
		writel(reg, pin + S5PC1XX_GPIO_CON_OFFSET);
	}
}

void i2c_init_board(void)
{
	if (cpu_is_s5pc100())
		return;

	/* init all i2c gpio buses */
	i2c_gpio_init();

	/* set default bus to gpio_pmic */
	i2c_gpio_set_bus(1);
}

void i2c_gpio_set(int line, int value)
{
	unsigned int pin, reg;

	pin = i2c_gpio[bus_index].bus->gpio_base;

	if (line)
		line = i2c_gpio[bus_index].bus->sda_pin;
	else
		line = i2c_gpio[bus_index].bus->scl_pin;

	reg = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
	if (value)
		reg |= (1 << line);
	else
		reg &= ~(1 << line);
	writel(reg, pin + S5PC1XX_GPIO_DAT_OFFSET);
}

int i2c_gpio_get(void)
{
	unsigned int pin, reg;

	pin = i2c_gpio[bus_index].bus->gpio_base;
	reg = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
	return reg & (1 << i2c_gpio[bus_index].bus->sda_pin);
}

void i2c_gpio_dir(int dir)
{
	unsigned int pin, reg;

	pin = i2c_gpio[bus_index].bus->gpio_base;

	reg = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	reg &= ~(0xf << (i2c_gpio[bus_index].bus->sda_pin * 4));
	if (dir)
		reg |= (1 << (i2c_gpio[bus_index].bus->sda_pin * 4));
	writel(reg, pin + S5PC1XX_GPIO_CON_OFFSET);
}
