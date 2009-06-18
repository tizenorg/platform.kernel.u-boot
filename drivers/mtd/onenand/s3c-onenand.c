/*
 * S3C64XX/S5PC100 OneNAND driver at U-Boot
 *
 *  Copyright (C) 2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Implementation:
 *	Emulate the pseudo BufferRAM
 */

#include <common.h>
#include <malloc.h>
#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#if defined(CONFIG_S3C64XX)
#include <s3c6400.h>
#include <s3c64xx-onenand.h>
#elif defined(CONFIG_S5PC1XX)
#include <s5pc1xx-onenand.h>
#endif

#include <asm/io.h>
#include <asm/errno.h>

#if 0
#define DPRINTK(format, args...)					\
do {									\
	printk("%s[%d]: " format "\n", __func__, __LINE__, ##args);	\
} while (0)
#else
#define DPRINTK(...)			do { } while (0)
#endif

#if defined(CONFIG_S3C64XX)
#define AHB_ADDR			0x20000000
#elif defined(CONFIG_S5PC1XX)
#define AHB_ADDR			0xB0000000
#endif

#define ONENAND_ERASE_STATUS		0x00
#define ONENAND_MULTI_ERASE_SET		0x01
#define ONENAND_ERASE_START		0x03
#define ONENAND_UNLOCK_START		0x08
#define ONENAND_UNLOCK_END		0x09
#define ONENAND_LOCK_START		0x0A
#define ONENAND_LOCK_END		0x0B
#define ONENAND_LOCK_TIGHT_START	0x0C
#define ONENAND_LOCK_TIGHT_END		0x0D
#define ONENAND_UNLOCK_ALL		0x0E
#define ONENAND_OTP_ACCESS		0x12
#define ONENAND_SPARE_ACCESS_ONLY	0x13
#define ONENAND_MAIN_ACCESS_ONLY	0x14
#define ONENAND_ERASE_VERIFY		0x15
#define ONENAND_MAIN_SPARE_ACCESS	0x16
#define ONENAND_PIPELINE_READ		0x4000

#if defined(CONFIG_S3C64XX)
#define MAP_00				(0x0 << 24)
#define MAP_01				(0x1 << 24)
#define MAP_10				(0x2 << 24)
#define MAP_11				(0x3 << 24)
#elif defined(CONFIG_S5PC1XX)
#define MAP_00				(0x0 << 26)
#define MAP_01				(0x1 << 26)
#define MAP_10				(0x2 << 26)
#define MAP_11				(0x3 << 26)
#endif

#if defined(CONFIG_S3C64XX)
#define MEM_ADDR(fba, fpa, fsa)		(((fba) << 12 | (fpa) << 6 | \
					(fsa) << 4))
#elif defined(CONFIG_S5PC1XX)
#define MEM_ADDR(fba, fpa, fsa)		(((fba) << 13 | (fpa) << 7 | \
					(fsa) << 5))
#endif

/* The 'addr' is byte address. It makes a 16-bit word */
#define CMD_MAP_00(addr)		(MAP_00 | ((addr) << 1))
#define CMD_MAP_01(mem_addr) 		(MAP_01 | (mem_addr))
#define CMD_MAP_10(mem_addr)		(MAP_10 | (mem_addr))
#define CMD_MAP_11(addr)		(MAP_11 | ((addr) << 2))

//#define USE_STATIC_BUFFER

struct s3c_onenand {
	struct mtd_info	*mtd;

	void __iomem	*base;
	void __iomem	*ahb_addr;

	int		bootram_command;

#ifdef USE_STATIC_BUFFER
	char		page_buf[4096];
	char		oob_buf[128];
#else
	void __iomem	*page_buf;
	void __iomem	*oob_buf;
#endif

	unsigned int	(*mem_addr)(int fba, int fpa, int fsa);
};

#ifdef USE_STATIC_BUFFER
static struct s3c_onenand static_onenand;
#endif
static struct s3c_onenand *onenand;

static int s3c_read_reg(int offset)
{
	return readl(onenand->base + offset);
}

static void s3c_write_reg(int value, int offset)
{
	writel(value, onenand->base + offset);
}

static int s3c_read_cmd(unsigned int cmd)
{
	return readl(onenand->ahb_addr + cmd);
}

