/*
 * (C) Copyright 2005-2009 Samsung Electronics
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
#include <malloc.h>

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#include <asm/io.h>

#include "onenand.h"

#ifdef printk
#undef printk
#endif
#define printk(...)	do{} while(0)
#define puts(...)	do{} while(0)
#ifdef printf
#undef printf
#endif
#define printf(...)	do{} while(0)

struct mtd_info onenand_mtd;
struct onenand_chip onenand_chip;
struct onenand_op onenand_ops;

static __attribute__((unused)) char dev_name[] = "onenand0";
static struct mtd_info *mtd = &onenand_mtd;

static loff_t next_ofs;
static loff_t skip_ofs;

static inline int str2long(char *p, ulong *num)
{
	char *endptr;

	*num = simple_strtoul(p, &endptr, 16);
	return (*p != '\0' && *endptr == '\0') ? 1 : 0;
}
#if 0
static int arg_off_size(int argc, char *argv[], ulong *off, ssize_t *size)
{
	if (argc >= 1) {
		if (!(str2long(argv[0], off))) {
			printf("'%s' is not a number\n", argv[0]);
			return -1;
		}
	} else {
		*off = 0;
	}

	if (argc >= 2) {
		if (!(str2long(argv[1], (ulong *)size))) {
			printf("'%s' is not a number\n", argv[1]);
			return -1;
		}
	} else {
		*size = mtd->size - *off;
	}

	if ((*off + *size) > mtd->size) {
		printf("total chip size (0x%llx) exceeded!\n", mtd->size);
		return -1;
	}
#if 0
	if (*size == mtd->size)
		puts("whole chip\n");
	else
		printf("offset 0x%lx, size 0x%x\n", *off, *size);
#endif
	return 0;
}
#endif
static int onenand_block_read(loff_t from, ssize_t len,
			      ssize_t *retlen, u_char *buf, int oob)
{
	struct onenand_chip *this = mtd->priv;
	int blocksize = (1 << this->erase_shift);
	loff_t ofs = from;
	struct mtd_oob_ops ops = {
		.retlen		= 0,
	};
	ssize_t thislen;
	int ret;

	while (len > 0) {
		thislen = min_t(ssize_t, len, blocksize);
		thislen = ALIGN(thislen, mtd->writesize);

		ret = mtd->block_isbad(mtd, ofs);
		if (ret) {
			printk("Bad blocks %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			ofs += blocksize;
			/* FIXME need to check how to handle the 'len' */
			len -= blocksize;
			continue;
		}

		if (oob) {
			ops.oobbuf = buf;
			ops.ooblen = thislen;
		} else {
			ops.datbuf = buf;
			ops.len = thislen;
		}

		ops.retlen = 0;
		ret = mtd->read_oob(mtd, ofs, &ops);
		if (ret) {
			printk("Read failed 0x%x, %d\n", (u32)ofs, ret);
			ofs += thislen;
			continue;
		}
		ofs += thislen;
		buf += thislen;
		len -= thislen;
		*retlen += ops.retlen;
	}

	return 0;
}

