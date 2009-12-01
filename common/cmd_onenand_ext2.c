/*
 * ext2 filesystem for onenand device.
 *
 * Copyright(C) 2009 Samsung Electronics
 * InKi Dae <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <malloc.h>
#include <lcd.h>
#include <linux/types.h>
#include <asm/io.h>
#include <asm/global_data.h>

#include <onenand_ext2.h>

#define IMAGE_BASE		(0x380000)
#define IMAGE_SIZE		(8 * 1024 * 1024)
#define ONENAND_READ_SIZE	(4 * 1024)
#define FRAMEBUFFER_SIZE	(480 * 800 * 4)

#define BOOT_BLOCK_SIZE		(1024)
#define DATA_BLOCK_UNIT		(512)
#define INODE_TABLE_ENTRY_SIZE	(128)

extern vidinfo_t panel_info;
extern int onenand_read(ulong off, char *buf, unsigned int *out_size);

static char *ext2_buf = NULL;

/* it indicates block size of ext2 filesystem formatted. */
unsigned int block_size_of_fs = 0;

unsigned int inode_block_size = 0;
unsigned int inode_table_location;

/* set system memory region stored with onenand region. */
static unsigned int allocate_ext2_buf(void)
{
	int addr = 0;
	unsigned int fb_size = 0;

	addr = (_bss_end + (PAGE_SIZE - 1)) & ~(PAGE_SIZE -1);
	fb_size = panel_info.vl_col * panel_info.vl_row *
		panel_info.vl_bpix / 8;
	addr -= IMAGE_SIZE + fb_size;

	return addr;
}

/* load onenand region into system memory. */
void load_onenand_to_ram(void)
{
	unsigned int i, out_size;
	char *buf = NULL;

	ext2_buf = (char *) allocate_ext2_buf();

	buf = malloc(ONENAND_READ_SIZE);

	for (i = 0; i < IMAGE_SIZE ; i+=ONENAND_READ_SIZE) {
		onenand_read(IMAGE_BASE + i, buf, &out_size);
		memcpy(ext2_buf + i, buf, ONENAND_READ_SIZE);
	}

	if (buf)
		free(buf);
}

static char *get_sblock(struct ext2_sblock *sblock)
{
	char *buf_sblock = NULL;

	if (ext2_buf == NULL) {
		dprint("ext2_buf is NULL.\n");
		return NULL;
	}

	/* first 1k is boot block. */
	buf_sblock = ext2_buf + BOOT_BLOCK_SIZE;

	/* get super block. */
	memcpy(sblock, buf_sblock, sizeof(struct ext2_sblock));

	dprint("total_inodes = %d, block_size = %d\n", sblock->total_inodes,
		sblock->log2_block_size);

	return buf_sblock;
}

static char *get_group_dec(struct ext2_block_group *group)
{
	char *buf_group = NULL;

	if (ext2_buf == NULL) {
		dprint("ext2_buf is NULL.\n");
		return NULL;
	}

	/* get group descriptor. */
	if (block_size_of_fs == 1024)
		/* boot block : 1k, super block : 1k. */
		buf_group = ext2_buf + block_size_of_fs * 2;
	else if (block_size_of_fs == 4096)
		/* boot block : 1k, super block : 3k. */
		buf_group = ext2_buf + block_size_of_fs;

	dprint("buf_group = 0x%8x\n", buf_group);

	memcpy(group, buf_group, sizeof(struct ext2_block_group));

	dprint("block_id = %d, inode_id = %d, inode_table_id = %d\n",
		group->block_id, group->inode_id, group->inode_table_id);

	return buf_group;
}

static char *get_root_inode_entry(struct ext2_inode *inode,
	struct ext2_block_group *group)
{
	char *buf_root_dir = NULL;

	if (ext2_buf == NULL) {
		dprint("ext2_buf is NULL.\n");
		return NULL;
	}

	if (group == NULL) {
		dprint("group is NULL.\n");
		return NULL;
	}

	/* get location of inode table. */
	inode_table_location = group->inode_table_id * block_size_of_fs;

	dprint("inode table location = %d\n", inode_table_location);

	/* 
	 * get inode entry of root directory.
	 *
	 * inode table has 128byte unit per entry and
	 * root inode is placed in second entry.
	 */
	buf_root_dir = ext2_buf + inode_table_location + INODE_TABLE_ENTRY_SIZE;
	memcpy(inode, buf_root_dir, sizeof(struct ext2_inode));