static void s3c_write_cmd(int value, unsigned int cmd)
{
	writel(value, onenand->ahb_addr + cmd);
}

static unsigned int s5pc100_mem_addr(int fba, int fpa, int fsa)
{
	return ((fba << 13) | (fpa << 7) | (fsa << 5));
}

static void s3c_onenand_reset(void)
{
	unsigned long timeout = 0x10000;
	int stat;

	s3c_write_reg(ONENAND_MEM_RESET_COLD, MEM_RESET_OFFSET);
	while (1 && timeout--) {
		stat = s3c_read_reg(INT_ERR_STAT_OFFSET);
		if (stat & INT_ACT)
			break;
	}
	stat = s3c_read_reg(INT_ERR_STAT_OFFSET);
	s3c_write_reg(stat, INT_ERR_ACK_OFFSET);

	/* Clear interrupt */
	s3c_write_reg(0x0, INT_ERR_ACK_OFFSET);
	/* Clear the ECC status */
	s3c_write_reg(0x0, ECC_ERR_STAT_OFFSET);
}

static unsigned short s3c_onenand_readw(void __iomem * addr)
{
	struct onenand_chip *this = onenand->mtd->priv;
	int reg = addr - this->base;
	int word_addr = reg >> 1;
	int value;

	/* It's used for probing time */
	switch (reg) {
	case ONENAND_REG_MANUFACTURER_ID:
		return s3c_read_reg(MANUFACT_ID_OFFSET);
	case ONENAND_REG_DEVICE_ID:
		return s3c_read_reg(DEVICE_ID_OFFSET);
	case ONENAND_REG_VERSION_ID:
		return s3c_read_reg(FLASH_VER_ID_OFFSET);
	case ONENAND_REG_DATA_BUFFER_SIZE:
		return s3c_read_reg(DATA_BUF_SIZE_OFFSET);
	case ONENAND_REG_SYS_CFG1:
		return s3c_read_reg(MEM_CFG_OFFSET);

	/* Used at unlock all status */
	case ONENAND_REG_CTRL_STATUS:
		return 0;

	case ONENAND_REG_WP_STATUS:
		return ONENAND_WP_US;

	default:
		break;
	}

	/* BootRAM access control */
	if ((unsigned int) addr < ONENAND_DATARAM && onenand->bootram_command) {
		if (word_addr == 0)
			return s3c_read_reg(MANUFACT_ID_OFFSET);
		if (word_addr == 1)
			return s3c_read_reg(DEVICE_ID_OFFSET);
		if (word_addr == 2)
			return s3c_read_reg(FLASH_VER_ID_OFFSET);
	}

	value = s3c_read_cmd(CMD_MAP_11(word_addr)) & 0xffff;
	printk(KERN_INFO "s3c_onenand_readw:  Illegal access"
		" at reg 0x%x, value 0x%x\n", word_addr, value);
	return value;
}

static void s3c_onenand_writew(unsigned short value, void __iomem * addr)
{
	struct onenand_chip *this = onenand->mtd->priv;
	int reg = addr - this->base;
	int word_addr = reg >> 1;

	/* It's used for probing time */
	switch (reg) {
	case ONENAND_REG_SYS_CFG1:
		s3c_write_reg(value, MEM_CFG_OFFSET);
		return;

	case ONENAND_REG_START_ADDRESS1:
	case ONENAND_REG_START_ADDRESS2:
		return;

	/* Lock/lock-tight/unlock/unlock_all */
	case ONENAND_REG_START_BLOCK_ADDRESS:
		return;

	default:
		break;
	}

	/* BootRAM access control */
	if ((unsigned int) addr < ONENAND_DATARAM) {
		if (value == ONENAND_CMD_READID) {
			onenand->bootram_command = 1;
			return;
		}
		if (value == ONENAND_CMD_RESET) {
			s3c_write_reg(ONENAND_MEM_RESET_COLD, MEM_RESET_OFFSET);
			onenand->bootram_command = 0;
			return;
		}
	}

	printk(KERN_INFO "s3c_onenand_writew: Illegal access"
		" at reg 0x%x, value 0x%x\n", word_addr, value);

	s3c_write_cmd(value, CMD_MAP_11(word_addr));
}

