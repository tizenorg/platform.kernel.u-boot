/*
 * f_usbd_thor.c -- USB THOR Downloader gadget function
 *
 * Copyright (C) 2011-2012 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
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
#include <errno.h>
#include <common.h>
#include <malloc.h>
#include <linux/usb/ch9.h>
#include <usbdescriptors.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>
#include <linux/usb/f_usbd_thor.h>
#include <linux/usb/cdc.h>

#include <g_dnl.h>
#include <mmc.h>
#include <part.h>

#include "usbd_thor.h"
#include "prot_thor.h"
#include "gadget_chips.h"

#define DMA_BUFFER_SIZE	(4096*4)

static struct f_usbd *usbd_func;
static const unsigned buflen = 512; /* Standard request buffer length */

struct usbd_dev {
	struct usb_gadget	*gadget;
	struct usb_request	*req;		/* for control responses */

	/* IN/OUT EP's and correspoinding requests */
	struct usb_ep		*in_ep, *out_ep, *int_ep;
	struct usb_request	*in_req, *out_req;

	/* Control flow variables*/
	int configuration_done;
	int stop_done;
	int rxdata;
	int txdata;
};

struct f_usbd {
	struct usb_function		usb_function;
	struct usbd_dev *dev;
};

static inline struct f_usbd *func_to_usbd(struct usb_function *f)
{
	return container_of(f, struct f_usbd, usb_function);
}

/* maxpacket and other transfer characteristics vary by speed. */
static inline struct usb_endpoint_descriptor *
ep_desc(struct usb_gadget *g, struct usb_endpoint_descriptor *hs,
		struct usb_endpoint_descriptor *fs)
{
	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return hs;
	return fs;
}

/* one interface in each configuration */
static struct usb_interface_descriptor usb_downloader_intf_data = {
	.bLength =		sizeof usb_downloader_intf_data,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
};


/* two full speed bulk endpoints; their use is config-dependent */
static struct usb_endpoint_descriptor fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

/* CDC configuration */

static struct usb_interface_descriptor usb_downloader_intf_int = {
	.bLength =		sizeof usb_downloader_intf_int,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_COMM,
	 /* 0x02 Abstract Line Control Model */
	.bInterfaceSubClass =   COMMUNICATIONS_INTERFACE_CLASS_CONTROL,
	/* 0x01 Common AT commands */
	.bInterfaceProtocol =   COMMUNICATIONS_V25TER_PROTOCOL,
};

static struct usb_class_header_function_descriptor usb_downloader_cdc_header = {
	.bFunctionLength =    sizeof usb_downloader_cdc_header,
	.bDescriptorType =    CS_INTERFACE, /* 0x24 */
	.bDescriptorSubtype = 0x00,	/* 0x00 */
	.bcdCDC =             0x0110,
};


static struct usb_class_call_management_descriptor usb_downloader_cdc_call = {
	.bFunctionLength =    sizeof usb_downloader_cdc_call,
	.bDescriptorType =    CS_INTERFACE, /* 0x24 */
	.bDescriptorSubtype = 0x01,	/* 0x01 */
	.bmCapabilities =     0x00,
	.bDataInterface =     0x01,
};

struct usb_class_abstract_control_descriptor usb_downloader_cdc_abstract = {
	.bFunctionLength =    sizeof usb_downloader_cdc_abstract,
	.bDescriptorType =    CS_INTERFACE,
	.bDescriptorSubtype = 0x02,	/* 0x02 */
	.bmCapabilities =     0x00,
};

struct usb_class_union_function_descriptor usb_downloader_cdc_union = {
	.bFunctionLength =     sizeof usb_downloader_cdc_union,
	.bDescriptorType =     CS_INTERFACE,
	.bDescriptorSubtype =  0x06,	/* 0x06 */
};


static struct usb_endpoint_descriptor fs_int_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bEndpointAddress = 3 | USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize = __constant_cpu_to_le16(16),

	.bInterval = 0x9,
};

static struct usb_interface_assoc_descriptor
usbd_iad_descriptor = {
	.bLength =		sizeof usbd_iad_descriptor,
	.bDescriptorType =	USB_DT_INTERFACE_ASSOCIATION,

	.bFirstInterface =	0,
	.bInterfaceCount =	2,	/* control + data */
	.bFunctionClass =	USB_CLASS_COMM,
	.bFunctionSubClass =	USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol =	USB_CDC_PROTO_NONE,
	/* .iFunction = DYNAMIC */
};


