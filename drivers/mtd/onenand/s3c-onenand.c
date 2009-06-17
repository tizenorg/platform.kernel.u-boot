/*
 * OneNAND initialization at U-Boot
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
					(fsa) << 4) & 0xffffff)
#elif defined(CONFIG_S5PC1XX)
#define MEM_ADDR(fba, fpa, fsa)		(((fba) << 13 | (fpa) << 7 | \
					(fsa) << 5))
#endif

/* The 'addr' is byte address. It makes a 16-bit word */
#define CMD_MAP_00(addr)		(AHB_ADDR | MAP_00 | ((addr) << 1))
#define CMD_MAP_01(mem_addr) 		(AHB_ADDR | MAP_01 | (mem_addr))
#define CMD_MAP_10(mem_addr)		(AHB_ADDR | MAP_10 | (mem_addr))
#define CMD_MAP_11(addr)		(AHB_ADDR | MAP_11 | ((addr) << 2))

struct s3c_onenand {
	struct mtd_info *mtd;

	int		bootram_command;
	int		map_shift;
	int		fba_shift;
	int		fpa_shift;
	int		fsa_shift;
	unsigned char	oobbuf[64];
};

static struct s3c_onenand onenand;

static void s3c_onenand_ecc_clear(void)
{
	INT_ERR_ACK0_REG = LD_FAIL_ECC_ERR;

	/* Clear the ECC status */
	ECC_ERR_STAT0_REG = 0x0;
}

static unsigned int s3c_mem_addr(struct onenand_chip *this,
				int fba, int fpa, int fsa)
{
	struct s3c_onenand *s3c = &onenand;
	unsigned int mem_addr;

	mem_addr = fba << s3c->fba_shift |
		fpa << s3c->fpa_shift |
		fsa << s3c->fsa_shift;
	return mem_addr;
}

static unsigned short s3c_onenand_readw(void __iomem * addr)
{
	struct onenand_chip *this = onenand.mtd->priv;
	int reg = addr - this->base;
	int word_addr = reg >> 1;

	/* It's used for probing time */
	switch (reg) {
	case ONENAND_REG_MANUFACTURER_ID:
		return MANUFACT_ID0_REG;
	case ONENAND_REG_DEVICE_ID:
		return DEVICE_ID0_REG;
	case ONENAND_REG_VERSION_ID:
		return FLASH_VER_ID0_REG;
	case ONENAND_REG_DATA_BUFFER_SIZE:
		return DATA_BUF_SIZE0_REG;
	case ONENAND_REG_SYS_CFG1:
		return MEM_CFG0_REG;

	default:
		break;
	}

	/* BootRAM access control */
	if ((unsigned int) addr < ONENAND_DATARAM && onenand.bootram_command) {
		if (word_addr == 0)
			return MANUFACT_ID0_REG;
		if (word_addr == 1)
			return DEVICE_ID0_REG;
		if (word_addr == 2)
			return FLASH_VER_ID0_REG;
	}

	printf("%s[%d] illegal reg 0x%x, -> 0x%x 0x%x\n", __func__, __LINE__, word_addr, readl(CMD_MAP_11(word_addr)), (unsigned int) INT_ERR_STAT0_REG);
	return readl(CMD_MAP_11(word_addr)) & 0xffff;
}

