/*
 * (C) Copyright 2009
 * Samsung Electronics, <www.samsung.com/sec>
 * MyungJoo Ham <myungjoo.ham@samsung.com>
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

#ifndef __ASM_ARM_ARCH_RTC_H_
#define __ASM_ARM_ARCH_RTC_H_

/* DRAM Memory Controller */
#define S5PC110_INTP		0xE2800030
#define S5PC110_RTCCON		0xE2800040
#define S5PC110_TICCNT		0xE2800044
#define S5PC110_RTCALM		0xE2800050
#define S5PC110_ALMSEC		0xE2800054
#define S5PC110_ALMMIN		0xE2800058
#define S5PC110_ALMHOUR		0xE280005C
#define S5PC110_ALMDAY		0xE2800060
#define S5PC110_ALMMON		0xE2800064
#define S5PC110_ALMYEAR		0xE2800068
#define S5PC110_BCDSEC		0xE2800070
#define S5PC110_BCDMIN		0xE2800074
#define S5PC110_BCDHOUR		0xE2800078
#define S5PC110_BCDDAYWEEK	0xE280007C
#define S5PC110_BCDDAY		0xE2800080
#define S5PC110_BCDMON		0xE2800084
#define S5PC110_BCDYEAR		0xE2800088
#define S5PC110_CURTICCNT	0xE2800090

#endif
