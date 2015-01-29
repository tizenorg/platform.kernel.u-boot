/*
 * Copyright (C) 2015 Samsung Electronics
 * Inha Song <ideal.song@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <part.h>
#include <samsung/misc.h>
#include <samsung/report.h>

void report_dfu_env_entities(int argc, char * const argv[])
{
	char *env_setup_active;
	int setup_active_num;
	char *interface;
	char *devstring;
	block_dev_desc_t *desc;
	disk_partition_t partinfo;
	int ret;
	const char *platname = getenv("platname");

	if (!platname) {
		report("Undefined platname");
		return;
	}

	env_setup_active = getenv_by_args("%s_setup_active", platname);
	if (env_setup_active)
		setup_active_num = simple_strtoul(env_setup_active, NULL, 10);
	else
		return;

	switch (argc) {
	case 1:
		interface = strdup(getenv("dfu_interface"));
		devstring = strdup(getenv("dfu_device"));
		break;
	case 4:
		interface = argv[2];
		devstring = argv[3];
		break;
	default:
		return;
	}

	ret = get_device(interface, devstring, &desc);
	if (ret < 0)
		report("Get device error");

	switch (setup_active_num) {
	/* Tizen 2.x */
	case 1:
		if (get_partition_info(desc, 4, &partinfo) != 0) {
			report("Version and Part are mismatch");
			report("%s", getenv_by_args("%s_setup_%d_name",
						    platname,
						    setup_active_num));
		}
		break;
	/* Tizen 3.0 */
	case 2:
		if (get_partition_info(desc, 5, &partinfo) != 0) {
			report("Version and Part are mismatch");
			report("%s", getenv_by_args("%s_setup_%d_name",
						    platname,
						    setup_active_num));
		}
		break;
	default:
		report("Unknown Version: %d", setup_active_num);
	}

	if (argc == 1) {
		free(interface);
		free(devstring);
	}

	return;
}