static void s3c_onenand_writew(unsigned short value, void __iomem * addr)
{
	struct onenand_chip *this = onenand.mtd->priv;
	int reg = addr - this->base;
	int word_addr = reg >> 1;

	/* It's used for probing time */
	switch (reg) {
	case ONENAND_REG_SYS_CFG1:
		MEM_CFG0_REG = value;
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
			onenand.bootram_command = 1;
			return;
		}
		if (value == ONENAND_CMD_RESET) {
			MEM_RESET0_REG = ONENAND_MEM_RESET_COLD;
			onenand.bootram_command = 0;
			return;
		}
	}

	printf("%s[%d] illegal reg 0x%x, value 0x%x\n", __func__, __LINE__, word_addr, value);
	writel(value, CMD_MAP_11(word_addr));
}
static int s3c_onenand_wait(struct mtd_info *mtd, int state)
{
	unsigned int flags = INT_ACT;
	unsigned int stat = 0;
	unsigned int timeout = 0x1000000;

	switch (state) {
	case FL_READING:
		flags = BLK_RW_CMP | LOAD_CMP;
		break;
	case FL_WRITING:
		flags = BLK_RW_CMP | PGM_CMP;
		break;
	case FL_ERASING:
		flags = BLK_RW_CMP | ERS_CMP;
		break;
	default:
		break;
	}

	while (1 && timeout--) {
		stat = INT_ERR_STAT0_REG;
		if (stat & flags)
			break;
	}

//	printf("enter stat 0x%04x, flags 0x%04x\n", stat, flags);

	if (stat & (LOCKED_BLK | ERS_FAIL | PGM_FAIL | LD_FAIL_ECC_ERR)) {
		printk("s3c_onenand_wait: controller error = 0x%04x, state %d, flags 0x%x\n", stat, state, flags);
		printk("interrupt moniter status 0x%lx page 0x%lx\n", INT_MON_STATUS_REG, ERR_PAGE_ADDR_REG);
		printk("Error block address 0x%lx\n", ERR_BLK_ADDR_REG);
		
		if (stat & LOCKED_BLK)
			printk("s3c_onenand_wait: it's locked error = 0x%04x\n", stat);
		INT_ERR_ACK0_REG = stat;

		return -EIO;
	}

	INT_ERR_ACK0_REG = flags;

	if (stat & LOAD_CMP) {
		int ecc = ECC_ERR_STAT0_REG;
		if (ecc & ONENAND_ECC_4BIT_UNCORRECTABLE) {
			MTDDEBUG (MTD_DEBUG_LEVEL0,
					"s3c_onenand_wait: ECC error = 0x%04x\n", ecc);
			INT_ERR_ACK0_REG = stat;
			return -EBADMSG;
		}
	}

	return 0;
}

