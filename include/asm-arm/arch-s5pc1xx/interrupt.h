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

#ifndef __ASM_ARM_ARCH_INTERRUPT_H_
#define __ASM_ARM_ARCH_INTERRUPT_H_

/* Vector Interrupt Offset */
#define VIC_IRQSTATUS_OFFSET		0x0
#define VIC_FIQSTATUS_OFFSET		0x4
#define VIC_RAWINTR_OFFSET		0x8
#define VIC_INTSELECT_OFFSET		0xc
#define VIC_INTENABLE_OFFSET		0x10
#define VIC_INTENCLEAR_OFFSET		0x14
#define VIC_SOFTINT_OFFSET		0x18
#define VIC_SOFTINTCLEAR_OFFSET		0x1c
#define VIC_PROTECTION_OFFSET		0x20
#define VIC_SWPRIORITYMASK_OFFSET	0x24
#define VIC_PRIORITYDAISY_OFFSET	0x28
#define VIC_INTADDRESS_OFFSET		0xf00

#endif
