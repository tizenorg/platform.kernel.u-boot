/*
 * (C) Copyright 2005-2008 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
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

#ifndef _ONENAND_IPL_H
#define _ONENAND_IPL_H

#include <linux/mtd/onenand_regs.h>

#define onenand_readw(a)        readw(THIS_ONENAND(a))
#define onenand_writew(v, a)    writew(v, THIS_ONENAND(a))

#define THIS_ONENAND(a)         (CONFIG_SYS_ONENAND_BASE + (a))

#define READ_INTERRUPT()                                                \
				onenand_readw(ONENAND_REG_INTERRUPT)

/* S5PC100 specific */
#define S5PC100_AHB_ADDR		0xB0000000
#define MEM_ADDR(fba, fpa, fsa)		((fba) << 13 | (fpa) << 7 | (fsa) << 5)
#define CMD_MAP_01(mem_addr) 	(S5PC100_AHB_ADDR | (1 << 26) | (mem_addr))
#define CMD_MAP_11(addr)	(S5PC100_AHB_ADDR | (3 << 26) | ((addr) << 2))
#define onenand_ahb_readw(a)	(readl(CMD_MAP_11((a) >> 1)) & 0xffff)

extern int onenand_read_block(unsigned char *buf);
#endif
