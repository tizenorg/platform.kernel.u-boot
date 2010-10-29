/*
 * (C) Copyright 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 */
#include <common.h>
#include <ramoops.h>

static char msg[RAMOOPS_SIZE];
static int dumped;

static void ramoops_show(void)
{
	int i;

	if (!dumped) {
		puts("No Messages\n");
		return;
	}

	printf("\n\n");
	for (i = 0; i < RAMOOPS_SIZE; i++)
		printf("%c", msg[i]);
	printf("\n\n");
}

int ramoops_init(unsigned int base)
{
	unsigned int *msg_header = (unsigned int *)base;

	dumped = 0;

	if (*msg_header != RAMOOPS_HEADER)
		return -1;

	memcpy(msg, (void *)base, RAMOOPS_SIZE);
	memset((void *)base, 0x0, RAMOOPS_SIZE);

	dumped = 1;

	ramoops_show();

	return 0;
}

static int do_ramoops(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	switch (argc) {
	case 2:
		if (strncmp(argv[1], "show", 2) == 0) {
			ramoops_show();
		} else {
			cmd_usage(cmdtp);
			return 1;
		}
		break;
	default:
		cmd_usage(cmdtp);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	ramoops,		CONFIG_SYS_MAXARGS,	1, do_ramoops,
	"RAM Oops/Panic Logger",
	"show - show messages\n"
);
