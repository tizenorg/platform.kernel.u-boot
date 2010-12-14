#include <common.h>
#include <command.h>
#include <usb_mass_storage.h>

static int do_usb_mass_storage(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("ums command placeholder\n");
	fsg_init();
}

U_BOOT_CMD(ums, CONFIG_SYS_MAXARGS, 1, do_usb_mass_storage,
	"Initialize USB mass storage and download the image",
	"ums"
);

