/*
 * Originates from Samsung's u-boot 1.1.6 port to S5PC1xx
 *
 * Copyright (C) 2009
 * Inki Dae, SAMSUNG Electronics, <inki.dae@samsung.com>
 * Heungjun Kim, SAMSUNG Electronics, <riverful.kim@samsung.com>
 * Minkyu Kang, SAMSUNG Electronics, <mk7.kang@samsung.com>
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
	ldr	r6, =S5P_CONCONTROL			@ 0xE6000000

	/* DLL parameter setting */
	ldr	r1, =0x50101000
	str	r1, [r6, #0x018]			@ S5P_PHYCONTROL0
	ldr	r1, =0xf4
	str	r1, [r6, #0x01C]			@ S5P_PHYCONTROL1
	ldr	r1, =0x0
	str	r1, [r6, #0x020]			@ S5P_PHYCONTROL2

	/* DLL on */
	ldr	r1, =0x50101002
	str	r1, [r6, #0x018]			@ S5P_PHYCONTROL0

	/* DLL start */
	ldr	r1, =0x50101003
	str	r1, [r6, #0x018]			@ S5P_PHYCONTROL0

	/* Force value locking for DLL off */
	str	r1, [r6, #0x018]			@ S5P_PHYCONTROL0

	/* DLL off */
	ldr	r1, =0x50101001
	str	r1, [r6, #0x018]			@ S5P_PHYCONTROL0

	/* auto refresh off */
	ldr	r1, =0xff001010
	str	r1, [r6, #0x000]			@ S5P_CONCONTROL

	/*
	 * Burst Length 4, 2 chips, 32-bit, LPDDR
	 * OFF: dynamic self refresh, force precharge, dynamic power down off
	 */
	ldr	r1, =0x00212100
	str	r1, [r6, #0x004]			@ S5P_MEMCONTROL

	/*
	 * 0x20 -> 0x20000000
	 * 0xf8 -> 0x27FFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	ldr	r1, =0x20f80222
	str	r1, [r6, #0x008]			@ S5P_MEMCONFIG0

	/*
	 * 0x28 -> 0x28000000
	 * 0xf8 -> 0x2fFFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	ldr	r1, =0x28f80222
	str	r1, [r6, #0x00c]			@ S5P_MEMCONFIG1

	ldr	r1, =0x20000000
	str	r1, [r6, #0x014]			@ S5P_PRECHCONFIG

	/*
	 * FIXME: Please verify these values
	 * 7.8us * 166MHz %LE %LONG1294(0x50E)
	 * 7.8us * 133MHz %LE %LONG1038(0x40E),
	 * 7.8us * 100MHz %LE %LONG780(0x30C),
	 * 7.8us * 20MHz  %LE %LONG156(0x9C),
	 * 7.8us * 10MHz  %LE %LONG78(0x4E)
	 */
	ldr	r1, =0x0000050e
	str	r1, [r6, #0x030]			@ S5P_TIMINGAREF

	/* 166 MHz */
	ldr	r1, =0x0c233287
	str	r1, [r6, #0x034]			@ S5P_TIMINGROW

	/* twtr=3 twr=2 trtp=3 cl=3 wl=3 rl=3 */
	ldr	r1, =0x32330303
	str	r1, [r6, #0x038]			@ S5P_TIMINGDATA

	/* tfaw=4 sxsr=0x14 txp=0x14 tcke=3 tmrd=3 */
	ldr	r1, =0x04141433
	str	r1, [r6, #0x03C]			@ S5P_TIMINGPOWER

	/* chip0 Deselect */
	ldr	r1, =0x07000000
	str	r1, [r6, #0x010]			@ S5P_DIRECTCMD

	/* chip0 PALL */
	ldr	r1, =0x01000000
	str	r1, [r6, #0x010]			@ S5P_DIRECTCMD

	/* chip0 REFA */
	ldr	r1, =0x05000000
	str	r1, [r6, #0x010]			@ S5P_DIRECTCMD
	/* chip0 REFA */
	str	r1, [r6, #0x010]			@ S5P_DIRECTCMD

	/* chip0 MRS, CL%LE %LONG3, BL%LE %LONG4 */
	ldr	r1, =0x00000032
	str	r1, [r6, #0x010]			@ S5P_DIRECTCMD

	/* chip1 Deselect */
	ldr	r1, =0x07100000
	str	r1, [r6, #0x010]			@ S5P_DIRECTCMD

	/* chip1 PALL */
	ldr	r1, =0x01100000
	str	r1, [r6, #0x010]			@ S5P_DIRECTCMD

	/* chip1 REFA */
	ldr	r1, =0x05100000
	str	r1, [r6, #0x010]			@ S5P_DIRECTCMD
	/* chip1 REFA */
	str	r1, [r6, #0x010]			@ S5P_DIRECTCMD

	/* chip1 MRS, CL%LE %LONG3, BL%LE %LONG4 */
	ldr	r1, =0x00100032
	str	r1, [r6, #0x010]			@ S5P_DIRECTCMD

	/* auto refresh on */
	ldr	r1, =0xff002030
	str	r1, [r6, #0x000]			@ S5P_CONCONTROL

	/* PwrdnConfig */
	ldr	r1, =0x00100002
	str	r1, [r6, #0x028]			@ S5P_PWRDNCONFIG

	/* BL%LE %LONG */
	ldr	r1, =0xff212100
	str	r1, [r6, #0x004]			@ S5P_MEMCONTROL

	mov	pc, lr

	.ltorg
