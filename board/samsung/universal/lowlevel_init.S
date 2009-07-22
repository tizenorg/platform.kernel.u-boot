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

#ifdef CONFIG_SERIAL0
#define UART_CONSOLE_BASE	UARTx_OFFSET(0)
#elif defined(CONFIG_SERIAL1)
#define UART_CONSOLE_BASE	UARTx_OFFSET(1)
#elif defined(CONFIG_SERIAL2)
#define UART_CONSOLE_BASE	UARTx_OFFSET(2)
#else
#define UART_CONSOLE_BASE	UARTx_OFFSET(3)
#endif

/*
 * Register usages:
 *
 * r5 has zero always
 */

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r9, lr

	/* Turn on KEY_LED_ON [GPJ4(1)] */
	ldr	r0, =S5PC100_GPIO_BASE(S5PC100_GPIO_J4_OFFSET)
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
	ldr	r0, =S5P_OTHERS				@0xE0108200
	ldr	r1, [r0]
	ldr	r2, =(1 << 31)				@IO_RET_REL
	orr	r1, r1, r2
	str	r1, [r0]

	/* Disable Watchdog */
	ldr	r0, =S5P_WATCHDOG_BASE(0x0)		@0xEA200000
	orr	r0, r0, #0x0
	mov	r5, #0
	str	r5, [r0]

#ifndef CONFIG_ONENAND_IPL
	/* setting SRAM */
	ldr	r0, =S5P_SROMC_BASE(0x0)
	ldr	r1, =0x9
	str	r1, [r0]
#endif

	/* S5PC100 has 3 groups of interrupt sources */
	ldr	r0, =S5P_VIC0_BASE(0x0)			@0xE4000000
	ldr	r1, =S5P_VIC1_BASE(0x0)			@0xE4000000
	ldr	r2, =S5P_VIC2_BASE(0x0)			@0xE4000000

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

	/* for DMA */
	bl	dma_asm_init
#endif

#ifdef CONFIG_ONENAND_IPL
	/* init system clock */
	bl	system_clock_init

	bl	mem_ctrl_asm_init

	/* Wakeup support. Don't know if it's going to be used, untested. */
	ldr	r0, =S5P_RST_STAT
	ldr	r1, [r0]
	bic	r1, r1, #0xfffffff7
	cmp	r1, #0x8
	beq	wakeup_reset
#endif

1:
	mov	lr, r9
	mov	pc, lr

#ifdef CONFIG_ONENAND_IPL
wakeup_reset:

	/* Turn on 7color [GPJ4(2)] at universal */
	ldr	r0, =S5PC100_GPIO_BASE(S5PC100_GPIO_J4_OFFSET)
	ldr	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]
	bic	r1, r1, #(0xf << 8)			@ 2 * 4-bit
	orr	r1, r1, #(0x1 << 8)
	str	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]

	ldr	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]
	orr	r1, r1, #(1 << 2)			@ 2 * 1-bit
	str	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]

	/* Clear wakeup status register */
	ldr	r0, =S5P_WAKEUP_STAT
	ldr	r1, [r0]
	str	r1, [r0]

	/* Load return address and jump to kernel */
	ldr	r0, =S5P_INFORM0

	/* r1 = physical address of s5pc100_cpu_resume function */
	ldr	r1, [r0]

	/* Jump to kernel (sleep-s5pc100.S) */
	mov	pc, r1
	nop
	nop
#endif

