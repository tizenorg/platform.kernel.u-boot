/*
 * (C) Copyright 2016 Samsung Electronics
 * Lukasz Majewski
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _EXYNOS64_CPU_H
#define _EXYNOS64_CPU_H

#define DEVICE_NOT_AVAILABLE		0

#define EXYNOS_CPU_NAME			"Exynos5433"

/* EXYNOS5433 */
#define EXYNOS5433_CLOCK_BASE		0x10040000
#define EXYNOS5433_PWMTIMER_BASE	0x14DD0000
#define EXYNOS5433_USB3PHY_BASE         0x15500000
#define EXYNOS5433_POWER_BASE		0x105C0000

#ifndef __ASSEMBLY__
#include <asm/io.h>
/* CPU detection macros */
extern unsigned int s5p_cpu_id;
extern unsigned int s5p_cpu_rev;

static inline int s5p_get_cpu_rev(void)
{
	return s5p_cpu_rev;
}

static inline void s5p_set_cpu_id(void)
{
	return;
}

static inline char *s5p_get_cpu_name(void)
{
	return EXYNOS_CPU_NAME;
}

#define SAMSUNG_BASE(device, base)				\
static inline unsigned long __attribute__((no_instrument_function)) \
	samsung_get_base_##device(void) \
{								\
	return EXYNOS5433_##base;		\
}

SAMSUNG_BASE(clock, CLOCK_BASE)
SAMSUNG_BASE(timer, PWMTIMER_BASE)
SAMSUNG_BASE(usb3_phy, USB3PHY_BASE)
SAMSUNG_BASE(power, POWER_BASE)
#endif

#endif	/* _EXYNOS64_CPU_H */
