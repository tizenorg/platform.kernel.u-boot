/*
 * USB Downloader for SAMSUNG Platform
 *
 * Copyright (C) 2007-2008 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 */

#include <common.h>
#include <usbd.h>
#include <asm/errno.h>
#include <malloc.h>

/* version of USB Downloader Application */
#define APP_VERSION	"2.0.2"

#define OPS_READ	0
#define OPS_WRITE	1

#ifdef CONFIG_CMD_MTDPARTS
#include <jffs2/load_kernel.h>
static struct part_info *parts[16];
#endif

#ifdef CONFIG_UBIFS_MK
#include <mkfs.ubifs.h>
#endif

#ifdef CONFIG_UBINIZE
#include <ubinize.h>
#endif

#include <mbr.h>

static const char pszMe[] = "usbd: ";

static struct usbd_ops usbd_ops;

static unsigned int part_id;
static unsigned int write_part = 0;
static unsigned int fs_offset = 0x0;

#define NAND_PAGE_SIZE 2048

static unsigned long down_ram_addr;

static int down_mode;

/* common commands */
extern int do_run(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);

#ifdef CONFIG_CMD_UBI
static int check_ubi_mode(void)
{
	char *env_ubifs;
	int ubi_mode;

	env_ubifs = getenv("ubi");
	ubi_mode = !strcmp(env_ubifs, "enabled");

	return ubi_mode;
}
#else
#define check_ubi_mode()		0
#endif

#ifdef CONFIG_CMD_MTDPARTS
int mtdparts_init(void);
int find_dev_and_part(const char*, struct mtd_device**, u8*, struct part_info**);

/* common/cmd_jffs2.c */
extern struct list_head devices;

static u8 count_mtdparts(void)
{
	struct list_head *dentry, *pentry;
	struct mtd_device *dev;
	u8 part_num = 0;

	list_for_each(dentry, &devices) {
		dev = list_entry(dentry, struct mtd_device, link);

		/* list partitions for given device */
		list_for_each(pentry, &dev->parts)
			part_num++;
	}

	return part_num;
}

static int get_part_info(void)
{
	struct mtd_device *dev;
	u8 out_partnum;
	char part_name[12];
	char nand_name[12];
	int i;
	int part_num;
	int ubi_mode = 0;

#if defined(CONFIG_CMD_NAND)
	sprintf(nand_name, "nand0");
#elif defined(CONFIG_CMD_ONENAND)
	sprintf(nand_name, "onenand0");
#else
	printf("Configure your NAND sub-system\n");
	return 0;
#endif

	if (mtdparts_init())
		return 0;

	ubi_mode = check_ubi_mode();

	part_num = count_mtdparts();
	for (i = 0; i < part_num; i++) {
		sprintf(part_name, "%s,%d", nand_name, i);

		if (find_dev_and_part(part_name, &dev, &out_partnum, &parts[i]))
			return -EINVAL;
	}

	return 0;
}

static int get_part_id(char *name)
{
	int nparts = count_mtdparts();
	int i;

	for (i = 0; i < nparts; i++) {
		if (strcmp(parts[i]->name, name) == 0)
			return i;
	}

	printf("Error: Unknown partition -> %s\n", name);
	return -1;
}
#else
static int get_part_info(void)
{
#ifdef CONFIG_CMD_MBR
	return 0;
#else
	printf("Error: Can't get patition information\n");
	return -EINVAL;
#endif
}

static int get_part_id(char *name)
{
	return 0;
}
#endif

static void boot_cmd(char *addr)
{
	char *argv[] = { "bootm", addr };
	do_bootm(NULL, 0, 2, argv);
}

