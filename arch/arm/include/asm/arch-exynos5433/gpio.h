/*
 * (C) Copyright 2016 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_GPIO_H
#define __ASM_ARCH_GPIO_H

#ifndef __ASSEMBLY__

#include <asm/io.h>

/* hsi2c_8 */
/* Pins gpb0-1 gpb0-0 */

#define GPIO_GPB0_BASE 0x14CC0020
#define GPIO_I2C_SDA_PIN 0
#define GPIO_I2C_SCL_PIN 1

#define GPB0_CON 0x00
#define GPB0_DAT 0x04
#define GPB0_PUD 0x08
#define GPB0_DRV 0x0C
#define GPB0_CONPDN 0x10
#define GPB0_PUDPDN 0x14

#define CON_MASK(val)			(0xf << ((val) << 2))
#define CON_SFR(gpio, cfg)		((cfg) << ((gpio) << 2))

#define DAT_MASK(gpio)			(0x1 << (gpio))
#define DAT_SET(gpio)			(0x1 << (gpio))

#define PULL_MASK(gpio)		(0x3 << ((gpio) << 1))
#define PULL_MODE(gpio, pull)		((pull) << ((gpio) << 1))

#define DRV_MASK(gpio)			(0x3 << ((gpio) << 1))
#define DRV_SET(gpio, mode)		((mode) << ((gpio) << 1))

/* Pin configurations */
#define S5P_GPIO_INPUT 0x0
#define S5P_GPIO_OUTPUT        0x1
#define S5P_GPIO_IRQ   0xf
#define S5P_GPIO_FUNC(x)       (x)

/* Pull mode */
#define S5P_GPIO_PULL_NONE     0x0
#define S5P_GPIO_PULL_DOWN     0x1
#define S5P_GPIO_PULL_UP       0x3

/* Drive Strength level */
#define S5P_GPIO_DRV_1X        0x0
#define S5P_GPIO_DRV_3X        0x1
#define S5P_GPIO_DRV_2X        0x2
#define S5P_GPIO_DRV_4X        0x3
#define S5P_GPIO_DRV_FAST      0x0
#define S5P_GPIO_DRV_SLOW      0x1


static inline int gpio_request(unsigned gpio, const char *label)
{
	debug("%s: GPIO: %d -> %s\n", __func__, gpio, label);
	return 0;
}

static inline int gpio_free(unsigned gpio)
{
	debug("%s: GPIO: %d\n", __func__, gpio);
	return 0;
}

static inline void gpio_set_pin_value(void *addr, unsigned gpio, int en)
{
	unsigned int value;

	value = readl(addr);
	value &= ~DAT_MASK(gpio);
	if (en)
		value |= DAT_SET(gpio);
	writel(value, addr);
}

static inline void gpio_cfg_pin(void *addr, unsigned gpio, int cfg)
{
	unsigned int value;

	value = readl(addr);
	value &= ~CON_MASK(gpio);
	value |= CON_SFR(gpio, cfg);
	writel(value, addr);

}

static inline int gpio_direction_input(unsigned gpio)
{
	void *base = (void*) GPIO_GPB0_BASE;

	debug("%s: GPIO: %d\n", __func__, gpio);

	gpio_cfg_pin(base + GPB0_CON, gpio, S5P_GPIO_INPUT);

	return 0;
}

static inline int gpio_direction_output(unsigned gpio, int value)
{
	void *base = (void*) GPIO_GPB0_BASE;

	debug("%s: GPIO: %d VAL: %d\n", __func__, gpio, value);

	gpio_set_pin_value(base + GPB0_DAT, gpio, value);
	gpio_cfg_pin(base + GPB0_CON, gpio, S5P_GPIO_OUTPUT);

	return 0;
}

static inline int gpio_get_value(unsigned gpio)
{
	void *base = (void*) GPIO_GPB0_BASE;
	unsigned int value;

	debug("%s: GPIO: %d\n", __func__, gpio);

	value = readl(base + GPB0_DAT);
	return !!(value & DAT_MASK(gpio));
}

static inline int gpio_set_value(unsigned gpio, int value)
{
	void *base = (void*) GPIO_GPB0_BASE;

	debug("%s: GPIO: %d VAL: %d\n", __func__, gpio, value);

	gpio_set_pin_value(base + GPB0_DAT, gpio, value);

	return 0;
}

static inline void gpio_set_pull(int gpio, int mode)
{
	void *base = (void*) GPIO_GPB0_BASE;
	unsigned int value;

	value = readl(base + GPB0_PUD);
	value &= ~PULL_MASK(gpio);

	switch (mode) {
	case S5P_GPIO_PULL_DOWN:
	case S5P_GPIO_PULL_UP:
		value |= PULL_MODE(gpio, mode);
		break;
	default:
		break;
	}

	writel(value, base + GPB0_PUD);
}

static inline void gpio_set_drv(int gpio, int mode)
{
	void *base = (void*) GPIO_GPB0_BASE;
	unsigned int value;

	value = readl(base + GPB0_DRV);
	value &= ~DRV_MASK(gpio);

	switch (mode) {
	case S5P_GPIO_DRV_1X:
	case S5P_GPIO_DRV_2X:
	case S5P_GPIO_DRV_3X:
	case S5P_GPIO_DRV_4X:
		value |= DRV_SET(gpio, mode);
		break;
	default:
		return;
	}

	writel(value, base + GPB0_DRV);
}

#endif
#endif
