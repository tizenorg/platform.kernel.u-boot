#include <common.h>
#include <command.h>
#include <usb_mass_storage.h>

static int do_usb_mass_storage(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("ums command placeholder\n");
	board_usb_init();
	fsg_init();
	printf("START LOOP\n");
	while (1)
	{
		int irq_res;
		/* Handle control-c and timeouts */
		if (ctrlc()) {
			printf("The remote end did not respond in time.\n");
			goto fail;
		}

		irq_res = usb_gadget_handle_interrupts();

	}
fail:
	return -1;
}

U_BOOT_CMD(ums, CONFIG_SYS_MAXARGS, 1, do_usb_mass_storage,
	"Initialize USB mass storage and download the image",
	"ums"
);

