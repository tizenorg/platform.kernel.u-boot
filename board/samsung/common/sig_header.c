#include <common.h>
#include <errno.h>
#include <samsung/sighdr.h>

int get_image_signature(struct sig_header *hdr,
			phys_addr_t base_addr,
			phys_size_t size)
{
	memcpy((void *)hdr, (const void *)(base_addr + size - HDR_SIZE),
		HDR_SIZE);

	if (hdr->magic != HDR_BOOT_MAGIC)
		return -EFAULT;

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
				  CONFIG_SIG_IMAGE_SIZE);
	if (ret)
		return 0;

	printf("U-Boot signature: \"%.*s\"\n", 16, bh_target.bd_name);

	if (size != CONFIG_SIG_IMAGE_SIZE) {
		printf("Bad file size for: %s.\n", fname);
		printf("Expected: %#x bytes, has: %#x bytes.\n",
		       CONFIG_SIG_IMAGE_SIZE, (unsigned)size);
		return -EINVAL;
	}

	printf("%s ", fname);
	/* can't found signature in address - download stop */
	ret = get_image_signature(&bh_addr, dn_addr, CONFIG_SIG_IMAGE_SIZE);
	if (ret) {
		printf("signature not found.\n");
		return -EFAULT;
	}
	printf("signature: \"%.*s\". ", 16, bh_addr.bd_name);

	if (strncmp(bh_target.bd_name, bh_addr.bd_name,
		    ARRAY_SIZE(bh_target.bd_name))) {
		printf("Invalid!\n");
		return -EPERM;
	}
	printf("OK!\n");

	return 0;
}