/*
 * usb 2.0 devices need to expose both high speed and full speed
 * descriptors, unless they only run at full speed.
 *
 */

static struct usb_endpoint_descriptor hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_int_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bmAttributes = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize = __constant_cpu_to_le16(16),

	.bInterval = 0x9,
};

static struct usb_qualifier_descriptor dev_qualifier = {
	.bLength =		sizeof dev_qualifier,
	.bDescriptorType =	USB_DT_DEVICE_QUALIFIER,

	.bcdUSB =		__constant_cpu_to_le16(0x0200),
	.bDeviceClass =		USB_CLASS_VENDOR_SPEC,

	.bNumConfigurations =	2,
};

static const struct usb_descriptor_header *hs_usb_downloader_function[] = {
	/* (struct usb_descriptor_header *) &otg_descriptor, */
	(struct usb_descriptor_header *) &usbd_iad_descriptor,

	(struct usb_descriptor_header *) &usb_downloader_intf_int,
	(struct usb_descriptor_header *) &usb_downloader_cdc_header,
	(struct usb_descriptor_header *) &usb_downloader_cdc_call,
	(struct usb_descriptor_header *) &usb_downloader_cdc_abstract,
	(struct usb_descriptor_header *) &usb_downloader_cdc_union,
	(struct usb_descriptor_header *) &hs_int_desc,

	(struct usb_descriptor_header *) &usb_downloader_intf_data,
	(struct usb_descriptor_header *) &hs_in_desc,
	(struct usb_descriptor_header *) &hs_out_desc,
	NULL,
};

/*-------------------------------------------------------------------------*/


static struct usb_request *alloc_ep_req(struct usb_ep *ep, unsigned length)
{
	struct usb_request	*req;

	req = usb_ep_alloc_request(ep, GFP_ATOMIC);
	if (req) {
		req->length = length;
		req->buf = malloc(length);
		if (!req->buf) {
			usb_ep_free_request(ep, req);
			req = NULL;
		}
	}
	return req;
}

unsigned int usbd_rx_data(void)
{
	int status;
	struct usbd_dev *dev = usbd_func->dev;

	static int data_to_rx = 0, tmp;

	data_to_rx = dev->out_req->length;
	tmp = data_to_rx;

	do {

		if (data_to_rx > DMA_BUFFER_SIZE)
			dev->out_req->length = DMA_BUFFER_SIZE;
		else
			dev->out_req->length = data_to_rx;

		debug("dev->out_req->length:%d dev->rxdata:%d\n",
		     dev->out_req->length, dev->rxdata);

		status = usb_ep_queue(dev->out_ep, dev->out_req, GFP_ATOMIC);
		if (status) {
			printf("kill %s:  resubmit %d bytes --> %d\n",
			       dev->out_ep->name, dev->out_req->length, status);
			usb_ep_set_halt(dev->out_ep);
			/* FIXME recover later ... somehow */
			return -1;
		}

		while (!dev->rxdata)
			usb_gadget_handle_interrupts();

		dev->rxdata = 0;

		if (data_to_rx > DMA_BUFFER_SIZE)
			dev->out_req->buf += DMA_BUFFER_SIZE;

		data_to_rx -= dev->out_req->actual;

	} while (data_to_rx);

	return tmp;
}

void usbd_tx_data(char *data, int len)
{
	int status;
	struct usbd_dev *dev = usbd_func->dev;

	unsigned char *ptr = dev->in_req->buf;

	memset(ptr, '\0', len);
	memcpy(ptr, data, len);

	dev->in_req->length = len;

	debug("%s: dev->in_req->length:%d to_cpy:%d\n", __func__,
	    dev->in_req->length, sizeof(data));

	status = usb_ep_queue(dev->in_ep, dev->in_req, GFP_ATOMIC);
	if (status) {
		printf("kill %s:  resubmit %d bytes --> %d\n",
		       dev->in_ep->name, dev->in_req->length, status);
		usb_ep_set_halt(dev->in_ep);
		/* FIXME recover later ... somehow */
	}

	/* Wait until tx interrupt received */
	while (!dev->txdata)
		usb_gadget_handle_interrupts();

	dev->txdata = 0;
}

