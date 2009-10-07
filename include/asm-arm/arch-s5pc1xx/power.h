/*
 * Copyright (c) 2009 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
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
 *
 */

#ifndef __ASM_ARM_ARCH_POWER_H_
#define __ASM_ARM_ARCH_POWER_H_

/*
 * Power control
 */
#define S5PC100_OTHERS			0xE0108200
#define S5PC100_RST_STAT		0xE0108300
#define S5PC100_SLEEP_WAKEUP		(1 << 3)
#define S5PC100_WAKEUP_STAT		0xE0108304
#define S5PC100_INFORM0			0xE0108400

#define S5PC110_RST_STAT		0xE010A000
#define S5PC110_SLEEP_WAKEUP		(1 << 16)
#define S5PC110_PWR_CFG			0xE010C000
#define S5PC110_CFG_STANDBYWFI_MASK	(0x3 << 8)
#define S5PC110_CFG_STANDBYWFI_IDLE	(0x1 << 8)
#define S5PC110_CFG_STANDBYWFI_STOP	(0x2 << 8)
#define S5PC110_CFG_STANDBYWFI_SLEEP	(0x3 << 8)
#define S5PC110_EINT_WAKEUP_MASK	0xE010C004
#define S5PC110_WAKEUP_MASK		0xE010C008
#define S5PC110_SLEEP_CFG		0xE010C040
#define S5PC110_WAKEUP_STAT		0xE010C200
#define S5PC110_OTHERS			0xE010E000
#define S5PC110_OTHERS_SYSCON_INT_DISABLE	(1 << 0)
#define S5PC110_USB_PHY_CON		0xE010E80C
#define S5PC110_INFORM0			0xE010F000

#endif
