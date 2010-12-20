/*
 * fat_write.c
 *
 * R/W (V)FAT 12/16/32 filesystem implementation by Donggun Kim
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
#include <config.h>
#include <fat.h>
#include <asm/byteorder.h>
#include <part.h>
#include "fat.c"

static int fragmented_count;

static void uppercase (char *str, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		TOUPPER(*str);
		str++;
	}
}

static int total_sector;
static int disk_write (__u32 startblock, __u32 getsize, __u8 * bufptr)
{
	if (cur_dev == NULL)
		return -1;

	if (startblock + getsize > total_sector) {
		printf("error: overflow occurs\n");
		return -1;
	}

	startblock += part_offset;

	if (cur_dev->block_read) {
		return cur_dev->block_write(cur_dev->dev, startblock, getsize,
					   (unsigned long *) bufptr);
	}
	return -1;
}

/*
 * Set short name in directory entry
 */
static void set_name (dir_entry *dirent, const char *filename)
{
	char s_name[256];
	char *period;
	int period_location, len, i, ext_num;

	if (filename == NULL)
		return;

	len = strlen(filename);
	if (len == 0)
		return;

	memcpy(s_name, filename, len);
	uppercase(s_name, len);

	period = strchr(s_name, '.');
	if (period == NULL) {
		period_location = len - 1;
		ext_num = 0;
	} else {
		period_location = period - s_name;
		ext_num = len - period_location - 1;
	}

	/* Pad spaces when file name length is shortter than eight */
	if (period_location < 8) {
		memcpy(dirent->name, s_name, period_location);
		for (i = period_location; i < 8; i++)
			dirent->name[i] = ' ';
	} else if (period_location == 8) {
		memcpy(dirent->name, s_name, period_location);
	} else {
		memcpy(dirent->name, s_name, 6);
		dirent->name[6] = '~';
		dirent->name[7] = '1';
	}

	if (ext_num < 3) {
		memcpy(dirent->ext, s_name + period_location + 1, ext_num);
		for (i = ext_num; i < 3; i++)
			dirent->name[i] = ' ';
	} else
		memcpy(dirent->ext, s_name + period_location + 1, 3);

	debug("name : %s\n", dirent->name);
	debug("ext : %s\n", dirent->ext);
}

/*
 * Write fat buffer into block device
 */
static int flush_fat_buffer(fsdata *mydata)
{
	int getsize = FATBUFSIZE / FS_BLOCK_SIZE;
	__u32 fatlength = mydata->fatlength;
	__u8 *bufptr = mydata->fatbuf;
	__u32 startblock = mydata->fatbufnum * FATBUFBLOCKS;

	fatlength *= SECTOR_SIZE;
	startblock += mydata->fat_sect;

	if (getsize > fatlength)
		getsize = fatlength;

	/* Write FAT buf */
	if (disk_write(startblock, getsize, bufptr) < 0) {
		debug("error: writing FAT blocks\n");
		return -1;
	}

	return 0;
}

/*
 * Get the entry at index 'entry' in a FAT (12/16/32) table.
 * On failure 0x00 is returned.
 * When bufnum is changed, write back the previous fatbuf to the disk.
 */
