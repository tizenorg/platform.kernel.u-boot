#
# Copyright (C) 2010 Samsung Electronics.
# Sanghee Kim <sh0130.kim@samsung.com>
#

include $(TOPDIR)/$(RECOVERY_BLOCK)/config.mk

LIB	:= $(obj)libserial.a

COBJS-$(CONFIG_S5P) += serial_s5pc1xx.o

COBJS	:= $(sort $(COBJS-y))
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
