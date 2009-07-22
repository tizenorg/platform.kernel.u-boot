/*
 * Copyright (c) 2009 Samsung Electronics.
 * Minkyu Kang <mk7.kang@samsung.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/arch/cpu.h>

.globl reset_cpu
reset_cpu:
	ldr	r1, =S5P_PRO_ID
	ldr	r2, [r1]
	ldr	r3, =0xFFF
	and	r2, r3, r2, lsr #12
	ldr	r3, =0xC000
	orr	r2, r3, r2
	ldr	r1, =S5P_SW_RST
	str	r2, [r1]
_loop_forever:
	b	_loop_forever