static __u32 get_fatent_value (fsdata *mydata, __u32 entry)
{
	__u32 bufnum;
	__u32 off16, offset;
	__u32 ret = 0x00;
	__u16 val1, val2;

	switch (mydata->fatsize) {
	case 32:
		bufnum = entry / FAT32BUFSIZE;
		offset = entry - bufnum * FAT32BUFSIZE;
		break;
	case 16:
		bufnum = entry / FAT16BUFSIZE;
		offset = entry - bufnum * FAT16BUFSIZE;
		break;
	case 12:
		bufnum = entry / FAT12BUFSIZE;
		offset = entry - bufnum * FAT12BUFSIZE;
		break;

	default:
		/* Unsupported FAT size */
		return ret;
	}

	debug("FAT%d: entry: 0x%04x = %d, offset: 0x%04x = %d\n",
	       mydata->fatsize, entry, entry, offset, offset);

	/* Read a new block of FAT entries into the cache. */
	if (bufnum != mydata->fatbufnum) {
		int getsize = FATBUFSIZE / FS_BLOCK_SIZE;
		__u8 *bufptr = mydata->fatbuf;
		__u32 fatlength = mydata->fatlength;
		__u32 startblock = bufnum * FATBUFBLOCKS;

		fatlength *= SECTOR_SIZE;	/* We want it in bytes now */
		startblock += mydata->fat_sect;	/* Offset from start of disk */

		if (getsize > fatlength)
			getsize = fatlength;
		/* Write back the fatbuf to the disk */
		if (mydata->fatbufnum != -1) {
			if (flush_fat_buffer(mydata) < 0)
				return -1;
		}

		if (disk_read(startblock, getsize, bufptr) < 0) {
			debug("Error reading FAT blocks\n");
			return ret;
		}
		mydata->fatbufnum = bufnum;
	}

	/* Get the actual entry from the table */
	switch (mydata->fatsize) {
	case 32:
		ret = FAT2CPU32(((__u32 *) mydata->fatbuf)[offset]);
		break;
	case 16:
		ret = FAT2CPU16(((__u16 *) mydata->fatbuf)[offset]);
		break;
	case 12:
		off16 = (offset * 3) / 4;

		switch (offset & 0x3) {
		case 0:
			ret = FAT2CPU16(((__u16 *) mydata->fatbuf)[off16]);
			ret &= 0xfff;
			break;
		case 1:
			val1 = FAT2CPU16(((__u16 *)mydata->fatbuf)[off16]);
			val1 &= 0xf000;
			val2 = FAT2CPU16(((__u16 *)mydata->fatbuf)[off16 + 1]);
			val2 &= 0x00ff;
			ret = (val2 << 4) | (val1 >> 12);
			break;
		case 2:
			val1 = FAT2CPU16(((__u16 *)mydata->fatbuf)[off16]);
			val1 &= 0xff00;
			val2 = FAT2CPU16(((__u16 *)mydata->fatbuf)[off16 + 1]);
			val2 &= 0x000f;
			ret = (val2 << 8) | (val1 >> 8);
			break;
		case 3:
			ret = FAT2CPU16(((__u16 *)mydata->fatbuf)[off16]);
			ret = (ret & 0xfff0) >> 4;
			break;
		default:
			break;
		}
		break;
	}
	debug("FAT%d: ret: %08x, entry: %08x, offset: %04x\n",
	       mydata->fatsize, ret, entry, offset);

	return ret;
}

#ifdef CONFIG_SUPPORT_VFAT
/*
 * Set the file name information from 'name' into 'slot',
 */
static int str2slot (dir_slot *slotptr, const char *name, int *idx)
{
	int j, end_idx = 0;

	for (j = 0; j <= 8; j += 2) {
		if (name[*idx] == 0x00) {
			slotptr->name0_4[j] = 0;
			slotptr->name0_4[j + 1] = 0;
			end_idx++;
			goto name0_4;
		}
		slotptr->name0_4[j] = name[*idx];
		(*idx)++;
		end_idx++;
	}
	for (j = 0; j <= 10; j += 2) {
		if (name[*idx] == 0x00) {
			slotptr->name5_10[j] = 0;
			slotptr->name5_10[j + 1] = 0;
			end_idx++;
			goto name5_10;
		}
		slotptr->name5_10[j] = name[*idx];
		(*idx)++;
		end_idx++;
	}
	for (j = 0; j <= 2; j += 2) {
		if (name[*idx] == 0x00) {
			slotptr->name11_12[j] = 0;
			slotptr->name11_12[j + 1] = 0;
			end_idx++;
			goto name11_12;
		}
		slotptr->name11_12[j] = name[*idx];
		(*idx)++;
		end_idx++;
	}

	if (name[*idx] == 0x00)
		return 1;

	return 0;
/* Not used characters are filled with 0xff 0xff */
name0_4:
	for (; end_idx < 5; end_idx++) {
		slotptr->name0_4[end_idx * 2] = 0xff;
		slotptr->name0_4[end_idx * 2 + 1] = 0xff;
	}
	end_idx = 5;
name5_10:
	end_idx -= 5;
	for (; end_idx < 6; end_idx++) {
		slotptr->name5_10[end_idx * 2] = 0xff;
		slotptr->name5_10[end_idx * 2 + 1] = 0xff;
	}
	end_idx = 11;
name11_12:
	end_idx -= 11;
	for (; end_idx < 2; end_idx++) {
		slotptr->name11_12[end_idx * 2] = 0xff;
		slotptr->name11_12[end_idx * 2 + 1] = 0xff;
	}

	return 1;
}

