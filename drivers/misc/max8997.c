/*
 * Command for max8997 / max8996 pmic
 *
 * Copyright (C) 2011 Samsung Electrnoics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

#include <common.h>
#include <i2c.h>
#include <pmic.h>

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
#define i2c_read(addr, reg, alen, val, len)	\
			i2c_read_r(addr, reg, alen, val, len)
#endif

/*
 * enable 0: disable charger
 * 600: 600mA
 * 500: 500mA
 */
void pmic_charger_en(int enable)
{
	unsigned char addr = 0xCC >> 1;
	unsigned char val[2];
	unsigned char fc;

	if (pmic_probe())
		return;

	if (!enable) {
		puts("Disable the charger.\n");
		i2c_read(addr, 0x51, 1, val, 1);
		val[0] &= ~(1 << 6);
		i2c_write(addr, 0x51, 1, val, 1);
		return;
	}

	fc = (enable - 200) / 50;
	fc = fc & 0xf; /* up to 950 mA */

	printf("Enable the charger @ %d mA\n", fc * 50 + 200);
	val[0] = fc;
	i2c_write(addr, 0x53, 1, val, 1);

	i2c_read(addr, 0x51, 1, val, 1);
	val[0] |= 1 << 6;
	i2c_write(addr, 0x51, 1, val, 1);
}

unsigned int pmic_get_irq(int irq)
{
	unsigned char addr, val[4];
	unsigned int ret;
	unsigned int reg;
	unsigned int shift;

	addr = 0xCC >> 1;
	if (pmic_probe())
		return 0;

	switch (irq) {
	case PWRONR:
		reg = 0;
		shift = 0;
		break;
	case PWRONF:
		reg = 0;
		shift = 1;
		break;
	case PWRON1S:
		reg = 0;
		shift = 3;
		break;
	default:
		return 0;
	}

	i2c_read(addr, 0x3, 1, val, 4);

	ret = val[reg] & (1 << shift);

	return !!ret;
}

static unsigned char reg_buck[7] = {0x18, 0x21, 0x2a, 0x2c, 0x3e, 0x37, 0x39};
static int pmic_status(void)
{
	unsigned char addr, val[2];
	int i;

	addr = 0xCC >> 1;

	if (pmic_probe())
		return -1;

	for (i = 0; i < 7; i++) {
		i2c_read(addr, reg_buck[i], 1, val, 1);
		printf("BUCK%d %s\n", i + 1,
				(val[0] & 0x01) ? "on" : "off");
	}

	for (i = 1; i <= 18; i++) {
		i2c_read(addr, 0x3a + i, 1, val, 1);
		printf("LDO%d %s\n", i,
				(val[0] & 0xC0) ? "on" : "off");
	}
	i2c_read(addr, 0x4d, 1, val, 1);
	printf("LDO21 %s\n", (val[0] & 0xC0) ? "on" : "off");

	i2c_read(addr, 0x5a, 1, val, 1);
	printf("SAFEOUT1 %s\n", (val[0] & (1 << 6)) ? "on" : "off");
	printf("SAFEOUT2 %s\n", (val[0] & (1 << 7)) ? "on" : "off");

	return 0;
}

static int pmic_ldo_voltage(int buck, int ldo, int safeout, ulong uV)
{
	unsigned char addr, val[2];
	unsigned int reg = 0, set = 0, mask = 0;

	if (ldo) {
		if (ldo < 1)
			return -1;
		if (ldo <= 18) {
			reg = 0x3a + ldo;
		} else if (ldo == 21) {
			reg = 0x4d;
		} else
			return -1;

		mask = 0x3f;

		if (uV < 800000)
			set = 0x0;
		else if (uV > 3950000)
			set = 0x3f;
		else
			set = (uV - 800000) / 50000;

		set &= 0x3f;

	} else if (buck) {
		if (buck < 1 || buck > 7)
			return -1;
		reg = reg_buck[buck - 1] + 1;
		mask = 0x3f;

		if (buck == 1 || buck == 2 || buck == 4 || buck == 5)
			if (uV < 650000)
				set = 0;
			else if (uV > 2225000)
				set = 0x3f;
			else
				set = (uV - 650000) / 25000;
		else if (buck == 3 || buck == 7)
			if (uV < 750000)
				set = 0;
			else if (uV > 3900000)
				set = 0x3f;
			else
				set = (uV - 750000) / 50000;

		set &= 0x3f;

	} else if (safeout) {
		if (safeout < 1 || safeout > 2)
			return 01;
		reg = 0x5a;
		mask = 0x3 << (safeout - 1);

		if (uV <= 3300000)
			set = 0x03; /* 3.3V */
		else if (uV <= 4850000)
			set = 0x00; /* 4.85V */
		else if (uV <= 4900000)
			set = 0x01; /* 4.90V */
		else
			set = 0x02; /* 4.95V */

		set &= 0x03;
		set <<= safeout - 1;

	} else
		return -1;

	addr = 0xCC >> 1;

	if (pmic_probe())
		return -1;

	i2c_read(addr, reg, 1, val, 1);
	val[0] &= ~mask;
	val[0] |= set;
	i2c_write(addr, reg, 1, val, 1);
	i2c_read(addr, reg, 1, val, 1);
	printf("MAX8997 REG %2.2xh has %2.2xh\n",
			reg, val[0]);

	return 0;
}

static int pmic_ldo_control(int buck, int ldo, int safeout, int on)
{
	unsigned char addr, val[2];
	unsigned int reg, set;

	if (ldo) {
		if (ldo < 1)
			return -1;
		if (ldo <= 18) {
			reg = 0x3a + ldo;
			set = 0xc0;
		} else if (ldo == 21) {
			reg = 0x4d;
			set = 0xc0;
		} else
			return -1;
	} else if (buck) {
		if (buck < 1 || buck > 7)
			return -1;
		reg = reg_buck[buck - 1];
		set = 0x01;
	} else if (safeout) {
		if (safeout < 1 || safeout > 2)
			return 01;
		reg = 0x5a;
		set = 1 << (5 + safeout);
	} else
		return -1;

	addr = 0xCC >> 1;

	if (pmic_probe())
		return -1;

	i2c_read(addr, reg, 1, val, 1);
	if (on)
		val[0] |= set;
	else
		val[0] &= ~set;

	i2c_write(addr, reg, 1, val, 1);
	i2c_read(addr, reg, 1, val, 1);

	return 0;
}

static int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int buck = 0, ldo = 0, safeout = 0;
	int ret = 0;

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
			ret = pmic_ldo_control(buck, ldo, safeout, 1);
		else if (strncmp(argv[3], "off", 3) == 0)
			ret = pmic_ldo_control(buck, ldo, safeout, 0);
		else {
			ulong volt = simple_strtoul(argv[3], NULL, 10);
			printf("volt = %duV\n", volt);
			if (volt <= 0)
				ret = -1;
			else
				ret = pmic_ldo_voltage(buck, ldo, safeout, volt);
		}

		if (!ret)
			printf("%s %s %s\n", argv[1], argv[2], argv[3]);
		if (ret < 0)
			printf("Error.\n");
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
	"pmic ldo num on/off/volt - Turn on/off the LDO\n"
	"pmic buck num on/off/volt - Turn on/off the BUCK\n"
	"pmic safeout num on/off/volt - Turn on/off the SAFEOUT\n"
	"	volt is voltage in uV\n"
);
