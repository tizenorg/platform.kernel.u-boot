
include $(TOPDIR)/config.mk
include $(TOPDIR)/onenand_ipl/board/$(BOARDDIR)/config.mk

LDSCRIPT= $(TOPDIR)/onenand_ipl/board/$(BOARDDIR)/u-boot.onenand.lds
LDFLAGS	= -Bstatic -T $(onenandobj)u-boot.lds -Ttext $(TEXT_BASE) $(PLATFORM_LDFLAGS)
AFLAGS	+= -DCONFIG_PRELOADER -DCONFIG_ONENAND_IPL -DCONFIG_SKIP_RELOCATE_UBOOT
CFLAGS	+= -DCONFIG_PRELOADER -DCONFIG_ONENAND_IPL -DCONFIG_SKIP_ONENAND_BOARD_INIT -DCONFIG_ONENAND_XIP_IPL -D__HAVE_ARCH_MEMCPY32
OBJCFLAGS += --gap-fill=0x00

SOBJS	:= lowlevel_init.o
SOBJS	+= start.o _memcpy32.o
COBJS	:= onenand_read.o
COBJS	+= onenand_boot.o

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(OBJTREE)/onenand_ipl/board/$(BOARDDIR)

onenandobj	:= $(OBJTREE)/onenand_ipl/

ALL	= $(onenandobj)onenand-ipl $(onenandobj)onenand-ipl.bin $(onenandobj)onenand-ipl-2k.bin

all:	$(obj).depend $(ALL)

$(onenandobj)onenand-ipl-2k.bin:	$(onenandobj)onenand-ipl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=0x800 -O binary $< $@

$(onenandobj)onenand-ipl.bin:	$(onenandobj)onenand-ipl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(onenandobj)onenand-ipl:	$(OBJS) $(onenandobj)u-boot.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $$UNDEF_SYM $(__OBJS) \
		-Map $@.map -o $@

$(onenandobj)u-boot.lds:	$(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

# create symbolic links from common files

# from cpu directory
$(obj)start.S:
	@rm -f $@
	ln -s $(SRCTREE)/$(CPUDIR)/start.S $@

# from lib_arm
$(obj)_memcpy32.S:
	ln -sf $(SRCTREE)/arch/$(ARCH)/lib/_memcpy32.S $@

# from onenand_ipl directory
$(obj)onenand_ipl.h:
	@rm -f $@
	ln -s $(SRCTREE)/onenand_ipl/onenand_ipl.h $@

$(obj)onenand_boot.c:	$(obj)onenand_ipl.h
	@rm -f $@
	ln -s $(SRCTREE)/onenand_ipl/onenand_boot.c $@

$(obj)onenand_read.c:	$(obj)onenand_ipl.h
	@rm -f $@
	ln -s $(SRCTREE)/onenand_ipl/onenand_read.c $@

$(obj)lowlevel_init.S:
	ln -sf $(SRCTREE)/board/$(BOARDDIR)/lowlevel_init.S $@

#########################################################################

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)$.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
