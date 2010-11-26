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

struct mbr {
	char code_area[440];
	char disk_signature[4];
	char nulls[2];
	struct mbr_partition parts[4];
	unsigned short signature;
};

static inline int get_cylinder(char chs1, char chs2, int *sector)
{
	*sector = chs1 & 0x3f;

	return ((chs1 & 0xc0) << 2) | chs2;
}

static int extended_lba;

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
	ret = mmc_read_blocks(mmc, buf, lba , 1);
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
	ret = mmc_read_blocks(mmc, msg, lba, 1);

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

	ret = mmc_read_blocks(mmc, buf, 0, 1);
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

static int do_mbr(cmd_tbl_t *cmdtp, int flag, int argc, const char *argv[])
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
