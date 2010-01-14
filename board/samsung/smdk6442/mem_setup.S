/*
 * Originates from Samsung's u-boot 1.1.6 port to S5PC1xx
 *
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

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:
	ldr	r6, =S5P6442_DMC_BASE			@ 0xE6000000

	/* DLL parameter setting */
	ldr	r1, =0x00141408
	str	r1, [r6, #0x018]			@ PHYCONTROL0
	ldr	r1, =0x00008044
	str	r1, [r6, #0x01C]			@ PHYCONTROL1

	/* DLL on */
	ldr	r1, =0x0014140a
	str	r1, [r6, #0x018]			@ PHYCONTROL0

	/* DLL start */
	ldr	r1, =0x0014140b
	str	r1, [r6, #0x018]			@ PHYCONTROL0

	/* auto refresh off */
	ldr	r1, =0x0f001090
	str	r1, [r6, #0x000]			@ CONCONTROL

	/*
	 * Burst Length 4, 2 chips, 32-bit, LPDDR
	 * OFF: dynamic self refresh, force precharge, dynamic power down off
	 */
	ldr	r1, =0x00202100
	str	r1, [r6, #0x004]			@ MEMCONTROL

	/*
	 * Bank0
	 * 0x20 -> 0x20000000
	 * 0xf0 -> 0x2FFFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 3: 10 bits
	 * [ 7:4 ] 1: 13 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	ldr	r1, =0x20f00312
	str	r1, [r6, #0x008]			@ MEMCONFIG0

	/*
	 * Bank1
	 * 0x30 -> 0x30000000
	 * 0xf0 -> 0x3FFFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	ldr	r1, =0x30f00222
	str	r1, [r6, #0x00c]			@ MEMCONFIG1

	ldr	r1, =0xf0000000
	str	r1, [r6, #0x014]			@ PRECHCONFIG

	/*
	 * FIXME: Please verify these values
	 * 7.8us * 166MHz %LE %LONG1294(0x50E)
	 * 7.8us * 133MHz %LE %LONG1038(0x40E),
	 * 7.8us * 100MHz %LE %LONG780(0x30C),
	 * 7.8us * 20MHz  %LE %LONG156(0x9C),
	 * 7.8us * 10MHz  %LE %LONG78(0x4E)
	 */
	ldr	r1, =0x0000040e
	str	r1, [r6, #0x030]			@ TIMINGAREF

	/* 133 MHz */
	ldr	r1, =0x0c233286
	str	r1, [r6, #0x034]			@ TIMINGROW

	/* t_wtr=1 t_wr=2 t_rtp=1 cl=3 wl=0 rl=5 */
	ldr	r1, =0x12130005
	str	r1, [r6, #0x038]			@ TIMINGDATA

	/* t_faw=0xe t_xsr=0x10 t_xp=2 t_cke=2 t_mrd=2 */
	ldr	r1, =0x0e100222
	str	r1, [r6, #0x03C]			@ TIMINGPOWER

	/* chip0 Deselect */
	ldr	r1, =0x07000000
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* chip0 PALL */
	ldr	r1, =0x01000000
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* chip0 REFA */
	ldr	r1, =0x05000000
	str	r1, [r6, #0x010]			@ DIRECTCMD
	/* chip0 REFA */
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* chip0 MRS, CL%LE %LONG3, BL%LE %LONG4 */
	ldr	r1, =0x00000032
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* auto refresh on */
	ldr	r1, =0x0f0010b0
	str	r1, [r6, #0x000]			@ CONCONTROL

	/* PwrdnConfig */
	ldr	r1, =0x00100002
	str	r1, [r6, #0x028]			@ PWRDNCONFIG

	/* BL%LE %LONG */
	ldr	r1, =0x00202100
	str	r1, [r6, #0x004]			@ MEMCONTROL

	mov	pc, lr

	.ltorg