/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:
	ldr	r8, =S5P_PA_CLK			@ 0xE0100000

	/* Set Lock Time */
	ldr	r1, =0xe10			@ Locktime : 0xe10 = 3600
	str	r1, [r8, #0x000]		@ S5P_APLL_LOCK
	str	r1, [r8, #0x004]		@ S5P_MPLL_LOCK
	str	r1, [r8, #0x008]		@ S5P_EPLL_LOCK
	str	r1, [r8, #0x00C]		@ S5P_HPLL_LOCK

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
	str	r1, [r8, #0x100]
	/* S5P_MPLL_CON */
	ldr	r1, =0x80590201		@ SDIV 1, PDIV 2, MDIV 89 (267MHz)
	str	r1, [r8, #0x104]
	/* S5P_EPLL_CON */
	ldr	r1, =0x80870303		@ SDIV 3, PDIV 3, MDIV 135 (67.5MHz)
	str	r1, [r8, #0x108]
	/* S5P_HPLL_CON */
	ldr	r1, =0x80600603		@ SDIV 3, PDIV 6, MDIV 96
	str	r1, [r8, #0x10C]

	/* Set Clock divider */
	ldr	r1, [r8, #0x300]
	ldr	r2, =0x00003fff
	bic	r1, r1, r2
#ifdef CONFIG_CLK_800_166_66
	ldr	r2, =0x00011401
#elif defined(CONFIG_CLK_500_166_66)
	ldr	r2, =0x00011201
#elif defined(CONFIG_CLK_666_166_66)
	ldr	r2, =0x00011300
#else
	ldr	r2, =0x00011301
#endif
	orr	r1, r1, r2
	str	r1, [r8, #0x300]
	ldr	r1, [r8, #0x304]
	ldr	r2, =0x00011110
	orr	r1, r1, r2
	str	r1, [r8, #0x304]
	ldr	r1, =0x00000001
	str	r1, [r8, #0x308]

	/* Set Source Clock */
	ldr	r1, =0x1111			@ A, M, E, HPLL Muxing
	str	r1, [r8, #0x200]		@ S5P_CLK_SRC0

#if 0
	ldr	r1, =0x1000001			@ Uart Clock & CLK48M Muxing
	str	r1, [r8, #0x204]		@ S5P_CLK_SRC1

	ldr	r1, =0x9000			@ ARMCLK/4
	str	r1, [r8, #0x400]		@ S5P_CLK_OUT
#endif

	/* wait at least 200us to stablize all clock */
	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b

	mov	pc, lr

#ifndef CONFIG_ONENAND_IPL
/*
 * uart_asm_init: Initialize UART's pins
 */
uart_asm_init:
	/* set GPIO to enable UART0-UART4 */
	ldr	r0, =S5PC100_GPIO_BASE(0)
	ldr	r1, =0x22222222
	str	r1, [r0, #S5PC100_GPIO_A0_OFFSET]		@ GPA0_CON
	ldr	r1, =0x00002222
	str	r1, [r0, #S5PC100_GPIO_A1_OFFSET]		@ GPA1_CON

	/* UART_SEL GPK0[5] at S5PC100 */
	ldr	r0, =S5PC100_GPIO_BASE(S5PC100_GPIO_K0_OFFSET)
	ldr	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]
	bic	r1, r1, #(0xf << 20)		@ 20 = 5 * 4-bit
	orr	r1, r1, #(0x1 << 20)		@ Output
	str   	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]

	ldr	r1, [r0, #S5PC1XX_GPIO_PULL_OFFSET]
	bic	r1, r1, #(0x3 << 10)		@ 10 = 5 * 2-bit
	orr	r1, r1, #(0x2 << 10)		@ Pull-up enabled
	str	r1, [r0, #S5PC1XX_GPIO_PULL_OFFSET]

	ldr	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]
	orr	r1, r1, #(1 << 5)		@ 5 = 5 * 1-bit
	str	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]

	/*
	 * Note that the following address
	 * 0xE020'0360 is reserved address at S5PC100
	 */
	/* UART_SEL MP0_5[7] at S5PC110 */
	ldr	r0, =S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_5_OFFSET)
	ldr	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]
	bic	r1, r1, #(0xf << 28)		@ 28 = 7 * 4-bit
	orr	r1, r1, #(0x1 << 28)		@ Output
	str   	r1, [r0, #S5PC1XX_GPIO_CON_OFFSET]

	ldr	r1, [r0, #S5PC1XX_GPIO_PULL_OFFSET]
	bic	r1, r1, #(0x3 << 14)		@ 14 = 7 * 2-bit
	orr	r1, r1, #(0x2 << 14)		@ Pull-up enabled
	str	r1, [r0, #S5PC1XX_GPIO_PULL_OFFSET]

	ldr	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]
	orr	r1, r1, #(1 << 7)		@ 7 = 7 * 1-bit
	str	r1, [r0, #S5PC1XX_GPIO_DAT_OFFSET]

	mov	pc, lr

/*
 * dma_asm_init: Initialize DMA
 */
dma_asm_init:
	ldr     r0, =0xE3800000
	mov	r1, #0x0
	str	r1, [r0]
	mov	r1, #0xff
	str	r1, [r0, #0x804]
	str	r1, [r0, #0x810]

	ldr	r0, =0xE2800000
	str	r1, [r0, #0x804]
	str	r1, [r0, #0x810]
	str	r1, [r0, #0x81C]

	ldr	r0, =0xE2900000
	str	r1, [r0, #0x804]
	str	r1, [r0, #0x810]

	mov	pc, lr

#endif
