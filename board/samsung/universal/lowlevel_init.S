/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 2009 Samsung Electronics
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
#include <asm/arch/mem.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/power.h>
#include <asm/arch/watchdog.h>
#include <asm/arch/interrupt.h>

/*
 * Register usages:
 *
 * r5 has zero always
 * r7 has S5PC100 GPIO base, 0xE0300000
 * r8 has real GPIO base, 0xE0300000, 0xE0200000 at S5PC100, S5PC110 repectively
 */

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r9, lr

	/* r5 has always zero */
	mov	r5, #0

	ldr	r7, =S5PC100_GPIO_BASE(0)
	ldr	r8, =S5PC100_GPIO_BASE(0)
	/* Read CPU ID */
	ldr	r2, =S5PC1XX_PRO_ID
	ldr	r0, [r2]
	mov	r1, #0x00010000
	and	r0, r0, r1
	cmp	r0, r5
	beq	100f
	ldr	r8, =S5PC110_GPIO_BASE(0)
100:
	/* Turn on KEY_LED_ON [GPJ4(1)] */
	cmp	r7, r8
	addeq	r0, r8, #S5PC100_GPIO_J4_OFFSET
	addne	r0, r8, #S5PC110_GPIO_J4_OFFSET
	ldr	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]
	bic	r1, r1, #(0xf << 4)			@ 1 * 4-bit
	orr	r1, r1, #(0x1 << 4)
	str	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]

	ldr	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]
#ifdef CONFIG_ONENAND_IPL
	orr	r1, r1, #(1 << 1)			@ 1 * 1-bit
#else
	bic	r1, r1, #(1 << 1)
#endif
	str	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]

	/* IO retension release */
	ldreq	r0, =S5PC100_OTHERS			@0xE0108200
	ldrne	r0, =S5PC110_OTHERS			@0xE010E000
	ldr	r1, [r0]
	ldreq	r2, =(1 << 31)				@IO_RET_REL
	ldrne	r2, =((1 << 31) | (1 << 29) | (1 << 28))	@ GPIO, UART_IO
	orr	r1, r1, r2
	str	r1, [r0]

#ifndef CONFIG_ONENAND_IPL
	/* Disable Watchdog */
	ldreq	r0, =S5PC100_WATCHDOG_BASE		@0xEA200000
	ldrne	r0, =S5PC110_WATCHDOG_BASE		@0xE2700000
	str	r5, [r0]

	/* setting SRAM */
	ldreq	r0, =S5PC100_SROMC_BASE
	ldrne	r0, =S5PC110_SROMC_BASE
	ldr	r1, =0x9
	str	r1, [r0]
