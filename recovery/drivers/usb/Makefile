#
# Copyright (C) 2010 Samsung Electronics.
# Minkyu Kang <mk7.kang@samsung.com>
#

include $(TOPDIR)/$(RECOVERY_BLOCK)/config.mk

LIB := $(obj)libusb.a

COBJS-$(CONFIG_S5P) += s5p_usb_hs_otg.o s5p_usb_downloader.o

SRCS := $(COBJS-y:.o=.c)
OBJS := $(addprefix $(obj),$(COBJS-y))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