static void usbd_rx_tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	int		status = req->status;
	struct usbd_dev *dev = usbd_func->dev;

	debug("%s: ep_ptr:%p, req_ptr:%p\n", __func__, ep, req);
	switch (status) {

	case 0:				/* normal completion? */
		if (ep == dev->out_ep)
			dev->rxdata = 1;
		else
			dev->txdata = 1;

		break;

	/* this endpoint is normally active while we're configured */
	case -ECONNABORTED:		/* hardware forced ep reset */
	case -ECONNRESET:		/* request dequeued */
	case -ESHUTDOWN:		/* disconnect from host */
		/* Exeptional situation - print error message */

	case -EOVERFLOW:
		printf("%s: ERROR:%d\n", __func__, status);
	default:
		debug("%s complete --> %d, %d/%d\n", ep->name,
				status, req->actual, req->length);
	case -EREMOTEIO:		/* short read */
		break;
	}
}

static struct usb_request *usbd_start_ep(struct usb_ep *ep)
{
	struct usb_request	*req;

	req = alloc_ep_req(ep, buflen);
	debug("%s: ep:%p req:%p\n", __func__, ep, req);

	if (!req)
		return NULL;

	memset(req->buf, 0, req->length);
	req->complete = usbd_rx_tx_complete;

	memset(req->buf, 0x55, req->length);

	return req;
}

static void usbd_setup_complete(struct usb_ep *ep, struct usb_request *req)
{
	if (req->status || req->actual != req->length)
		debug("setup complete --> %d, %d/%d\n",
		    req->status, req->actual, req->length);
}

static int
usbd_func_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	int value = 0;
	struct usbd_dev         *dev = usbd_func->dev;
	struct usb_request      *req = dev->req;
	struct usb_gadget	*gadget = dev->gadget;

	u16 len = le16_to_cpu(ctrl->wLength);
	u16 wValue = le16_to_cpu(ctrl->wValue);

	debug("Req_Type: 0x%x Req: 0x%x wValue: 0x%x wIndex: 0x%x wLen: 0x%x\n",
	       ctrl->bRequestType, ctrl->bRequest, ctrl->wValue, ctrl->wIndex,
	       ctrl->wLength);

	switch (ctrl->bRequest) {
	case SET_CONTROL_LINE_STATE:
		value = 0;

		switch (wValue) {
		case DEACTIVATE_CARRIER:
			/* Before reset wait for all ACM requests to be done */
			dev->stop_done = 1;
			break;
		case ACTIVATE_CARRIER:
			break;
		default:
			printf("usbd_setup:SetControlLine-unknown wValue: %d\n",
			       wValue);
		}
		break;
	case SET_LINE_CODING:
		value = len;

		/* Line Coding set done = configuration done */
		usbd_func->dev->configuration_done = 1;
		break;

	default:
		printf("usbd_setup: unknown request: %d\n", ctrl->bRequest);
	}

	if (value >= 0) {
		req->length = value;
		req->zero = value < len;
		value = usb_ep_queue(gadget->ep0, req, GFP_ATOMIC);
		if (value < 0) {
			debug("ep_queue --> %d\n", value);
			req->status = 0;
		}
	}

	return value;
}

/* Specific to the USBD_THOR protocol */
void usbd_set_dma(char *addr, int len)
{
	struct usbd_dev *dev = usbd_func->dev;

	debug("in_req:%p, out_req:%p\n", dev->in_req, dev->out_req);
	debug("addr:%p, len:%d\n", addr, len);

	dev->out_req->buf = addr;
	dev->out_req->length = len;

}

static void usbd_cancel(int mode)
{

	switch (mode) {
	case END_BOOT:
		run_command("run bootcmd", 0);
		break;
	default:
		break;
	}
}

__attribute__ ((__aligned__ (__alignof__ (long long))))
static char usbd_tx_buf[TX_DATA_LEN];
__attribute__ ((__aligned__ (__alignof__ (long long))))
static char usbd_rx_buf[RX_DATA_LEN];

char *usbd_tx_data_buf = usbd_tx_buf;
char *usbd_rx_data_buf = usbd_rx_buf;

static const char *recv_key = "THOR";
static const char *tx_key = "ROHT";

static int usbd_init(struct g_dnl *dnl)
{
	int ret;
	struct usbd_dev *dev = usbd_func->dev;

	/* Wait for a device enumeration and configuration settings */
	debug("USBD enumeration/configuration setting....\n");
	while (!dev->configuration_done)
		usb_gadget_handle_interrupts();

	usbd_set_dma(usbd_rx_data_buf, strlen(recv_key));
	/* detect the download request from Host PC */
	ret = usbd_rx_data();

	if (strncmp(usbd_rx_data_buf, recv_key, strlen(recv_key)) == 0) {
		printf("Download request from the Host PC\n");
		msleep(30);

		strncpy(usbd_tx_data_buf, tx_key, strlen(tx_key));
		usbd_tx_data(usbd_tx_data_buf, strlen(tx_key));
	} else {
		puts("Wrong reply information\n");
		return -1;
	}

	return 0;
}