static int s3c_onenand_command(struct mtd_info *mtd, int cmd, loff_t addr,
                           size_t len)
{
	struct onenand_chip *this = mtd->priv;
	unsigned int *m, *s;
	unsigned char *cm, *cs;
	int fba, fpa, fsa = 0;
	int mem_addr;
	int i, ret, count, pages;
	void __iomem *cmd_addr;

	fba = (int) (addr >> this->erase_shift);
	fpa = (int) (addr >> this->page_shift);
	fpa &= this->page_mask;

	mem_addr = MEM_ADDR(fba, fpa, fsa);

	if (cmd != ONENAND_CMD_READOOB) {
		DPRINTK("cmd 0x%x, addr 0x%x, fba %d, fpa %d, len 0x%x, mem_addr 0x%x, trans mode %lu", cmd, (unsigned int) addr, fba, fpa, len, mem_addr, TRANS_MODE_REG);
	}

	switch (cmd) {
	case ONENAND_CMD_READ:
		TRANS_SPARE0_REG = TSRF;

		pages = len >> this->page_shift;
		if (len & (mtd->writesize - 1))
			pages++;
		if (fpa == 0 && pages > 1)
			writel(ONENAND_PIPELINE_READ | pages, CMD_MAP_10(mem_addr));

		cmd_addr = CMD_MAP_01(mem_addr);
		count = len >> 2;
		if (len != mtd->writesize)
			DPRINTK("length error %d", len);
		m = (unsigned int *) this->main_buf;
		for (i = 0; i < count; i++)
			*m++ = readl(cmd_addr);
		s = (unsigned int *) onenand.oobbuf;
		count = mtd->oobsize >> 2;
		for (i = 0; i < count; i++)
			*s++ = readl(cmd_addr);
		TRANS_SPARE0_REG = ~TSRF;
		return 0;

	case ONENAND_CMD_READOOB:
		TRANS_SPARE0_REG = TSRF;

		m = (unsigned int *) onenand.oobbuf;
		count = mtd->writesize >> 2;
		cmd_addr = (void *) CMD_MAP_01(mem_addr);
		/* redundant read */
		for (i = 0; i < count; i++)
			*m = readl(cmd_addr);
		memset(onenand.oobbuf, 0xff, mtd->oobsize);
		s = (unsigned int *) onenand.oobbuf;
		count = mtd->oobsize >> 2;
		/* read shared area */
		for (i = 0; i < count; i++) {
			*s = readl(CMD_MAP_01(mem_addr));
			/* It's initial bad */
			if (i == 0 && ((*s & 0xffff) != 0xffff))
				break;
			s++;
		}
		TRANS_SPARE0_REG = ~TSRF;
		return 0;

	case ONENAND_CMD_PROG:
		TRANS_SPARE0_REG = TSRF;
		m = (unsigned int *) this->main_buf;
		if (len != mtd->writesize)
			DPRINTK("length error");
		count = len >> 2;
		cmd_addr = (void *) CMD_MAP_01(mem_addr);
		printf("count %d, cmd_addr 0x%x\n", count, (unsigned int) cmd_addr);

		for (i = 0; i < count; i++) {
			writel(*m, CMD_MAP_01(mem_addr));
			m++;
		}

		/* FIXME how to write oob together */
		s = (unsigned int *) this->spare_buf;
		count = mtd->oobsize >> 2;
		for (i = 0; i < count; i++) {
			writel(*s++, CMD_MAP_01(mem_addr));
		}
		printf("%s[%d]\n", __func__, __LINE__);

		TRANS_SPARE0_REG = ~TSRF;
		return 0;

	case ONENAND_CMD_PROGOOB:
		TRANS_SPARE0_REG |= TSRF;
		count = mtd->writesize >> 2;
		for (i = 0; i < count; i++)
			writel(0xffffffff, CMD_MAP_01(mem_addr));

		memset(onenand.oobbuf, 0xff, mtd->oobsize);
		cm = (unsigned char *) onenand.oobbuf;
		cs = (unsigned char *) this->spare_buf;
		count = len;
		for (i = 0; i < count; i++)
			*cm++ = *cs++;

		s = (unsigned int *) onenand.oobbuf;
		count = mtd->oobsize >> 2;
		for (i = 0; i < count; i++)
			writel(*s++, CMD_MAP_01(mem_addr));
		TRANS_SPARE0_REG &= ~TSRF;
		return 0;

	case ONENAND_CMD_UNLOCK_ALL:
		writel(ONENAND_UNLOCK_ALL, CMD_MAP_10(mem_addr));
		return 0;

	case ONENAND_CMD_ERASE:
		writel(ONENAND_ERASE_START, CMD_MAP_10(mem_addr));

		ret = this->wait(mtd, FL_ERASING);
		if (ret)
			return ret;

		writel(ONENAND_ERASE_VERIFY, CMD_MAP_10(mem_addr));

		return 0;

	default:
		break;
	}

	return 0;
}

static int onenand_read_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				  unsigned char *buffer, int offset,
				  size_t count)
{
	struct onenand_chip *this = mtd->priv;

	DPRINTK("addr 0x%x, 0x%x, 0x%x, %d, 0x%x", (unsigned int) addr, area, (unsigned int) buffer, offset, count);

	if (area == ONENAND_SPARERAM)
		memcpy(this->spare_buf, onenand.oobbuf, mtd->oobsize);
	return 0;
}

static int onenand_write_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				   const unsigned char *buffer, int offset,
				   size_t count)
{
	struct onenand_chip *this = mtd->priv;

	DPRINTK("addr 0x%x, 0x%x, 0x%x, %d, 0x%x", (unsigned int) addr, area, (unsigned int) buffer, offset, (unsigned int) count);
	if (area == ONENAND_DATARAM)
		this->main_buf = (unsigned char *) buffer;
	else 
		this->spare_buf = (unsigned char *) buffer;

	return 0;
}

static void s3c_write_data(unsigned int value, void *addr)
{
	writel(value, addr);
}

