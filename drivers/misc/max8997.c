/*
 * Command for max8997 / max8996 pmic
 *
 * Copyright (C) 2011 Samsung Electrnoics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <i2c.h>

unsigned int pmic_bus;

void pmic_bus_init(int bus_num)
{
	pmic_bus = bus_num;
}

static int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int buck = 0, ldo = 0, safeout = 0, on = -1;
	int ret = 0;

	switch (argc) {
	case 2:
		if (strncmp(argv[1], "status", 6) == 0)
			printf("Don't support\n");
			/* return pmic_status(); */
		break;
	case 4:
		if (strncmp(argv[1], "ldo", 3) == 0)
			ldo = simple_strtoul(argv[2], NULL, 10);
		else if (strncmp(argv[1], "buck", 4) == 0)
			buck = simple_strtoul(argv[2], NULL, 10);
		else if (strncmp(argv[1], "safeout", 7) == 0)
			safeout = simple_strtoul(argv[2], NULL, 10);
		else
			break;

		if (strncmp(argv[3], "on", 2) == 0)
			on = 1;
		else if (strncmp(argv[3], "off", 3) == 0)
			on = 0;
		else
			break;

		/* ret = pmic_ldo_control(buck, ldo, safeout, on); */

		if (!ret)
			printf("%s %s %s\n", argv[1], argv[2], argv[3]);
		return ret;

	default:
		break;
	}

	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	pmic,		4,	1, do_pmic,
	"PMIC LDO & BUCK control",
	"status - Display PMIC LDO & BUCK status\n"
	"pmic ldo num on/off - Turn on/off the LDO\n"
	"pmic buck num on/off - Turn on/off the BUCK\n"
	"pmic safeout num on/off - Turn on/off the SAFEOUT\n"
);