static int usbd_handle(struct g_dnl *dnl)
{
	int ret;

	debug("USBD Handle start....\n");

	/* receive the data from Host PC */
	while (1) {
		usbd_set_dma(usbd_rx_data_buf, sizeof(rqt_box));
		ret = usbd_rx_data();

		if (ret > 0) {
			ret = process_data(dnl);

			if (ret < 0)
				return -1;
			else if (ret == 0)
				return 0;
		} else {
			puts("No data received!\n");
			usbd_cancel(END_BOOT);
		}
	}

	return 0;
}


/* USB Downloader's specific definitions - required for this framework */

void usbd_fill_dnl(struct g_dnl *dnl)
{
	dnl->prot = THOR;
	dnl->init = &usbd_init;
	dnl->dl = &usbd_handle;
	dnl->cmd = &usbd_handle;

	/* If the following pointers are set to NULL -> error */
	dnl->rx_buf = (unsigned int *) CONFIG_SYS_DOWN_ADDR;
	dnl->tx_buf = NULL;
}

static int usbd_func_bind(struct usb_configuration *c, struct usb_function *f)
{
	int status;
	struct usb_ep		*ep;
	struct usbd_dev		*dev;
	struct f_usbd           *f_usbd = func_to_usbd(f);
	struct usb_gadget       *gadget = c->cdev->gadget;
	struct g_dnl            *dnl = get_udc_gadget_private_data(gadget);

	usbd_func = f_usbd;
	dev = calloc(sizeof(*dev), 1);
	if (!dev)
		return -ENOMEM;

	dev->gadget = gadget;

	f_usbd->dev = dev;
	f_usbd->dev->configuration_done = 0;
	f_usbd->dev->rxdata = 0;
	f_usbd->dev->txdata = 0;

	debug("%s: usb_configuration: 0x%p usb_function: 0x%p\n",
	    __func__, c, f);
	debug("f_usbd: 0x%p usbd: 0x%p\n", f_usbd, dev);

	/* EP0  */
	/* preallocate control response and buffer */
	dev->req = usb_ep_alloc_request(gadget->ep0, 0);
	if (!dev->req) {
		status = -ENOMEM;
		goto fail;
	}
	dev->req->buf = malloc(buflen);
	if (!dev->req->buf) {
		status = -ENOMEM;
		goto fail;
	}

	dev->req->complete = usbd_setup_complete;

	/* DYNAMIC interface numbers assignments */
	status = usb_interface_id(c, f);

	if (status < 0)
		goto fail;

	usb_downloader_intf_int.bInterfaceNumber = status;
	usb_downloader_cdc_union.bMasterInterface = status;

	status = usb_interface_id(c, f);

	if (status < 0)
		goto fail;

	usb_downloader_intf_data.bInterfaceNumber = status;
	usb_downloader_cdc_union.bSlaveInterface0 = status;


	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(gadget, &fs_in_desc);
	if (!ep) {
		status = -ENODEV;
		goto fail;
	}

	if (gadget_is_dualspeed(gadget)) {
		hs_in_desc.bEndpointAddress =
				fs_in_desc.bEndpointAddress;
	}

	dev->in_ep = ep; /* Store IN EP for enabling @ setup */



	ep = usb_ep_autoconfig(gadget, &fs_out_desc);
	if (!ep) {
		status = -ENODEV;
		goto fail;
	}

	if (gadget_is_dualspeed(gadget)) {
		hs_out_desc.bEndpointAddress =
				fs_out_desc.bEndpointAddress;
	}

	dev->out_ep = ep; /* Store OUT EP for enabling @ setup */

	/* note:  a status/notification endpoint is, strictly speaking,
	 * optional.  We don't treat it that way though!  It's simpler,
	 * and some newer profiles don't treat it as optional.
	 */
	ep = usb_ep_autoconfig(gadget, &fs_int_desc);
	if (!ep) {
		status = -ENODEV;
		goto fail;
	}

	dev->int_ep = ep;

	if (gadget_is_dualspeed(gadget)) {
		hs_int_desc.bEndpointAddress =
				fs_int_desc.bEndpointAddress;

		/* copy descriptors, and track endpoint copies */
		f->hs_descriptors = (struct usb_descriptor_header **)
			&hs_usb_downloader_function;

		if (!f->hs_descriptors)
			goto fail;
	}

	debug("%s: out_ep:%p out_req:%p\n",
	      __func__, dev->out_ep, dev->out_req);
	printf("%s: dnl: 0x%p\n", __func__, dnl);
	usbd_fill_dnl(dnl);

	return 0;

 fail:
	free(dev);
	return status;
}

