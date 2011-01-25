#ifndef __USB_MASS_STORAGE_H__
#define __USB_MASS_STORAGE_H__

#define SECTOR_SIZE		0x200

struct ums_board_info {
	int (*read_sector)(unsigned int n, void *buf);
	int (*write_sector)(unsigned int n, void *buf);
	int (*get_capacity)(void);
	const char* name;
};

extern int fsg_init(struct ums_board_info*);
extern struct ums_board_info* board_ums_init(void);
extern int usb_gadget_handle_interrupts(void);
extern int fsg_main_thread(void*);

#endif
