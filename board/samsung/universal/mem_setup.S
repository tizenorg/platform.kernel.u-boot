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
#include <asm/arch/mem.h>

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:
	cmp	r7, r8
	ldreq	r0, =S5PC100_DMC_BASE			@ 0xE6000000
	ldrne	r0, =S5PC110_DMC0_BASE			@ 0xF0000000
	ldrne	r6, =S5PC110_DMC1_BASE			@ 0xF1400000

	/* DLL parameter setting */
	ldr	r1, =0x50101000
	str	r1, [r0, #0x018]			@ PHYCONTROL0_OFFSET
	strne	r1, [r6, #0x018]			@ PHYCONTROL0_OFFSET
	ldr	r1, =0xf4
	str	r1, [r0, #0x01C]			@ PHYCONTROL1_OFFSET
	strne	r1, [r6, #0x01C]			@ PHYCONTROL1_OFFSET
	ldreq	r1, =0x0
	streq	r1, [r0, #0x020]			@ PHYCONTROL2_OFFSET

	/* DLL on */
	ldr	r1, =0x50101002
	str	r1, [r0, #0x018]			@ PHYCONTROL0_OFFSET
	strne	r1, [r6, #0x018]			@ PHYCONTROL0_OFFSET

	/* DLL start */
	ldr	r1, =0x50101003
	str	r1, [r0, #0x018]			@ PHYCONTROL0_OFFSET
	strne	r1, [r6, #0x018]			@ PHYCONTROL0_OFFSET

	/* Force value locking for DLL off */
	str	r1, [r0, #0x018]			@ PHYCONTROL0_OFFSET
	strne	r1, [r6, #0x018]			@ PHYCONTROL0_OFFSET

	/* DLL off */
	ldr	r1, =0x50101001
	str	r1, [r0, #0x018]			@ PHYCONTROL0_OFFSET
	strne	r1, [r6, #0x018]			@ PHYCONTROL0_OFFSET

	/* auto refresh off */
	ldr	r1, =0xff001010
	str	r1, [r0, #0x000]			@ CONCONTROL_OFFSET
	strne	r1, [r6, #0x000]			@ CONCONTROL_OFFSET

	/*
	 * Burst Length 4, 2 chips, 32-bit, LPDDR
	 * OFF: dynamic self refresh, force precharge, dynamic power down off
	 */
	ldr	r1, =0x00212100
	str	r1, [r0, #0x004]			@ MEMCONTROL_OFFSET
	strne	r1, [r6, #0x004]			@ MEMCONTROL_OFFSET

	/*
	 * Note:
	 * If Bank0 has OneDRAM we place it at 0x2800'0000
	 * So finally Bank1 should address start at at 0x2000'0000
	 */
	mov	r4, #0x0

swap_memory:
	cmp	r7, r8
	/*
	 * DMC0: CS0 : S5PC100/S5PC110
	 * 0x20 -> 0x20000000
	 * 0xf8 -> 0x27FFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	ldr	r1, =0x20f80222
	/* if r4 is 1, swap the bank */
	cmp	r4, #0x1
	orreq	r1, r1, #0x08000000
	str	r1, [r0, #0x008]			@ MEMCONFIG0_OFFSET

	/*
	 * DMC1: CS0 : S5PC110
	 * 0x40 -> 0x40000000
	 * 0xf8 -> 0x47FFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	cmp	r7, r8
	ldrne	r1, =0x40f80222
	strne	r1, [r6, #0x008]			@ MEMCONFIG0_OFFSET

	/*
	 * DMC0: CS1: S5PC100
	 * 0x28 -> 0x28000000
	 * 0xf8 -> 0x2fFFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	cmp	r7, r8
	ldr	r1, =0x28f80222
	/* if r4 is 1, swap the bank */
	cmp	r4, #0x1
	biceq	r1, r1, #0x08000000
	streq	r1, [r0, #MEMCONFIG1_OFFSET]

	cmp	r7, r8
	ldr	r1, =0x20000000
	str	r1, [r0, #0x014]			@ PRECHCONFIG_OFFSET
	strne	r1, [r0, #0x014]			@ PRECHCONFIG_OFFSET
	strne	r1, [r6, #0x014]			@ PRECHCONFIG_OFFSET

	/*
	 * S5PC100:
	 * DMC:  CS0: 166MHz
	 *       CS1: 166MHz
	 * S5PC110:
	 * DMC0: CS0: 166MHz
	 * DMC1: CS0: 200MHz
	 * 
	 * 7.8us * 200MHz %LE %LONG1560(0x618)
	 * 7.8us * 166MHz %LE %LONG1294(0x50E)
	 * 7.8us * 133MHz %LE %LONG1038(0x40E),
	 * 7.8us * 100MHz %LE %LONG780(0x30C),
	 */
	ldr	r1, =0x0000050E
	str	r1, [r0, #0x030]			@ TIMINGAREF_OFFSET
	ldrne	r1, =0x00000618
	strne	r1, [r6, #0x030]			@ TIMINGAREF_OFFSET

	ldr	r1, =0x0c233287
	str	r1, [r0, #0x034]			@ TIMINGROW_OFFSET
	ldrne	r1, =0x11344309
	strne	r1, [r6, #0x034]			@ TIMINGROW_OFFSET

	/* twtr=3 twr=2 trtp=3 cl=3 wl=3 rl=3 */
	ldr	r1, =0x32330303
	str	r1, [r0, #0x038]			@ TIMINGDATA_OFFSET
	ldrne	r1, =0x12130005
	strne	r1, [r6, #0x038]			@ TIMINGDATA_OFFSET

	/* tfaw=4 sxsr=0x14 txp=0x14 tcke=3 tmrd=3 */
	ldr	r1, =0x04141433
	str	r1, [r0, #0x03C]			@ TIMINGPOWER_OFFSET
	ldrne	r1, =0x0E190222
	strne	r1, [r6, #0x03C]			@ TIMINGPOWER_OFFSET

	/* chip0 Deselect */
	ldr	r1, =0x07000000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip0 PALL */
	ldr	r1, =0x01000000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip0 REFA */
	ldr	r1, =0x05000000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET
	/* chip0 REFA */
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip0 MRS, CL%LE %LONG3, BL%LE %LONG4 */
	ldr	r1, =0x00000032
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip1 Deselect */
	ldr	r1, =0x07100000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip1 PALL */
	ldr	r1, =0x01100000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip1 REFA */
	ldr	r1, =0x05100000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET
	/* chip1 REFA */
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip1 MRS, CL%LE %LONG3, BL%LE %LONG4 */
	ldr	r1, =0x00100032
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* auto refresh on */
	ldr	r1, =0xff002030
	str	r1, [r0, #0x000]			@ S5P_CONCONTROL
	strne	r1, [r6, #0x000]			@ S5P_CONCONTROL

	/* PwrdnConfig */
	ldr	r1, =0x00100002
	str	r1, [r0, #0x028]			@ S5P_PWRDNCONFIG
	strne	r1, [r6, #0x028]			@ S5P_PWRDNCONFIG

	/* BL%LE %LONG */
	ldr	r1, =0xff212100
	str	r1, [r0, #0x004]			@ S5P_MEMCONTROL
	strne	r1, [r6, #0x004]			@ S5P_MEMCONTROL

	/* Skip when S5PC110 */
	cmp	r7, r8
	bne	1f

	/* Try to test memory area */
	cmp	r4, #0x1
	beq	1f

	mov	r4, #0x1
	ldr	r1, =0x27ffff00
	str	r4, [r1]
	str	r4, [r1, #0x4]				@ dummy write
	ldr	r2, [r1]
	cmp	r2, r4
	bne	swap_memory
1:
	mov	pc, lr

	.ltorg
