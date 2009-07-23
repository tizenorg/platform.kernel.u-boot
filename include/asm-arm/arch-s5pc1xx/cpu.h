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

#define S5P_ADDR_BASE		0xe0000000
#define S5P_ADDR(x)		(S5P_ADDR_BASE + (x))

#define S5P_PA_ID		S5P_ADDR(0x00000000)	/* Chip ID Base */
#define S5P_PA_CLK		S5P_ADDR(0x00100000)	/* Clock Base */
#define S5P_PA_PWR		S5P_ADDR(0x00108000)	/* Power Base */
#define S5P_PA_CLK_OTHERS	S5P_ADDR(0x00200000)	/* Clock Others Base */
#define S5P_PA_VIC0		S5P_ADDR(0x04000000)    /* Vector Interrupt Controller 0 */
#define S5P_PA_VIC1		S5P_ADDR(0x04100000)    /* Vector Interrupt Controller 1 */
#define S5P_PA_VIC2		S5P_ADDR(0x04200000)    /* Vector Interrupt Controller 2 */
#define S5P_PA_DMC		S5P_ADDR(0x06000000)    /* Dram Memory Controller */
#define S5P_PA_SROMC		S5P_ADDR(0x07000000)    /* SROM Controller */
#define S5P_PA_WATCHDOG		S5P_ADDR(0x0a200000)    /* Watchdog Timer */
#define S5P_PA_PWMTIMER		S5P_ADDR(0x0a000000)    /* PWM Timer */

/*
 * Chip ID
 */
#define S5P_ID(x)		(S5P_PA_ID + (x))
#define S5PC1XX_PRO_ID		S5P_ID(0)
#define S5PC1XX_OMR		S5P_ID(4)

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