/*
 * Fill dir_slot entries with appropriate name, id, and attr
 * The real directory entry is returned by dentptr
 */
static void
fill_dir_slot (dir_entry **dentptr, const char *l_name)
{
	dir_slot *slotptr = (dir_slot *)get_vfatname_block;
	__u8 counter;
	int idx = 0, ret;

	do {
		memset(slotptr, 0x00, sizeof(dir_slot));
		ret = str2slot(slotptr, l_name, &idx);
		slotptr->id = ++counter;
		slotptr->attr = ATTR_VFAT;
		slotptr++;
	} while (ret == 0);

	slotptr--;
	slotptr->id |= LAST_LONG_ENTRY_MASK;

	while (counter >= 1) {
		memcpy(*dentptr, slotptr, sizeof(dir_slot));
		(*dentptr)++;
		slotptr--;
		counter--;
	}
}

/*
 * Fill alias_checksum in dir_slot with calculated checksum value
 */
static void fill_alias_checksum(dir_slot *slotptr, char *name)
{
	int i;
	__u8 counter = (slotptr->id & ~LAST_LONG_ENTRY_MASK) & 0xff;

	for (i = 0; i < counter; i++, slotptr++)
		slotptr->alias_checksum = mkcksum(name);
}

static int
get_long_file_name (fsdata *mydata, int curclust, __u8 *cluster,
	      dir_entry **retdent, char *l_name)
{
	dir_entry *realdent;
	dir_slot *slotptr = (dir_slot *)(*retdent);
	__u8 *nextclust = cluster + mydata->clust_size * SECTOR_SIZE;
	__u8 counter = (slotptr->id & ~LAST_LONG_ENTRY_MASK) & 0xff;
	int idx = 0;

	while ((__u8 *)slotptr < nextclust) {
		if (counter == 0)
			break;
		if (((slotptr->id & ~LAST_LONG_ENTRY_MASK) & 0xff) != counter)
			return -1;
		slotptr++;
		counter--;
	}

	if ((__u8 *)slotptr >= nextclust) {
		dir_slot *slotptr2;

		slotptr--;
		curclust = get_fatent_value(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return -1;
		}

		if (get_cluster(mydata, curclust, get_vfatname_block,
				mydata->clust_size * SECTOR_SIZE) != 0) {
			debug("Error: reading directory block\n");
			return -1;
		}

		slotptr2 = (dir_slot *)get_vfatname_block;
		while (slotptr2->id > 0x01)
			slotptr2++;

		/* Save the real directory entry */
		realdent = (dir_entry *)slotptr2 + 1;
		while ((__u8 *)slotptr2 >= get_vfatname_block) {
			slot2str(slotptr2, l_name, &idx);
			slotptr2--;
		}
	} else {
		/* Save the real directory entry */
		realdent = (dir_entry *)slotptr;
	}

	do {
		slotptr--;
		if (slot2str(slotptr, l_name, &idx))
			break;
	} while (!(slotptr->id & LAST_LONG_ENTRY_MASK));

	l_name[idx] = '\0';
	if (*l_name == DELETED_FLAG)
		*l_name = '\0';
	else if (*l_name == aRING)
		*l_name = DELETED_FLAG;
	downcase(l_name);

	/* Return the real directory entry */
	*retdent = realdent;

	return 0;
}

#endif
/*
 * Set the entry at index 'entry' in a FAT (16/32) table.
 */
