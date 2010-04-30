/*
 * Copyright (C) 2005-2010 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <malloc.h>

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#include <asm/io.h>

#include "recovery.h"
#include "onenand.h"

#ifdef RECOVERY_DEBUG
#define	PUTS(s)	serial_puts(DEBUG_MARK"onenand: "s)
#else
#define PUTS(s)
#endif

struct mtd_info onenand_mtd;
struct onenand_chip onenand_chip;
struct onenand_op onenand_ops;

static __attribute__((unused)) char dev_name[] = "onenand0";
static struct mtd_info *mtd = &onenand_mtd;

static loff_t next_ofs;
static loff_t skip_ofs;

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
		if (ofs > mtd->size) {
			PUTS(" range overflow\n");
			break;
		}

		ret = mtd->block_isbad(mtd, ofs);
		if (ret) {
			PUTS("Bad blocks\n");
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
			PUTS("read failed\n");
			ofs += thislen;
			continue;
		}
		ofs += thislen;
		buf += thislen;
		len -= thislen;
		if (retlen != NULL)
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
		.mode		= MTD_OOB_PLACE,
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
		if (ofs > mtd->size) {
			PUTS(" range overflow\n");
			break;
		}

		ret = mtd->block_isbad(mtd, ofs);
		if (ret) {
			PUTS("bad blocks\n");
			skip_ofs += blocksize;
			goto next;
		}

		ops.datbuf = (u_char *) buf;
		ops.len = thislen;
		ops.retlen = 0;
		ret = mtd->write_oob(mtd, ofs, &ops);
		if (ret) {
			PUTS("write failed\n");
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
			PUTS("skip erase bad block \n");
			continue;
		}

		instr.addr = ofs;
		instr.len = blocksize;
		instr.priv = force;
		instr.mtd = mtd;
		ret = mtd->erase(mtd, &instr);
		if (ret) {
			PUTS("erase failed\n");
			return ret;
		}
	}

	return 0;
}

static int onenand_block_lock_tight(u32 start, u32 size)
{
	struct onenand_chip *this = mtd->priv;
	int ret;

	ret = this->lock_tight(mtd, start, size);

	return ret;
}

void onenand_set_interface(struct onenand_op *onenand)
{
	onenand->read = onenand_block_read;
	onenand->write = onenand_block_write;
	onenand->erase = onenand_block_erase;
	onenand->lock_tight = onenand_block_lock_tight;

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

	onenand_set_interface(&onenand_ops);
}