static int onenand_block_write(loff_t to, ssize_t len,
			       ssize_t *retlen, u_char * buf)
{
	struct onenand_chip *this = mtd->priv;
	int blocksize = (1 << this->erase_shift);
	struct mtd_oob_ops ops = {
		.retlen		= 0,
		.oobbuf		= NULL,
	};
	loff_t ofs;
	ssize_t thislen;
	int ret;

	if (to == next_ofs) {
		next_ofs = to + len;
		to += skip_ofs;
	} else {
		next_ofs = to + len;
		skip_ofs = 0;
	}
	ofs = to;

	while (len > 0) {
		thislen = min_t(ssize_t, len, blocksize);
		thislen = ALIGN(thislen, mtd->writesize);

		ret = mtd->block_isbad(mtd, ofs);
		if (ret) {
			printk("Bad blocks %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			skip_ofs += blocksize;
			goto next;
		}

		ops.datbuf = (u_char *) buf;
		ops.len = thislen;
		ops.retlen = 0;
		ret = mtd->write_oob(mtd, ofs, &ops);
		if (ret) {
			printk("Write failed 0x%x, %d", (u32)ofs, ret);
			skip_ofs += thislen;
			goto next;
		}

		buf += thislen;
		len -= thislen;
		if (retlen != NULL)
			*retlen += ops.retlen;
next:
		ofs += blocksize;
	}

	return 0;
}

static int onenand_block_erase(u32 start, u32 size, int force)
{
	struct onenand_chip *this = mtd->priv;
	struct erase_info instr = {
		.callback	= NULL,
	};
	loff_t ofs;
	int ret;
	int blocksize = 1 << this->erase_shift;

	for (ofs = start; ofs < (start + size); ofs += blocksize) {
		ret = mtd->block_isbad(mtd, ofs);
		if (ret && !force) {
			printf("Skip erase bad block %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			continue;
		}

		instr.addr = ofs;
		instr.len = blocksize;
		instr.priv = force;
		instr.mtd = mtd;
		ret = mtd->erase(mtd, &instr);
		if (ret) {
			printf("erase failed block %d at 0x%x\n",
			       (u32)(ofs >> this->erase_shift), (u32)ofs);
			continue;
		}
	}

	return 0;
}
#if 0
static int onenand_dump(struct mtd_info *mtd, ulong off, int only_oob)
{
	int i;
	u_char *datbuf, *oobbuf, *p;
	struct mtd_oob_ops ops;
	loff_t addr;

	datbuf = malloc(mtd->writesize + mtd->oobsize);
	oobbuf = malloc(mtd->oobsize);
	if (!datbuf || !oobbuf) {
		puts("No memory for page buffer\n");
		return 1;
	}
	off &= ~(mtd->writesize - 1);
	addr = (loff_t) off;
	memset(&ops, 0, sizeof(ops));
	ops.datbuf = datbuf;
	ops.oobbuf = oobbuf; /* must exist, but oob data will be appended to ops.datbuf */
	ops.len = mtd->writesize;
	ops.ooblen = mtd->oobsize;
	ops.retlen = 0;
	i = mtd->read_oob(mtd, addr, &ops);
	if (i < 0) {
		printf("Error (%d) reading page %08lx\n", i, off);
		free(datbuf);
		free(oobbuf);
		return 1;
	}
	printf("Page %08lx dump:\n", off);
	i = mtd->writesize >> 4;
	p = datbuf;

	while (i--) {
		if (!only_oob)
			printf("\t%02x %02x %02x %02x %02x %02x %02x %02x"
			       "  %02x %02x %02x %02x %02x %02x %02x %02x\n",
			       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			       p[8], p[9], p[10], p[11], p[12], p[13], p[14],
			       p[15]);
		p += 16;
	}
	puts("OOB:\n");
	p = oobbuf;
	i = mtd->oobsize >> 3;
	while (i--) {
		printf("\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
		       p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}
	free(datbuf);
	free(oobbuf);

	return 0;
}
#endif
void onenand_set_interface(struct onenand_op *onenand)
{
	onenand->read = onenand_block_read;
	onenand->write = onenand_block_write;
	onenand->erase = onenand_block_erase;
	
	onenand->mtd = &onenand_mtd;
	onenand->this = &onenand_chip;
}

struct onenand_op *onenand_get_interface(void)
{
	return &onenand_ops;
}

void onenand_init(void)
{
	memset(&onenand_mtd, 0, sizeof(struct mtd_info));
	memset(&onenand_chip, 0, sizeof(struct onenand_chip));
	memset(&onenand_ops, 0, sizeof(struct onenand_op));

	onenand_mtd.priv = &onenand_chip;

	onenand_chip.base = (void *) CONFIG_SYS_ONENAND_BASE;
	/*onenand_chip.options |= ONENAND_RUNTIME_BADBLOCK_CHECK;*/

	onenand_scan(&onenand_mtd, 1);

#if 0
	if (onenand_chip.device_id & DEVICE_IS_FLEXONENAND)
		puts("Flex-");
	puts("OneNAND: ");
	print_size(onenand_chip.chipsize, "\n");
#endif

#if 0
	/*
	 * Add MTD device so that we can reference it later
	 * via the mtdcore infrastructure (e.g. ubi).
	 */
	onenand_mtd.name = dev_name;
	add_mtd_device(&onenand_mtd);
#else
	onenand_set_interface(&onenand_ops);
#endif
}

