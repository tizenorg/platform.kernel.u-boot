/*
 * Copyright (C) 2009 Samsung Electrnoics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Kyungmin Park <kyungmin.park@samsung.com>
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

#include <config.h>
#include <asm/arch/cpu.h>

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:

	ldr	r0, =S5PC210_CLOCK_BASE		@ 0x10030000
	/* CLK_DIV_DMC0 on iROM DMC=50MHz for init DMC */
	ldr	r1, =0x13113113
	ldr	r2, =0x10500			@ CLK_DIV_DMC0_OFFSET
	str	r1, [r0, r2]

	ldr	r0, =S5PC210_MIU_BASE		@ 0x10600000
	/* MIU_1BIT_INTERLEAVED */
	ldr	r1, =0x0000000C
	str	r1, [r0, #0x400]
	ldr	r1, =0x40000000
	str	r1, [r0, #0x808]
	ldr	r1, =0x5FFFFFFF
	str	r1, [r0, #0x810]
	ldr	r1, =0x00000001
	str	r1, [r0, #0x800]

	ldr	r0, =S5PC210_DMC0_BASE		@ 0x10400000
	ldr	r6, =S5PC210_DMC1_BASE		@ 0x10410000
	ldr	r1, =0xE3855503
	str	r1, [r0, #0x44]
	str	r1, [r6, #0x44]
	ldr	r1, =0x71101008
	str	r1, [r0, #0x18]			@ DMC_PHYCONTROL0
	str	r1, [r6, #0x18]			@ DMC_PHYCONTROL0
	ldr	r1, =0x7110100A
	str	r1, [r0, #0x18]			@ DMC_PHYCONTROL0
	str	r1, [r6, #0x18]			@ DMC_PHYCONTROL0
	ldr	r1, =0x00000084
	str	r1, [r0, #0x1C]			@ DMC_PHYCONTROL1
	str	r1, [r6, #0x1C]			@ DMC_PHYCONTROL1
	ldr	r1, =0x71101008
	str	r1, [r0, #0x18]			@ DMC_PHYCONTROL0
	str	r1, [r6, #0x18]			@ DMC_PHYCONTROL0

	ldr	r1, =0x0000008C
	str	r1, [r0, #0x1C]			@ DMC_PHYCONTROL1
	str	r1, [r6, #0x1C]			@ DMC_PHYCONTROL1
	ldr	r1, =0x00000084
	str	r1, [r0, #0x1C]			@ DMC_PHYCONTROL1
	str	r1, [r6, #0x1C]			@ DMC_PHYCONTROL1
	ldr	r1, =0x0000008C
	str	r1, [r0, #0x1C]			@ DMC_PHYCONTROL1
	str	r1, [r6, #0x1C]			@ DMC_PHYCONTROL1
	ldr	r1, =0x00000084
	str	r1, [r0, #0x1C]			@ DMC_PHYCONTROL1
	str	r1, [r6, #0x1C]			@ DMC_PHYCONTROL1

	ldr	r1, =0x00000000
	str	r1, [r0, #0x20]			@ DMC_PHYCONTROL2
	str	r1, [r6, #0x20]			@ DMC_PHYCONTROL2

	/* ConControl */
	ldr	r1, =0x0FFF30DA
	str	r1, [r0, #0x00]			@ DMC_CONCONTROL
	str	r1, [r6, #0x00]			@ DMC_CONCONTROL
	/* MemControl */
	ldr	r1, =0x00202500
	str	r1, [r0, #0x04]			@ DMC_MEMCONTROL
	str	r1, [r6, #0x04]			@ DMC_MEMCONTROL
	ldr	r1, =0x20f00223
	str	r1, [r0, #0x08]			@ DMC_MEMCONFIG0
	ldr	r1, =0x20f00223
	str	r1, [r6, #0x08]			@ DMC_MEMCONFIG0
	ldr	r1, =0xff000000
	str	r1, [r0, #0x14]			@ DMC_PRECHCONFIG
	str	r1, [r6, #0x14]			@ DMC_PRECHCONFIG
	ldr	r1, =0x0000005D
	str	r1, [r0, #0x30]			@ DMC_TIMINGAREF
	str	r1, [r6, #0x30]			@ DMC_TIMINGAREF

	/* CLK 330 */
	ldr	r1, =0x2B47654E
	str	r1, [r0, #0x34]
	str	r1, [r6, #0x34]
	ldr	r1, =0x35330306
	str	r1, [r0, #0x38]
	str	r1, [r6, #0x38]
	ldr	r1, =0x442f0365
	str	r1, [r0, #0x3C]
	str	r1, [r6, #0x3C]

	mov	r2, #0x10000
2:	subs	r2, r2, #1
	bne	2b

	ldr	r1, =0x07000000
	str	r1, [r0, #0x10]
	str	r1, [r6, #0x10]
	mov	r2, #0x10000
3:	subs	r2, r2, #1
	bne	3b

	ldr	r1, =0x00071C00
	str	r1, [r0, #0x10]
	str	r1, [r6, #0x10]
	mov	r2, #0x10000
4:	subs	r2, r2, #1
	bne	4b

	ldr	r1, =0x00010BFC
	str	r1, [r0, #0x10]
	str	r1, [r6, #0x10]
	mov	r2, #0x10000
5:	subs	r2, r2, #1
	bne	5b

	ldr	r1, =0x00000488
	str	r1, [r0, #0x10]
	str	r1, [r6, #0x10]
	ldr	r1, =0x00000810
	str	r1, [r0, #0x10]
	str	r1, [r6, #0x10]
	ldr	r1, =0x00000C10
	str	r1, [r0, #0x10]
	str	r1, [r6, #0x10]

	mov	pc, lr

	.globl mem_ctrl_asm_init2
mem_ctrl_asm_init2:

	ldr	r0, =S5PC210_DMC0_BASE		@ 0x10400000
	ldr	r6, =S5PC210_DMC1_BASE		@ 0x10410000

	ldr	r1, =0x7110100A
	str	r1, [r0, #0x18]
	str	r1, [r6, #0x18]
	ldr	r1, =0x00000084
	str	r1, [r0, #0x1C]
	str	r1, [r6, #0x1C]
	ldr	r1, =0x7110100B
	str	r1, [r0, #0x18]
	str	r1, [r6, #0x18]

	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	ldr	r1, =0x0000008C
	str	r1, [r0, #0x1C]
	str	r1, [r6, #0x1C]
	ldr	r1, =0x00000084
	str	r1, [r0, #0x1C]
	str	r1, [r6, #0x1C]

	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	ldr	r1, =0x0FFF30FA
	str	r1, [r0, #0x00]
	str	r1, [r6, #0x00]
	mov	pc, lr

	.ltorg
