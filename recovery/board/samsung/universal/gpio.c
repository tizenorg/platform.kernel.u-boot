/*
 * Copyright (C) 2009-2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>

#define CON_MASK(x)		(0xf << ((x) << 2))
#define CON_SFR(x, v)		((v) << ((x) << 2))

#define DAT_MASK(x)		(0x1 << (x))
#define DAT_SET(x)		(0x1 << (x))

#define PULL_MASK(x)		(0x3 << ((x) << 1))
#define PULL_MODE(x, v)		((v) << ((x) << 1))

#define DRV_MASK(x)		(0x3 << ((x) << 1))
#define DRV_SET(x, m)		((m) << ((x) << 1))
#define RATE_MASK(x)		(0x1 << (x + 16))
#define RATE_SET(x)		(0x1 << (x + 16))

void gpio_cfg_pin(struct s5pc1xx_gpio_bank *bank, int gpio, int cfg)
{
	unsigned int value;

	value = readl(&bank->con);
	value &= ~CON_MASK(gpio);
	value |= CON_SFR(gpio, cfg);
	writel(value, &bank->con);
#if 0
	if (s5pc1xx_get_cpu_rev() == 0)
#endif
		value = readl(&bank->con);
}

void gpio_direction_output(struct s5pc1xx_gpio_bank *bank, int gpio, int en)
{
	unsigned int value;

	gpio_cfg_pin(bank, gpio, GPIO_OUTPUT);

	value = readl(&bank->dat);
	value &= ~DAT_MASK(gpio);
	if (en)
		value |= DAT_SET(gpio);
	writel(value, &bank->dat);
#if 0
	if (s5pc1xx_get_cpu_rev() == 0)
#endif
		value = readl(&bank->dat);
}

void gpio_direction_input(struct s5pc1xx_gpio_bank *bank, int gpio)
{
	gpio_cfg_pin(bank, gpio, GPIO_INPUT);
}

void gpio_set_value(struct s5pc1xx_gpio_bank *bank, int gpio, int en)
{
	unsigned int value;

	value = readl(&bank->dat);
	value &= ~DAT_MASK(gpio);
	if (en)
		value |= DAT_SET(gpio);
	writel(value, &bank->dat);
#if 0
	if (s5pc1xx_get_cpu_rev() == 0)
#endif
		value = readl(&bank->dat);
}

unsigned int gpio_get_value(struct s5pc1xx_gpio_bank *bank, int gpio)
{
	unsigned int value;

	value = readl(&bank->dat);
	return !!(value & DAT_MASK(gpio));
}

void gpio_set_pull(struct s5pc1xx_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->pull);
	value &= ~PULL_MASK(gpio);

	switch (mode) {
	case GPIO_PULL_DOWN:
	case GPIO_PULL_UP:
		value |= PULL_MODE(gpio, mode);
		break;
	default:
		break;
	}

	writel(value, &bank->pull);
#if 0
	if (s5pc1xx_get_cpu_rev() == 0)
#endif
		value = readl(&bank->pull);
}

void gpio_set_drv(struct s5pc1xx_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->drv);
	value &= ~DRV_MASK(gpio);

	switch (mode) {
	case GPIO_DRV_1X:
	case GPIO_DRV_2X:
	case GPIO_DRV_3X:
	case GPIO_DRV_4X:
		value |= DRV_SET(gpio, mode);
		break;
	default:
		return;
	}

	writel(value, &bank->drv);
#if 0
	if (s5pc1xx_get_cpu_rev() == 0)
#endif
		value = readl(&bank->drv);
}

void gpio_set_rate(struct s5pc1xx_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->drv);
	value &= ~RATE_MASK(gpio);

	switch (mode) {
	case GPIO_DRV_FAST:
	case GPIO_DRV_SLOW:
		value |= RATE_SET(gpio);
		break;
	default:
		return;
	}

	writel(value, &bank->drv);
#if 0
	if (s5pc1xx_get_cpu_rev() == 0)
#endif
		value = readl(&bank->drv);
}

