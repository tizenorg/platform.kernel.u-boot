#
#  Samsung Universal(S5PC110) board Recovery block
#
#  Copyright (C) 2010 Samsung Electronics
#  Minkyu Kang <mk7.kang@samsung.com>
#  Kyungmin Park <kyungmin.park@samsung.com>
#

include $(TOPDIR)/$(RECOVERY_BLOCK)/config.mk

LIB	:= $(obj)lib$(BOARD).a

SOBJS = start.o reset.o _memcpy32.o
COBJS = string.o dlmalloc.o
COBJS += gpio.o
COBJS += universal.o

SRCS := $(COBJS:.o=.c)
OBJS := $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(SOBJS) $(OBJS)
	$(AR) $(ARFLAGS) $@ $(SOBJS) $(OBJS)

# create symbolic links from common files

$(obj)start.S:
	@rm -f $@
	ln -s $(SRCTREE)/cpu/$(CPU)/$@ $@

$(obj)reset.S:
	@rm -f $@
	ln -s $(SRCTREE)/cpu/$(CPU)/s5pc1xx/$@ $@

$(obj)_memcpy32.S:
	ln -sf $(SRCTREE)/lib_arm/$@ $@

$(obj)string.c:
	@rm -f $@
	ln -s $(SRCTREE)/lib_generic/$@ $@

$(obj)dlmalloc.c:
	@rm -f $@
	ln -s $(SRCTREE)/common/$@ $@

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
