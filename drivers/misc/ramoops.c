/*
 * (C) Copyright 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 */
#include <common.h>
#include <ramoops.h>

int ramoops_show_msg(unsigned int base)
{
	unsigned int *msg_header = (unsigned int *)base;
	char msg[RAMOOPS_SIZE];
	int i;

	if (*msg_header != RAMOOPS_HEADER)
		return -1;

	memcpy(msg, (void *)base, RAMOOPS_SIZE);

	printf("\n\n");
	for (i = 0; i < RAMOOPS_SIZE; i++) {
		printf("%c", msg[i]);
	}
	printf("\n\n");

	memset((void *)base, 0x0, RAMOOPS_SIZE);

	return 0;
}
