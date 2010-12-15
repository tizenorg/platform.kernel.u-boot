#ifndef __USB_MASS_STORAGE_H__
#define __USB_MASS_STORAGE_H__

extern void fsg_init();
extern void board_usb_init();
extern int usb_gadget_handle_interrupts();
extern int fsg_main_thread(void*);

#endif