static int s3c_onenand_wait(struct mtd_info *mtd, int state)
{
	unsigned int flags = INT_ACT;
	unsigned int stat, ecc;
	unsigned int timeout = 0x100000;

	switch (state) {
	case FL_READING:
		flags |= BLK_RW_CMP | LOAD_CMP;
		break;
	case FL_WRITING:
		flags |= BLK_RW_CMP | PGM_CMP;
		break;
	case FL_ERASING:
		flags |= BLK_RW_CMP | ERS_CMP;
		break;
	case FL_LOCKING:
		flags |= BLK_RW_CMP;
		break;
	default:
		break;
	}

	while (1 && timeout--) {
		stat = s3c_read_reg(INT_ERR_STAT_OFFSET);
		if (stat & flags)
			break;
	}

	/* To get correct interrupt status in timeout case */
	stat = s3c_read_reg(INT_ERR_STAT_OFFSET);
	s3c_write_reg(stat, INT_ERR_ACK_OFFSET);
	printf("enter stat 0x%04x, flags 0x%04x\n", stat, flags);

	/*
	 * 	
	 * In the Spec. it checks the controller status first
	 * However if you get the correct information in case of
	 * power off recovery (POR) test, it should read ECC status first
	 */
	if (stat & LOAD_CMP) {
		ecc = s3c_read_reg(ECC_ERR_STAT_OFFSET);
		if (ecc & ONENAND_ECC_4BIT_UNCORRECTABLE) {
			printk(KERN_INFO "onenand_wait: ECC error = 0x%04x\n", ecc);
			mtd->ecc_stats.failed++;
			return -EBADMSG;
		}
#if 0
		} else if (ecc & ONENAND_ECC_1BIT_ALL) {
			printk(KERN_INFO "onenand_wait: correctable ECC error = 0x%04x\n", ecc);
			mtd->ecc_stats.corrected++;
		}
#endif
	}

	if (stat & (LOCKED_BLK | ERS_FAIL | PGM_FAIL | LD_FAIL_ECC_ERR)) {
		printk(KERN_INFO "s3c_onenand_wait: controller error = 0x%04x\n", stat);
		if (stat & LOCKED_BLK)
			printk(KERN_INFO "s3c_onenand_wait: it's locked error = 0x%04x\n", stat);

		return -EIO;
	}

	return 0;
}

static int s3c_onenand_command(struct mtd_info *mtd, int cmd, loff_t addr,
                           size_t len)
{
	struct onenand_chip *this = mtd->priv;
	unsigned int *m, *s;
	int fba, fpa, fsa = 0;
	unsigned int mem_addr;
	int i, ret, mcount, scount;
	int dummy, index;

	fba = (int) (addr >> this->erase_shift);
	fpa = (int) (addr >> this->page_shift);
	fpa &= this->page_mask;

	mem_addr = onenand->mem_addr(fba, fpa, fsa);

	switch (cmd) {
	case ONENAND_CMD_READ:
	case ONENAND_CMD_READOOB:
	case ONENAND_CMD_BUFFERRAM:
		ONENAND_SET_NEXT_BUFFERRAM(this);
	default:
		break;
	}

	index = ONENAND_CURRENT_BUFFERRAM(this);

	/*
	 * Emulate Two BufferRAMs and access with 4 bytes pointer
	 */
	m = (unsigned int *) onenand->page_buf;
	s = (unsigned int *) onenand->oob_buf;

	if (index) {
		m += (this->writesize >> 2);
		s += (mtd->oobsize >> 2);
	}

	mcount = mtd->writesize >> 2;
	scount = mtd->oobsize >> 2;

	switch (cmd) {
	case ONENAND_CMD_READ:
		/* Main */
		for (i = 0; i < mcount; i++)
			*m++ = s3c_read_cmd(CMD_MAP_01(mem_addr));
		break;

	case ONENAND_CMD_READOOB:
		s3c_write_reg(TSRF, TRANS_SPARE_OFFSET);
		/* Main - dummy read */
		for (i = 0; i < mcount; i++)
			dummy = s3c_read_cmd(CMD_MAP_01(mem_addr));

		/* Spare */
		for (i = 0; i < scount; i++)
			*s++ = s3c_read_cmd(CMD_MAP_01(mem_addr));

		s3c_write_reg(0, TRANS_SPARE_OFFSET);
		return 0;

	case ONENAND_CMD_PROG:
		/* Main */
		for (i = 0; i < mcount; i++)
			s3c_write_cmd(*m++, CMD_MAP_01(mem_addr));
		return 0;

	case ONENAND_CMD_PROGOOB:
		s3c_write_reg(TSRF, TRANS_SPARE_OFFSET);

		/* Main - dummy write */
		for (i = 0; i < mcount; i++)
			s3c_write_cmd(0xffffffff, CMD_MAP_01(mem_addr));

		/* Spare */
		for (i = 0; i < scount; i++)
			s3c_write_cmd(*s++, CMD_MAP_01(mem_addr));

		s3c_write_reg(0, TRANS_SPARE_OFFSET);
		return 0;

	case ONENAND_CMD_UNLOCK_ALL:
		s3c_write_cmd(ONENAND_UNLOCK_ALL, CMD_MAP_10(mem_addr));
		return 0;

	case ONENAND_CMD_ERASE:
		s3c_write_cmd(ONENAND_ERASE_START, CMD_MAP_10(mem_addr));

		ret = this->wait(mtd, FL_ERASING);
		if (ret)
			return ret;

		s3c_write_cmd(ONENAND_ERASE_VERIFY, CMD_MAP_10(mem_addr));
		return 0;

	default:
		break;
	}

	return 0;
}

