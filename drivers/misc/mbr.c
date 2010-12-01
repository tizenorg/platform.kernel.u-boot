/*
 * Copyright (C) 2010 Samsung Electrnoics
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <mmc.h>

struct mbr_partition {
	char status;
	char f_chs[3];
	char partition_type;
	char l_chs[3];
	int lba;
	int nsectors;
} __attribute__((packed));

#define SIGNATURE	((unsigned short) 0xAA55)

static int logical = 4;
static int extended_lba;

struct mbr {
	char code_area[440];
	char disk_signature[4];
	char nulls[2];
	struct mbr_partition parts[4];
	unsigned short signature;
};

static inline void set_chs_value(struct mbr_partition *part, int part_num)
{
	/* FIXME */
	if (part_num == 0) {
		part->f_chs[0] = 0x01;
		part->f_chs[1] = 0x01;
		part->f_chs[2] = 0x00;
	} else {
		part->f_chs[0] = 0x03;
		part->f_chs[1] = 0xd0;
		part->f_chs[2] = 0xff;
	}
	part->l_chs[0] = 0x03;
	part->l_chs[1] = 0xd0;
	part->l_chs[2] = 0xff;
}

void set_mbr_table(unsigned int start_addr, int parts,
		unsigned int *blocks, unsigned int *part_offset)
{
	struct mmc *mmc = find_mmc_device(0);
	struct mbr mbr_table;
	unsigned int offset = start_addr;
	unsigned int max = 0;
	unsigned int size = 0;
	int i, j;

	mmc_init(mmc);
	max = mmc->capacity / mmc->read_bl_len;

	memset(&mbr_table, 0, sizeof(struct mbr));

	mbr_table.signature = SIGNATURE;

	if (blocks[parts - 1] == 0) {
		size = start_addr;
		for (i = 0; i < parts; i++)
			size += blocks[i];
		blocks[parts - 1] = max - size;
	}

	/* Primary */
	for (i = 0; i < 3; i++) {
		mbr_table.parts[i].partition_type = 0x83;	/* Linux */
		mbr_table.parts[i].lba = offset;
		mbr_table.parts[i].nsectors = blocks[i];
		part_offset[i] = offset;
		offset += blocks[i];

		set_chs_value(&mbr_table.parts[i], i);
	}

	if (parts < 4) {
		mmc->block_dev.block_write(0, 0, 1, &mbr_table);
		return;
	}

	/* Extended */
	mbr_table.parts[i].partition_type = 0x05;	/* Extended */
	mbr_table.parts[i].lba = offset;
	mbr_table.parts[i].nsectors = max - offset;
	set_chs_value(&mbr_table.parts[i], i);

	mmc->block_dev.block_write(0, 0, 1, &mbr_table);

	for (; i < parts; i++) {
		struct mbr ebr_table;
		memset(&ebr_table, 0, sizeof(struct mbr));

		ebr_table.signature = SIGNATURE;

		for (j = 0; j < 2; j++) {
			if (j == 0) {
				blocks[parts - 1] -= 0x10;
				ebr_table.parts[j].partition_type = 0x83;
				ebr_table.parts[j].lba = 0x10;
				ebr_table.parts[j].nsectors = blocks[i];
				set_chs_value(&ebr_table.parts[j], i);
			} else {
				if (parts != i + 1) {
					ebr_table.parts[j].partition_type = 0x05;
					ebr_table.parts[j].lba = 0x10 + blocks[i];
					ebr_table.parts[j].nsectors = blocks[i + 1];
					set_chs_value(&ebr_table.parts[j], i);
				}
			}
		}

		mmc->block_dev.block_write(0, offset, 1, &ebr_table);

		offset += ebr_table.parts[0].lba;
		part_offset[i] = offset;
		offset += ebr_table.parts[0].nsectors;
	}
}

static int get_ebr_table(struct mmc *mmc, struct mbr_partition *mp,
		int ebr_next, unsigned int *part_offset, int parts)
{
	struct mbr *ebr;
	struct mbr_partition *p;
	char buf[512];
	int ret, i;
	int lba = 0;

	if (ebr_next)
		lba = extended_lba;

	lba += mp->lba;
	ret = mmc->block_dev.block_read(0, lba, 1, buf);
	if (ret != 1) {
		printf("%s[%d] mmc_read_blocks %d\n", __func__, __LINE__, ret);
		return 0;
	}
	ebr = (struct mbr *) buf;

	if (ebr->signature != SIGNATURE) {
		printf("Signature error 0x%x\n", ebr->signature);
		return 0;
	}

	for (i = 0; i < 2; i++) {
		p = (struct mbr_partition *) &ebr->parts[i];

		if (i == 0) {
			logical++;
			lba += p->lba;
			if (p->partition_type == 0x83) {
				part_offset[parts] = lba;
				parts++;
			}
		}
	}

	if (p->lba && p->partition_type == 0x5)
		parts = get_ebr_table(mmc, p, 1, part_offset, parts);

	return parts;
}

