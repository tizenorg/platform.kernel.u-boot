#include <common.h>
#include <samsung/sighdr.h>

//static struct sig_header *

int get_image_signature(struct sig_header *hdr,
			phys_addr_t base_addr,
			phys_size_t size)
{
	memcpy((void *)hdr, (const void *)(base_addr + size - HDR_SIZE),
		HDR_SIZE);

	if (hdr->magic != HDR_BOOT_MAGIC) {
		printf("no sign found\n");
		debug("can't found signature\n");
		return 1;
	}

	return 0;
}

int check_board_signature(char *fname, phys_addr_t dn_addr, phys_size_t size)
{
	struct sig_header bh_target;
	struct sig_header bh_addr;
	int ret;

	/* u-boot-mmc.bin */
	if (strcmp(fname, "u-boot-mmc.bin"))
		return 0;

	/* can't found signature in target - download continue */
	ret = get_image_signature(&bh_target, (phys_addr_t)CONFIG_SYS_TEXT_BASE,
					size);
	if (ret)
		return 0;

	/* can't found signature in address - download stop */
	ret = get_image_signature(&bh_addr, dn_addr, size);
	if (ret)
		return -1;

	if (strncmp(bh_target.bd_name, bh_addr.bd_name,
		    ARRAY_SIZE(bh_target.bd_name))) {
		debug("board name Inconsistency\n");
		return -1;
	}

	debug("board signature check OK\n");

	return 0;
}