static int set_fatent_value (fsdata *mydata, __u32 entry, __u32 entry_value)
{
	__u32 bufnum, offset;

	switch (mydata->fatsize) {
	case 32:
		bufnum = entry / FAT32BUFSIZE;
		offset = entry - bufnum * FAT32BUFSIZE;
		break;
	case 16:
		bufnum = entry / FAT16BUFSIZE;
		offset = entry - bufnum * FAT16BUFSIZE;
		break;
	default:
		/* Unsupported FAT size */
		return -1;
	}

	/* Read a new block of FAT entries into the cache. */
	if (bufnum != mydata->fatbufnum) {
		int getsize = FATBUFSIZE / FS_BLOCK_SIZE;
		__u8 *bufptr = mydata->fatbuf;
		__u32 fatlength = mydata->fatlength;
		__u32 startblock = bufnum * FATBUFBLOCKS;

		fatlength *= SECTOR_SIZE;
		startblock += mydata->fat_sect;

		if (getsize > fatlength)
			getsize = fatlength;

		if (mydata->fatbufnum != -1) {
			if (flush_fat_buffer(mydata) < 0)
				return -1;
		}

		if (disk_read(startblock, getsize, bufptr) < 0) {
			debug("Error reading FAT blocks\n");
			return -1;
		}
		mydata->fatbufnum = bufnum;
	}

	/* Set the actual entry */
	switch (mydata->fatsize) {
	case 32:
		((__u32 *) mydata->fatbuf)[offset] = cpu_to_le32(entry_value);
		break;
	case 16:
		((__u16 *) mydata->fatbuf)[offset] = cpu_to_le16(entry_value);
		break;
	default:
		return -1;
	}

	return 0;
}

/*
 * Determine the entry value at index 'entry' in a FAT (16/32) table
 */
static __u32 determine_fatent (fsdata *mydata, __u32 entry)
{
	__u32 next_fat, next_entry = entry + 1;

	while (1) {
		next_fat = get_fatent_value(mydata, next_entry);
		if (next_fat == 0) {
			set_fatent_value(mydata, entry, next_entry);
			break;
		}
		next_entry++;
	}
	debug("FAT%d: entry: %08x, entry_value: %04x\n",
	       mydata->fatsize, entry, next_entry);

	return next_entry;
}

/*
 * Write at most 'size' bytes from 'buffer' into the specified cluster.
 * Return 0 on success, -1 otherwise.
 */
static int
set_cluster (fsdata *mydata, __u32 clustnum, __u8 *buffer,
	     unsigned long size)
{
	int idx = 0;
	__u32 startsect;

	if (clustnum > 0) {
		startsect = mydata->data_begin +
				clustnum * mydata->clust_size;
	} else {
		startsect = mydata->rootdir_sect;
	}

	debug("gc - clustnum: %d, startsect: %d\n", clustnum, startsect);

	if (disk_write(startsect, size / FS_BLOCK_SIZE, buffer) < 0) {
		debug("Error writing data\n");
		return -1;
	}

	if (size % FS_BLOCK_SIZE) {
		__u8 tmpbuf[FS_BLOCK_SIZE];

		idx = size / FS_BLOCK_SIZE;
		buffer += idx * FS_BLOCK_SIZE;
		memcpy(tmpbuf, buffer, size % FS_BLOCK_SIZE);

		if (disk_write(startsect + idx, 1, tmpbuf) < 0) {
			debug("Error writing data\n");
			return -1;
		}

		return 0;
	}

	return 0;
}

/*
 * Find the first empty cluster starting from cluster #2
 */
static int find_empty_cluster(fsdata *mydata)
{
	__u32 fat_val, entry = 3;

	while (1) {
		fat_val = get_fatent_value(mydata, entry);
		if (fat_val == 0)
			break;
		entry++;
	}

	return entry;
}

/*
 * Set empty cluster from 'entry' to the end of a file
 */
static int clear_fatent(fsdata *mydata, __u32 entry)
{
	__u32 fat_val;

	while (1) {
		fat_val = get_fatent_value(mydata, entry);
		if (fat_val != 0)
			set_fatent_value(mydata, entry, 0);
		else
			break;

		if (fat_val == 0xfffffff || fat_val == 0xffff)
			break;

		entry = fat_val;
	}

	/* Flush fat buffer */
	if (flush_fat_buffer(mydata) < 0)
		return -1;

	return 0;
}

/*
 * Write at most 'maxsize' bytes from 'buffer' into
 * the file associated with 'dentptr'
 * Return the number of bytes read or -1 on fatal errors.
 */
