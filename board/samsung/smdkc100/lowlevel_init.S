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

	/* init system clock */
	bl system_clock_init

#ifndef CONFIG_ONENAND_IPL
	/* for UART */
	bl uart_asm_init
#endif

	/* Memory subsystem address 0xe0200200 */
	ldr	r0, =S5P_MEM_SYS_CFG
	str	r5, [r0]

	/* DRAM I/O Drive-Strength */
	ldr	r0, =S5P_MP_0DRV
	ldr	r1, =0x5555
	str	r1, [r0, #S5P_MP_0_OFFSET]
	str	r1, [r0, #S5P_MP_1_OFFSET]
	str	r1, [r0, #S5P_MP_2_OFFSET]
	str	r1, [r0, #S5P_MP_3_OFFSET]
	str	r1, [r0, #S5P_MP_4_OFFSET]
	str	r1, [r0, #S5P_MP_5_OFFSET]
	str	r1, [r0, #S5P_MP_6_OFFSET]
	str	r1, [r0, #S5P_MP_7_OFFSET]

#ifdef CONFIG_ONENAND_IPL
	bl	mem_ctrl_asm_init

	/* Wakeup support. Don't know if it's going to be used, untested. */
	ldr	r0, =S5P_OTHERS_BASE
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

	/* Set Clock divider */
	ldr	r1, =0x00011110
	str	r1, [r8, #0x304]
	ldr	r1, =0x1
	str	r1, [r8, #0x308]
	ldr	r1, =0x00011301
	str	r1, [r8, #0x300]

	/* Set Lock Time */
	ldr	r1, =0xe10			@ Locktime : 0xe10 = 3600
	str	r1, [r8, #0x000]		@ S5P_APLL_LOCK
	str	r1, [r8, #0x004]		@ S5P_MPLL_LOCK
	str	r1, [r8, #0x008]		@ S5P_EPLL_LOCK

	/* S5P_APLL_CON */
	ldr	r1, =0x81bc0400		@ SDIV 0, PDIV 4, MDIV 444 (1332MHz)
	str	r1, [r8, #0x100]
	/* S5P_MPLL_CON */
	ldr	r1, =0x80590201		@ SDIV 1, PDIV 2, MDIV 89 (267MHz)
	str	r1, [r8, #0x104]
	/* S5P_EPLL_CON */
	ldr	r1, =0x80870303		@ SDIV 3, PDIV 3, MDIV 135 (67.5MHz)
	str	r1, [r8, #0x108]
	/* S5P_HPLL_CON */
	ldr	r1, =0x80600603
	str	r1, [r8, #0x10C]

	/* Set Source Clock */
	ldr	r1, =0x1111			@ A, M, E, HPLL Muxing
	str	r1, [r8, #0x200]		@ S5P_CLK_SRC0

	ldr	r1, =0x1000001			@ Uart Clock & CLK48M Muxing
	str	r1, [r8, #0x204]		@ S5P_CLK_SRC1

	ldr	r1, =0x9000			@ ARMCLK/4
	str	r1, [r8, #0x400]		@ S5P_CLK_OUT

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
	/* set GPIO to enable UART */
	ldr	r0, =S5P_GPIO_A0_CON
	ldr	r1, =0x22222222
	str	r1, [r0]

	/* uart_sel GPK0[5] */
	ldr	r0, =S5P_GPIO_A1_CON
	ldr	r1, =0x2222
	ldr	r1, [r0]

	mov	pc, lr
#endif