static int s3c_onenand_write(struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
        struct onenand_chip *this = mtd->priv;
        int written = 0;
        int i, ret = 0;
        void __iomem *cmd_addr;
	int fba, fpa, fsa = 0;
	int mem_addr;
        uint *buf_poi = (uint *) buf;

	fba = (int) (to >> this->erase_shift);
	fpa = (int) (to >> this->page_shift);
	fpa &= this->page_mask;

	mem_addr = MEM_ADDR(fba, fpa, fsa);

        printf("onenand_write: to = 0x%08x, len = %i\n", (unsigned int) to, (int) len);

        /* Initialize retlen, in case of early exit */
        *retlen = 0;

        /* Do not allow writes past end of device */
        if (unlikely((to + len) > mtd->size)) {
                MTDDEBUG(MTD_DEBUG_LEVEL3, "\nonenand_write: Attempt write to past end of device\n");
                return -EINVAL;
        }

        /* Grab the lock and see if the device is available */

        /* Loop until all data write */
        while (written < len) {

                /* get address to write data */
                cmd_addr = (void __iomem *) CMD_MAP_01(mem_addr);

                /* write all data of 1 page by 4 bytes at a time */
                for (i = 0; i < (mtd->writesize / 4); i++) {
                        s3c_write_data(*buf_poi, cmd_addr);
			writel(*buf_poi, cmd_addr + 0x80000);
                        buf_poi++;
                }

		this->wait(mtd, FL_WRITING);

                written += mtd->writesize;

                if (written == len)
                        break;

                to += mtd->writesize;
        }

        *retlen = written;

        return ret;
}

static int s3c_onenand_bbt_wait(struct mtd_info *mtd, int state)
{
	unsigned int flags = INT_ACT;
	unsigned int stat;
	unsigned int timeout = 0x10000;

	flags |= BLK_RW_CMP | LOAD_CMP;

	while (1 && timeout--) {
		stat = INT_ERR_STAT0_REG;
		if (stat & flags)
			break;
	}
	INT_ERR_ACK0_REG = flags;

	if (stat & LD_FAIL_ECC_ERR) {
		s3c_onenand_ecc_clear();
		return ONENAND_BBT_READ_ERROR;
	}

	if (stat & LOAD_CMP) {
		int ecc = ECC_ERR_STAT0_REG;
		if (ecc & ONENAND_ECC_4BIT_UNCORRECTABLE) {
			s3c_onenand_ecc_clear();
			return ONENAND_BBT_READ_ERROR;
		}
	}

	return 0;
}

static void s3c_onenand_do_lock_cmd(struct mtd_info *mtd, loff_t ofs, size_t len, int cmd)
{
	struct onenand_chip *this = mtd->priv;
	int start, end, start_mem_addr, end_mem_addr;

	start = ofs >> this->erase_shift;
	start_mem_addr = MEM_ADDR(start, 0, 0);
	end = start + (len >> this->erase_shift) - 1;
	end_mem_addr = MEM_ADDR(end, 0, 0);

	if (cmd == ONENAND_CMD_LOCK) {
		writel(ONENAND_LOCK_START, CMD_MAP_10(start_mem_addr));
		writel(ONENAND_LOCK_END, CMD_MAP_10(end_mem_addr));
	} else {
		writel(ONENAND_UNLOCK_START, CMD_MAP_10(start_mem_addr));
		writel(ONENAND_UNLOCK_END, CMD_MAP_10(end_mem_addr));
	}

	this->wait(mtd, FL_LOCKING);
}

static void s3c_unlock_all(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;
	loff_t ofs = 0;
	size_t len = this->chipsize;

	if (this->options & ONENAND_HAS_UNLOCK_ALL) {
		/* Write unlock command */
		this->command(mtd, ONENAND_CMD_UNLOCK_ALL, 0, 0);

		/* No need to check return value */
		this->wait(mtd, FL_LOCKING);

		/* Workaround for all block unlock in DDP */
		if (!ONENAND_IS_DDP(this))
			return;

		/* All blocks on another chip */
		ofs = this->chipsize >> 1;
		len = this->chipsize >> 1;
	}

	s3c_onenand_do_lock_cmd(mtd, ofs, len, ONENAND_CMD_UNLOCK);
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

	onenand.mtd = mtd;

	this->read_word = s3c_onenand_readw;
	this->write_word = s3c_onenand_writew;

	this->wait = s3c_onenand_wait;
	this->bbt_wait = s3c_onenand_bbt_wait;
	this->unlock_all = s3c_unlock_all;
	this->command = s3c_onenand_command;

	this->read_bufferram = onenand_read_bufferram;
	this->write_bufferram = onenand_write_bufferram;

	mtd->write = s3c_onenand_write;
}