static int
set_contents (fsdata *mydata, dir_entry *dentptr, __u8 *buffer,
	      unsigned long maxsize)
{
	unsigned long filesize = FAT2CPU32(dentptr->size), gotsize = 0;
	unsigned int bytesperclust = mydata->clust_size * SECTOR_SIZE;
	__u32 curclust = START(dentptr);
	__u32 endclust = 0, newclust = 0;
	unsigned long actsize;

	debug("Filesize: %ld bytes\n", filesize);

	if (maxsize > 0 && filesize > maxsize)
		filesize = maxsize;

	debug("%ld bytes\n", filesize);

	actsize = bytesperclust;
	endclust = curclust;
	do {
		/* search for consecutive clusters */
		while (actsize < filesize) {
			newclust = determine_fatent(mydata, endclust);

			if ((newclust - 1) != endclust)
				goto getit;

			if (CHECK_CLUST(newclust, mydata->fatsize)) {
				debug("curclust: 0x%x\n", newclust);
				debug("Invalid FAT entry\n");
				return gotsize;
			}
			endclust = newclust;
			actsize += bytesperclust;
		}
		/* actsize >= file size */
		actsize -= bytesperclust;
		/* set remaining clusters */
		if (set_cluster(mydata, curclust, buffer, (int)actsize) != 0) {
			debug("error: writing cluster\n");
			return -1;
		}

		/* set remaining bytes */
		gotsize += (int)actsize;
		filesize -= actsize;
		buffer += actsize;
		actsize = filesize;

		if (set_cluster(mydata, endclust, buffer, (int)actsize) != 0) {
			debug("error: writing cluster\n");
			return -1;
		}
		gotsize += actsize;

		/* Mark end of file in FAT */
		if (mydata->fatsize == 16)
			newclust = 0xffff;
		else if (mydata->fatsize == 32)
			newclust = 0xfffffff;
		set_fatent_value(mydata, endclust, newclust);

		return gotsize;
getit:
		if (set_cluster(mydata, curclust, buffer, (int)actsize) != 0) {
			debug("error: writing cluster\n");
			return -1;
		}
		gotsize += (int)actsize;
		filesize -= actsize;
		buffer += actsize;

		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			debug("Invalid FAT entry\n");
			return gotsize;
		}
		actsize = bytesperclust;
		curclust = endclust = newclust;
		fragmented_count++;
	} while (1);
}

/*
 * Fill dir_entry entry
 */
static void fill_dentry (fsdata *mydata, dir_entry *dentptr,
	const char *filename, __u32 start_cluster, __u32 size, __u8 attr)
{
	if (mydata->fatsize == 32)
		dentptr->starthi = cpu_to_le16(start_cluster & 0xffff0000);
	dentptr->start = cpu_to_le16(start_cluster & 0xffff);
	dentptr->size = cpu_to_le32(size);

	dentptr->attr = attr;

	set_name(dentptr, filename);
}

/* 
 * Check whether adding a file makes the file system to
 * exceed the size of the block device
 * Return 1 when overflow occurs, otherwise return 0
 */
static int check_overflow(fsdata *mydata, __u32 clustnum, unsigned long size)
{
	__u32 startsect, sect_num;

	if (clustnum > 0) {
		startsect = mydata->data_begin +
				clustnum * mydata->clust_size;
	} else {
		startsect = mydata->rootdir_sect;
	}

	sect_num = size / FS_BLOCK_SIZE;
	if (size % FS_BLOCK_SIZE)
		sect_num++;

	if (startsect + sect_num > total_sector)
		return 1;

	return 0;
}

static dir_entry *empty_dentptr = NULL;

/*
 * Find a directory entry based on filename or start cluster number
 * If the directory entry is not found,
 * the new position for writing a directory entry will be returned
 */
