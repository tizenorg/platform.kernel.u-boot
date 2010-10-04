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
#define APP_VERSION	"1.5.3"

#define OPS_READ	0
#define OPS_WRITE	1

#ifdef CONFIG_CMD_MTDPARTS
#include <jffs2/load_kernel.h>
static struct part_info *parts[16];
#endif

static const char pszMe[] = "usbd: ";

static struct usbd_ops usbd_ops;

static unsigned int part_id;
static unsigned int write_part = 0;
static unsigned long fs_offset = 0x0;

#ifdef CONFIG_USE_YAFFS
static unsigned int yaffs_len = 0;
static unsigned char yaffs_data[2112];
#define YAFFS_PAGE 2112
#endif

#define NAND_PAGE_SIZE 2048

static unsigned long down_ram_addr;

static int down_mode;

/* cpu/${CPU} dependent */
extern void do_reset(void);

/* common commands */
extern int do_bootm(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_run(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);

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

#ifdef CONFIG_MTD_PARTITIONS
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
	printf("Error: Can't get patition information\n");
	return -EINVAL;
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

static void run_cmd(char *cmd)
{
	char *argv[] = { "run", cmd };
	do_run(NULL, 0, 2, argv);
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
#include <fat.h>

extern int do_mmcops(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

static int mmc_cmd(int ops, char *p1, char *p2, char *p3)
{
	int ret;

	if (ops) {
		char *argv[] = {"mmc", "write", "0", p1, p2, p3};
		ret = do_mmcops(NULL, 0, 6, argv);
	} else {
		char *argv[] = {"mmc", "read", "0", p1, p2, p3};
		ret = do_mmcops(NULL, 0, 6, argv);
	}

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
	u8			res[112];	/* reserved */
} __attribute__((packed));

struct partition_table {
	u8	boot_flag;
	u8	chs_begin[3];
	u8	type_code;
	u8	chs_end[3];
	u32	lba_begin;
	u32	num_sectors;
} __attribute__((packed));

struct mbr_table {
	u8			boot_code[446];
	struct partition_table	partition[4];
	u16			signature;
} __attribute__((packed));

struct mul_partition_info {
	u32 lba_begin;
	u32 num_sectors;
} __attribute__((packed));

#define MBR_OFFSET	0x10

struct partition_header part_info;
struct mbr_table mbr_info;
struct mul_partition_info mul_info[EXTEND_MAX_PART];

static int write_mmc_partition(struct usbd_ops *usbd, u32 *ram_addr, u32 len)
{
	unsigned int blocks;
	char offset[12], length[12], ramaddr[12];
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

		sprintf(length, "%x", cnt);
		sprintf(offset, "%x", cur_blk_offset);
		sprintf(ramaddr, "0x%x", *ram_addr);
		mmc_cmd(OPS_WRITE, ramaddr, offset, length);

		cur_blk_offset += cnt;

		*ram_addr += (cnt * usbd->mmc_blk);
	}

	return ret;
}

static int write_file_mmc(struct usbd_ops *usbd, char *ramaddr, u32 len,
		char *offset, char *length)
{
	u32 ram_addr;
	int i;
	int ret;
	struct mbr_table *mbr;

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
		mbr = (struct mbr_table*)ram_addr;
		mbr_blk_size = mbr->partition[0].lba_begin;

		if (mbr->partition[0].type_code != EXTEND_PART_TYPE) {
			part_mode = NORMAL_PARTITION;

			/* modify sectors of p1 */
			mbr->partition[0].num_sectors = usbd->mmc_total -
					(mbr_blk_size +
					mbr->partition[1].num_sectors +
					mbr->partition[2].num_sectors +
					mbr->partition[3].num_sectors);

			mmc_parts++;

			/* modify lba_begin of p2 and p3 and p4 */
			for (i = 1; i < 4; i++) {
				if (part_info.partition[i].size == 0)
					break;

				mmc_parts++;
				mbr->partition[i].lba_begin =
					mbr->partition[i - 1].lba_begin +
					mbr->partition[i - 1].num_sectors;
			}

			/* copy MBR */
			memcpy(&mbr_info, mbr, sizeof(struct mbr_table));

			printf("Total Size: 0x%08x #parts %d\n",
					(unsigned int)usbd->mmc_total,
					mmc_parts);
			for (i = 0; i < mmc_parts; i++) {
				printf("p%d\t0x%08x\t0x%08x\n", i + 1,
					mbr_info.partition[i].lba_begin,
					mbr_info.partition[i].num_sectors);
			}

			/* write MBR */
			sprintf(length, "%x", mbr_blk_size);
			sprintf(offset, "%x", 0);
			sprintf(ramaddr, "0x%x", (u32)ram_addr);
			ret = mmc_cmd(OPS_WRITE, ramaddr, offset, length);

			ram_addr += mbr_blk_size * usbd->mmc_blk;
			len -= mbr_blk_size * usbd->mmc_blk;

			cur_blk_offset = mbr_blk_size;
			cur_part = 0;
			cur_part_size = part_info.partition[0].size;

			/* modify p1's total sector */
			bs = (boot_sector *)ram_addr;
			bs->total_sect = mbr_info.partition[0].num_sectors;

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
				usbd->mmc_total - mbr_blk_size * 2;
			for (i = 1; i < mmc_parts; i++) {
				mul_info[i].num_sectors =
					part_info.partition[i].res;
			}

			for (i = 1; i < mmc_parts; i++) {
				mul_info[0].num_sectors -=
					mul_info[i].num_sectors;
			}

			mul_info[0].lba_begin = mbr->partition[0].lba_begin;
			for (i = 1; i < mmc_parts; i++) {
				mul_info[i].lba_begin =
					mul_info[i-1].lba_begin	+
					mul_info[i-1].num_sectors;
			}

			/* modify main MBR */
			mbr->partition[0].num_sectors =
				usbd->mmc_total - mbr_blk_size;

			/* modify MBR data of p1 */
			mbr = (struct mbr_table *)
				(ram_addr + mbr_blk_size * usbd->mmc_blk);

			mbr->partition[0].num_sectors =
				mul_info[0].num_sectors - mbr_blk_size;
			mbr->partition[1].lba_begin =
				mul_info[1].lba_begin - mbr_blk_size;

			/* modify BPB data of p1 */
			bs = (boot_sector *)
				((u32)mbr + mbr_blk_size * usbd->mmc_blk);
			memset(&bs->sectors, 0, 2);
			bs->total_sect = mbr->partition[0].num_sectors;

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
					mbr_info.partition[cur_part].lba_begin;
			} else {
				cur_blk_offset =
					mul_info[cur_part].lba_begin;
				/* modify MBR */
				if (cur_part <= mmc_parts) {
					mbr = (struct mbr_table *)ram_addr;
					mbr->partition[1].lba_begin =
						mul_info[cur_part+1].lba_begin -
						mbr->partition[0].lba_begin;
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

static int write_file_mmc_part(struct usbd_ops *usbd, char *ramaddr, u32 len,
		char *offset, char *length)
{
	u32 ram_addr;
	u32 ofs;
	int i;
	struct mbr_table *mbr;
	u32 mbr_blk_size;

	ram_addr = (u32)down_ram_addr;
	mbr = &mbr_info;

	if (cur_blk_offset == 0) {
		/* read MBR */
		sprintf(length, "%x", (unsigned int)
			(sizeof(struct mbr_table)/usbd->mmc_blk));
		sprintf(offset, "%x", 0);
		sprintf(ramaddr, "0x%x", (u32)mbr);
		mmc_cmd(OPS_READ, ramaddr, offset, length);

		mbr_blk_size = mbr->partition[0].lba_begin;

		if (mbr->partition[0].type_code != EXTEND_PART_TYPE) {
			part_mode = NORMAL_PARTITION;

			for (i = 0; i < 4; i++) {
				printf("p%d\t0x%08x\t0x%08x\n", i + 1,
					mbr_info.partition[i].lba_begin,
					mbr_info.partition[i].num_sectors);
			}

			cur_blk_offset =
				mbr->partition[mmc_part_write - 1].lba_begin;
			cur_part = mmc_part_write - 1;
			cur_part_size = len;

			if (mmc_part_write == 1) {
				ram_addr += sizeof(struct mbr_table);
				cur_blk_offset += sizeof(struct mbr_table) /
					usbd->mmc_blk;
				len -= sizeof(struct mbr_table);
			}
		} else {
			part_mode = EXTEND_PARTITION;

			mbr_blk_size = mbr->partition[0].lba_begin;

			for (i = 1; i < mmc_part_write; i++) {
				ofs = mbr->partition[0].lba_begin +
					mbr->partition[1].lba_begin;
				printf("P%d start blk: 0x%x, size: 0x%x\n", i,
					ofs, mbr->partition[0].num_sectors);
				sprintf(length, "%x", (unsigned int)
					(sizeof(struct mbr_table) /
					usbd->mmc_blk));
				sprintf(offset, "%x", ofs);
				sprintf(ramaddr, "0x%x", (u32)mbr);
				mmc_cmd(OPS_READ, ramaddr, offset, length);
			}

			ofs = mbr->partition[0].lba_begin +
				mbr->partition[1].lba_begin;

			ofs += mbr_blk_size; /* skip MBR */
			cur_blk_offset = ofs;
			cur_part = mmc_part_write - 1;
			cur_part_size = len;

			if (mmc_part_write == 1) {
				boot_sector *bs;
				u32 total_sect;
				u8 *tmp;
				/* modify BPB data of p1 */
				sprintf(length, "%x", (unsigned int)
					(sizeof(struct mbr_table) /
					usbd->mmc_blk));
				sprintf(offset, "%x", cur_blk_offset);
				sprintf(ramaddr, "0x%x", (u32)mbr);
				mmc_cmd(OPS_READ, ramaddr, offset, length);

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

static int write_file_system(char *ramaddr, ulong len, char *offset,
		char *length, int part_num, int ubi_update)
{
#ifdef CONFIG_USE_YAFFS
	int actual_len = 0;
	int yaffs_write = 0;
#endif
	int ret = 0;

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

#ifdef CONFIG_USE_YAFFS
	/* if using yaffs, wirte size must be 2112*X
	 * so, must have to realloc, and writing */
	if (!strcmp("yaffs", getenv(parts[part_num]->name))) {
		yaffs_write = 1;

		memcpy((void *)down_ram_addr, yaffs_data, yaffs_len);

		actual_len = len + yaffs_len;
		yaffs_len = actual_len % YAFFS_PAGE;
		len = actual_len - yaffs_len;

		memset(yaffs_data, 0x00, YAFFS_PAGE);
		memcpy(yaffs_data, (char *)down_ram_addr + len, yaffs_len);
	}
#endif

	sprintf(offset, "0x%x", (uint)(parts[part_num]->offset + fs_offset));
	sprintf(length, "0x%x", (uint)len);

#ifdef CONFIG_USE_YAFFS
	if (yaffs_write)
		ret = nand_cmd(2, ramaddr, offset, length);
	else
		ret = nand_cmd(1, ramaddr, offset, length);

	if (!strcmp("yaffs", getenv(parts[part_num]->name)))
		fs_offset += len / YAFFS_PAGE * NAND_PAGE_SIZE;
	else
		fs_offset += len;

#else
	fs_offset += len;
	ret = nand_cmd(1, ramaddr, offset, length);
#endif

	return ret;
}

static int qboot_erase = 0;

/* Erase the qboot */
static void erase_qboot_area(void)
{
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
}

/* Parsing received data packet and Process data */
static int process_data(struct usbd_ops *usbd)
{
	ulong cmd = 0, arg = 0, ofs = 0, len = 0, flag = 0;
	char offset[12], length[12], ramaddr[12];
	int recvlen = 0;
	unsigned int blocks = 0;
	int ret = 0;
	int ubi_update = 0;
	int ubi_mode = 0;
	int img_type;

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

#ifdef CONFIG_USE_YAFFS
		usbd->recv_setup((char *)down_ram_addr + yaffs_len, (int)len);
		printf("Download to 0x%08x, %d bytes\n",
				(uint)down_ram_addr + yaffs_len, (int)len);
#else
		usbd->recv_setup((char *)down_ram_addr, (int)len);
		printf("Download to 0x%08x, %d bytes\n",
				(uint)down_ram_addr, (int)len);
#endif
		/* response */
		usbd->send_data(usbd->tx_data, usbd->tx_len);

		/* Receive image by using dma */
		recvlen = usbd->recv_data();
		if (recvlen < len) {
			printf("Error: wrong image size -> %d/%d\n",
					(int)recvlen, (int)len);

			/* Retry this commad */
			*((ulong *) usbd->tx_data) = STATUS_RETRY;
		} else
			*((ulong *) usbd->tx_data) = STATUS_DONE;

		usbd->send_data(usbd->tx_data, usbd->tx_len);
		return 1;

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
#ifdef CONFIG_MIRAGE
		if (part_id)
			part_id--;
#endif
		printf("COMMAND_PARTITION_SYNC - Part%d\n", part_id);

		blocks = parts[part_id]->size / 1024 / 128;
		printf("COMMAND_PARTITION_SYNC - Part%d, %d blocks\n",
				part_id, blocks);

		*((ulong *) usbd->tx_data) = blocks;
		usbd->send_data(usbd->tx_data, usbd->tx_len);
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

		*((ulong *) usbd->tx_data) = ret;
		/* Write image success -> Report status */
		usbd->send_data(usbd->tx_data, usbd->tx_len);

		return !ret;
	/* Download complete -> reset */
	case COMMAND_RESET_PDA:
		printf("\nDownload finished and Auto reset!\nWait........\n");

		/* Stop USB */
		usbd->usb_stop();

		if (usbd->cpu_reset)
			usbd->cpu_reset();
		else
			do_reset();

		return 0;

	/* Error */
	case COMMAND_RESET_USB:
		printf("\nError is occured!(maybe previous step)->\
				Turn off and restart!\n");

		/* Stop USB */
		usbd->usb_stop();
		return 0;

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
			run_cmd("ramboot");
		}
#endif
		return 1;

#ifdef CONFIG_DOWN_PHONE
	case COMMAND_DOWN_PHONE:
		printf("COMMAND_RESET_PHONE\n");

		usbd_phone_down();

		*((ulong *) usbd->tx_data) = STATUS_DONE;
		usbd->send_data(usbd->tx_data, usbd->tx_len);
		return 1;

	case COMMAND_CHANGE_USB:
		printf("COMMAND_CHANGE_USB\n");

		/* Stop USB */
		usbd->usb_stop();

		usbd_path_change();

		do_reset();
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

	/* Erase and Write to NAND */
	switch (img_type) {
	case IMG_BOOT:
		ofs = parts[part_id]->offset;
#ifdef CONFIG_RECOVERY
	{
		/* block is fixed:
			1m = ipl(16k)+recovery(240k)+bootloader(768k)*/
		long *buf = (long *)down_ram_addr;
		u32 ofst;
		u32 bootloader_edge = parts[part_id]->size;
		u32 bootloader_addr = bootloader_edge >> 2;
		u32 recovery_edge = bootloader_addr;
		u32 recovery_addr = recovery_edge >> 4;
		u32 ipl_addr = 0;

		if (len > bootloader_addr) {
			ofst = bootloader_addr / sizeof(buf);
			if (*(buf + ofst) == 0xea000012) {
				/* case: ipl + recovery + bootloader */
				printf("target: ipl + recovery + loader\n");
				ofs = ipl_addr;
			} else {
				/* case: unknown format */
				printf("target: unknown\n");
				*((ulong *) usbd->tx_data) = STATUS_ERROR;
				usbd->send_data(usbd->tx_data, usbd->tx_len);
				return 0;
			}
		} else {
			ofst = recovery_addr/sizeof(buf);
			if (*(buf + ofst) == 0xea000012 &&
				*(buf + ofst - 1) == 0x00000000) {
				/* case: ipl + bootloader (old type) */
				printf("target: ipl + bootloader\n");
				ofs = ipl_addr;
			} else {
				/* case: bootloader only */
				printf("target: bootloader\n");
				ofs = bootloader_addr;

				/* skip revision check */
				down_mode = MODE_FORCE;
			}
		}

		sprintf(offset, "%x", (uint)ofs);
		sprintf(length, "%x", parts[part_id]->size);

		/* check block is locked/locked-tight */
		ret = nand_cmd(3, offset, length, NULL);
		if (ret) {
			printf("target is locked%s\n",
				(ret == 1) ? "-tight" : "");
			printf("-> try at recovery mode "
				"to update 'system'.\n");
			printf("   how-to: reset "
				"while pressing volume up and down.\n");
			*((ulong *) usbd->tx_data) = STATUS_ERROR;
			usbd->send_data(usbd->tx_data, usbd->tx_len);
			return 0;
		}
	}
#endif
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
				*((ulong *) usbd->tx_data) = STATUS_ERROR;
				usbd->send_data(usbd->tx_data, usbd->tx_len);
				return 0;
			}
		}
#endif
#if defined(CONFIG_ENV_IS_IN_NAND) || defined(CONFIG_ENV_IS_IN_ONENAND)
		/* Erase the environment also when write bootloader */
		{
			int param_id;
			param_id = get_part_id("params");

			if (param_id == -1) {
				sprintf(offset, "%x", CONFIG_ENV_ADDR);
				sprintf(length, "%x", CONFIG_ENV_SIZE);
			} else {
				sprintf(offset, "%x", parts[param_id]->offset);
				sprintf(length, "%x", parts[param_id]->size);
			}
			nand_cmd(0, offset, length, NULL);
		}
#endif
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
		ret = write_file_system(ramaddr, len, offset, length,
				part_id, ubi_update);

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

#ifdef CONFIG_UBIFS_MK
#include <mkfs.ubifs.h>
		void *dest_addr;
		void *src_addr = (void *) down_ram_addr;
		int leb_size, max_leb_cnt, mkfs_min_io_size;
#ifdef CONFIG_S5PC110
		mkfs_min_io_size = 4096;
		leb_size = 248 * 1024;
		max_leb_cnt = 4096;
#elif CONFIG_S5PC210
		mkfs_min_io_size = 2048;
		leb_size = 126 * 1024;
		max_leb_cnt = 4096;
#endif
		unsigned long ubifs_dest_size, ubi_dest_size;
		printf("Start making ubifs\n");
		ret = mkfs(src_addr, len, &dest_addr, &ubifs_dest_size,
			   mkfs_min_io_size, leb_size, max_leb_cnt);
		if (ret) {
			printf("Error : making ubifs failed\n");
			goto out;
		}
		printf("Complete making ubifs\n");
#endif

#ifdef CONFIG_UBINIZE
#include <ubinize.h>
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

		free(dest_addr);
#endif
		/* Write : arg (0 Modem) / (1 CSA) */
		if (!arg) {
			sprintf(length, "%x", (unsigned int) len);
			ret = nand_cmd(1, ramaddr, offset, length);
		}
out:
		break;

#ifdef CONFIG_CMD_MMC
	case IMG_MMC:

		if (mmc_part_write)
			ret = write_file_mmc_part(usbd, ramaddr,
						len, offset, length);
		else
			ret = write_file_mmc(usbd, ramaddr,
						len, offset, length);

		erase_qboot_area();
		break;
#endif

	default:
		/* Retry? */
		write_part--;
	}

	if (ret) {
		/* Retry this commad */
		*((ulong *) usbd->tx_data) = STATUS_RETRY;
		usbd->send_data(usbd->tx_data, usbd->tx_len);
		return 1;
	} else
		*((ulong *) usbd->tx_data) = STATUS_DONE;

	/* Write image success -> Report status */
	usbd->send_data(usbd->tx_data, usbd->tx_len);

	write_part++;

	/* Reset write count for another image */
	if (flag) {
		write_part = 0;
		fs_offset = 0;
#ifdef CONFIG_USE_YAFFS
		yaffs_len = 0;
#endif
	}

	return 1;
}

static const char *recv_key = "SAMSUNG";
static const char *tx_key = "MPL";

int do_usbd_down(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct usbd_ops *usbd;
	int err;

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

	/* init the usb controller */
	usbd->usb_init();

	/* receive setting */
	usbd->recv_setup(usbd->rx_data, usbd->rx_len);

	/* detect the download request from Host PC */
	if (usbd->recv_data()) {
		if (strncmp(usbd->rx_data, recv_key, strlen(recv_key)) == 0) {
			printf("Download request from the Host PC\n");
			msleep(30);

			strcpy(usbd->tx_data, tx_key);
			usbd->send_data(usbd->tx_data, usbd->tx_len);
		} else {
			printf("No download request from the Host PC!! 1\n");
			return 0;
		}
	} else {
		printf("No download request from the Host PC!!\n");
		return 0;
	}

	printf("Receive the packet\n");

	/* receive the data from Host PC */
	while (1) {
		usbd->recv_setup(usbd->rx_data, usbd->rx_len);

		if (usbd->recv_data()) {
			if (process_data(usbd) == 0)
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
