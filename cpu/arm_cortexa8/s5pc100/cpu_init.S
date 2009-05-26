/*
 * Originates from Samsung's u-boot 1.1.6 port to S5PC1xx
 *
 * Copyright (C) 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>

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
#include <s5pc1xx.h>

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:
	/* DLL parameter setting */
	ldr r0, =S5P_PHYCONTROL0			@ 0xe6000018
	ldr r1, =0x50101000
	str r1, [r0]

	ldr r0, =S5P_PHYCONTROL1			@ 0xe600001c
	ldr r1, =0xf4
	str r1, [r0]

	ldr r0, =S5P_PHYCONTROL2			@ 0xe6000020
	ldr r1, =0x0
	str r1, [r0]

	/* DLL on */
	ldr r0, =S5P_PHYCONTROL0			@ 0xe6000018
	ldr r1, =0x50101002
	str r1, [r0]

	/* DLL start */
	ldr r0, =S5P_PHUCONTROL0			@ 0xe6000018
	ldr r1, =0x50101003
	str r1, [r0]

	/* Force value locking for DLL off */
	ldr r0, =S5P_PHYCONTROL0			@ 0xe6000018
	ldr r1, =0x50101003
	str r1, [r0]

	/* DLL off */
	ldr r0, =S5P_PHYCONTROL0			@ 0xe6000018
	ldr r1, =0x50101001
	str r1, [r0]

	/* auto refresh off */
	ldr r0, =S5P_CONCONTROL				@ 0xe6000000
	ldr r1, =0xff001010
	str r1, [r0]

	/*
	 * BL%LE %LONG4, 2 chip, LPDDR, dynamic self refresh,
	 * force precharge, dynamic power down off
	 */
	ldr r0, =S5P_MEMCONTROL				@ 0xe6000004
	ldr r1, =0x00212100
	str r1, [r0]

	/* 128MB config, 4banks */
	ldr r0, =S5P_MEMCONFIG0				@ 0xe6000008
	ldr r1, =0x20f80222
	str r1, [r0]

	ldr r0, =S5P_MEMCONFIG1				@ 0xe600000c
	ldr r1, =0x28f80222
	str r1, [r0]

	ldr r0, =S5P_PRECHCONFIG			@ 0xe6000014
	ldr r1, =0x20000000
	str r1, [r0]

	/*
	 * 7.8us*166MHz%LE %LONG1294(0x50E) 7.8us*133MHz%LE %LONG1038(0x40E),
	 * 100MHz%LE %LONG780(0x30C),
	 * 20MHz%LE %LONG156(0x9C),
	 * 10MHz%LE %LONG78(0x4E)
	 */
	ldr r0, =S5P_TIMINGAREF				@ 0xe6000030
	ldr r1, =0x0000050e
	str r1, [r0]

	/* 133MHz */
	ldr r0, =S5P_TIMINGROW				@ 0xe6000034
	ldr r1, =0x0c233287
	str r1, [r0]

	/* twtr=3 twr=2 trtp=3 cl=3 wl=3 rl=3 */
	ldr r0, =S5P_TIMINGDATA				@ 0xe6000038
	ldr r1, =0x32330303
	str r1, [r0]

	/* tfaw=4 sxsr=0x14 txp=0x14 tcke=3 tmrd=3 */
	ldr r0, =S5P_TIMINGPOWER			@ 0xe600003c
	ldr r1, =0x04141433
	str r1, [r0]

	/* chip0 Deselect */
	ldr r0, =S5P_DIRECTCMD				@ 0xe6000010
	ldr r1, =0x07000000
	str r1, [r0]

	/* chip0 PALL */
	ldr r0, =S5P_DIRECTCMD				@ 0xe6000010
	ldr r1, =0x01000000
	str r1, [r0]

	/* chip0 REFA */
	ldr r0, =S5P_DIRECTCMD				@ 0xe6000010
	ldr r1, =0x05000000
	str r1, [r0]

	/* chip0 REFA */
	ldr r0, =S5P_DIRECTCMD				@ 0xe6000010
	ldr r1, =0x05000000
	str r1, [r0]

	/* chip0 MRS, CL%LE %LONG3, BL%LE %LONG4 */
	ldr r0, =S5P_DIRECTCMD				@ 0xe6000010
	ldr r1, =0x00000032
	str r1, [r0]

	/* chip1 Deselect */
	ldr r0, =S5P_DIRECTCMD				@ 0xe6000010
	ldr r1, =0x07100000
	str r1, [r0]

	/* chip1 PALL */
	ldr r0, =S5P_DIRECTCMD				@ 0xe6000010
	ldr r1, =0x01100000
	str r1, [r0]

	/* chip1 REFA */
	ldr r0, =S5P_DIRECTCMD				@ 0xe6000010
	ldr r1, =0x05100000
	str r1, [r0]

	/* chip1 REFA */
	ldr r0, =S5P_DIRECTCMD				@ 0xe6000010
	ldr r1, =0x05100000
	str r1, [r0]

	/* chip1 MRS, CL%LE %LONG3, BL%LE %LONG4 */
	ldr r0, =S5P_DIRECTCMD				@ 0xe6000010
	ldr r1, =0x00100032
	str r1, [r0]

	/* auto refresh on */
	ldr r0, =S5P_CONCONTROL				@ 0xe6000000
	ldr r1, =0xff002030
	str r1, [r0]

	/* PwrdnConfig */
	ldr r0, =S5P_PWRDNCONFIG			@ 0xe6000028
	ldr r1, =0x00100002
	str r1, [r0]

	/* BL%LE %LONG */
	ldr r0, =S5P_MEMCONROL				@ 0xe6000004
	ldr r1, =0xff212100
	str r1, [r0]
	mov	pc, lr

	.ltorg
