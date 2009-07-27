/*
 * (C) Copyright 2009
 * Samsung Electronics, <www.samsung.com/sec>
 * Heungjun Kim <riverful.kim@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
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
 *
 */

#ifndef _S5PC1XX_CPU_H
#define _S5PC1XX_CPU_H

#include <asm/hardware.h>

#define S5PC1XX_ADDR_BASE	0xe0000000
#define S5P_ADDR(x)		(S5PC1XX_ADDR_BASE + (x))

#define S5PC1XX_CLOCK_BASE	0xE0100000
#define S5P_PA_PWR		S5P_ADDR(0x00108000)	/* Power Base */
#define S5P_PA_CLK_OTHERS	S5P_ADDR(0x00200000)	/* Clock Others Base */

/* Note that write the macro by address order */
#define S5PC100_VIC0_BASE	0xE4000000
#define S5PC100_VIC1_BASE	0xE4100000
#define S5PC100_VIC2_BASE	0xE4200000
#define S5PC100_SROMC_BASE	0xE7000000
#define S5PC100_WATCHDOG_BASE	0xEA200000

#define S5PC110_WATCHDOG_BASE	0xE2700000
#define S5PC110_SROMC_BASE	0xE8000000
#define S5PC110_VIC0_BASE	0xF2000000
#define S5PC110_VIC1_BASE	0xF2100000
#define S5PC110_VIC2_BASE	0xF2200000
#define S5PC110_VIC3_BASE	0xF2300000

/*
 * Chip ID
 */
#define S5PC1XX_CHIP_ID(x)	(0xE0000000 + (x))
#define S5PC1XX_PRO_ID		S5PC1XX_CHIP_ID(0)
#define S5PC1XX_OMR		S5PC1XX_CHIP_ID(4)

#ifndef __ASSEMBLY__
/* CPU detection macros */
extern unsigned int s5pc1xx_cpu_id;

#define IS_SAMSUNG_TYPE(type, id)					\
static inline int is_##type(void)					\
{									\
	return (s5pc1xx_cpu_id == (id)) ? 1 : 0;			\
}

IS_SAMSUNG_TYPE(s5pc100, 0xc100)
IS_SAMSUNG_TYPE(s5pc110, 0xc110)

#define cpu_is_s5pc100()	is_s5pc100()
#define cpu_is_s5pc110()	is_s5pc110()
#endif

#endif	/* _S5PC1XX_CPU_H */