static dir_entry *find_directory_entry (fsdata *mydata, int startsect,
	char *filename, dir_entry *retdent, __u32 start, int find_name)
{
	__u16 prevcksum = 0xffff;
	__u32 curclust = startsect;

	debug("get_dentfromdir: %s\n", filename);

	while (1) {
		dir_entry *dentptr;

		int i;

		if (disk_read(curclust, mydata->clust_size,
			    get_dentfromdir_block) < 0) {
			printf("Error: reading directory block\n");
			return NULL;
		}

		dentptr = (dir_entry *)get_dentfromdir_block;

		for (i = 0; i < DIRENTSPERCLUST; i++) {
			char s_name[14], l_name[256];

			l_name[0] = '\0';
			if (dentptr->name[0] == DELETED_FLAG) {
				dentptr++;
				continue;
			}
			if ((dentptr->attr & ATTR_VOLUME)) {
#ifdef CONFIG_SUPPORT_VFAT
				if ((dentptr->attr & ATTR_VFAT) &&
				    (dentptr-> name[0] & LAST_LONG_ENTRY_MASK)) {
					prevcksum = ((dir_slot *)dentptr)->alias_checksum;
					get_long_file_name(mydata, curclust,
						     get_dentfromdir_block,
						     &dentptr, l_name);
					debug("vfatname: |%s|\n", l_name);
				} else
#endif
				{
					/* Volume label or VFAT entry */
					dentptr++;
					continue;
				}
			}
			if (dentptr->name[0] == 0) {
				debug("Dentname == NULL - %d\n", i);
				empty_dentptr = dentptr;
				return NULL;
			}
			if (find_name) {
				get_name(dentptr, s_name);

				if (strcmp(filename, s_name)
				    && strcmp(filename, l_name)) {
					debug("Mismatch: |%s|%s|\n", s_name, l_name);
					dentptr++;
					continue;
				}

				memcpy(retdent, dentptr, sizeof(dir_entry));

				debug("DentName: %s", s_name);
				debug(", start: 0x%x", START(dentptr));
				debug(", size:  0x%x %s\n",
				      FAT2CPU32(dentptr->size),
				      (dentptr->attr & ATTR_DIR) ? "(DIR)" : "");

				return dentptr;
			} else {
				if (start == START(dentptr))
					return dentptr;
				else
					dentptr++;
				continue;
			}
		}

		curclust = get_fatent_value(mydata, curclust);
		if (CHECK_CLUST(curclust, mydata->fatsize)) {
			debug("curclust: 0x%x\n", curclust);
			printf("Invalid FAT entry\n");
			return NULL;
		}
	}

	return NULL;
}

/*
 * Find a fregmented area for a file
 * An area is seperated by three parameters (start, middle, end)
 * The data for a file is located from middle to end
 * The data for other files is located from start + 1 to middle - 1
 */
static void find_fragmented_area (fsdata *mydata, __u32 *start,
			__u32 *middle, __u32 *end)
{
	__u32 entry = *start;
	__u32 fat_val;
	int fragmented = 0;

	while (1) {
		fat_val = get_fatent_value(mydata, entry);
		if (fat_val == 0xfffffff || fat_val == 0xffff) {
			*end = entry;
			break;
		}

		if ((fat_val - 1) != entry) {
			if (!fragmented) {
				*start = entry;
				*middle = fat_val;
				fragmented = 1;

			} else {
				*end = entry;
				break;
			}
		}

		entry = fat_val;
	}

	debug("start : %d, middle : %d, end : %d\n", *start, *middle, *end);
}

static void *memory_for_swap;

/*
 * Change location between the data for other files and the data for target file
 * Fetch the whole data of the fragmented area from a block device to memory
 * Write the target file data into the first part of the area
 * and write the data for other files into the second part of the area 
 */
static void swap_file_contents (fsdata *mydata,
		__u32 start, __u32 middle, __u32 end)
{
	int first_area_size, second_area_size, total_size;
	__u32 clust_num_first, clust_num_second;

	first_area_size = (middle - 1 - start) *
			mydata->clust_size * FS_BLOCK_SIZE;
	second_area_size = (end + 1 - middle) *
			mydata->clust_size * FS_BLOCK_SIZE;
	total_size = first_area_size + second_area_size;

	clust_num_first = start + 1;
	clust_num_second = clust_num_first + end + 1 - middle;

	memset(memory_for_swap, 0x0, 0x2800000); /* 40 MB */
	get_cluster(mydata, start + 1, memory_for_swap, total_size);

	set_cluster(mydata, clust_num_second,
		memory_for_swap, first_area_size);

	memory_for_swap += first_area_size;
	set_cluster(mydata, clust_num_first,
		memory_for_swap, second_area_size);

}

/*
 * Get the fat entry based on the fat entry value
 */