static unsigned char *s3c_get_bufferram(struct mtd_info *mtd, int area)
{
	struct onenand_chip *this = mtd->priv;
	int index = ONENAND_CURRENT_BUFFERRAM(this);
	unsigned char *p;

	if (area == ONENAND_DATARAM) {
		p = (unsigned char *) onenand->page_buf;
		if (index == 1)
			p += this->writesize;
	} else {
		p = (unsigned char *) onenand->oob_buf;
		if (index == 1)
			p += mtd->oobsize;
	}

	return p;
}

static int onenand_read_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				  unsigned char *buffer, int offset,
				  size_t count)
{
	unsigned char *p;

	p = s3c_get_bufferram(mtd, area);
	memcpy(buffer, p + offset, count);
	return 0;
}

static int onenand_write_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				   const unsigned char *buffer, int offset,
				   size_t count)
{
	unsigned char *p;

	p = s3c_get_bufferram(mtd, area);
	memcpy(p + offset, buffer, count);
	return 0;
}

static int s3c_onenand_bbt_wait(struct mtd_info *mtd, int state)
{
	unsigned int flags = INT_ACT;
	unsigned int stat;
	unsigned int timeout = 0x10000;

	flags |= BLK_RW_CMP | LOAD_CMP;

	while (1 && timeout--) {
		stat = s3c_read_reg(INT_ERR_STAT_OFFSET);
		if (stat & flags)
			break;
	}
	stat = s3c_read_reg(INT_ERR_STAT_OFFSET);
	s3c_write_reg(stat, INT_ERR_ACK_OFFSET);

	if (stat & LD_FAIL_ECC_ERR) {
		s3c_onenand_reset();
		return ONENAND_BBT_READ_ERROR;
	}

	if (stat & LOAD_CMP) {
		int ecc = ECC_ERR_STAT0_REG;
		if (ecc & ONENAND_ECC_4BIT_UNCORRECTABLE) {
			s3c_onenand_reset();
			return ONENAND_BBT_READ_ERROR;
		}
	}

	return 0;
}

static void s3c_onenand_check_lock_status(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;
	unsigned int block, end;
	int tmp;

	end = this->chipsize >> this->erase_shift;

	for (block = 0; block < end; block++) {
		tmp = s3c_read_cmd(CMD_MAP_01(onenand->mem_addr(block, 0, 0)));

		if (s3c_read_reg(INT_ERR_STAT_OFFSET) & LOCKED_BLK) {
			printf("block %d is write-protected!\n", block);
			s3c_write_reg(LOCKED_BLK, INT_ERR_ACK_OFFSET);
		}
	}
}