#endif

	/* S5PC100 has 3 groups of interrupt sources */
	ldreq	r0, =S5PC100_VIC0_BASE			@0xE4000000
	ldrne	r0, =S5PC110_VIC0_BASE			@0xF2000000
	add	r1, r0, #0x00100000
	add	r2, r0, #0x00200000

	/* Disable all interrupts (VIC0, VIC1 and VIC2) */
	mvn	r3, #0x0
	str	r3, [r0, #VIC_INTENCLEAR_OFFSET]
	str	r3, [r1, #VIC_INTENCLEAR_OFFSET]
	str	r3, [r2, #VIC_INTENCLEAR_OFFSET]

#ifndef CONFIG_ONENAND_IPL
	/* Set all interrupts as IRQ */
	str	r5, [r0, #VIC_INTSELECT_OFFSET]
	str	r5, [r1, #VIC_INTSELECT_OFFSET]
	str	r5, [r2, #VIC_INTSELECT_OFFSET]

	/* Pending Interrupt Clear */
	str	r5, [r0, #VIC_INTADDRESS_OFFSET]
	str	r5, [r1, #VIC_INTADDRESS_OFFSET]
	str	r5, [r2, #VIC_INTADDRESS_OFFSET]
#endif

#ifndef CONFIG_ONENAND_IPL
	/* for UART */
	bl	uart_asm_init

	/* for TZPC */
	bl	tzpc_asm_init

	bl	internal_ram_init
#endif

#ifdef CONFIG_ONENAND_IPL
	/* init system clock */
	bl	system_clock_init

	bl	mem_ctrl_asm_init

	/* Wakeup support. Don't know if it's going to be used, untested. */
	ldreq	r0, =S5PC100_RST_STAT
	ldrne	r0, =S5PC110_RST_STAT
	ldr	r1, [r0]
	bne	110f
	bic	r1, r1, #0xfffffff7
	mov	r2, #(1 << 3)
	b	200f
110:
	bic	r1, r1, #0xfffeffff
	mov	r2, #(1 << 16)
200:
	cmp	r1, r2
	bne	1f
wakeup:
	cmp	r7, r8
	/* Clear wakeup status register */
	ldreq	r0, =S5PC100_WAKEUP_STAT
	ldrne	r0, =S5PC110_WAKEUP_STAT
	ldr	r1, [r0]
	str	r1, [r0]

	/* Load return address and jump to kernel */
	ldreq	r0, =S5PC100_INFORM0
	ldrne	r0, =S5PC110_INFORM0

	/* r1 = physical address of s5pc1xx_cpu_resume function */
	ldr	r1, [r0]

	/* Jump to kernel (sleep-s5pc1xx.S) */
	mov	pc, r1
	nop
	nop
#endif
1:
	mov	lr, r9
	mov	pc, lr

/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:
	ldr	r0, =S5PC1XX_CLOCK_BASE		@ 0xE0100000

	/* Check S5PC100 */
	cmp	r7, r8
	bne	110f
100:
	/* Set Lock Time */
	ldr	r1, =0xe10			@ Locktime : 0xe10 = 3600
	str	r1, [r0, #0x000]		@ S5PC100_APLL_LOCK
	str	r1, [r0, #0x004]		@ S5PC100_MPLL_LOCK
	str	r1, [r0, #0x008]		@ S5PC100_EPLL_LOCK
	str	r1, [r0, #0x00C]		@ S5PC100_HPLL_LOCK

	/* S5P_APLL_CON */
#ifdef CONFIG_CLK_667_166_83
	ldr	r1, =0x81bc0400		@ SDIV 0, PDIV 4, MDIV 444 (1333MHz)
#elif defined(CONFIG_CLK_666_166_66)
	ldr	r1, =0x814d0301		@ SDIV 1, PDIV 3, MDIV 333 (1333MHz)
#elif defined(CONFIG_CLK_600_150_75)
	ldr	r1, =0x812C0300		@ SDIV 0, PDIV 3, MDIV 300 (1200MHz)
#elif defined(CONFIG_CLK_533_133_66)
	ldr	r1, =0x810b0300		@ SDIV 0, PDIV 3, MDIV 267 (1066MHz)
#elif defined(CONFIG_CLK_500_166_66)
	ldr	r1, =0x81f40301		@ SDIV 1, PDIV 3, MDIV 500 (1000MHz)
#elif defined(CONFIG_CLK_467_117_59)
	ldr	r1, =0x826E0401		@ SDIV 1, PDIV 4, MDIV 622 (933MHz)
#elif defined(CONFIG_CLK_400_100_50)
	ldr	r1, =0x81900301		@ SDIV 1, PDIV 3, MDIV 400 (800MHz)
#else
#error you should set the correct clock configuration
#endif
	str	r1, [r0, #0x100]
	/* S5P_MPLL_CON */
	ldr	r1, =0x80590201		@ SDIV 1, PDIV 2, MDIV 89 (267MHz)
	str	r1, [r0, #0x104]
	/* S5P_EPLL_CON */
	ldr	r1, =0x80870303		@ SDIV 3, PDIV 3, MDIV 135 (67.5MHz)
	str	r1, [r0, #0x108]
	/* S5P_HPLL_CON */
	ldr	r1, =0x80600603		@ SDIV 3, PDIV 6, MDIV 96
	str	r1, [r0, #0x10C]

	ldr     r1, [r0, #0x300]
	ldr     r2, =0x00003fff
	bic     r1, r1, r2
#ifdef CONFIG_CLK_800_166_66
	ldr     r2, =0x00011401
#elif defined(CONFIG_CLK_500_166_66)
	ldr     r2, =0x00011201
#elif defined(CONFIG_CLK_666_166_66)
	ldr     r2, =0x00011300
#else
	ldr     r2, =0x00011301
#endif
	orr	r1, r1, r2
	str	r1, [r0, #0x300]
	ldr     r1, [r0, #0x304]
	ldr     r2, =0x00011110
	orr     r1, r1, r2
	str     r1, [r0, #0x304]
	ldr     r1, =0x00000001
	str     r1, [r0, #0x308]

	/* Set Source Clock */
	ldr	r1, =0x00001111			@ A, M, E, HPLL Muxing
	str	r1, [r0, #0x200]		@ S5PC1XX_CLK_SRC0

	b	200f
110:
	/* Set Clock divider */
	ldr	r1, =0x14131330			@ 1:1:4:4, 1:4:5
	str	r1, [r0, #0x300]
	ldr	r1, =0x11110111			@ UART[3210]: MMC[3210]
	str	r1, [r0, #0x310]

	/* Set Lock Time */
	ldr	r1, =0xe10			@ Locktime : 0xe10 = 3600
	str	r1, [r0, #0x000]		@ S5PC110_APLL_LOCK
	str	r1, [r0, #0x010]		@ S5PC110_MPLL_LOCK
	str	r1, [r0, #0x018]		@ S5PC110_EPLL_LOCK
	str	r1, [r0, #0x020]		@ S5PC110_VPLL_LOCK

	/* S5PC110_APLL_CON */
	ldr	r1, =0x80C80601			@ 800MHz
	str	r1, [r0, #0x100]
	/* S5PC110_MPLL_CON */
	ldr	r1, =0x829B0C01			@ 667MHz
	str	r1, [r0, #0x108]
	/* S5PC110_EPLL_CON */
	ldr	r1, =0x80600602			@  96MHz
	str	r1, [r0, #0x110]
	/* S5PC110_VPLL_CON */
	ldr	r1, =0x806C0603			@  54MHz
	str	r1, [r0, #0x120]

	/* Set Source Clock */
	ldr	r1, =0x10001111			@ A, M, E, VPLL Muxing
	str	r1, [r0, #0x200]		@ S5PC1XX_CLK_SRC0

	/* OneDRAM(DMC0) clock setting */
	ldr	r1, =0x01000000
	str	r1, [r0, #0x218]
	ldr	r1, =0x30000000
	str	r1, [r0, #0x318]

	ldr	r1, =0x00909000
	str	r1, [r0, #0x500]

200:
	/* wait at least 200us to stablize all clock */
	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b

	mov	pc, lr

#ifndef CONFIG_ONENAND_IPL
internal_ram_init:
	ldreq	r0, =0xE3800000
	ldrne	r0, =0xF1500000
	ldr	r1, =0x0
	str	r1, [r0]

	mov	pc, lr
#endif

#ifndef CONFIG_ONENAND_IPL
/*
 * uart_asm_init: Initialize UART's pins
 */
uart_asm_init:
	/* set GPIO to enable UART0-UART4 */
	mov	r0, r8
	ldr	r1, =0x22222222
	str	r1, [r0, #S5PC100_GPIO_A0_OFFSET]		@ GPA0_CON
	ldr	r1, =0x00002222
	str	r1, [r0, #S5PC100_GPIO_A1_OFFSET]		@ GPA1_CON

	/* Check S5PC100 */
	cmp	r7, r8
	bne	110f

	/* UART_SEL GPK0[5] at S5PC100 */
	add	r0, r8, #S5PC100_GPIO_K0_OFFSET
	ldr	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]
	bic	r1, r1, #(0xf << 20)		@ 20 = 5 * 4-bit
	orr	r1, r1, #(0x1 << 20)		@ Output
	str	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]

	ldr	r1, [r0, #S5PC1XX_GPIO_PULL_OFFSET]
	bic	r1, r1, #(0x3 << 10)		@ 10 = 5 * 2-bit
	orr	r1, r1, #(0x2 << 10)		@ Pull-up enabled
	str	r1, [r0, #S5PC1XX_GPIO_PULL_OFFSET]

	ldr	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]
	orr	r1, r1, #(1 << 5)		@ 5 = 5 * 1-bit
	str	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]
	b	200f
110:
	/*
	 * Note that the following address
	 * 0xE020'0360 is reserved address at S5PC100
	 */
	/* UART_SEL MP0_5[7] at S5PC110 */
	add	r0, r8, #S5PC110_GPIO_MP0_5_OFFSET
	ldr	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]
	bic	r1, r1, #(0xf << 28)		@ 28 = 7 * 4-bit
	orr	r1, r1, #(0x1 << 28)		@ Output
	str	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]

	ldr	r1, [r0, #S5PC1XX_GPIO_PULL_OFFSET]
	bic	r1, r1, #(0x3 << 14)		@ 14 = 7 * 2-bit
	orr	r1, r1, #(0x2 << 14)		@ Pull-up enabled
	str	r1, [r0, #S5PC1XX_GPIO_PULL_OFFSET]

	ldr	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]
	orr	r1, r1, #(1 << 7)		@ 7 = 7 * 1-bit
	str	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]
200:
	mov	pc, lr

/*
 * tzpc_asm_init: Initialize TZPC
 */
tzpc_asm_init:
	cmp	r7, r8
	ldreq	r0, =0xE3800000			@ TZPC0
	ldrne	r0, =0xF1500000			@ TZPC0
	mov	r1, #0x0
	str	r1, [r0]
	mov	r1, #0xff
	str	r1, [r0, #0x804]
	str	r1, [r0, #0x810]
	str	r1, [r0, #0x81C]

	ldreq	r0, =0xE2800000			@ TZPC1
	ldrne	r0, =0xFAD00000			@ TZPC1
	str	r1, [r0, #0x804]
	str	r1, [r0, #0x810]
	str	r1, [r0, #0x81C]

	ldreq	r0, =0xE2900000			@ TZPC2
	ldrne	r0, =0xE0600000			@ TZPC2
	str	r1, [r0, #0x804]
	str	r1, [r0, #0x810]
	str	r1, [r0, #0x81C]
	str	r1, [r0, #0x828]

	ldrne	r0, =0xE1C00000			@ TZPC3 S5PC110 only
	strne	r1, [r0, #0x804]
	strne	r1, [r0, #0x810]
	strne	r1, [r0, #0x81C]

	mov	pc, lr
#endif
