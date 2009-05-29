/*
 * OneNAND initialization at U-Boot
 */

#include <common.h>
#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/onenand.h>

#if defined(CONFIG_S3C64XX)
#include <s3c6400.h>
#include <s3c64xx-onenand.h>
#elif defined(CONFIG_S5PC1XX)
#include <s5pc1xx.h>
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
#define ONENAND_ERASE_VERIFY		0x15

#define ONENAND_UNLOCK_START		0x08
#define ONENAND_UNLOCK_END		0x09
#define ONENAND_LOCK_START		0x0A
#define ONENAND_LOCK_END		0x0B
#define ONENAND_LOCK_TIGHT_START	0x0C
#define ONENAND_LOCK_TIGHT_END		0x0D
#define ONENAND_UNLOCK_ALL		0x0E

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
					(fsa) << 5) & 0x3ffffff)
#endif

#define GET_FBA(mem_addr)		((mem_addr) & 0x3ff000)

/* The 'addr' is byte address. It makes a 16-bit word */
#define CMD_MAP_00(addr)		(AHB_ADDR | MAP_00 | ((addr) << 1))
#define CMD_MAP_01(mem_addr) 		(AHB_ADDR | MAP_01 | (mem_addr))
#define CMD_MAP_10(mem_addr)		(AHB_ADDR | MAP_10 | (mem_addr))
#define CMD_MAP_11(addr)		(AHB_ADDR | MAP_11 | ((addr) << 2))

struct s3c_onenand {
	struct mtd_info *mtd;

	int bootram_command;
	unsigned char oobbuf[64];
};

static struct s3c_onenand onenand;