	/* get inode block size and used to find the end of directory entry. */
	inode_block_size = inode->blockcnt * DATA_BLOCK_UNIT;

	dprint("dir_blocks = 0x%8x, size = %d, blockcnt = %d\n", inode->b.blocks.dir_blocks[0],
		inode->size, inode->blockcnt);

	return buf_root_dir;
}

static char *get_root_dir_entry(struct ext2_dirent *dirent, struct ext2_inode *inode)
{
	char *buf_root_dir = NULL;

	if (ext2_buf == NULL) {
		dprint("ext2_buf is NULL.\n");
		return NULL;
	}

	if (inode == NULL) {
		dprint("inode is NULL.\n");
		return NULL;
	}

	/* get block number stored with data of inode. */
	buf_root_dir = ext2_buf + inode->b.blocks.dir_blocks[0] * block_size_of_fs;
	memcpy(dirent, buf_root_dir, sizeof(struct ext2_dirent));

	dprint("first entry name of root directory is name = %s\n",
		dirent->name);

	return buf_root_dir;
}

/* initialize ext2 filesystem and return pointer of root directory entry. */
char *mount_ext2fs(void)
{
	struct ext2_sblock sblock;
	struct ext2_block_group group;
	struct ext2_inode inode;
	struct ext2_dirent dirent;

	unsigned int need_block = 0;
	char *buf = NULL;

	buf = get_sblock(&sblock);
	if (buf == NULL) {
		dprint("sblock is NULL.\n");
		return NULL;
	}

	/* 
	 * in case that log2_block_size is 0, block_size is 1024,
	 * 2048 for 1 and 4096 for 2. 
	 */
	switch (sblock.log2_block_size + 1) {
	case 1:
		block_size_of_fs = 1024;
		break;
	case 2:
		block_size_of_fs = 2048;
		break;
	case 3:
		block_size_of_fs = 4096;
		break;
	default:
		block_size_of_fs = 4096;
		break;
	}

	dprint("block size of filesystem = %d\n", block_size_of_fs);

	/* get block size for inode table. */
	need_block = sblock.total_inodes * INODE_TABLE_ENTRY_SIZE /
		block_size_of_fs;

	dprint("need_block for inode table = %d\n", need_block);

	buf = get_group_dec(&group);
	if (buf == NULL) {
		dprint("group_dec is NULL.\n");
		return NULL;
	}

	buf = get_root_inode_entry(&inode, &group);
	if (buf == NULL) {
		dprint("root_inode_entry is NULL.\n");
		return NULL;
	}

	buf = get_root_dir_entry(&dirent, &inode);
	if (buf == NULL) {
		dprint("root_dir_entry is NULL.\n");
		return NULL;
	}

	return buf;
}

static int get_dir_entry(unsigned int *inode, struct ext2_dirent *dirent)
{
	static unsigned int tmp_pt = NULL;
	static unsigned first = 1;

	if (first) {
		tmp_pt = *inode;
		first = 0;
	}

	if ((*inode - tmp_pt) >= inode_block_size)
		return -1;

	memcpy(dirent, (char *) *inode, sizeof(struct ext2_dirent));
	*inode += dirent->direntlen;

	dirent->name[dirent->namelen] = '\0';

	return 0;
}

unsigned int find_file_ext2(unsigned int inode, const char *filename)
{
	struct ext2_dirent dirent;
	int ret;

	ret = get_dir_entry(&inode, &dirent);
	if (ret < 0) {
		dprint("failed to get directory entry.\n");
		return;
	}

	if ((strcmp(dirent.name, filename)) == 0)
		return dirent.inode;

	do {
		ret = get_dir_entry(&inode, &dirent);

		dprint("soure file = %s, dst file = %s, len = %d\n", filename,
			dirent.name, dirent.namelen);

		if ((strncmp(dirent.name, filename, dirent.namelen)) == 0)
			return dirent.inode;

	} while (ret == 0);

	dprint("failed to find file.\n");

	return -1;
}

/*
 * open inode table for filename.
 *
 * @f_inode : inode number for file.
 *
 * return value is inode number for data block.
 */
int open_file_ext2(unsigned int f_inode)
{
	unsigned int d_inode;

	d_inode = inode_table_location + (f_inode - 1) *
		INODE_TABLE_ENTRY_SIZE;

	dprint("f_inode = %d, d_inode = %d\n", f_inode, d_inode);

	return d_inode;
}

