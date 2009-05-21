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

#include <s5pc100.h>

#ifdef CONFIG_SERIAL0
#define ELFIN_UART_CONSOLE_BASE	ELFIN_UARTx_OFFSET(0)
#elif defined(CONFIG_SERIAL1)
#define ELFIN_UART_CONSOLE_BASE ELFIN_UARTx_OFFSET(1)
#elif defined(CONFIG_SERIAL2)
#define ELFIN_UART_CONSOLE_BASE ELFIN_UARTx_OFFSET(2)
#else
#define ELFIN_UART_CONSOLE_BASE ELFIN_UARTx_OFFSET(3)
#endif

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r12, lr

	/* Disable Watchdog */
	ldr	r0, =0xEA200000		@0xEA200000
	orr	r0, r0, #0x0
	mov	r1, #0
	str	r1, [r0]

	/* setting SRAM */
	ldr r0, =0xe7000000
	ldr r1, =0x9
	str r1, [r0]

	/* External interrupt pending clear : GPIO_BASE = 0xE0300000 */
	ldr	r0, =(ELFIN_GPIO_BASE+EINTPEND_OFFSET)	/*EINTPEND*/
	add r4, r0, #0x54
interrupt_pending_loop:
	ldr	r1, [r0]
	str	r1, [r0]

	/* It compares to end offset value of external interrupt pending register */
	cmp r0, r4
	add r0, r0, #0x04
	bcc interrupt_pending_loop

	/* s5pc100 has 3 groups of interrupt sources */
	ldr	r0, =ELFIN_VIC0_BASE_ADDR	@0xE4000000
	ldr	r1, =ELFIN_VIC1_BASE_ADDR	@0xE4100000
	ldr	r2, =ELFIN_VIC2_BASE_ADDR	@0xE4200000

	/* Disable all interrupts (VIC0, VIC1 and VIC2) */
	mvn	r3, #0x0
	str	r3, [r0, #oINTMSK]
	str	r3, [r1, #oINTMSK]
	str	r3, [r2, #oINTMSK]

#ifndef CONFIG_ONENAND_IPL
	/* Set all interrupts as IRQ */
	mov	r3, #0x0
	str	r3, [r0, #oINTMOD]
	str	r3, [r1, #oINTMOD]
	str	r3, [r2, #oINTMOD]

	/* Pending Interrupt Clear */
	mov	r3, #0x0
	str	r3, [r0, #oVECTADDR]
	str	r3, [r1, #oVECTADDR]
	str	r3, [r2, #oVECTADDR]
#endif

	/* init system clock */
	bl system_clock_init

#ifndef CONFIG_NAND_SPL
	/* for UART */
	bl uart_asm_init
#endif

#ifdef CONFIG_BOOT_NAND
	/* simple init for NAND */
	bl nand_asm_init
#endif

	/* Memory subsystem address 0xe0200200 */
	ldr	r0, =ELFIN_MEM_SYS_CFG

	/* */
	mov	r1, #0
	str	r1, [r0]

#	bl	mem_ctrl_asm_init

/* Wakeup support. Don't know if it's going to be used, untested. */
#	ldr	r0, =(OTHERS_REGISTER_BASE)
#	ldr	r1, [r0]
#	bic	r1, r1, #0xfffffff7
#	cmp	r1, #0x8
#	beq	wakeup_reset

	/* DRAM I/O Drive-Strength */
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x5555
	str	r1, [r0, #MP_0DRV_OFFSET]
	str	r1, [r0, #MP_1DRV_OFFSET]
	str	r1, [r0, #MP_2DRV_OFFSET]
	str	r1, [r0, #MP_3DRV_OFFSET]
	str	r1, [r0, #MP_4DRV_OFFSET]
	str	r1, [r0, #MP_5DRV_OFFSET]
	str	r1, [r0, #MP_6DRV_OFFSET]
	str	r1, [r0, #MP_7DRV_OFFSET]
1:
	mov	lr, r12
	mov	pc, lr

wakeup_reset:

	/* Clear wakeup status register */
	ldr	r0, =(CLOCK_REGISTER_BASE + WAKEUP_STAT_OFFSET)
	ldr	r1, [r0]
	str	r1, [r0]

	/* Load return address and jump to kernel */
	ldr	r0, =(CLOCK_REGISTER_BASE + INF_REG0_OFFSET)
	/* r1 = physical address of s3c6400_cpu_resume function */
	ldr	r1, [r0]
	/* Jump to kernel (sleep-s3c6400.S) */
	mov	pc, r1
	nop
	nop
/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:
	ldr	r0, =CLOCK_REGISTER_BASE	/* 0xE0100000 */

#ifdef CONFIG_SYNC_MODE
	ldr	r1, [r0, #OTHERS_OFFSET]
	mov	r2, #0x40
	orr	r1, r1, r2
	str	r1, [r0, #OTHERS_OFFSET]

	nop
	nop
	nop
	nop
	nop

	ldr	r2, =0x80
	orr	r1, r1, r2
	str	r1, [r0, #OTHERS_OFFSET]

check_syncack:
	ldr	r1, [r0, #OTHERS_OFFSET]
	ldr	r2, =0xf00
	and	r1, r1, r2
	cmp	r1, #0xf00
	bne	check_syncack
#else	/* ASYNC Mode */
	nop
	nop
	nop
	nop
	nop

	/*
	 * This was unconditional in original Samsung sources, but it doesn't
	 * seem to make much sense on S3C6400.
	 */
#ifndef CONFIG_S5PC100
	ldr	r1, [r0, #OTHERS_OFFSET]
	bic	r1, r1, #0xC0
	orr	r1, r1, #0x40
	str	r1, [r0, #OTHERS_OFFSET]

wait_for_async:
	ldr	r1, [r0, #OTHERS_OFFSET]
	and	r1, r1, #0xf00
	cmp	r1, #0x0
	bne	wait_for_async
#endif

	/* clock initialization */
	ldr r0, =0xe0100304
	ldr r1, =0x10000
	str r1, [r0]

	ldr r1, =0x11000
	str r1, [r0]

	ldr r1, =0x11100
	str r1, [r0]

	ldr r1, =0x11110
	str r1, [r0]

	ldr r1, =0x11110
	str r1, [r0]

	ldr r0, =0xe0100308
	ldr r1, =0x1
	str r1, [r0]

	/* APLL Enable */
	ldr r0, =0xe1000000
	ldr r1, =0xe10
	str r1, [r0]

	ldr r0, =0xe0100100
	ldr r1, =0x81bc0400
	str r1, [r0]

ldr r3, =loop1
	b dummy_loop
loop1:
	/* MPLL Enable */
	ldr r0, =0xe0100004
	ldr r1, =0xe10
	str r1, [r0]

	ldr r0, =0xe0100104
	ldr r1, =0x80590201
	str r1, [r0]

ldr r3, =loop2
	b dummy_loop
loop2:
	/* EPLL Enable */
	ldr r0, =0xe0100008
	ldr r1, =0xe10
	str r1, [r0]

	ldr r0, =0xe0100108
	ldr r1, =0x80870303
	str r1, [r0]

ldr r3, =loop3
	b dummy_loop
loop3:
	/* HPLL Enable */
	ldr r0, =0xe0100200
	ldr r1, =0x0
	str r1, [r0]

	ldr r0, =0xe010000c
	ldr r1, =0xe10
	str r1, [r0]

	ldr r0, =0xe010010c
	ldr r1, =0x80600603
	str r1, [r0]

ldr r3, =loop4
	b dummy_loop
loop4:
	/* Set Source Clock */
	ldr r0, =0xe0100200
	ldr r1, =0x0
	str r1, [r0]

	ldr r1, =0x1
	str r1, [r0]

	ldr r1, =0x11
	str r1, [r0]

	ldr r1, =0x111
	str r1, [r0]

	ldr r1, =0x1111
	str r1, [r0]

	ldr r0, =0xe0100204
	ldr r1, =0x1
	str r1, [r0]

	ldr r1, =0x1000001
	str r1, [r0]

	ldr r0, =0xe0100400
	ldr r1, =0x9000
	str r1, [r0]

	b setting_end
	
	/* wait at least 1000us to stablize all clock */
dummy_loop:
	mov	r2, #0x2000
1:	subs	r2, r2, #1
	bne	1b
	mov pc, r3

setting_end:
#endif

#	/* wait at least 200us to stablize all clock */
#	mov	r2, #0x10000
#1:	subs	r2, r2, #1
#	bne	1b

	/* Synchronization for VIC port */
#if defined(CONFIG_SYNC_MODE)
	ldr	r1, [r0, #OTHERS_OFFSET]
	orr	r1, r1, #0x20
	str	r1, [r0, #OTHERS_OFFSET]
#elif !defined(CONFIG_S5PC100)
	/* According to 661558um_S3C6400X_rev10.pdf 0x20 is reserved */
	ldr	r1, [r0, #OTHERS_OFFSET]
	bic	r1, r1, #0x20
	str	r1, [r0, #OTHERS_OFFSET]
#endif
	mov	pc, lr

/*
 * uart_asm_init: Initialize UART's pins
 */
uart_asm_init:
	/* set GPIO to enable UART */
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x22
	str	r1, [r0, #GPA1CON_OFFSET]

	/* uart_sel GPK0[5] */
	ldr	r1, [r0, #GPK0CON_OFFSET]
	bic	r1, r1, #0xf00000
	orr	r1, r1, #0x100000
	str   	r1, [r0, #GPK0CON_OFFSET]

	ldr	r1, [r0, #GPK0PUD_OFFSET]
	bic	r1, r1, #0xc00
	orr	r1, r1, #0x800
	str	r1, [r0, #GPK0PUD_OFFSET]

	ldr	r1, [r0, #GPK0DAT_OFFSET]
	orr	r1, r1, #0x20
	str	r1, [r0, #GPK0DAT_OFFSET]

	mov	pc, lr

#ifdef CONFIG_BOOT_NAND
/*
 * NAND Interface init for SMDK6400
 */
nand_asm_init:
	ldr	r0, =ELFIN_NAND_BASE
	ldr	r1, [r0, #NFCONF_OFFSET]
	orr	r1, r1, #0x70
	orr	r1, r1, #0x7700
	str	r1, [r0, #NFCONF_OFFSET]

	ldr	r1, [r0, #NFCONT_OFFSET]
	orr	r1, r1, #0x07
	str	r1, [r0, #NFCONT_OFFSET]

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
