#
# Copyright (C) 2005-2007 Samsung Electronics.
# Kyungmin Park <kyungmin.park@samsung.com>
#

include $(TOPDIR)/$(RECOVERY_BLOCK)/config.mk

LIB := $(obj)libonenand.a

COBJS := onenand_base.o onenand_bbt.o

SRCS := $(COBJS:.o=.c)
OBJS := $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
