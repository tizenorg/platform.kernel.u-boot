#include <common.h>

static int do_usb_mass_storage(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("ums command placeholder\n");
}

U_BOOT_CMD(ums, CONFIG_SYS_MAXARGS, 1, do_usb_mass_storage,
	"Initialize USB mass storage and download the image",
	"ums"
);

