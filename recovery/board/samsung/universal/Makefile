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

# Recovery block' size should be 1 block (256K)
# Recovery block includes the onenand_ipl(16K), so actual size is 240K
TEXT_BASE 	= 0x34000000
# 256K - 16K(IPL Size)
TEXT_BASE_256K 	= 0x3403C000

LDSCRIPT= $(TOPDIR)/recovery/board/$(BOARDDIR)/recovery.lds
LDFLAGS	= -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE) $(PLATFORM_LDFLAGS)
AFLAGS	+= -DCONFIG_RECOVERY_BLOCK -g -UTEXT_BASE -DTEXT_BASE=$(TEXT_BASE)
CFLAGS	+= -DCONFIG_RECOVERY_BLOCK -g -D__HAVE_ARCH_MEMCPY32
OBJCFLAGS += --gap-fill=0x00

SOBJS	+= start.o _memcpy32.o
COBJS	+= recovery.o onenand.o
COBJS	+= usbd.o usb_downloader.o usb-hs-otg.o

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(OBJTREE)/recovery/board/$(BOARDDIR)

recoveryobj	:= $(OBJTREE)/recovery/

ALL	= $(recoveryobj)recovery $(recoveryobj)recovery.bin $(recoveryobj)recovery-256k.bin

all:	$(obj).depend $(ALL)

$(recoveryobj)recovery-256k.bin:	$(recoveryobj)recovery
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(TEXT_BASE_256K) -O binary $< $@
	cat $(OBJTREE)/onenand_ipl/onenand-ipl-16k-evt0.bin $@ > $(recoveryobj)recovery-evt0.bin
	cat $(OBJTREE)/onenand_ipl/onenand-ipl-16k-fused.bin $@ > $(recoveryobj)recovery-fused.bin
	cat $(OBJTREE)/onenand_ipl/onenand-ipl-16k.bin $@ > $(recoveryobj)recovery.bin

$(recoveryobj)recovery.bin:	$(recoveryobj)recovery
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(recoveryobj)recovery:	$(OBJS)
	cd $(LNDIR) && $(LD) $(LDFLAGS) $$UNDEF_SYM $(__OBJS) \
		-Map $@.map -o $@

# create symbolic links from common files

# from cpu directory
$(obj)start.S:
	@rm -f $@
	ln -s $(SRCTREE)/cpu/$(CPU)/start.S $@

# from SoC directory

# from lib_arm
$(obj)_memcpy32.S:
	ln -sf $(SRCTREE)/lib_arm/_memcpy32.S $@

# from recovery directory
$(obj)recovery.c:
	@rm -f $@
	ln -s $(SRCTREE)/recovery/recovery.c $@

$(obj)onenand.h:
	@rm -f $@
	ln -s $(SRCTREE)/recovery/onenand.h $@

$(obj)onenand.c:	$(obj)onenand.h
	@rm -f $@
	ln -s $(SRCTREE)/recovery/onenand.c $@

$(obj)usbd.h:
	@rm -f $@
	ln -s $(SRCTREE)/recovery/usbd.h $@

$(obj)usbd.c:	$(obj)usbd.h
	@rm -f $@
	ln -s $(SRCTREE)/recovery/usbd.c $@

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)$.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

clean:
	rm recovery.c onenand.c

