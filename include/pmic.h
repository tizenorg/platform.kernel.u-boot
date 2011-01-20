/*
 * Copyright (C) 2011 Samsung Electrnoics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Kyungmin Park <kyungmin.park@samsung.com>
 */

enum {
	PWRONR,
	PWRONF,
	PWRON1S,
};

unsigned int pmic_get_irq(int irq);
void pmic_bus_init(int bus_num);
