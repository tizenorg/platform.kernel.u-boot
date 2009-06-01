/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *		       Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Modified for the Samsung SMDK2410 by
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
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

#include <s5pc1xx.h>

#ifdef CONFIG_SERIAL0
#define UART_CONSOLE_BASE	UARTx_OFFSET(0)
#elif defined(CONFIG_SERIAL1)
#define UART_CONSOLE_BASE	UARTx_OFFSET(1)
#elif defined(CONFIG_SERIAL2)
#define UART_CONSOLE_BASE	UARTx_OFFSET(2)
#else
#define UART_CONSOLE_BASE	UARTx_OFFSET(3)
#endif

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r9, lr

	/* Disable Watchdog */
	ldr	r0, =S5P_WATCHDOG_BASE(0x0)		@0xEA200000
	orr	r0, r0, #0x0
	mov	r1, #0
	str	r1, [r0]

	/* setting SRAM */
	ldr r0, =S5P_SROMC_BASE(0x0)
	ldr r1, =0x9
	str r1, [r0]

#ifndef CONFIG_ONENAND_IPL
	/* External interrupt pending clear : GPIO_BASE = 0xE0300000 */
	ldr	r0, =S5P_GPIO_INT_PEND_REG(0x0)	/*EINTPEND*/
	add r4, r0, #0x54

