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

	/* init system clock */
	bl	system_clock_init

	/* UART */
	bl	uart_asm_init

	mov	lr, r11
	mov	pc, lr

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

#if 1
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

system_clock_init:
	ldr	r0, =S5PC210_CLOCK_BASE

	ldr	r1, =0x6666666
	ldr	r2, =0x0C250			@ CLK_SRC_PERIL0_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =0x777777
	ldr	r2, =0x0C550			@ CLK_DIV_PERIL0_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =0x8
	ldr	r2, =0x0C55C			@ CLK_DIV_PERIL3_OFFSET
	str	r1, [r0, r2]

	mov	pc, lr