int get_mbr_table(unsigned int *part_offset)
{
	struct mmc *mmc = find_mmc_device(0);
	struct mbr_partition *mp;
	struct mbr *mbr;
	char buf[512];
	int ret, i;
	int parts = 0;

	mmc_init(mmc);

	ret = mmc->block_dev.block_read(0, 0, 1, buf);
	if (ret != 1) {
		printf("%s[%d] mmc_read_blocks %d\n", __func__, __LINE__, ret);
		return 0;
	}

	mbr = (struct mbr *) buf;

	if (mbr->signature != SIGNATURE)
		printf("Signature error 0x%x\n", mbr->signature);

	logical = 4;

	for (i = 0; i < 4; i++) {
		mp = (struct mbr_partition *) &mbr->parts[i];

		if (!mp->partition_type)
			continue;

		if (mp->partition_type == 0x83) {
			part_offset[parts] = mp->lba;
			parts++;
		}

		if (mp->lba && mp->partition_type == 0x5) {
			extended_lba = mp->lba;
			parts = get_ebr_table(mmc, mp, 0, part_offset, parts);
		}
	}

	return parts;
}

static inline int get_cylinder(char chs1, char chs2, int *sector)
{
	*sector = chs1 & 0x3f;

	return ((chs1 & 0xc0) << 2) | chs2;
}

static void ebr_show(struct mmc *mmc, struct mbr_partition *mp, int ebr_next)
{
	struct mbr *ebr;
	struct mbr_partition *p;
	char buf[512], msg[512];
	int ret, i, sector, cylinder;
	int lba = 0;

	if (ebr_next)
		lba = extended_lba;

	lba += mp->lba;
	printf(">>> Read sector from 0x%08x (LBA: 0x%08x + 0x%x)\n",
		lba, mp->lba, (lba == mp->lba) ? 0 : lba - mp->lba);

	ret = mmc->block_dev.block_read(0, lba, 1, buf);
	if (ret != 1) {
		printf("%s[%d] mmc_read_blocks %d\n", __func__, __LINE__, ret);
		return;
	}
	ebr = (struct mbr *) buf;

	if (ebr->signature != SIGNATURE) {
		printf("Signature error 0x%x\n", ebr->signature);
		return;
	}

	for (i = 0; i < 2; i++) {
		p = (struct mbr_partition *) &ebr->parts[i];

		if (i == 0) {
			logical++;
			printf("Extended Part %d\n", logical);
		} else
			printf("Extended Part next\n");

		printf("status   0x%02x\n", p->status);
		printf("head     0x%02x,", p->f_chs[0]);
		cylinder = get_cylinder(p->f_chs[1], p->f_chs[2], &sector);
		printf("\tsector   0x%02x,", sector);
		printf("\tcylinder 0x%04x\n", cylinder);
		printf("type     0x%02x\n", p->partition_type);
		printf("head     0x%02x,", p->l_chs[0]);
		cylinder = get_cylinder(p->l_chs[1], p->l_chs[2], &sector);
		printf("\tsector   0x%02x,", sector);
		printf("\tcylinder 0x%04x\n", cylinder);
		printf("lba      0x%08x (%d), ", p->lba, p->lba);
		printf("nsectors 0x%08x (%d)\n", p->nsectors, p->nsectors);
	}

	lba += 16;
	ret = mmc->block_dev.block_read(0, lba, 1, msg);

	for (i = 0; i < 8; i++)
		putc(msg[i]);
	putc('\n');

	if (p->lba && p->partition_type == 0x5)
		ebr_show(mmc, p, 1);

}

static void mbr_show(void)
{
	struct mmc *mmc = find_mmc_device(0);
	struct mbr_partition *mp;
	struct mbr *mbr;
	char buf[512];
	int ret, i;
	int cylinder;

	mmc_init(mmc);

	ret = mmc->block_dev.block_read(0, 0, 1, buf);
	if (ret != 1) {
		printf("%s[%d] mmc_read_blocks %d\n", __func__, __LINE__, ret);
		return;
	}

	mbr = (struct mbr *) buf;

	if (mbr->signature != SIGNATURE)
		printf("Signature error 0x%x\n", mbr->signature);

	logical = 4;
	printf("MBR partition info\n");
	for (i = 0; i < 4; i++) {
		mp = (struct mbr_partition *) &mbr->parts[i];

		if (!mp->partition_type)
			continue;

		printf("Part %d\n", i + 1);
		printf("status   0x%02x\n", mp->status);
		printf("head     0x%02x,", mp->f_chs[0]);
		printf("\tsector   0x%02x,", mp->f_chs[1] & 0x3f);
		cylinder = (mp->f_chs[1] & 0xc0) << 2;
		cylinder |= mp->f_chs[2];
		printf("\tcylinder 0x%04x\n", cylinder);
		printf("type     0x%02x\n", mp->partition_type);
		printf("head     0x%02x,", mp->l_chs[0]);
		printf("\tsector   0x%02x,", mp->l_chs[1] & 0x3f);
		cylinder = (mp->l_chs[1] & 0xc0) << 2;
		cylinder |= mp->l_chs[2];
		printf("\tcylinder 0x%04x\n", cylinder);
		printf("lba      0x%08x (%d), ", mp->lba, mp->lba);
		printf("nsectors 0x%08x (%d)\n", mp->nsectors, mp->nsectors);

		if (mp->lba && mp->partition_type == 0x5) {
			extended_lba = mp->lba;
			ebr_show(mmc, mp, 0);
		}
	}
}

static int do_mbr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	switch (argc) {
	case 2:
		if (strncmp(argv[1], "show", 2) == 0) {
			mbr_show();
			break;
		}
	default:
		cmd_usage(cmdtp);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	mbr,	CONFIG_SYS_MAXARGS,	1, do_mbr,
	"Master Boot Record",
	"show - show MBR\n"
);