static __u32 get_reverse_fatent (fsdata *mydata, __u32 entry_value)
{
	__u32 entry = entry_value - 1;
	__u32 fat_val = get_fatent_value(mydata, entry);

	if (entry_value == 0)
		return 0;

	if (fat_val == entry_value) {
		debug("entry : %d, entry_value : %d\n", entry, entry_value);
		return entry;
	}

	entry = 3;
	while (1) {
		if (entry > entry_value) {
			entry = 1;
			break;
		}
		fat_val = get_fatent_value(mydata, entry);
		if (entry_value == fat_val)
			break;
		entry++;
	}

	debug("entry : %d, entry_value : %d\n", entry, entry_value);
	return entry;
}

/*
 * Swap the fat entry value for a target file
 * with the fat entry value for the other files
 */
static __u32 swap_fat_values (fsdata *mydata,
		__u32 start, __u32 middle, __u32 end)
{
	__u32 start_second = start + 1 + (end + 1 - middle);
	__u32 start_first = middle - 1;
	__u32 num_fatent_first = middle - (start + 1);
	__u32 num_fatent_second = (end + 1) - middle;
	__u32 reverse_fatent, new_fat_val;
	__u32 end_second_fatval = get_fatent_value(mydata, end);
	__u32 startsect = mydata->rootdir_sect;
	__u32 fat_val;
	dir_entry *dentptr;
	int i;

	for (i = 0; i < num_fatent_first; i++) {
		fat_val = get_fatent_value(mydata, start_first - i);

		if (fat_val == 0)
			continue;

		if (fat_val != start_first - i + 1)
			set_fatent_value(mydata, end - i, fat_val);

		reverse_fatent = get_reverse_fatent(mydata, start_first - i);

		new_fat_val = end - i;

		/* find directory entry to update the start cluster number */
		if (reverse_fatent == 1) {
			dentptr = find_directory_entry(mydata, startsect,
					"", dentptr, start_first - i, 0);
			if (dentptr == NULL) {
				printf("error : no matching dir entry\n");
				return 0;
			}

			if (mydata->fatsize == 32)
				dentptr->starthi =
				    cpu_to_le16(new_fat_val & 0xffff0000);
			dentptr->start = cpu_to_le16(new_fat_val & 0xffff);

			if (disk_write(startsect, mydata->clust_size,
				    get_dentfromdir_block) < 0) {
				printf("error: wrinting directory entry\n");
				return 0;
			}
		} else {
			if (reverse_fatent == (start_first - i - 1))
				set_fatent_value(mydata,
					new_fat_val - 1, new_fat_val);
			else
				set_fatent_value(mydata,
					reverse_fatent, new_fat_val);
		}
	}

	for (i = 0; i < num_fatent_second; i++)
		set_fatent_value(mydata, start + i, start + i + 1);
	set_fatent_value(mydata, start + i, end_second_fatval);

	flush_fat_buffer(mydata);

	return start_second;
}

/*
 * Remove fragments for a file
 */
static void defragment_file (fsdata *mydata, dir_entry *dentptr)
{
	__u32 start = 0, middle = 0, end = 0;

	start = end = START(dentptr);
	while (1) {
		find_fragmented_area(mydata, &start, &middle, &end);
		if (middle == 0)
			break;
		swap_file_contents(mydata, start, middle, end);
		swap_fat_values(mydata, start, middle, end);
		middle = 0;
	}
}

