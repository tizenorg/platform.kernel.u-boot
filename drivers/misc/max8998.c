/*
 * Command for max8998 / lp3974 pmic
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

static int pmic_probe(void)
{
	unsigned char addr = 0xCC >> 1;

	i2c_set_bus_num(pmic_bus);

	if (i2c_probe(addr)) {
		puts("Can't found pmic\n");
		return 1;
	}

	return 0;
}

#ifdef CONFIG_SOFT_I2C_READ_REPEATED_START
#define i2c_read_func(addr, reg, alen, val, len)	\
			i2c_read_r(addr, reg, alen, val, len)
#else
#define i2c_read_func(addr, reg, alen, val, len)	\
			i2c_read(addr, reg, alen, val, len)
#endif

static int pmic_status(void)
{
	unsigned char addr, val[2];
	int reg, i;

	addr = 0xCC >> 1;

	if (pmic_probe())
		return -1;

	reg = 0x11;
	i2c_read_func(addr, reg, 1, val, 1);
	for (i = 7; i >= 4; i--)
		printf("BUCK%d %s\n", 7 - i + 1,
			val[0] & (1 << i) ? "on" : "off");
	for (; i >= 0; i--)
		printf("LDO%d %s\n", 5 - i,
			val[0] & (1 << i) ? "on" : "off");
	reg = 0x12;
	i2c_read_func(addr, reg, 1, val, 1);
	for (i = 7; i >= 0; i--)
		printf("LDO%d %s\n", 7 - i + 6,
			val[0] & (1 << i) ? "on" : "off");
	reg = 0x13;
	i2c_read_func(addr, reg, 1, val, 1);
	for (i = 7; i >= 4; i--)
		printf("LDO%d %s\n", 7 - i + 14,
			val[0] & (1 << i) ? "on" : "off");

	reg = 0xd;
	i2c_read_func(addr, reg, 1, val, 1);
	for (i = 7; i >= 6; i--)
		printf("SAFEOUT%d %s\n", 7 - i + 1,
			val[0] & (1 << i) ? "on" : "off");
	return 0;
}

static int pmic_ldo_control(int buck, int ldo, int safeout, int on)
{
	unsigned char addr, val[2];
	unsigned int reg, shift;

	if (ldo) {
		if (ldo < 2)
			return -1;
		if (ldo <= 5) {
			reg = 0x11;
			shift = 5 - ldo;
		} else if (ldo <= 13) {
			reg = 0x12;
			shift = 13 - ldo;
		} else if (ldo <= 17) {
			reg = 0x13;
			shift = 17 - ldo + 4;
		} else
			return -1;
	} else if (buck) {
		if (buck > 4)
			return -1;
		reg = 0x11;
		shift = 4 - buck + 4;
	} else if (safeout) {
		if (safeout > 3)
			return -1;
		reg = 0xd;
		shift = 8 - safeout;
	} else
		return -1;

	addr = 0xCC >> 1;

	if (pmic_probe())
		return -1;

	i2c_read_func(addr, reg, 1, val, 1);
	if (on)
		val[0] |= (1 << shift);
	else
		val[0] &= ~(1 << shift);
	i2c_write(addr, reg, 1, val, 1);
	i2c_read_func(addr, reg, 1, val, 1);

	return 0;
}

static int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int buck = 0, ldo = 0, safeout = 0, on = -1;
	int ret;

	switch (argc) {
	case 2:
		if (strncmp(argv[1], "status", 6) == 0)
			return pmic_status();
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

		ret = pmic_ldo_control(buck, ldo, safeout, on);

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
