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

/* version of USB Downloader Application */
#define APP_VERSION	"1.2.7"
#define APP_DATE	"17 June 2009"

#ifdef CONFIG_CMD_MTDPARTS
#include <jffs2/load_kernel.h>
static struct part_info *parts[6];
#endif

static const char pszMe[] = "usbd: ";

static struct usbd_ops usbd_ops;

static unsigned int part_id = BOOT_PART_ID;
static unsigned int write_part = 0;
static unsigned long fs_offset = 0x0;

#ifdef CONFIG_USE_YAFFS
static unsigned int yaffs_len = 0;
static unsigned char yaffs_data[2112];
#define YAFFS_PAGE 2112
#endif

#define NAND_PAGE_SIZE 2048

static unsigned long down_ram_addr;

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

static int get_part_id(char *name, int id)
{
	int nparts = count_mtdparts();
	int i;

	for (i = 0; i < nparts; i++) {
		if (strncmp(parts[i]->name, name, 4) == 0)
			return i;
	}

	printf("Error: Unknown partition -> %s(%d)\n", name, id);
	return -1;
}
#else
static int get_part_info(void)
{
	printf("Error: Can't get patition information\n");
	return -EINVAL;
}

static int get_part_id(char *name, int id)
{
	return id;
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

int write_file_system(char *ramaddr, ulong len, char *offset,
		char *length, int part_num, int ubi_update)
{
#ifdef CONFIG_USE_YAFFS
	int actual_len = 0;
	int yaffs_write = 0;
#endif
	int ret = 0;
	int ubi_set = 0;

	if (part_num == FILESYSTEM3_PART_ID) {
		part_num = FILESYSTEM_PART_ID;
		ubi_set = 1;
	}

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

/* Parsing received data packet and Process data */
static int process_data(struct usbd_ops *usbd)
{
	ulong cmd = 0, arg = 0, len = 0, flag = 0;
	char offset[12], length[12], ramaddr[12];
	int recvlen = 0;
	int blocks = 0;
	int ret = 0;
	int ubi_update = 0;
	int ubi_mode = 0;

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
			*((ulong *) usbd->tx_data) = 1;
		} else
			*((ulong *) usbd->tx_data) = 0;

		usbd->send_data(usbd->tx_data, usbd->tx_len);
		return 1;

	/* Report number of partitions - NOT USE */
	case COMMAND_PARTITION_CHECK:
		printf("COMMAND_PARTITION_CHECK\n");
		return 1;

	/* Report partition info */
	case COMMAND_PARTITION_SYNC:
		part_id = arg;

		printf("COMMAND_PARTITION_SYNC - Part%d\n", part_id);
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

		if (part_id == FILESYSTEM3_PART_ID)
			part_id = get_part_id("UBI", FILESYSTEM_PART_ID);
#endif
#ifdef CONFIG_MIRAGE
		if (part_id)
			part_id--;
#endif

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
		part_id = get_part_id("boot", BOOT_PART_ID);
		break;

	case COMMAND_WRITE_PART_2:
	case COMMAND_ERASE_PARAMETER:
		printf("COMMAND_PARAMETER - not support!\n");
		break;

	case COMMAND_WRITE_PART_3:
		printf("COMMAND_WRITE_KERNEL\n");
		part_id = get_part_id("kernel", KERNEL_PART_ID);
		break;

	case COMMAND_WRITE_PART_4:
		printf("COMMAND_WRITE_ROOTFS\n");
		part_id = get_part_id("Root", RAMDISK_PART_ID);
		ubi_update = arg;
		break;

	case COMMAND_WRITE_PART_5:
		printf("COMMAND_WRITE_FACTORYFS\n");
		part_id = get_part_id("Fact", FILESYSTEM_PART_ID);
		ubi_update = arg;
		break;

	case COMMAND_WRITE_PART_6:
		printf("COMMAND_WRITE_DATAFS\n");
		part_id = get_part_id("Data", FILESYSTEM2_PART_ID);
		ubi_update = arg;
		break;

	case COMMAND_WRITE_PART_7:
		printf("COMMAND_WRITE_UBI\n");
		part_id = get_part_id("UBI", FILESYSTEM3_PART_ID);
		ubi_update = 0;
		/* someday, it will be deleted */
		get_part_info();
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

		*((ulong *) usbd->tx_data) = ret;
		/* Write image success -> Report status */
		usbd->send_data(usbd->tx_data, usbd->tx_len);

		return !ret;
	/* Download complete -> reset */
	case COMMAND_RESET_PDA:
		printf("\nDownload finished and Auto reset!\nWait........\n");

		/* Stop USB */
		usbd->usb_stop();

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

		*((ulong *) usbd->tx_data) = 0;
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

	default:
		printf("Error: Unknown command -> (%d)\n", (int)cmd);
		return 0;
	}

	/* Erase and Write to NAND */
	switch (part_id) {
	case BOOT_PART_ID:
#if defined(CONFIG_ENV_IS_IN_NAND) || defined(CONFIG_ENV_IS_IN_ONENAND)
		/* Erase Environment */
		sprintf(offset, "%x", CONFIG_ENV_OFFSET);
		sprintf(length, "%x", CONFIG_ENV_SIZE);
		nand_cmd(0, offset, length, NULL);
#endif
	case KERNEL_PART_ID:
		sprintf(offset, "%x", parts[part_id]->offset);
		sprintf(length, "%x", parts[part_id]->size);

		/* Erase */
		nand_cmd(0, offset, length, NULL);
		/* Write */
		ret = nand_cmd(1, ramaddr, offset, length);
		break;

	/* File System */
	case RAMDISK_PART_ID:		/* rootfs */
	case FILESYSTEM_PART_ID:	/* factoryfs */
	case FILESYSTEM2_PART_ID:	/* datafs */
	case FILESYSTEM3_PART_ID:	/* ubifs */
		ret = write_file_system(ramaddr, len, offset, length,
				part_id, ubi_update);
		break;

	default:
		/* Retry? */
		write_part--;
	}

	if (ret) {
		/* Retry this commad */
		*((ulong *) usbd->tx_data) = 1;
		usbd->send_data(usbd->tx_data, usbd->tx_len);
		return 1;
	} else
		*((ulong *) usbd->tx_data) = 0;

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

int do_usbd_down(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	struct usbd_ops *usbd;
	int err;

	printf("Start USB Downloader v%s (%s)\n", APP_VERSION, APP_DATE);

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

U_BOOT_CMD(usbdown, 3, 0, do_usbd_down,
	"usbdown - initialize USB device and ready to receive"
		" for Windows server (specific)\n",
	"[download address]\n"
);
