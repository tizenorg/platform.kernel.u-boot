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

#ifndef __ASM_ARM_ARCH_MEM_H_
#define __ASM_ARM_ARCH_MEM_H_

/*
 * SROMC Controller
 */
/* DRAM Memory Controller */
#define S5P_DMC_BASE(x)		(S5P_PA_DMC + (x))
/* SROMC Base */
#define S5P_SROMC_BASE(x)	(S5P_PA_SROMC + (x))

/* SROMC offset */
#define CONCONTROL_OFFSET	0x0
#define MEMCONTROL_OFFSET	0x04
#define MEMCONFIG0_OFFSET	0x08
#define MEMCONFIG1_OFFSET	0x0c
#define DIRECTCMD_OFFSET	0x10
#define PRECHCONFIG_OFFSET	0x14
#define PHYCONTROL0_OFFSET	0x18
#define PHYCONTROL1_OFFSET	0x1c
#define PHYCONTROL2_OFFSET	0x20
#define PWRDNCONFIG_OFFSET	0x28
#define TIMINGAREF_OFFSET	0x30
#define TIMINGROW_OFFSET	0x34
#define TIMINGDATA_OFFSET	0x38
#define TIMINGPOWER_OFFSET	0x3c
#define PHYSTATUS0_OFFSET	0x4
#define PHYSTATUS1_OFFSET	0x44
#define CHIP0STATUS_OFFSET	0x48
#define CHIP1STATUS_OFFSET	0x4c
#define AREFSTATUS_OFFSET	0x50
#define MRSTATUS_OFFSET		0x54
#define PHYTEST0_OFFSET		0x58
#define PHYTEST1_OFFSET		0x5c

#define S5P_CONCONTROL		S5P_DMC_BASE(CONCONTROL_OFFSET)
#define S5P_MEMCONTROL		S5P_DMC_BASE(MEMCONTROL_OFFSET)
#define S5P_MEMCONFIG0		S5P_DMC_BASE(MEMCONFIG0_OFFSET)
#define S5P_MEMCONFIG1		S5P_DMC_BASE(MEMCONFIG1_OFFSET)
#define S5P_DIRECTCMD		S5P_DMC_BASE(DIRECTCMD_OFFSET)
#define S5P_PRECHCONFIG		S5P_DMC_BASE(PRECHCONFIG_OFFSET)
#define S5P_PHYCONTROL0		S5P_DMC_BASE(PHYCONTROL0_OFFSET)
#define S5P_PHYCONTROL1		S5P_DMC_BASE(PHYCONTROL1_OFFSET)
#define S5P_PHYCONTROL2		S5P_DMC_BASE(PHYCONTROL2_OFFSET)
#define S5P_PWRDNCONFIG		S5P_DMC_BASE(PWRDNCONFIG_OFFSET)
#define S5P_TIMINGAREF		S5P_DMC_BASE(TIMINGAREF_OFFSET)
#define S5P_TIMINGROW		S5P_DMC_BASE(TIMINGROW_OFFSET)
#define S5P_TIMINGDATA		S5P_DMC_BASE(TIMINGDATA_OFFSET)
#define S5P_TIMINGPOWER		S5P_DMC_BASE(TIMINGPOWER_OFFSET)
#define S5P_PHYSTATUS0		S5P_DMC_BASE(PHYSTATUS0_OFFSET)
#define S5P_PHYSTATUS1		S5P_DMC_BASE(PHYSTATUS1_OFFSET)
#define S5P_CHIP0STATUS		S5P_DMC_BASE(CHIP0STATUS_OFFSET)
#define S5P_CHIP1STATUS		S5P_DMC_BASE(CHIP1STATUS_OFFSET)
#define S5P_AREFSTATUS		S5P_DMC_BASE(AREFSTATUS_OFFSET)
#define S5P_MRSTATUS		S5P_DMC_BASE(MRSTATUS_OFFSET)
#define S5P_PHYTEST0		S5P_DMC_BASE(PHYTEST0_OFFSET)
#define S5P_PHYTEST1		S5P_DMC_BASE(PHYTEST1_OFFSET)

#endif
