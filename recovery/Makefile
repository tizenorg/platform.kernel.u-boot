#
#  Samsung Universal(S5PC110) board Recovery block
#
#  Copyright (C) 2010 Samsung Electronics
#  Minkyu Kang <mk7.kang@samsung.com>
#  Kyungmin Park <kyungmin.park@samsung.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#

include $(TOPDIR)/config.mk

recoveryobj := $(OBJTREE)/recovery/
RECOVERYCFG := $(recoveryobj)board/$(BOARDDIR)/config.mk
include $(RECOVERYCFG)

LDSCRIPT = board/$(BOARDDIR)/recovery.lds
LDSCRIPT := $(addprefix $(recoveryobj),$(LDSCRIPT))
LDFLAGS	= -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE) $(PLATFORM_LDFLAGS)
OBJCFLAGS += --gap-fill=0x00

OBJS = recovery.o
OBJS += onenand.o
OBJS += usbd.o

SRCS := $(OBJS:.o=.c)
OBJS := $(addprefix $(obj),$(OBJS))

LIBS = drivers/onenand/libonenand.a
LIBS := $(addprefix $(recoveryobj),$(LIBS))
# .PHONY : $(LIBS) $(TIMESTAMP_FILE) $(VERSION_FILE)

LIBBOARD = board/$(BOARDDIR)/lib$(BOARD).a
LIBBOARD := $(addprefix $(recoveryobj),$(LIBBOARD))

# Add GCC lib
PLATFORM_LIBGCC = -L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc

__OBJS := $(subst $(obj),,$(OBJS))
__LIBS := $(subst $(obj),,$(LIBS)) $(subst $(obj),,$(LIBBOARD))

#########################################################################
#########################################################################

ALL	= $(recoveryobj)recovery $(recoveryobj)recovery.bin $(recoveryobj)recovery-256k.bin

all:	$(obj).depend $(ALL)

$(recoveryobj)recovery-256k.bin:	$(recoveryobj)recovery
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(TEXT_BASE_256K) -O binary $< $@
	cat $(OBJTREE)/onenand_ipl/onenand-ipl-16k-evt0.bin $@ > $(recoveryobj)recovery-evt0.bin
	cat $(OBJTREE)/onenand_ipl/onenand-ipl-16k-fused.bin $@ > $(recoveryobj)recovery-fused.bin
	cat $(OBJTREE)/onenand_ipl/onenand-ipl-16k.bin $@ > $(recoveryobj)recovery-evt1.bin

$(recoveryobj)recovery.bin:	$(recoveryobj)recovery
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(recoveryobj)recovery:	$(obj).depend $(OBJS) $(LIBS) $(LIBBOARD) $(RECOVERYCFG)
	$(LD) $(LDFLAGS) $$UNDEF_SYM $(__OBJS) \
		--start-group $(__LIBS) --end-group $(PLATFORM_LIBGCC) \
		-Map $@.map -o $@

$(LIBS):
	$(MAKE) -C $(dir $(subst $(obj),,$@))

$(LIBBOARD):	$(LIBS)
	$(MAKE) -C $(dir $(subst $(obj),,$@))

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