static int do_fat_write (const char *filename, void *buffer,
	unsigned long size)
{
	dir_entry *dentptr, *retdent;
	dir_slot *slotptr;
	__u32 startsect;
	__u32 start_cluster;
	boot_sector bs;
	volume_info volinfo;
	fsdata datablock;
	fsdata *mydata = &datablock;
	int cursect;
	int root_cluster, ret, name_len;
	char l_filename[256];

	fragmented_count = 0;

	if (read_bootsectandvi(&bs, &volinfo, &mydata->fatsize)) {
		debug("error: reading boot sector\n");
		return 0;
	}

	total_sector = bs.total_sect;
	if (total_sector == 0) {
		total_sector = next_part_offset - part_offset;
	}

	root_cluster = bs.root_cluster;

	if (mydata->fatsize == 32)
		mydata->fatlength = bs.fat32_length;
	else
		mydata->fatlength = bs.fat_length;

	mydata->fat_sect = bs.reserved;

	cursect = mydata->rootdir_sect
		= mydata->fat_sect + mydata->fatlength * bs.fats;

	mydata->clust_size = bs.cluster_size;

	if (mydata->fatsize == 32) {
		mydata->data_begin = mydata->rootdir_sect -
					(mydata->clust_size * 2);
	} else {
		int rootdir_size;

		rootdir_size = ((bs.dir_entries[1]  * (int)256 +
				 bs.dir_entries[0]) *
				 sizeof(dir_entry)) /
				 SECTOR_SIZE;
		mydata->data_begin = mydata->rootdir_sect +
					rootdir_size -
					(mydata->clust_size * 2);
	}

	mydata->fatbufnum = -1;
	startsect = mydata->rootdir_sect;

	if (disk_read(cursect, mydata->clust_size, do_fat_read_block) < 0) {
		printf("error: reading rootdir block\n");
		return -1;
	}
	dentptr = (dir_entry *) do_fat_read_block;

	memory_for_swap = buffer + size;

	name_len = strlen(filename);
	memcpy(l_filename, filename, name_len);
	downcase(l_filename);
	retdent = find_directory_entry(mydata, startsect,
				l_filename, dentptr, 0, 1);
	if (retdent) {
		/* Update file size and start_cluster in a directory entry */
		retdent->size = cpu_to_le32(size);
		start_cluster = FAT2CPU16(retdent->start);
		if (mydata->fatsize == 32)
			start_cluster |=
				(FAT2CPU16(retdent->starthi) << 16);

		ret = check_overflow(mydata, start_cluster, size);
		if (ret) {
			printf("error: %ld overflow\n", size);
			return 0;
		}

		ret = clear_fatent(mydata, start_cluster);
		if (ret < 0) {
			printf("error: clearing FAT entries\n");
			return 0;
		}

		ret = set_contents(mydata, retdent, buffer, size);
		if (ret < 0) {
			printf("error: writing contents\n");
			return 0;
		}

		/* Flush fat buffer */
		if (flush_fat_buffer(mydata) < 0)
			return 0;

		if (fragmented_count > 0) {
			defragment_file(mydata, retdent);
			retdent->size = cpu_to_le32(size);
		}

		/* Write directory table to device */
		if (disk_write(cursect, mydata->clust_size,
			    get_dentfromdir_block) < 0) {
			printf("error: wrinting directory entry\n");
			return 0;
		}
	} else {
		ret = start_cluster = find_empty_cluster(mydata);
		if (ret < 0) {
			printf("error: finding empty cluster\n");
			return 0;
		}

		ret = check_overflow(mydata, start_cluster, size);
		if (ret) {
			printf("error: %ld overflow\n", size);
			return 0;
		}

		slotptr = (dir_slot *)empty_dentptr;
		fill_dir_slot(&empty_dentptr, filename);

		/* Set attribute as archieve for regular file */
		fill_dentry(mydata, empty_dentptr, filename,
			start_cluster, size, 0x20);

		fill_alias_checksum(slotptr, empty_dentptr->name);

		ret = set_contents(mydata, empty_dentptr, buffer, size);
		if (ret < 0) {
			printf("error: writing contents\n");
			return 0;
		}

		/* Flush fat buffer */
		if (flush_fat_buffer(mydata) < 0)
			return 0;

		/* Write directory table to device */
		if (disk_write(cursect, mydata->clust_size,
			    get_dentfromdir_block) < 0) {
			printf("error: writing directory entry\n");
			return 0;
		}

		if (fragmented_count > 0) {
			defragment_file(mydata, empty_dentptr);
			empty_dentptr->size = cpu_to_le32(size);

			/* Write directory table to device */
			if (disk_write(cursect, mydata->clust_size,
				    get_dentfromdir_block) < 0) {
				printf("error: writing directory entry\n");
				return 0;
			}

		}
	}

	return 0;
}

int file_fat_write (const char *filename, void *buffer, unsigned long maxsize)
{
	printf("writing %s\n", filename);
	return do_fat_write(filename, buffer, maxsize);
}
