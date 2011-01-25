#include <common.h>
#include <command.h>
#include <usb_mass_storage.h>

static int do_usb_mass_storage(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int dev_num = 0;
	struct ums_board_info* ums_info;

	if (argc < 2) {
		printf("usage: ums <part> [2 SD, 0 eMMC]\n");
		return 0;
	}
	
	dev_num = (int)simple_strtoul(argv[1], NULL, 16);

	ums_info = board_ums_init(dev_num);

	if (!ums_info) {
		printf("MMC: %d -> NOT available\n", dev_num);
		goto fail;
	}

	fsg_init(ums_info);
	while (1)
	{
		int irq_res;
		/* Handle control-c and timeouts */
		if (ctrlc()) {
			printf("The remote end did not respond in time.\n");
			goto fail;
		}

		irq_res = usb_gadget_handle_interrupts();
		fsg_main_thread(NULL);
	}
fail:
	return -1;
}

U_BOOT_CMD(ums, CONFIG_SYS_MAXARGS, 1, do_usb_mass_storage,
	"Use the UMS [User Mass Storage]",
        "ums"
);

