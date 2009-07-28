/*
 * (C) Copyright 2005-2008 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * Derived from x-loader
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

#include <common.h>
#include <version.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>

#include "onenand_ipl.h"

typedef int (init_fnc_t)(void);

void start_oneboot(void)
{
	uchar *buf;
	unsigned int value;

	buf = (uchar *) CONFIG_SYS_LOAD_ADDR;

#if 0
	value = readl(0xE0200000 + S5PC110_GPIO_J4_OFFSET + S5PC1XX_GPIO_DAT_OFFSET);
	value |= (1 << 4);
	writel(value, 0xE0200000 + S5PC110_GPIO_J4_OFFSET + S5PC1XX_GPIO_DAT_OFFSET);
#endif
	onenand_read_block(buf);


	((init_fnc_t *)CONFIG_SYS_LOAD_ADDR)();

	/* should never come here */
}

void hang(void)
{
       for (;;);
}