#if defined(CONFIG_CMD_NAND)
extern int do_nand(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#elif defined(CONFIG_CMD_ONENAND)
extern int do_onenand(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif

/* NAND erase and write using nand command */
static int nand_cmd(int type, char *p1, char *p2, char *p3)
{
	int ret = 1;
	char nand_name[12];
	int (*nand_func) (cmd_tbl_t *, int, int, char **);

#if defined(CONFIG_CMD_NAND)
	sprintf(nand_name, "nand");
	nand_func = do_nand;
#elif defined(CONFIG_CMD_ONENAND)
	sprintf(nand_name, "onenand");
	nand_func = do_onenand;
#else
	printf("Configure your NAND sub-system\n");
	return 0;
#endif

	if (type == 0) {
		char *argv[] = {nand_name, "erase", p1, p2};
		printf("%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
		ret = nand_func(NULL, 0, 4, argv);
	} else if (type == 1) {
		char *argv[] = {nand_name, "write", p1, p2, p3};
		printf("%s %s %s %s %s\n", argv[0], argv[1], argv[2],
				argv[3], argv[4]);
		ret = nand_func(NULL, 0, 5, argv);
	} else if (type == 2) {
		char *argv[] = {nand_name, "write.yaffs", p1, p2, p3};
		printf("%s %s %s %s %s\n", argv[0], argv[1], argv[2],
				argv[3], argv[4]);
		ret = nand_func(NULL, 0, 5, argv);
	} else if (type == 3) {
		char *argv[] = {nand_name, "lock", p1, p2};
		printf("%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
		ret = nand_func(NULL, 0, 4, argv);
	}

	if (ret)
		printf("Error: NAND Command\n");

	return ret;
}

#ifdef CONFIG_CMD_UBI
int do_ubi(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

int ubi_cmd(int part, char *p1, char *p2, char *p3)
{
	int ret = 1;

	if (part == RAMDISK_PART_ID) {
		char *argv[] = {"ubi", "write", p1, "rootfs.cramfs", p2, p3};
		printf("%s %s %s %s %s %s\n", argv[0], argv[1], argv[2],
				argv[3], argv[4], argv[5]);
		ret = do_ubi(NULL, 0, 6, argv);
	} else if (part == FILESYSTEM_PART_ID) {
		char *argv[] = {"ubi", "write", p1, "factoryfs.cramfs", p2, p3};
		printf("%s %s %s %s %s %s\n", argv[0], argv[1], argv[2],
				argv[3], argv[4], argv[5]);
		ret = do_ubi(NULL, 0, 6, argv);
	} else if (part == FILESYSTEM2_PART_ID) {
		char *argv[] = {"ubi", "write", p1, "datafs.ubifs", p2, p3};
		printf("%s %s %s %s %s %s\n", argv[0], argv[1], argv[2],
				argv[3], argv[4], argv[5]);
		ret = do_ubi(NULL, 0, 6, argv);
	}

	return ret;
}
#endif

#ifdef CONFIG_CMD_MMC
#include <mmc.h>
#include <fat.h>

static struct mmc *mmc;

static int mmc_cmd(int ops, int dev_num, ulong start, lbaint_t cnt, void *addr)
{
	int ret;

	if (ops) {
		printf("mmc write 0x%x 0x%x\n", start, cnt);
		ret = mmc->block_dev.block_write(dev_num, start, cnt, addr);
	} else {
		printf("mmc read 0x%x 0x%x\n", start, cnt);
		ret = mmc->block_dev.block_read(dev_num, start, cnt, addr);
	}

	if (ret > 0)
		ret = 0;
	else
		ret = 1;

	return ret;
}

#define NORMAL_PARTITION	0
#define EXTEND_PARTITION	1

#define EXTEND_PART_TYPE	5

#define EXTEND_MAX_PART		32

static unsigned int cur_blk_offset;
static unsigned int cur_part_size;
static unsigned int cur_part;

static unsigned int mmc_parts;
static unsigned int mmc_part_write;

static u8 part_mode = 0;

struct partition_info {
	u32 size;
	u32 checksum;
	u32 res;
} __attribute__((packed));

struct partition_header {
	u8			fat32head[16];	/* RFSHEAD identifier */
	struct partition_info	partition[EXTEND_MAX_PART];
	u8			res[112];	/* only data (without MBR) */
} __attribute__((packed));

struct mul_partition_info {
	u32 lba_begin;		/* absolute address from 0 block */
	u32 num_sectors;
} __attribute__((packed));

#define MBR_OFFSET	0x10

struct partition_header part_info;
struct mbr mbr_info;
struct mul_partition_info mul_info[EXTEND_MAX_PART];

static int write_mmc_partition(struct usbd_ops *usbd, u32 *ram_addr, u32 len)
{
	unsigned int blocks;
	int i;
	int loop;
	u32 cnt;
	int ret;

	if (cur_part_size > len) {
		blocks = len / usbd->mmc_blk;
		ret = -1;
	} else {
		blocks = cur_part_size / usbd->mmc_blk;
		ret = len - cur_part_size;
	}

	if (len % usbd->mmc_blk)
		blocks++;

	loop = blocks / usbd->mmc_max;
	if (blocks % usbd->mmc_max)
		loop++;

	for (i = 0; i < loop; i++) {
		if (i == 0) {
			cnt = blocks % usbd->mmc_max;
			if (cnt == 0)
				cnt = usbd->mmc_max;
		} else {
			cnt = usbd->mmc_max;
		}

		mmc_cmd(OPS_WRITE, usbd->mmc_dev, cur_blk_offset,
				cnt, (void *)*ram_addr);

		cur_blk_offset += cnt;

		*ram_addr += (cnt * usbd->mmc_blk);
	}

	return ret;
}

static int write_file_mmc(struct usbd_ops *usbd, u32 len)
{
	u32 ram_addr;
	int i;
	int ret;
	struct mbr *mbr;

	if (!usbd->mmc_total) {
		printf("MMC is not supported!\n");
		return 0;
	}

	ram_addr = (u32)down_ram_addr;

	if (cur_blk_offset == 0) {
		boot_sector *bs;
		u32 mbr_blk_size;

		memcpy(&part_info, (void *)ram_addr,
				sizeof(struct partition_header));

		ram_addr += sizeof(struct partition_header);
		len -= sizeof(struct partition_header);
		mbr = (struct mbr *)ram_addr;
		mbr_blk_size = mbr->parts[0].lba;

		if (mbr->parts[0].partition_type != EXTEND_PART_TYPE) {
			part_mode = NORMAL_PARTITION;

			/* modify sectors of p1 */
			mbr->parts[0].nsectors = usbd->mmc_total -
					(mbr_blk_size +
					mbr->parts[1].nsectors +
					mbr->parts[2].nsectors +
					mbr->parts[3].nsectors);

			mmc_parts++;

			/* modify lba_begin of p2 and p3 and p4 */
			for (i = 1; i < 4; i++) {
				if (part_info.partition[i].size == 0)
					break;

				mmc_parts++;
				mbr->parts[i].lba =
					mbr->parts[i - 1].lba +
					mbr->parts[i - 1].nsectors;
			}

			/* copy MBR */
			memcpy(&mbr_info, mbr, sizeof(struct mbr));

			printf("Total Size: 0x%08x #parts %d\n",
					(unsigned int)usbd->mmc_total,
					mmc_parts);
			for (i = 0; i < mmc_parts; i++) {
				printf("p%d\t0x%08x\t0x%08x\n", i + 1,
					mbr_info.parts[i].lba,
					mbr_info.parts[i].nsectors);
			}

			/* write MBR */
			mmc_cmd(OPS_WRITE, usbd->mmc_dev, 0,
					mbr_blk_size, (void *)ram_addr);

			ram_addr += mbr_blk_size * usbd->mmc_blk;
			len -= mbr_blk_size * usbd->mmc_blk;

			cur_blk_offset = mbr_blk_size;
			cur_part = 0;
			cur_part_size = part_info.partition[0].size;

			/* modify p1's total sector */
			bs = (boot_sector *)ram_addr;
			bs->total_sect = mbr_info.parts[0].nsectors;

			printf("\nWrite Partition %d.. %d blocks\n",
				cur_part + 1,
				part_info.partition[cur_part].size /
				(int)usbd->mmc_blk);
		} else {
			part_mode = EXTEND_PARTITION;

			for (i = 0; i < EXTEND_MAX_PART; i++) {
				if (part_info.partition[i].size == 0)
					break;
				mmc_parts++;
			}

			if (mmc_parts == 0)
				return -1;
			else
				printf("found %d partitions\n", mmc_parts);

			/* build partition table */

			mul_info[0].num_sectors =
				usbd->mmc_total - mbr_blk_size;
			for (i = 1; i < mmc_parts; i++) {
				mul_info[i].num_sectors =
					part_info.partition[i].res +
					mbr_blk_size; /* add MBR */
			}

			for (i = 1; i < mmc_parts; i++) {
				mul_info[0].num_sectors -=
					mul_info[i].num_sectors;
			}

			mul_info[0].lba_begin = mbr_blk_size;
			for (i = 1; i < mmc_parts; i++) {
				mul_info[i].lba_begin =
					mul_info[i-1].lba_begin	+
					mul_info[i-1].num_sectors;
			}

			/* modify MBR of extended partition: p1 */
			mbr->parts[0].nsectors =
				usbd->mmc_total - mbr_blk_size;

			/* modify MBR of first logical partition: p5 */
			mbr = (struct mbr *)
				(ram_addr + mbr_blk_size * usbd->mmc_blk);

			mbr->parts[0].nsectors =
				mul_info[0].num_sectors - mbr_blk_size;
			mbr->parts[1].lba=
				mul_info[1].lba_begin - mbr_blk_size;

			/* modify BPB data of p5 */
			bs = (boot_sector *)
				((u32)mbr + mbr_blk_size * usbd->mmc_blk);
			memset(&bs->sectors, 0, 2);
			bs->total_sect = mbr->parts[0].nsectors;

			printf("Total Size: 0x%08x #parts %d\n",
					(unsigned int)usbd->mmc_total,
					mmc_parts);
			for (i = 0; i < mmc_parts; i++) {
				printf("p%d\t0x%08x\t0x%08x\n", i + 1,
					mul_info[i].lba_begin,
					mul_info[i].num_sectors);
			}

			cur_blk_offset = 0;
			cur_part = 0;
			cur_part_size = part_info.partition[0].size;

			printf("\nWrite Partition %d.. %d blocks\n",
				cur_part + 1,
				part_info.partition[cur_part].size /
				(int)usbd->mmc_blk);
		}
	}

	for (i = cur_part; i < mmc_parts; i++) {
		ret = write_mmc_partition(usbd, &ram_addr, len);

		if (ret < 0) {
			cur_part_size -= len;
			break;
		} else {
			cur_part++;
			cur_part_size =
				part_info.partition[cur_part].size;

			if (part_mode == NORMAL_PARTITION) {
				cur_blk_offset =
					mbr_info.parts[cur_part].lba;
			} else {
				cur_blk_offset =
					mul_info[cur_part].lba_begin;
				/* modify MBR */
				if (cur_part < mmc_parts) {
					mbr = (struct mbr *)ram_addr;
					mbr->parts[1].lba=
						mul_info[cur_part+1].lba_begin -
						mbr->parts[0].lba;
				}
			}

			if (ret == 0)
				break;
			else
				len = ret;

			printf("\nWrite Partition %d.. %d blocks\n",
				cur_part + 1,
				part_info.partition[cur_part].size /
				(int)usbd->mmc_blk);
		}
	}

	return 0;
}

static int write_file_mmc_part(struct usbd_ops *usbd, u32 len)
{
	u32 ram_addr;
	u32 ofs;
	int i;
	struct mbr *mbr;
	u32 mbr_blk_size;

	ram_addr = (u32)down_ram_addr;
	mbr = &mbr_info;

	if (cur_blk_offset == 0) {
		/* read MBR */
		mmc_cmd(OPS_READ, usbd->mmc_dev, 0,
			sizeof(struct mbr)/usbd->mmc_blk, (void *)mbr);

		mbr_blk_size = mbr->parts[0].lba;

		if (mbr->parts[0].partition_type != EXTEND_PART_TYPE) {
			part_mode = NORMAL_PARTITION;

			for (i = 0; i < 4; i++) {
				printf("p%d\t0x%08x\t0x%08x\n", i + 1,
					mbr_info.parts[i].lba,
					mbr_info.parts[i].nsectors);
			}

			cur_blk_offset =
				mbr->parts[mmc_part_write - 1].lba;
			cur_part = mmc_part_write - 1;
			cur_part_size =
				usbd->mmc_blk *
				mbr->parts[mmc_part_write - 1].nsectors;

			if (mmc_part_write == 1) {
				ram_addr += sizeof(struct mbr);
				cur_blk_offset += sizeof(struct mbr) /
					usbd->mmc_blk;
				len -= sizeof(struct mbr);
			}
		} else {
			part_mode = EXTEND_PARTITION;

			for (i = 1; i < mmc_part_write; i++) {
				ofs = mbr->parts[0].lba + mbr->parts[1].lba;
				printf("P%d start blk: 0x%x, size: 0x%x\n", i,
					ofs, mbr->parts[1].nsectors);
				mmc_cmd(OPS_READ, usbd->mmc_dev, ofs,
					(sizeof(struct mbr) / usbd->mmc_blk),
					(void *)mbr);
			}

			ofs = mbr->parts[0].lba + mbr->parts[1].lba;
			printf("P%d start blk: 0x%x, size: 0x%x\n", i,
				ofs, mbr->parts[1].nsectors);

			ofs += mbr_blk_size; /* skip MBR */
			cur_blk_offset = ofs;
			cur_part = mmc_part_write - 1;
			cur_part_size =
				usbd->mmc_blk *
				mbr->parts[1].nsectors;

			if (mmc_part_write == 1) {
				boot_sector *bs;
				u32 total_sect;
				/* modify BPB data of p1 */
				mmc_cmd(OPS_READ, usbd->mmc_dev, cur_blk_offset,
					(sizeof(struct mbr) / usbd->mmc_blk),
					mbr);

				bs = (boot_sector *)mbr;
				total_sect = bs->total_sect;

				bs = (boot_sector *)ram_addr;
				memset(&bs->sectors, 0, 2);
				bs->total_sect = total_sect;
			}
		}
	}

	printf("\nWrite Partition %d.. %d blocks\n",
		cur_part + 1,
		len / (int)usbd->mmc_blk);

	write_mmc_partition(usbd, &ram_addr, len);

	return 0;
}
#endif

#ifdef CONFIG_CMD_MBR
static int write_mmc_image(struct usbd_ops *usbd, unsigned int len, int part_num)
{
	int ret = 0;

	if (mbr_parts <= part_num) {
		printf("Error: MBR table have %d partitions (request %d)\n",
				mbr_parts, part_num);
		return 1;
	}

#if 0
	/* modify size of UMS partition */
	if (part_num == 4 && fs_offset == 0) {
		boot_sector *bs;
		bs = (boot_sector *)down_ram_addr;
		memset(&bs->sectors, 0, 2);
		bs->total_sect = usbd->mmc_total - mbr_offset[part_num];
	}
#endif
	ret = mmc_cmd(OPS_WRITE, usbd->mmc_dev,
			mbr_offset[part_num] + fs_offset,
			len / usbd->mmc_blk,
			(void *)down_ram_addr);

	fs_offset += (len / usbd->mmc_blk);

	return ret;
}
#endif

static int write_fat_file(struct usbd_ops *usbd, char *file_name,
			int part_id, int len)
{
#ifdef CONFIG_FAT_WRITE
	int ret;

	ret = fat_register_device(&mmc->block_dev, part_id);
	if (ret < 0) {
		printf("error : fat_register_divce\n");
		return 0;
	}

	ret = file_fat_write(file_name, (void *)down_ram_addr, len);

	/* format and write again */
	if (ret == 1) {
		printf("formatting\n");
		if (mkfs_vfat(&mmc->block_dev, part_id)) {
			printf("error : format device\n");
			return 0;
		}
		ret = file_fat_write(file_name, (void *)down_ram_addr, len);
	}

	if (ret < 0) {
		printf("error : writing uImage\n");
		return 0;
	}
#else
	printf("error: doesn't support fat_write\n");
#endif
	return 0;
}


static int ubi_update = 0;

static int write_file_system(char *ramaddr, ulong len, char *offset,
		char *length, int part_num)
{
	int ret = 0;

#ifdef CONFIG_CMD_MTDPARTS
#ifdef CONFIG_CMD_UBI
	/* UBI Update */
	if (ubi_update) {
		sprintf(length, "0x%x", (uint)len);
		ret = ubi_cmd(part_id, ramaddr, length, "cont");
		return ret;
	}
#endif

	/* Erase entire partition at the first writing */
	if (write_part == 0 && ubi_update == 0) {
		sprintf(offset, "0x%x", (uint)parts[part_num]->offset);
		sprintf(length, "0x%x", (uint)parts[part_num]->size);
		nand_cmd(0, offset, length, NULL);
	}

	sprintf(offset, "0x%x", (uint)(parts[part_num]->offset + fs_offset));
	sprintf(length, "0x%x", (uint)len);

	fs_offset += len;
	ret = nand_cmd(1, ramaddr, offset, length);
#endif

	return ret;
}

static int qboot_erase = 0;

/* Erase the qboot */
static void erase_qboot_area(void)
{
#ifdef CONFIG_CMD_MTDPARTS
	char offset[12], length[12];
	int qboot_id;

	if (qboot_erase)
		return;

	qboot_id = get_part_id("qboot");

	if (qboot_id != -1) {
		printf("\nCOMMAND_ERASE_QBOOT\n");
		sprintf(offset, "%x", parts[qboot_id]->offset);
		sprintf(length, "%x", parts[qboot_id]->size);
		nand_cmd(0, offset, length, NULL);
		qboot_erase = 1;
	}
#endif
}

/* Erase the environment */
static void erase_env_area(struct usbd_ops *usbd)
{
#if defined(CONFIG_ENV_IS_IN_NAND) || defined(CONFIG_ENV_IS_IN_ONENAND)
	int param_id;
	char offset[12], length[12];

	param_id = get_part_id("params");

	if (param_id == -1) {
		sprintf(offset, "%x", CONFIG_ENV_ADDR);
		sprintf(length, "%x", CONFIG_ENV_SIZE);
	} else {
		sprintf(offset, "%x", parts[param_id]->offset);
		sprintf(length, "%x", parts[param_id]->size);
	}
	nand_cmd(0, offset, length, NULL);
#elif defined(CONFIG_ENV_IS_IN_MMC)
	char buf[usbd->mmc_blk];

	memset(buf, 0x0, usbd->mmc_blk);
	mmc_cmd(OPS_WRITE, CONFIG_SYS_MMC_ENV_DEV,
			CONFIG_ENV_OFFSET / usbd->mmc_blk,
			1, (void *)buf);
#endif
}

static inline void send_ack(struct usbd_ops *usbd, int data)
{
	*((ulong *) usbd->tx_data) = data;
	usbd->send_data(usbd->tx_data, usbd->tx_len);
}

static inline int check_mmc_device(struct usbd_ops *usbd)
{
	if (usbd->mmc_total)
		return 0;

	printf("\nError: Couldn't find the MMC device\n");
	return 1;
}

static int write_mtd_image(struct usbd_ops *usbd, int img_type,
		unsigned int len, unsigned int arg)
{
	int ret = -1;
#ifdef CONFIG_CMD_MTDPARTS
	unsigned int ofs = 0;
	char offset[12], length[12], ramaddr[12];

	sprintf(ramaddr, "0x%x", (uint) down_ram_addr);

	/* Erase and Write to NAND */
	switch (img_type) {
	case IMG_BOOT:
		ofs = parts[part_id]->offset;
#ifdef CONFIG_S5PC1XX
		/* Workaround: for prevent revision mismatch */
		if (cpu_is_s5pc110() && (down_mode != MODE_FORCE)) {
			int img_rev = 1;
			long *img_header = (long *)down_ram_addr;

			if (*img_header == 0xea000012)
				img_rev = 0;
			else if (*(img_header + 0x400) == 0xea000012)
				img_rev = 2;

			if (img_rev != s5p_get_cpu_rev()) {
				printf("CPU revision mismatch!\n"
					"bootloader is %s%s\n"
					"download image is %s%s\n",
					s5p_get_cpu_rev() ? "EVT1" : "EVT0",
					s5p_get_cpu_rev() == 2 ? "-Fused" : "",
					img_rev ? "EVT1" : "EVT0",
					img_rev == 2 ? "-Fused" : "");
				return -1;
			}
		}
#endif

		erase_env_area(usbd);

		sprintf(offset, "%x", (uint)ofs);
		if (ofs != 0)
			sprintf(length, "%x", parts[part_id]->size - (uint)ofs);
		else
			sprintf(length, "%x", parts[part_id]->size);

		/* Erase */
		nand_cmd(0, offset, length, NULL);
		/* Write */
		sprintf(length, "%x", (unsigned int) len);
		ret = nand_cmd(1, ramaddr, offset, length);
		break;

	case IMG_KERNEL:
		sprintf(offset, "%x", parts[part_id]->offset);
		sprintf(length, "%x", parts[part_id]->size);

		/* Erase */
		nand_cmd(0, offset, length, NULL);
		/* Write */
		sprintf(length, "%x", (unsigned int) len);
		ret = nand_cmd(1, ramaddr, offset, length);

		erase_qboot_area();
		break;

	/* File Systems */
	case IMG_FILESYSTEM:
		ret = write_file_system(ramaddr, len, offset, length, part_id);

		erase_qboot_area();
		break;

	case IMG_MODEM:
		sprintf(offset, "%x", parts[part_id]->offset);
		sprintf(length, "%x", parts[part_id]->size);

		/* Erase */
		if (!arg)
			nand_cmd(0, offset, length, NULL);
		else
			printf("CSA Clear will be skipped temporary\n");

		/* Check ubi image, 0x23494255 is UBI# */
		{
			long *img_header = (long *)down_ram_addr;

			if (*img_header == 0x23494255)
				goto ubi_img;
		}

#ifdef CONFIG_UBIFS_MK
		void *dest_addr = (void *) down_ram_addr + 0xc00000;
		void *src_addr = (void *) down_ram_addr;
		int leb_size, max_leb_cnt, mkfs_min_io_size;
		unsigned long ubifs_dest_size, ubi_dest_size;
#ifdef CONFIG_S5PC110
		mkfs_min_io_size = 4096;
		leb_size = 248 * 1024;
		max_leb_cnt = 4096;
#elif CONFIG_S5PC210
		mkfs_min_io_size = 2048;
		leb_size = 126 * 1024;
		max_leb_cnt = 4096;
#endif
		printf("Start making ubifs\n");
		ret = mkfs(src_addr, len, dest_addr, &ubifs_dest_size,
			   mkfs_min_io_size, leb_size, max_leb_cnt);
		if (ret) {
			printf("Error : making ubifs failed\n");
			goto out;
		}
		printf("Complete making ubifs\n");
#endif

#ifdef CONFIG_UBINIZE
		int peb_size, ubi_min_io_size, subpage_size, vid_hdr_offs;
#ifdef CONFIG_S5PC110
		ubi_min_io_size = 4096;
		peb_size = 256 * 1024;
		subpage_size = 4096;
		vid_hdr_offs = 0;
#elif CONFIG_S5PC210
		ubi_min_io_size = 2048;
		peb_size = 128 * 1024;
		subpage_size = 512;
		vid_hdr_offs = 512;
#endif
		printf("Start ubinizing\n");
		ret = ubinize(dest_addr, ubifs_dest_size,
			      src_addr, &ubi_dest_size,
			      peb_size, ubi_min_io_size,
			      subpage_size, vid_hdr_offs);
		if (ret) {
			printf("Error : ubinizing failed\n");
			goto out;
		}
		printf("Complete ubinizing\n");

		len = (unsigned int) ubi_dest_size;
#endif

ubi_img:
		/* Write : arg (0 Modem) / (1 CSA) */
		if (!arg) {
			sprintf(length, "%x", (unsigned int) len);
			ret = nand_cmd(1, ramaddr, offset, length);
		}
out:
		break;

#ifdef CONFIG_CMD_MMC
	case IMG_MMC:
		if (check_mmc_device(usbd))
			return -1;

		if (mmc_part_write)
			ret = write_file_mmc_part(usbd, len);
		else
			ret = write_file_mmc(usbd, len);

		erase_qboot_area();
		break;
#endif
	default:
		/* Retry? */
		write_part--;
	}
#endif
	return ret;
}

static int write_v2_image(struct usbd_ops *usbd, int img_type,
		unsigned int len, unsigned int arg)
{
	int ret;

	if (check_mmc_device(usbd))
		return -1;

	switch (img_type) {
	case IMG_V2:
#ifdef CONFIG_CMD_MBR
		ret = write_mmc_image(usbd, len, part_id);
#endif
		break;

	case IMG_MBR:
#ifdef CONFIG_CMD_MBR
		set_mbr_info((char *)down_ram_addr, len);
#endif
		break;

	case IMG_BOOTLOADER:
#ifdef CONFIG_BOOTLOADER_SECTOR
		erase_env_area(usbd);

		ret = mmc_cmd(OPS_WRITE, usbd->mmc_dev,
				CONFIG_BOOTLOADER_SECTOR,
				len / usbd->mmc_blk + 1,
				(void *)down_ram_addr);
#endif
		break;

	case IMG_KERNEL_V2:
		ret = write_fat_file(usbd, "uImage", part_id, len);
		break;

	case IMG_MODEM_V2:
		ret = write_fat_file(usbd, "modem.bin", part_id, len);
		break;

	default:
		/* Retry? */
		write_part--;
	}

	return ret;
}

/* Parsing received data packet and Process data */
static int process_data(struct usbd_ops *usbd)
{
	unsigned int cmd = 0, arg = 0, len = 0, flag = 0;
	char ramaddr[12];
	int recvlen = 0;
	unsigned int blocks = 0;
	int ret = 0;
	int ubi_mode = 0;
	int img_type = -1;

	sprintf(ramaddr, "0x%x", (uint) down_ram_addr);

	/* Parse command */
	cmd  = *((ulong *) usbd->rx_data + 0);
	arg  = *((ulong *) usbd->rx_data + 1);
	len  = *((ulong *) usbd->rx_data + 2);
	flag = *((ulong *) usbd->rx_data + 3);

	/* Reset tx buffer */
	memset(usbd->tx_data, 0, sizeof(usbd->tx_data));

	ubi_mode = check_ubi_mode();

	switch (cmd) {
	case COMMAND_DOWNLOAD_IMAGE:
		printf("\nCOMMAND_DOWNLOAD_IMAGE\n");

		if (arg)
			down_ram_addr = usbd->ram_addr + 0x1000000;
		else
			down_ram_addr = usbd->ram_addr;

		usbd->recv_setup((char *)down_ram_addr, (int)len);
		printf("Download to 0x%08x, %d bytes\n",
				(uint)down_ram_addr, (int)len);

		/* response */
		send_ack(usbd, STATUS_DONE);

		/* Receive image by using dma */
		recvlen = usbd->recv_data();
		if (recvlen < 0) {
			send_ack(usbd, STATUS_ERROR);
			return -1;
		} else if (recvlen < len) {
			printf("Error: wrong image size -> %d/%d\n",
					(int)recvlen, (int)len);

			/* Retry this commad */
			send_ack(usbd, STATUS_RETRY);
		} else
			send_ack(usbd, STATUS_DONE);

		return 1;

	case COMMAND_DOWNLOAD_SPMODE:
		printf("\nCOMMAND_DOWNLOAD_SPMODE\n");

		down_ram_addr = usbd->ram_addr + 0x2008000;

		usbd->recv_setup((char *)down_ram_addr, (int)len);
		printf("Download to 0x%08x, %d bytes\n",
				(uint)down_ram_addr, (int)len);

		/* response */
		send_ack(usbd, STATUS_DONE);

		/* Receive image by using dma */
		recvlen = usbd->recv_data();
		send_ack(usbd, STATUS_DONE);

		return 0;

	/* Report partition info */
	case COMMAND_PARTITION_SYNC:
		part_id = arg;

#ifdef CONFIG_CMD_UBI
		if (ubi_mode) {
			if (part_id == RAMDISK_PART_ID ||
			    part_id == FILESYSTEM_PART_ID ||
			    part_id == FILESYSTEM2_PART_ID) {
				/* change to yaffs style */
				get_part_info();
			}
		} else {
			if (part_id == FILESYSTEM3_PART_ID) {
				/* change ubi style */
				get_part_info();
			}
		}
#endif

		if (part_id == FILESYSTEM3_PART_ID)
			part_id = get_part_id("UBI");
		else if (part_id == MODEM_PART_ID)
			part_id = get_part_id("modem");
		else if (part_id == KERNEL_PART_ID)
			part_id = get_part_id("kernel");
		else if (part_id == BOOT_PART_ID)
			part_id = get_part_id("bootloader");
#ifdef CONFIG_MIRAGE
		if (part_id)
			part_id--;
#endif
		printf("COMMAND_PARTITION_SYNC - Part%d\n", part_id);

#ifdef CONFIG_CMD_MTDPARTS
		blocks = parts[part_id]->size / 1024 / 128;
#endif
		printf("COMMAND_PARTITION_SYNC - Part%d, %d blocks\n",
				part_id, blocks);

		send_ack(usbd, blocks);
		return 1;

	case COMMAND_WRITE_PART_0:
		/* Do nothing */
		printf("COMMAND_WRITE_PART_0\n");
		return 1;

	case COMMAND_WRITE_PART_1:
		printf("COMMAND_WRITE_PART_BOOT\n");
		part_id = get_part_id("bootloader");
		img_type = IMG_BOOT;
		break;

	case COMMAND_WRITE_PART_2:
	case COMMAND_ERASE_PARAMETER:
		printf("COMMAND_PARAMETER - not support!\n");
		break;

	case COMMAND_WRITE_PART_3:
		printf("COMMAND_WRITE_KERNEL\n");
		part_id = get_part_id("kernel");
		img_type = IMG_KERNEL;
		break;

	case COMMAND_WRITE_PART_4:
		printf("COMMAND_WRITE_ROOTFS\n");
		part_id = get_part_id("Root");
		img_type = IMG_FILESYSTEM;
		ubi_update = arg;
		break;

	case COMMAND_WRITE_PART_5:
		printf("COMMAND_WRITE_FACTORYFS\n");
		part_id = get_part_id("Fact");
		img_type = IMG_FILESYSTEM;
		ubi_update = arg;
		break;

	case COMMAND_WRITE_PART_6:
		printf("COMMAND_WRITE_DATAFS\n");
		part_id = get_part_id("Data");
		img_type = IMG_FILESYSTEM;
		ubi_update = arg;
		break;

	case COMMAND_WRITE_PART_7:
		printf("COMMAND_WRITE_UBI\n");
		part_id = get_part_id("UBI");
		img_type = IMG_FILESYSTEM;
		ubi_update = 0;
		/* someday, it will be deleted */
		get_part_info();
		break;

	case COMMAND_WRITE_PART_8:
		printf("COMMAND_WRITE_MODEM\n");
		part_id = get_part_id("modem");
		img_type = IMG_MODEM;
		break;

#ifdef CONFIG_CMD_MMC
	case COMMAND_WRITE_PART_9:
		printf("COMMAND_WRITE_MMC\n");
		img_type = IMG_MMC;
		mmc_part_write = arg;
		break;
#endif

	case COMMAND_WRITE_IMG_0:
		printf("COMMAND_WRITE_MBR\n");
		img_type = IMG_MBR;
		break;

	case COMMAND_WRITE_IMG_1:
		printf("COMMAND_WRITE_BOOTLOADER\n");
		img_type = IMG_BOOTLOADER;
		break;

	case COMMAND_WRITE_IMG_2:
		printf("COMMAND_WRITE_KERNEL\n");
		img_type = IMG_KERNEL_V2;
		part_id = 2;
		break;

	case COMMAND_WRITE_IMG_3:
		printf("COMMAND_WRITE_MODEM\n");
		img_type = IMG_MODEM_V2;
		part_id = 2;
		break;

	case COMMAND_WRITE_IMG_4:
		printf("COMMAND_WRITE_BOOT_PART\n");
		part_id = 1;
		img_type = IMG_V2;
		break;

	case COMMAND_WRITE_IMG_5:
		printf("COMMAND_WRITE_SYSTEM_PART\n");
		part_id = 2;
		img_type = IMG_V2;
		break;

	case COMMAND_WRITE_IMG_6:
		printf("COMMAND_WRITE_UMS_PART\n");
		part_id = 4;
		img_type = IMG_V2;
		break;

	case COMMAND_WRITE_UBI_INFO:
		printf("COMMAND_WRITE_UBI_INFO\n");

		if (ubi_mode) {
#ifdef CONFIG_CMD_UBI
			part_id = arg-1;
			sprintf(length, "0x%x", (uint)len);
			ret = ubi_cmd(part_id, ramaddr, length, "begin");
		} else {
#endif
			printf("Error: Not UBI mode\n");
			ret = 1;
		}

		/* Write image success -> Report status */
		send_ack(usbd, ret);

		return !ret;
	/* Download complete -> reset */
	case COMMAND_RESET_PDA:
		printf("\nDownload finished and Auto reset!\nWait........\n");

		/* Stop USB */
		usbd->usb_stop();

		if (usbd->cpu_reset)
			usbd->cpu_reset();
		else
			run_command("reset", 0);

		return 0;

	/* Error */
	case COMMAND_RESET_USB:
		printf("\nError is occured!(maybe previous step)->\
				Turn off and restart!\n");

		/* Stop USB */
		usbd->usb_stop();
		return -1;

	case COMMAND_RAM_BOOT:
		usbd->usb_stop();
		boot_cmd(ramaddr);
		return 0;

	case COMMAND_RAMDISK_MODE:
		printf("COMMAND_RAMDISK_MODE\n");
#ifdef CONFIG_RAMDISK_ADDR
		if (arg) {
			down_ram_addr = usbd->ram_addr;
		} else {
			down_ram_addr = CONFIG_RAMDISK_ADDR;
			run_command("run ramboot", 0);
		}
#endif
		return 1;

#ifdef CONFIG_DOWN_PHONE
	case COMMAND_DOWN_PHONE:
		printf("COMMAND_RESET_PHONE\n");

		usbd_phone_down();

		send_ack(usbd, STATUS_DONE);
		return 1;

	case COMMAND_CHANGE_USB:
		printf("COMMAND_CHANGE_USB\n");

		/* Stop USB */
		usbd->usb_stop();

		usbd_path_change();

		run_command("reset", 0);
		return 0;
#endif
	case COMMAND_CSA_CLEAR:
		printf("COMMAND_CSA_CLEAR\n");
		part_id = get_part_id("csa");
		img_type = IMG_MODEM;
		break;

	case COMMAND_PROGRESS:
		if (usbd->set_progress)
			usbd->set_progress(arg);
		return 1;

	default:
		printf("Error: Unknown command -> (%d)\n", (int)cmd);
		return 1;
	}

	if (img_type < IMG_V2)
		ret = write_mtd_image(usbd, img_type, len, arg);
	else
		ret = write_v2_image(usbd, img_type, len, arg);

	if (ret < 0) {
		send_ack(usbd, STATUS_ERROR);
		return -1;
	} else if (ret) {
		/* Retry this commad */
		send_ack(usbd, STATUS_RETRY);
		return 1;
	} else
		send_ack(usbd, STATUS_DONE);

	write_part++;

	/* Reset write count for another image */
	if (flag) {
		write_part = 0;
		fs_offset = 0;
	}

	return 1;
}

static const char *recv_key = "SAMSUNG";
static const char *tx_key = "MPL";

int do_usbd_down(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct usbd_ops *usbd;
	int err;
	int ret;

	if (argc > 1)
		down_mode = simple_strtoul(argv[1], NULL, 10);
	else
		down_mode = MODE_NORMAL;

	printf("USB Downloader v%s\n", APP_VERSION);

	/* get partition info */
	err = get_part_info();
	if (err)
		return err;

	/* interface setting */
	usbd = usbd_set_interface(&usbd_ops);
	down_ram_addr = usbd->ram_addr;

#ifdef CONFIG_CMD_MBR
	/* get mbr info */
	mbr_parts = get_mbr_table(mbr_offset);
	if (!mbr_parts)
		run_command("mbr default", 0);
#endif

	/* init the usb controller */
	if (!usbd->usb_init()) {
		usbd->down_cancel(END_BOOT);
		return 0;
	}
	mmc = find_mmc_device(usbd->mmc_dev);
	mmc_init(mmc);
#ifdef CONFIG_MMC_ASYNC_WRITE
	mmc_async_on(mmc, 1);
#endif

	/* receive setting */
	usbd->recv_setup(usbd->rx_data, usbd->rx_len);

	/* detect the download request from Host PC */
	ret = usbd->recv_data();
	if (ret > 0) {
		if (strncmp(usbd->rx_data, recv_key, strlen(recv_key)) == 0) {
			printf("Download request from the Host PC\n");
			msleep(30);

			strcpy(usbd->tx_data, tx_key);
			usbd->send_data(usbd->tx_data, usbd->tx_len);
		} else {
			printf("No download request from the Host PC!! 1\n");
			return 0;
		}
	} else if (ret < 0) {
		usbd->down_cancel(END_RETRY);
		return 0;
	} else {
		usbd->down_cancel(END_BOOT);
		return 0;
	}

	usbd->down_start();
	printf("Receive the packet\n");

	/* receive the data from Host PC */
	while (1) {
		usbd->recv_setup(usbd->rx_data, usbd->rx_len);

		ret = usbd->recv_data();
		if (ret > 0) {
			ret = process_data(usbd);
			if (ret < 0) {
				usbd->down_cancel(END_RETRY);
				return 0;
			} else if (ret == 0) {
				usbd->down_cancel(END_NORMAL);
				return 0;
			}
		} else if (ret < 0) {
			usbd->down_cancel(END_RETRY);
			return 0;
		} else {
			usbd->down_cancel(END_BOOT);
			return 0;
		}
	}

	return 0;
}

U_BOOT_CMD(usbdown, CONFIG_SYS_MAXARGS, 1, do_usbd_down,
	"Initialize USB device and Run USB Downloader (specific)",
	"- normal mode\n"
	"usbdown mode - specific mode (0: NORAML, 1: FORCE)"
);