static void s3c_onenand_do_lock_cmd(struct mtd_info *mtd, loff_t ofs, size_t len, int cmd)
{
	struct onenand_chip *this = mtd->priv;
	int start, end, start_mem_addr, end_mem_addr;

	start = ofs >> this->erase_shift;
	start_mem_addr = onenand->mem_addr(start, 0, 0);
	end = start + (len >> this->erase_shift) - 1;
	end_mem_addr = onenand->mem_addr(end, 0, 0);

	if (cmd == ONENAND_CMD_LOCK) {
		s3c_write_cmd(ONENAND_LOCK_START, CMD_MAP_10(start_mem_addr));
		s3c_write_cmd(ONENAND_LOCK_END, CMD_MAP_10(end_mem_addr));
	} else {
		s3c_write_cmd(ONENAND_UNLOCK_START, CMD_MAP_10(start_mem_addr));
		s3c_write_cmd(ONENAND_UNLOCK_END, CMD_MAP_10(end_mem_addr));
	}

	printk("%s[%d]\n", __func__, __LINE__);
	this->wait(mtd, FL_LOCKING);
	printk("%s[%d]\n", __func__, __LINE__);
}

static void s3c_unlock_all(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;
	loff_t ofs = 0;
	size_t len = this->chipsize;

	printk("%s[%d]\n", __func__, __LINE__);
	if (this->options & ONENAND_HAS_UNLOCK_ALL) {
		/* Write unlock command */
		this->command(mtd, ONENAND_CMD_UNLOCK_ALL, 0, 0);

		/* No need to check return value */
		this->wait(mtd, FL_LOCKING);

		/* Workaround for all block unlock in DDP */
		if (!ONENAND_IS_DDP(this)) {
			s3c_onenand_check_lock_status(mtd);
			return;
		}

		/* All blocks on another chip */
		ofs = this->chipsize >> 1;
		len = this->chipsize >> 1;
	}

	s3c_onenand_do_lock_cmd(mtd, ofs, len, ONENAND_CMD_UNLOCK);

//	s3c_onenand_check_lock_status(mtd);
}

#if 0
static void s3c_set_width_regs(struct onenand_chip *this)
{
	int dev_id, density;
	int fba, fpa, fsa;
	int dbs_dfs;

	dev_id = DEVICE_ID0_REG;

	density = (dev_id >> ONENAND_DEVICE_DENSITY_SHIFT) & 0xf;
	dbs_dfs = !!(dev_id & ONENAND_DEVICE_IS_DDP);

	fba = density + 7;
	if (dbs_dfs)
		fba--;		/* Decrease the fba */
	fpa = 6;
	if (density >= ONENAND_DEVICE_DENSITY_512Mb)
		fsa = 2;
	else
		fsa = 1;

	DPRINTK("FBA %lu, FPA %lu, FSA %lu, DDP %lu",
		FBA_WIDTH0_REG, FPA_WIDTH0_REG, FSA_WIDTH0_REG,
		DDP_DEVICE_REG);

	DPRINTK("mem_cfg0 0x%lx, sync mode %lu, dev_page_size %lu, BURST LEN %lu",
		MEM_CFG0_REG,
		SYNC_MODE_REG,
		DEV_PAGE_SIZE_REG,
		BURST_LEN0_REG
		);

	DEV_PAGE_SIZE_REG = 0x1;

#ifdef CONFIG_S3C64XX
	FBA_WIDTH0_REG = fba;
	FPA_WIDTH0_REG = fpa;
	FSA_WIDTH0_REG = fsa;
	DBS_DFS_WIDTH0_REG = dbs_dfs;
#endif
}
#endif

void s3c_onenand_init(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;

#ifdef USE_STATIC_BUFFER
	onenand = &static_onenand;
#else
	onenand = malloc(sizeof(struct s3c_onenand));
	if (!onenand)
		return;

	onenand->page_buf = malloc(SZ_4K * sizeof(char));
	if (!onenand->page_buf)
		return;

	onenand->oob_buf = malloc(128 * sizeof(char));
	if (!onenand->oob_buf)
		return;
#endif

	onenand->mtd = mtd;

	/* S5PC100 specific values */
	onenand->base = (void *) 0xE7100000;
	onenand->ahb_addr = (void *) 0xB0000000;
	onenand->mem_addr = s5pc100_mem_addr;

	this->read_word = s3c_onenand_readw;
	this->write_word = s3c_onenand_writew;

	this->wait = s3c_onenand_wait;
	this->bbt_wait = s3c_onenand_bbt_wait;
//	this->unlock_all = s3c_unlock_all;
	this->command = s3c_onenand_command;

	this->read_bufferram = onenand_read_bufferram;
	this->write_bufferram = onenand_write_bufferram;
}