static void s3c_onenand_reset(void)
{
	MEM_RESET0_REG = ONENAND_MEM_RESET_COLD;
	while (!(INT_ERR_STAT0_REG & INT_ACT))
		;
	/* Clear interrupt */
	INT_ERR_ACK0_REG = 0x0;
	/* Clear the ECC status */
	ECC_ERR_STAT0_REG = 0x0;
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

	DPRINTK("illegal reg 0x%x, -> 0x%x 0x%x", word_addr, readl(CMD_MAP_11(word_addr)), (unsigned int) INT_ERR_STAT0_REG);
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
	unsigned int stat, ecc;

	while (1) {
		stat = INT_ERR_STAT0_REG;
		if (stat & flags)
			break;
	}

	INT_ERR_ACK0_REG = stat;

	if (stat & (LOCKED_BLK | ERS_FAIL | PGM_FAIL | INT_TO | LD_FAIL_ECC_ERR)) {
		printk("onenand_wait: controller error = 0x%04x\n", stat);
		if (stat & LOCKED_BLK) {
			printk("onenand_wait: it's locked error = 0x%04x\n", stat);
		}

		return -EIO;
	}

	if (stat & LOAD_CMP) {
		ecc = ECC_ERR_STAT0_REG;
		if (ecc & ONENAND_ECC_2BIT_ALL) {
			MTDDEBUG (MTD_DEBUG_LEVEL0,
					"onenand_wait: ECC error = 0x%04x\n", ecc);
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
	int i, ret, count;

	fba = (int) (addr >> this->erase_shift);
	fpa = (int) (addr >> this->page_shift);
	fpa &= this->page_mask;

	mem_addr = MEM_ADDR(fba, fpa, fsa);

	if (cmd != ONENAND_CMD_READOOB)
		DPRINTK("cmd 0x%x, addr 0x%x, fba %d, fpa %d, len 0x%x", cmd, (unsigned int) addr, fba, fpa, len);

	switch (cmd) {
	case ONENAND_CMD_READ:
		count = len >> 2;
		if (len != mtd->writesize)
			DPRINTK("length error");
		m = (unsigned int *) this->main_buf;
		DPRINTK("read buffer 0x%x", (unsigned int) this->main_buf);
		for (i = 0; i < count; i++)
			*m++ = readl(CMD_MAP_01(mem_addr));
		return 0;

	case ONENAND_CMD_READOOB:
		TRANS_SPARE0_REG = TSRF;
		m = (unsigned int *) onenand.oobbuf;
		count = mtd->writesize >> 2;
		for (i = 0; i < count; i++)
			*m = readl(CMD_MAP_01(mem_addr));
		memset(onenand.oobbuf, 0xff, mtd->oobsize);
		s = (unsigned int *) onenand.oobbuf;
		count = mtd->oobsize >> 2;
		for (i = 0; i < count; i++) {
			*s = readl(CMD_MAP_01(mem_addr));
			/* It's initial bad */
			if (i == 0 && ((*s & 0xffff) != 0xffff))
				break;
			s++;
		}
		cm = (unsigned char *) this->spare_buf;
		cs = (unsigned char *) onenand.oobbuf;
		count = len;
		for (i = 0; i < len; i++)
			*cm++ = *cs++;
		TRANS_SPARE0_REG = ~TSRF;
		return 0;

	case ONENAND_CMD_PROG:
//		TRANS_SPARE0_REG = TSRF;
		m = (unsigned int *) this->main_buf;
		if (len != mtd->writesize)
			DPRINTK("length error");
		DPRINTK("write buffer 0x%x", (unsigned int) this->main_buf);
		count = len >> 2;
		for (i = 0; i < count; i++)
			writel(*m++, CMD_MAP_01(mem_addr));
		/* FIXME how to write oob together */
#if 0
		s = (unsigned int *) this->spare_buf;
		count = mtd->oobsize >> 2;
		for (i = 0; i < count; i++)
			writel(*s++, CMD_MAP_01(mem_addr));
#endif
//		TRANS_SPARE0_REG = ~TSRF;
		return 0;

	case ONENAND_CMD_PROGOOB:
		TRANS_SPARE0_REG = TSRF;
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
		TRANS_SPARE0_REG = ~TSRF;
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
	DPRINTK("addr 0x%x, 0x%x, 0x%x, %d, 0x%x", (unsigned int) addr, area, (unsigned int) buffer, offset, count);
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

static int s3c_onenand_bbt_wait(struct mtd_info *mtd, int state)
{
	unsigned int flags = INT_ACT;
	unsigned int stat;

	while (1) {
		stat = INT_ERR_STAT0_REG;
		if (stat & flags)
			break;
	}

	INT_ERR_ACK0_REG = stat;

	if (stat & LD_FAIL_ECC_ERR) {
		s3c_onenand_reset();
		return ONENAND_BBT_READ_ERROR;
	}

	if (stat & LOAD_CMP) {
		int ecc = ECC_ERR_STAT0_REG;
		if (ecc & ONENAND_ECC_2BIT_ALL) {
			s3c_onenand_reset();
			return ONENAND_BBT_READ_ERROR;
		}
	} else
		return ONENAND_BBT_READ_FATAL_ERROR;

	return 0;
}

static int s3c_onenand_read_spareram(struct mtd_info *mtd, loff_t addr,
		int area, unsigned char *buf, int offset, size_t count)
{
	return 0;
}

void s3c_set_width_regs(struct onenand_chip *this)
{
	int dev_id, density;
	int dbs_dfs, fba, fpa, fsa;

	dev_id = DEVICE_ID0_REG;

	density = (dev_id >> ONENAND_DEVICE_DENSITY_SHIFT) & 0xf;

	dbs_dfs = 0;
	fba = density + 7;
	fpa = 6;
	fsa = 2;

	FBA_WIDTH0_REG = fba;
	FPA_WIDTH0_REG = fpa;
	FSA_WIDTH0_REG = fsa;
	DBS_DFS_WIDTH0_REG = dbs_dfs;
}

void s3c_onenand_init(struct mtd_info *mtd)
{
	struct onenand_chip *this = mtd->priv;

	onenand.mtd = mtd;

	this->read_word = s3c_onenand_readw;
	this->write_word = s3c_onenand_writew;

	this->wait = s3c_onenand_wait;
	this->bbt_wait = s3c_onenand_bbt_wait;
	this->command = s3c_onenand_command;

	this->read_bufferram = onenand_read_bufferram;
	this->read_spareram = s3c_onenand_read_spareram;
	this->write_bufferram = onenand_write_bufferram;
}
