/*
 * (C) Copyright 2005-2008 Samsung Electronis
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

#include <common.h>

#include <asm/io.h>
#include <asm/string.h>

#include "onenand_ipl.h"

#define onenand_block_address(block)		(block)
#define onenand_sector_address(page)		(page << 2)
#define onenand_buffer_address()		((1 << 3) << 8)
#define onenand_bufferram_address(block)	(0)

#ifdef __HAVE_ARCH_MEMCPY32
extern void *memcpy32(void *dest, void *src, int size);
#endif

static int (*onenand_read_page)(ulong block, ulong page,
				u_char *buf, int pagesize);

static int s5pc100_onenand_read_page(ulong block, ulong page,
				u_char * buf, int pagesize)
{
	unsigned int *p = (unsigned int *) buf;
	int mem_addr, i;

	mem_addr = MEM_ADDR(block, page, 0);

	pagesize >>= 2;

	for (i = 0; i < pagesize; i++)
		*p++ = readl(CMD_MAP_01(mem_addr));

	return 0;
}

/* read a page with ECC */
static int generic_onenand_read_page(ulong block, ulong page,
				u_char * buf, int pagesize)
{
	unsigned long *base;

#ifndef __HAVE_ARCH_MEMCPY32
	unsigned int offset, value;
	unsigned long *p;
#endif

	onenand_writew(onenand_block_address(block),
			ONENAND_REG_START_ADDRESS1);

	onenand_writew(onenand_bufferram_address(block),
			ONENAND_REG_START_ADDRESS2);

	onenand_writew(onenand_sector_address(page),
			ONENAND_REG_START_ADDRESS8);

	onenand_writew(onenand_buffer_address(),
			ONENAND_REG_START_BUFFER);

	onenand_writew(ONENAND_INT_CLEAR, ONENAND_REG_INTERRUPT);

	onenand_writew(ONENAND_CMD_READ, ONENAND_REG_COMMAND);

#ifndef __HAVE_ARCH_MEMCPY32
	p = (unsigned long *) buf;
#endif
	base = (unsigned long *) (CONFIG_SYS_ONENAND_BASE + ONENAND_DATARAM);

	while (!(READ_INTERRUPT() & ONENAND_INT_READ))
		continue;

	/* Check for invalid block mark */
	if (page < 2 && (onenand_readw(ONENAND_SPARERAM) != 0xffff))
		return 1;

#ifdef __HAVE_ARCH_MEMCPY32
	/* 32 bytes boundary memory copy */
	memcpy32(buf, base, pagesize);
#else
	for (offset = 0; offset < (pagesize >> 2); offset++) {
		value = *(base + offset);
		*p++ = value;
	}
#endif

	return 0;
}

#ifndef CONFIG_ONENAND_START_PAGE
#define CONFIG_ONENAND_START_PAGE	1
#endif
#define ONENAND_PAGES_PER_BLOCK		64

/**
 * onenand_read_block - Read CONFIG_SYS_MONITOR_LEN from begining
 *                      of OneNAND, skipping bad blocks
 * @return 0 on success
 */
int onenand_read_block(unsigned char *buf)
{
	int block;
	int page = CONFIG_ONENAND_START_PAGE, offset = 0;
	int pagesize = 0, erase_shift = 0;
	int erasesize = 0, nblocks = 0;
	int page_is_4KiB = 0, dev_id, density;

	if ((readl(0xE0000000) & 0x00FFF000) == 0x00110000) {
		onenand_read_page = generic_onenand_read_page;
		if (onenand_readw(ONENAND_REG_TECHNOLOGY))
			page_is_4KiB = 1;
		dev_id = onenand_readw(ONENAND_REG_DEVICE_ID);
		density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;
		if (density >= ONENAND_DEVICE_DENSITY_4Gb &&
		    !(dev_id & ONENAND_DEVICE_IS_DDP))
			page_is_4KiB = 1;
	} else {
		onenand_read_page = s5pc100_onenand_read_page;
		if (onenand_ahb_readw(ONENAND_REG_TECHNOLOGY))
			page_is_4KiB = 1;
		page = 8;
	}

	if (page_is_4KiB) {
		pagesize = 4096; /* OneNAND has 4KiB pagesize */
		erase_shift = 18;
	} else {
		pagesize = 2048;
		erase_shift = 17;
	}

	erasesize = ONENAND_PAGES_PER_BLOCK * pagesize;
	nblocks = (CONFIG_SYS_MONITOR_LEN + erasesize - 1) >> erase_shift;

	/* NOTE: you must read page from page 1 of block 0 */
	/* read the block page by page*/
	for (block = 0; block < nblocks; block++) {
		for (; page < ONENAND_PAGES_PER_BLOCK; page++) {
			if (onenand_read_page(block, page, buf + offset,
						pagesize)) {
				/* This block is bad. Skip it
				 * and read next block */
				offset -= page * pagesize;
				nblocks++;
				break;
			}
			offset += pagesize;
		}
		page = 0;
	}

	return 0;
}