int get_filesize_ext2(unsigned int d_inode)
{
	struct ext2_inode inode;

	/* calculate location of system memory for data block inode. */
	d_inode = ext2_buf + d_inode;

	/* get inode table entry for file. */
	memcpy(&inode, (char *) d_inode, sizeof(struct ext2_inode));

	return inode.size;
}

int read_file_ext2(unsigned int d_inode, char *buf, unsigned int size)
{
	struct ext2_datablock d_block;
	struct ext2_inode inode;
	unsigned int d_block_addr, id_block_addr, read_size = 0, id_block_num;
	int i;

	/* calculate location of system memory for data block inode. */
	d_inode = ext2_buf + d_inode;

	/* get inode table entry for file. */
	memcpy(&inode, (char *) d_inode, sizeof(struct ext2_inode));

	/* get data block information for file. */
	memcpy(&d_block, (char *) &inode.b.blocks, sizeof(struct ext2_datablock));

	/* get real data from direct blocks. */
	for (i = 0; i < INDIRECT_BLOCKS; i++) {
		if (d_block.dir_blocks[i] > 0) {

			dprint("direct block[%d] num = %d\n", i, d_block.dir_blocks[i]);

			/* calculate real data address. */
			d_block_addr = (unsigned int) ext2_buf + d_block.dir_blocks[i] *
				block_size_of_fs;
			if (size > block_size_of_fs) {
				memcpy(buf, (char *) d_block_addr, block_size_of_fs);
				size -= block_size_of_fs;
				buf += block_size_of_fs;
				read_size += block_size_of_fs;

				if (size <= 0)
					return read_size;
			} else {
				memcpy(buf, (char *) d_block_addr, size);
				read_size += size;

				return read_size;
			}
		}
	}

	/* 
	 * get real data from 1-dim indirect blocks only in case that
	 * 2-dim indirect block has no block num. 
	 */
	if (d_block.indir_block > 0 && d_block.double_indir_block <= 0) {
		d_block_addr = (unsigned int) ext2_buf + d_block.indir_block *
			block_size_of_fs;
		dprint("1-dim indirect block address = 0x%8x\n",  d_block_addr);
		/* 1-dim indirect block has 1k block and the size per entry is 4byte. */
		for (i = 0; i < block_size_of_fs; i+=4) {
			/* get indirect block number. */
			memcpy(&id_block_num, (char *) d_block_addr + i, 4);

			dprint("1-dim indirect block[%d] num = %d\n", i, id_block_num);

			/* get real data from indirect block in case of hole. */
			if (id_block_num > 0) {
				/* calculate block number having real data. */
				id_block_addr = (unsigned int) ext2_buf + id_block_num *
					block_size_of_fs;

				if (size > block_size_of_fs) {
					memcpy(buf, (char *) id_block_addr, block_size_of_fs);
					size -= block_size_of_fs;
					buf += block_size_of_fs;
					read_size += block_size_of_fs;

					if (size <= 0)
						return read_size;
				} else {
					memcpy(buf, id_block_addr, size);
					read_size += size;

					return read_size;
				}
			}
		}
	}

	return read_size;
	/* to do */

	/*
	dprint("idir = %x\n",  (unsigned int) ext2_buf + d_block.indir_block * block_size_of_fs);
	*/
}

void ls_ext2(unsigned int inode)
{
	struct ext2_dirent dirent;
	int ret;

	ret = get_dir_entry(&inode, &dirent);
	if (ret < 0) {
		dprint("failed to get directory entry.\n");
		return;
	}

	dprint("inode = %d, name = %s\n", dirent.inode, dirent.name);

	do {
		ret = get_dir_entry(&inode, &dirent);
		dprint("inode = %d, name = %s\n", dirent.inode, dirent.name);
	} while (ret == 0);
}

void test_onenand_ext2(void)
{
	unsigned int inode, in_size, out_size;
	char *data;

	load_onenand_to_ram();

	inode = (unsigned int) mount_ext2fs();

	inode = find_file_ext2(inode, "s3cfb.c");
	inode = open_file_ext2(inode);
	in_size = get_filesize_ext2(inode);
	dprint("in size = %d\n", in_size);

	data = malloc(in_size);

	out_size = read_file_ext2(inode, data, in_size);
	dprint("out size = %d, data address = 0x%8x\n", out_size, data);

	free(data);

	/* ls test
	inode = (unsigned int) mount_ext2fs();
	ls_ext2(inode);
	*/
}