interrupt_pending_loop:
	ldr	r1, [r0]
	str	r1, [r0]

	/* It compares to end offset value of external interrupt pending register */
	cmp r0, r4
	add r0, r0, #0x04
	bcc interrupt_pending_loop

	/* s5pc100 has 3 groups of interrupt sources */
	ldr	r0, =S5P_VIC0_BASE(0x0)		@0xE4000000
	ldr	r1, =S5P_VIC1_BASE(0x0)		@0xE4000000
	ldr	r2, =S5P_VIC2_BASE(0x0)		@0xE4000000

	/* Disable all interrupts (VIC0, VIC1 and VIC2) */
	mvn	r3, #0x0
	str	r3, [r0, #VIC_INTENCLEAR_OFFSET]
	str	r3, [r1, #VIC_INTENCLEAR_OFFSET]
	str	r3, [r2, #VIC_INTENCLEAR_OFFSET]

	/* Set all interrupts as IRQ */
	mov	r3, #0x0
	str	r3, [r0, #VIC_INTSELECT_OFFSET]
	str	r3, [r1, #VIC_INTSELECT_OFFSET]
	str	r3, [r2, #VIC_INTSELECT_OFFSET]

	/* Pending Interrupt Clear */
	mov	r3, #0x0
	str	r3, [r0, #VIC_INTADDRESS_OFFSET]
	str	r3, [r1, #VIC_INTADDRESS_OFFSET]
	str	r3, [r2, #VIC_INTADDRESS_OFFSET]
#endif

	/* init system clock */
	bl system_clock_init

#ifndef CONFIG_ONENAND_IPL
	/* for UART */
	bl uart_asm_init
#endif

#ifdef CONFIG_ONENAND_IPL
	/* Memory subsystem address 0xe0200200 */
	ldr	r0, =S5P_MEM_SYS_CFG

	mov	r1, #0
	str	r1, [r0]

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
#endif
	bl	mem_ctrl_asm_init

#ifndef CONFIG_ONENAND_IPL
	/* Wakeup support. Don't know if it's going to be used, untested. */
	ldr	r0, =(S5P_OTHERS_BASE)
	ldr	r1, [r0]
	bic	r1, r1, #0xfffffff7
	cmp	r1, #0x8
	beq	wakeup_reset
#endif

1:
	mov	lr, r9
	mov	pc, lr

#ifndef CONFIG_ONENAND_IPL
wakeup_reset:

	/* Clear wakeup status register */
	ldr	r0, =S5P_WAKEUP_STAT
	ldr	r1, [r0]
	str	r1, [r0]

	/* Load return address and jump to kernel */
	ldr	r0, =S5P_INFORM0

	/* r1 = physical address of s3c6400_cpu_resume function */
	ldr	r1, [r0]

	/* Jump to kernel (sleep-s3c6400.S) */
	mov	pc, r1
	nop
	nop
#endif
/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:
#ifndef CONFIG_ONENAND_IPL
	nop
	nop
	nop
	nop
	nop
#endif

	/*
	 * Fout = MDIV * Fin / (PDIV * (2 ^ SDIV))
	 */

	/* Clock Divider Set */
	ldr r0, =S5P_CLK_DIV1
	ldr r1, =0x11110	@ MPLL, MPLL2, D1_BUS, PCLKD1 ratio set 1 -> 2
	str r1, [r0]

	ldr r0, =S5P_CLK_DIV2
	ldr r1, =0x1		@ UART ratio set 1 -> 2
	str r1, [r0]

	ldr r0, =S5P_CLK_DIV0
	ldr r1, =0x11301
	str r1, [r0]

	/* APLL Enable */
	ldr r0, =S5P_APLL_LOCK	
	ldr r1, =0xe10		@ Locktime : 0xe10 = 3600
	str r1, [r0]

	ldr r0, =S5P_APLL_CON
	ldr r1, =0x81bc0400		@ SDIV 0, PDIV 4, MDIV 444 (1332MHz)
#	ldr r1, =0x81f40302		@ SDIV 2, PDIV 3, MDIV 500 (500MHz)
	str r1, [r0]

	ldr r3, =loop1
	b dummy_loop

loop1:
	/* MPLL Enable */
	ldr r0, =S5P_MPLL_LOCK
	ldr r1, =0xe10			@ Locktime : 0x310 = 3600
	str r1, [r0]

	ldr r0, =S5P_MPLL_CON
	ldr r1, =0x80590201		@ SDIV 1, PDIV 2, MDIV 89 (267MHz)
	str r1, [r0]

	ldr r3, =loop2
	b dummy_loop

loop2:
	/* EPLL Enable */
	ldr r0, =S5P_EPLL_LOCK
	ldr r1, =0xe10			@ Locktime : 0x310 = 3600
	str r1, [r0]

	ldr r0, =S5P_EPLL_CON
	ldr r1, =0x80870303		@ SDIV 3, PDIV 3, MDIV 135 (67.5MHz)
	str r1, [r0]

	ldr r3, =loop3
	b dummy_loop

loop3:
	/* HPLL Enable */
	ldr r0, =S5P_CLK_SRC0
	ldr r1, =0x0
	str r1, [r0]

	ldr r0, =S5P_HPLL_LOCK
	ldr r1, =0xe10
	str r1, [r0]

	ldr r0, =S5P_HPLL_CON
	ldr r1, =0x80600603		@ SDIV 3, PDIV 6, MDIV 96 (54MHZ)
	str r1, [r0]

	ldr r3, =loop4
	b dummy_loop

loop4:
	/* Set Source Clock */
	ldr r0, =S5P_CLK_SRC0
	ldr r1, =0x1111			@ A, M, E, HPLL Muxing
	str r1, [r0]

	ldr r0, =S5P_CLK_SRC1
	ldr r1, =0x1000001		@ Uart Clock & CLK48M Muxing
	str r1, [r0]

	ldr r0, =S5P_CLK_OUT
	ldr r1, =0x9000			@ ARMCLK/4
	str r1, [r0]

	b setting_end
	
	/* wait at least 1000us to stablize all clock */
dummy_loop:
	mov	r2, #0x2000
1:	subs	r2, r2, #1
	bne	1b
	mov pc, r3

setting_end:
	/* wait at least 200us to stablize all clock */
	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b

	/* Synchronization for VIC port */
#if defined(CONFIG_SYNC_MODE)
	ldr	r1, [r0]
	orr	r1, r1, #0x20
	str	r1, [r0]
#elif !defined(CONFIG_S5PC100)
	/* According to 661558um_S3C6400X_rev10.pdf 0x20 is reserved */
	ldr	r1, [r0]
	bic	r1, r1, #0x20
	str	r1, [r0]
#endif
	mov	pc, lr

#ifndef CONFIG_ONENAND_IPL
/*
 * uart_asm_init: Initialize UART's pins
 */
uart_asm_init:
	/* set GPIO to enable UART */
	ldr	r0, =S5P_GPIO_A1_CON
	ldr	r1, =0x22
	str	r1, [r0]

	/* uart_sel GPK0[5] */
	ldr r0, =S5P_GPIO_K0_CON
	ldr	r1, [r0]
	bic	r1, r1, #0xf00000
	orr	r1, r1, #0x100000
	str   	r1, [r0]

	ldr r0, =S5P_GPIO_K0_PULL
	ldr	r1, [r0]
	bic	r1, r1, #0xc00
	orr	r1, r1, #0x800
	str	r1, [r0]

	ldr r0, =S5P_GPIO_K0_DAT
	ldr	r1, [r0]
	orr	r1, r1, #0x20
	str	r1, [r0]

	mov	pc, lr
#endif

#ifdef CONFIG_ENABLE_MMU
/*
 * MMU Table for SMDK6400
 */

	/* form a first-level section entry */
.macro FL_SECTION_ENTRY base,ap,d,c,b
	.word (\base << 20) | (\ap << 10) | \
	      (\d << 5) | (1<<4) | (\c << 3) | (\b << 2) | (1<<1)
.endm

.section .mmudata, "a"
	.align 14
	/* the following alignment creates the mmu table at address 0x4000. */
	.globl mmu_table
mmu_table:
	.set __base, 0
	/* 1:1 mapping for debugging */
	.rept 0xA00
	FL_SECTION_ENTRY __base, 3, 0, 0, 0
	.set __base, __base + 1
	.endr

	/* access is not allowed. */
	.rept 0xC00 - 0xA00
	.word 0x00000000
	.endr

	/* 128MB for SDRAM 0xC0000000 -> 0x50000000 */
	.set __base, 0x500
	.rept 0xC80 - 0xC00
	FL_SECTION_ENTRY __base, 3, 0, 1, 1
	.set __base, __base + 1
	.endr

	/* access is not allowed. */
	.rept 0x1000 - 0xc80
	.word 0x00000000
	.endr
#endif