static void free_ep_req(struct usb_ep *ep, struct usb_request *req)
{
	free(req->buf);
	usb_ep_free_request(ep, req);
}

static void usbd_func_disable(struct usb_function *f)
{
	struct f_usbd   *f_usbd = func_to_usbd(f);
	struct usbd_dev *dev = f_usbd->dev;

	debug("%s:\n", __func__);

	/* Avoid freeing memory when ep is still claimed */
	if (dev->in_ep->driver_data) {
		free_ep_req(dev->in_ep, dev->in_req);
		usb_ep_disable(dev->in_ep);
		dev->in_ep->driver_data = NULL;
	}

	if (dev->out_ep->driver_data) {
		free_ep_req(dev->out_ep, dev->out_req);
		usb_ep_disable(dev->out_ep);
		dev->out_ep->driver_data = NULL;
	}

	if (dev->int_ep->driver_data) {
		usb_ep_disable(dev->int_ep);
		dev->int_ep->driver_data = NULL;
	}
}

static int usbd_eps_setup(struct usb_function *f)
{
	int result;
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_ep		*ep;
	struct usb_request      *req;
	struct usb_gadget       *gadget = cdev->gadget;
	struct usbd_dev         *dev = usbd_func->dev;

	struct usb_endpoint_descriptor	*d;

	ep = dev->in_ep;
	d = ep_desc(gadget, &hs_in_desc, &fs_in_desc);
	debug("(d)bEndpointAddress: 0x%x\n", d->bEndpointAddress);

	result = usb_ep_enable(ep, d);

	if (result == 0) {
		ep->driver_data = cdev; /* claim */
		req = usbd_start_ep(ep);
		if (req != NULL) {
			dev->in_req = req;
		} else {
			usb_ep_disable(ep);
			result = -EIO;
		}
	} else
		goto exit;

	ep = dev->out_ep;
	d = ep_desc(gadget, &hs_out_desc, &fs_out_desc);
	debug("(d)bEndpointAddress: 0x%x\n", d->bEndpointAddress);

	result = usb_ep_enable(ep, d);

	if (result == 0) {
		ep->driver_data = cdev; /* claim */
		req = usbd_start_ep(ep);
		if (req != NULL) {
			dev->out_req = req;
		} else {
			usb_ep_disable(ep);
			result = -EIO;
		}
	} else
		goto exit;

	/* ACM control EP */
	ep = dev->int_ep;
	ep->driver_data = cdev;	/* claim */

 exit:
	return result;
}

static int usbd_func_set_alt(struct usb_function *f,
			     unsigned intf, unsigned alt)
{
	int result;
	debug("%s: func: %s intf: %d alt: %d\n",
	    __func__, f->name, intf, alt);

	switch (intf) {
	case 0:
		debug("ACM INTR interface\n");
		break;
	case 1:
		debug("Communication Data interface\n");

		result = usbd_eps_setup(f);
		if (result)
			printf("%s: EPs setup failed!\n", __func__);
		break;
	}

	return 0;
}

static int usbd_func_init(struct usb_configuration *c)
{
	int status;
	struct f_usbd *f_usbd;

	debug("%s: cdev: 0x%p\n", __func__, c->cdev);

	f_usbd = calloc(sizeof(*f_usbd), 1);
	if (!f_usbd)
		return -ENOMEM;

	f_usbd->usb_function.name = "f_usbd";
	f_usbd->usb_function.bind = usbd_func_bind;
	f_usbd->usb_function.setup = usbd_func_setup;
	f_usbd->usb_function.set_alt = usbd_func_set_alt;
	f_usbd->usb_function.disable = usbd_func_disable;

	status = usb_add_function(c, &f_usbd->usb_function);
	if (status)
		free(f_usbd);

	return status;
}

int f_usbd_add(struct usb_configuration *c)
{
	debug("%s:\n", __func__);

	return usbd_func_init(c);
}
