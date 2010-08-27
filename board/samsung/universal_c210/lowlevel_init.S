/*
 * Lowlevel setup for universal board based on S5PC210
 *
 * Copyright (C) 2010 Samsung Electronics
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
#include <version.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/power.h>

/*
 * Register usages:
 *
 * r5 has zero always
 * r6 is used at memory configuration
 * r7 has GPIO part1 base 0x11400000
 * r8 has GPIO part2 base 0x11000000
 */

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r11, lr

	/* r5 has always zero */
	mov	r5, #0

	ldr	r7, =S5PC210_GPIO_PART1_BASE
	ldr	r8, =S5PC210_GPIO_PART2_BASE

#ifndef CONFIG_PRELOADER
	/* Workaround: PMIC manual reset */
	/* nPOWER: XEINT_23: GPX2[7] */
	add	r0, r8, #0xC40			@ S5PC210_GPIO_X2_OFFSET
	ldr	r1, [r0, #0x0]
	bic	r1, r1, #(0xf << 28)		@ 28 = 7 * 4-bit
	orr	r1, r1, #(0x1 << 28)		@ Output
	str	r1, [r0, #0x0]

	ldr	r1, [r0, #0x4]
	orr	r1, r1, #(1 << 7)		@ 7 = 7 * 1-bit
	str	r1, [r0, #0x4]
#endif

#if 0
	/* IO retension release */
	ldr	r0, =0x10020000
	ldr	r2, =(1 << 28)
	ldr	r3, =0x3108
	add	r1, r0, r3
	str	r2, [r1]
	ldr	r3, =0x3108
	add	r1, r0, r3
	str	r2, [r1]
	ldr	r3, =0x3128
	add	r1, r0, r3
	str	r2, [r1]
	ldr	r3, =0x3148
	add	r1, r0, r3
	str	r2, [r1]
	ldr	r3, =0x3168
	add	r1, r0, r3
	str	r2, [r1]
	ldr	r3, =0x3188
	add	r1, r0, r3
	str	r2, [r1]
	ldr	r3, =0x31A8
	add	r1, r0, r3
	str	r2, [r1]
#endif

#ifdef CONFIG_PRELOADER
	bl	mem_ctrl_asm_init
#endif

	/* init system clock */
	bl	system_clock_init

#ifdef CONFIG_PRELOADER
	bl	mem_ctrl_asm_init2
#endif

#ifdef CONFIG_PRELOADER
	ldr	r0, =0x0C600000

	ldr	r1, =0x1212
	str	r1, [r0, #0x108]

1:	ldr	r1, [r0, #0x10C]			@ ONENAND_IF_STATUS
	and	r1, r1, #1
	cmp	r1, #1
	beq	1b

	/*
	 * OneNAND Sync Read Support
	 * RM[15]	: Sync Read
	 * BRWL[14:12]	: 4 CLK
	 * BL[11:9]	: Continuous
	 * VHF[3]	: Very High Frequency Enable (Over 83MHz)
	 * HF[2]	: High Frequency Enable (Over 66MHz)
	 * WM[1]	: Sync Write
	 */
        ldr	r1, =0xE006
        ldr	r2, =0x0C01E442
        strh	r1, [r2]
	/* Dummy read required */
	ldr	r2, =0x0C000000
	ldrh	r1, [r2]

	/* XXX: Must insert 2 nop */
	nop
	nop

	/*
	 * GCE[26]	: Gated Clock Enable
	 * RPE[17]	: Enables Read Prefetch
	 * HF[2]	: High Frequency
	 */
	ldr	r1, =((1 << 26) | (1 << 17) | 0xE006)
	str	r1, [r0, #0x100]                        @ ONENAND_IF_CTRL
#endif

#ifndef CONFIG_PRELOADER
	/* UART */
	bl	uart_asm_init
#endif

#ifndef CONFIG_PRELOADER
	/* PMU init */
	bl	system_power_init
#endif

	mov	lr, r11
	mov	pc, lr

#ifndef CONFIG_PRELOADER
/*
 * uart_asm_init: Initialize UART's pins
 */
uart_asm_init:
	/*
	 * setup UART0-UART4 GPIOs (part1)
	 * GPA1CON[3] = I2C_3_SCL (3)
	 * GPA1CON[2] = I2C_3_SDA (3)
	 */
	mov	r0, r7
	ldr	r1, =0x22222222
	str	r1, [r0, #0x00]			@ S5PC210_GPIO_A0_OFFSET
	ldr	r1, =0x00223322
	str	r1, [r0, #0x20]			@ S5PC210_GPIO_A1_OFFSET

	/* UART_SEL GPY4[7] (part2) at S5PC210 */
	add	r0, r8, #0x1A0			@ S5PC210_GPIO_Y4_OFFSET
	ldr	r1, [r0, #0x0]
	bic	r1, r1, #(0xf << 28)		@ 28 = 7 * 4-bit
	orr	r1, r1, #(0x1 << 28)
	str	r1, [r0, #0x0]

	ldr	r1, [r0, #0x8]
	bic	r1, r1, #(0x3 << 14)		@ 14 = 7 * 2-bit
	orr	r1, r1, #(0x3 << 14)		@ Pull-up enabled
	str	r1, [r0, #0x8]

	ldr	r1, [r0, #0x4]
	orr	r1, r1, #(1 << 7)		@ 7 = 7 * 1-bit
	str	r1, [r0, #0x4]

#if 0
	ldr	r0, =0x13800000                 @ S5PC210_PA_UART
	orr     r0, r0, #0x20000		@ UART2
	mov     r1, #0x3
	str     r1, [r0, #0x000]                @ ULCON
	ldr     r1, =0x3c5
	str     r1, [r0, #0x004]                @ UCON
	mov     r1, #0x2B
	str     r1, [r0, #0x028]                @ UBRDIV
	mov     r1, #0xC
	str     r1, [r0, #0x02C]                @ UFRACVAL

	mov     r2, #'W'
	strb    r2, [r0, #0x020]                @ UTXH
1001:
	ldrb    r3, [r0, #0x010]                @ UTRSTAT
	tst	r3, #(1 << 2)
	beq     1001b
#endif

	mov	pc, lr
#endif

system_clock_init:
	ldr	r0, =S5PC210_CLOCK_BASE

	/* APLL(1), MPLL(0), CORE(1), HPM(0) */
	ldr	r1, =0x0101
	ldr	r2, =0x14200			@ CLK_SRC_CPU
	str	r1, [r0, r2]
	/*
	 * CLK_SRC_TOP0
	 * MUX_ONENAND_SEL[28]	0: DOUT133, 1: DOUT166
	 */
	ldr	r1, =0x10000000
	ldr	r2, =0x0C210			@ CLK_SRC_TOP
	str	r1, [r0, r2]

	/* SATA: SCLKMPLL(0), MMC[0:4]: SCLKMPLL(6) */
	ldr	r1, =0x0066666
	ldr	r2, =0x0C240			@ CLK_SRC_FSYS
	str	r1, [r0, r2]
	/* UART[0:5], PWM: SCLKMPLL(6) */
	ldr	r1, =0x6666666
	ldr	r2, =0x0C250			@ CLK_SRC_PERIL0_OFFSET
	str	r1, [r0, r2]

	/* CPU0: CORE, COREM0, COREM1, PERI, ATB, PCLK_DBG, APLL */
	ldr	r1, =0x0133730
	ldr	r2, =0x14500			@ CLK_DIV_CPU0
	str	r1, [r0, r2]
	/* CPU1: COPY, HPM */
	ldr	r1, =0x03
	ldr	r2, =0x14504			@ CLK_DIV_CPU1
	str	r1, [r0, r2]
	/* DMC0: ACP, ACP_PCLK, DPHY, DMC, DMCD, DMCP, COPY2 CORE_TIMER */
	ldr	r1, =0x13111113
	ldr	r2, =0x10500			@ CLK_DIV_DMC0
	str	r1, [r0, r2]
	/* DMC1: PWI, DVSEM, DPM */
	ldr	r1, =0x01010100
	ldr	r2, =0x10504			@ CLK_DIV_DMC1
	str	r1, [r0, r2]
	/* LEFTBUS: GDL, GPL */
	ldr	r1, =0x13
	ldr	r2, =0x04500			@ CLK_DIV_LEFTBUS
	str	r1, [r0, r2]
	/* RIGHHTBUS: GDR, GPR */
	ldr	r1, =0x13
	ldr	r2, =0x08500			@ CLK_DIV_RIGHTBUS
	str	r1, [r0, r2]
	/*
	 * CLK_DIV_TOP
	 * ONENAND_RATIOD[18:16]: 0 SCLK_ONENAND = MOUTONENAND / (n + 1)
	 * ACLK_200, ACLK_100, ACLK_160, ACLK_133,
	 */
	ldr	r1, =0x00005473
	ldr	r2, =0x0C510			@ CLK_DIV_TOP
	str	r1, [r0, r2]
	/* MMC[0:1] */
	ldr	r1, =0x000d000d			/* FIXME check the value */
	ldr	r2, =0x0C544			@ CLK_DIV_FSYS1
	str	r1, [r0, r2]
	/* MMC[2:3] */
	ldr	r1, =0x000d000d			/* FIXME check the value */
	ldr	r2, =0x0C548			@ CLK_DIV_FSYS2
	str	r1, [r0, r2]
	/* MMC4 */
	ldr	r1, =0x000d			/* FIXME check the value */
	ldr	r2, =0x0C54C			@ CLK_DIV_FSYS3
	str	r1, [r0, r2]
	/* UART[0:5] */
	ldr	r1, =0x777777
	ldr	r2, =0x0C550			@ CLK_DIV_PERIL0
	str	r1, [r0, r2]
	/* SLIMBUS: ???, PWM */
	ldr	r1, =0x8
	ldr	r2, =0x0C55C			@ CLK_DIV_PERIL3
	str	r1, [r0, r2]

	/* PLL Setting */
	ldr	r1, =0x1C20
	ldr	r2, =0x14000			@ APLL_LOCK
	str	r1, [r0, r2]
	ldr	r2, =0x14008			@ MPLL_LOCK
	str	r1, [r0, r2]
	ldr	r2, =0x0C010			@ EPLL_LOCK
	str	r1, [r0, r2]
	ldr	r2, =0x0C020			@ VPLL_LOCK
	str	r1, [r0, r2]

	/* APLL */
	ldr	r1, =0x8000001c
	ldr	r2, =0x14104			@ APLL_CON1
	str	r1, [r0, r2]
	ldr	r1, =0x80c80601			@ 800MHz
	ldr	r2, =0x14100			@ APLL_CON0
	str	r1, [r0, r2]
	/* MPLL */
	ldr	r1, =0x8000000d
	ldr	r2, =0x1410C			@ MPLL_CON1
	str	r1, [r0, r2]
	ldr	r1, =0x806e0401			@ 660MHz
	ldr	r2, =0x14108			@ MPLL_CON0
	str	r1, [r0, r2]
	/* EPLL */
	ldr	r1, =0x0
	ldr	r2, =0x0C114			@ EPLL_CON1
	str	r1, [r0, r2]
	ldr	r1, =0x80300302			@ 96MHz
	ldr	r2, =0x0C110			@ EPLL_CON0
	str	r1, [r0, r2]
	/* VPLL */
	ldr	r1, =0x11000400
	ldr	r2, =0x0C124			@ VPLL_CON1
	str	r1, [r0, r2]
	ldr	r1, =0x80350302			@ 108MHz
	ldr	r2, =0x0C120			@ VPLL_CON0
	str	r1, [r0, r2]

#ifndef CONFIG_PRELOADER
	/* DSIM0[3]: 0, MDNIE0[2]: 0, MIE0[1]: 0 */
	ldr	r1, =0xFFFFFFF1
	ldr	r2, =0x0C934			@ CLK_GATE_IP_LCD0
	str	r1, [r0, r2]

	/* LCD1[5]: 0, G3D[3]: 0 */
	ldr	r1, =0xFFFFFFD7
	ldr	r2, =0x0C970			@ CLK_GATE_BLOCK
	str	r1, [r0, r2]
#endif
	mov	pc, lr

#ifndef CONFIG_PRELOADER
system_power_init:
	ldr	r0, =S5PC210_POWER_BASE

	ldr	r2, =0x330C			@ PS_HOLD_CONTROL
	ldr	r1, [r0, r2]
	orr	r1, r1, #(0x3 << 8)		@ Data High, Output En
	str	r1, [r0, r2]

	mov	pc, lr
#endif
