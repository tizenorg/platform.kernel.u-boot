/*
 * Copyright (C) 2005-2010 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

#ifndef _ONENAND_H
#define _ONENAND_H

struct onenand_op {
	int (*read)(loff_t, ssize_t, ssize_t *, u_char *, int);
	int (*write)(loff_t, ssize_t, ssize_t *, u_char *);
	int (*erase)(u32, u32, int);

	struct mtd_info *mtd;
	struct onenand_chip *this;
};

struct onenand_op *onenand_get_interface(void);
void onenand_init(void);

#endif
