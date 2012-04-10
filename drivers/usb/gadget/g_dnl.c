/*
 * g_dnl.c -- USB Downloader Gadget
 *
 * Copyright (C) 2012 Samsung Electronics
 * Lukasz Majewski  <l.majewski@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#undef DEBUG
#define DEBUG
#include <errno.h>
#include <common.h>
#include <malloc.h>
#include <linux/usb/ch9.h>
#include <usbdescriptors.h>
#include <linux/usb/gadget.h>
#include <linux/usb/f_usbd_thor.h>

#include <mmc.h>
#include <part.h>

#include <g_dnl.h>

#include "usbd_thor.h"
#include "gadget_chips.h"

#include "composite.c"

#define DRIVER_VERSION		"usb_dnl 2.0"

static const char shortname[] = "usb_dnl_";
static const char product[] = "SLP USB download gadget";
static const char manufacturer[] = "Samsung";

static struct usb_device_descriptor device_desc = {
	.bLength =		sizeof device_desc,
	.bDescriptorType =	USB_DT_DEVICE,

	.bcdUSB =		__constant_cpu_to_le16(0x0200),
	.bDeviceClass =		USB_CLASS_COMM,
	.bDeviceSubClass =      0x02,	/*0x02:CDC-modem , 0x00:CDC-serial*/

	.idVendor =		__constant_cpu_to_le16(DRIVER_VENDOR_NUM),
	.idProduct =		__constant_cpu_to_le16(DRIVER_PRODUCT_NUM),
	.iProduct =		STRING_PRODUCT,
	.bNumConfigurations =	1,
};

static const struct usb_descriptor_header *otg_desc[] = {
	(struct usb_descriptor_header *) &(struct usb_otg_descriptor){
		.bLength =		sizeof(struct usb_otg_descriptor),
		.bDescriptorType =	USB_DT_OTG,
		.bmAttributes =		USB_OTG_SRP,
	},
	NULL,
};

/* static strings, in UTF-8 */
static struct usb_string odin_string_defs[] = {
	{ 0, manufacturer, },
	{ 1, product, },
};

static struct usb_gadget_strings odin_string_tab = {
	.language	= 0x0409,	/* en-us */
	.strings	= odin_string_defs,
};

static struct usb_gadget_strings *g_dnl_composite_strings[] = {
	&odin_string_tab,
	NULL,
};

static int g_dnl_unbind(struct usb_composite_dev *cdev)
{
	debug("%s\n", __func__);
	return 0;
}

static int g_dnl_do_config(struct usb_configuration *c)
{
	int ret = -1;
	char *s = (char *) c->cdev->driver->name;

	debug("%s: c: 0x%p cdev: 0x%p\n", __func__, c, c->cdev);

	if (gadget_is_otg(c->cdev->gadget)) {
		c->descriptors = otg_desc;
		c->bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}

	printf("GADGET DRIVER: %s\n", s);

	if (!strcmp(s, "usb_dnl_thor"))
		ret = f_usbd_add(c);

	return ret;
}

static int g_dnl_config_register(struct usb_composite_dev *cdev)
{
	debug("%s:\n", __func__);
	static struct usb_configuration config = {
		.label = "usbd_thor_download",
		.bmAttributes =	USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
		.bConfigurationValue =	CONFIG_USBDOWNLOADER,
		.iConfiguration =	STRING_USBDOWN,

		.bind = g_dnl_do_config,
	};

	return usb_add_config(cdev, &config);
}

static int g_dnl_bind(struct usb_composite_dev *cdev)
{
	int			gcnum;
	int id, ret;
	struct usb_gadget	*gadget = cdev->gadget;

	debug("%s: gadget: 0x%p cdev: 0x%p\n", __func__, gadget, cdev);

	id = usb_string_id(cdev);

	if (id < 0)
		return id;
	odin_string_defs[0].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;

	odin_string_defs[1].id = id;
	device_desc.iProduct = id;

	ret = g_dnl_config_register(cdev);
	if (ret)
		goto error;


	gcnum = usb_gadget_controller_number(gadget);

	debug("gcnum: %d\n", gcnum);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0200 + gcnum);
	else {
		printf("%s: controller '%s' not recognized\n",
			shortname, gadget->name);
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	return 0;

 error:
	g_dnl_unbind(cdev);
	return -ENOMEM;
}

static void g_dnl_suspend(struct usb_composite_dev *cdev)
{
	if (cdev->gadget->speed == USB_SPEED_UNKNOWN)
		return;

	debug("suspend\n");
}

static void g_dnl_resume(struct usb_composite_dev *cdev)
{
	debug("resume\n");
}

static struct usb_composite_driver g_dnl_driver = {
	.name		= shortname,
	.dev		= &device_desc,
	.strings	= g_dnl_composite_strings,

	.bind		= g_dnl_bind,
	.unbind		= g_dnl_unbind,

	.suspend	= g_dnl_suspend,
	.resume		= g_dnl_resume,
};

int g_dnl_init(char *s, struct g_dnl *dnl)
{

	int ret;
	static char str[16];

	memset(str, 0x00, sizeof(str));
	strncpy(str, shortname, sizeof(shortname));

	if (!strncmp(s, "thor", sizeof(s))) {
		strncat(str, s, sizeof(str));
	} else {
		printf("%s: unknown command: %s\n", __func__, s);
		return -EINVAL;
	}

	strncpy((char *) g_dnl_driver.name, (const char *) str, sizeof(str));

	usbd_thor_udc_probe();
	set_udc_gadget_private_data(dnl);

	ret = usb_composite_register(&g_dnl_driver);

	if (ret) {
		printf("%s: failed!, error:%d ", __func__, ret);
		return ret;
	}

	return 0;
}

void g_dnl_cleanup(void)
{
	usb_composite_unregister(&g_dnl_driver);
}
