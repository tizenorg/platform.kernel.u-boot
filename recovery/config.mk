#
# Copyright (C) 2010 Samsung Electronics
#

#########################################################################

ifneq ($(OBJTREE),$(SRCTREE))
ifeq ($(CURDIR),$(SRCTREE))
dir :=
else
dir := $(subst $(SRCTREE)/,,$(CURDIR))
endif

obj := $(if $(dir),$(OBJTREE)/$(dir)/,$(OBJTREE)/)
src := $(if $(dir),$(SRCTREE)/$(dir)/,$(SRCTREE)/)

$(shell mkdir -p $(obj))
else
obj :=
src :=
endif

# clean the slate ...
PLATFORM_RELFLAGS =
PLATFORM_CPPFLAGS =
PLATFORM_LDFLAGS =

#########################################################################

HOSTCFLAGS	= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer \
		  $(HOSTCPPFLAGS)
HOSTSTRIP	= strip

#
# Mac OS X / Darwin's C preprocessor is Apple specific.  It
# generates numerous errors and warnings.  We want to bypass it
# and use GNU C's cpp.  To do this we pass the -traditional-cpp
# option to the compiler.  Note that the -traditional-cpp flag
# DOES NOT have the same semantics as GNU C's flag, all it does
# is invoke the GNU preprocessor in stock ANSI/ISO C fashion.
#
# Apple's linker is similar, thanks to the new 2 stage linking
# multiple symbol definitions are treated as errors, hence the
# -multiply_defined suppress option to turn off this error.
#

ifeq ($(HOSTOS),darwin)
HOSTCC		= cc
HOSTCFLAGS	+= -traditional-cpp
HOSTLDFLAGS	+= -multiply_defined suppress
else
HOSTCC		= gcc
endif

ifeq ($(HOSTOS),cygwin)
HOSTCFLAGS	+= -ansi
endif

# We build some files with extra pedantic flags to try to minimize things
# that won't build on some weird host compiler -- though there are lots of
# exceptions for files that aren't complaint.

HOSTCFLAGS_NOPED = $(filter-out -pedantic,$(HOSTCFLAGS))
HOSTCFLAGS	+= -pedantic

#########################################################################
#
# Option checker (courtesy linux kernel) to ensure
# only supported compiler options are used
#
cc-option = $(shell if $(CC) $(CFLAGS) $(1) -S -o /dev/null -xc /dev/null \
		> /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

#
# Include the make variables (CC, etc...)
#
AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
CPP	= $(CC) -E
AR	= $(CROSS_COMPILE)ar
NM	= $(CROSS_COMPILE)nm
LDR	= $(CROSS_COMPILE)ldr
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILE)RANLIB

#########################################################################

# Load generated board configuration
sinclude $(OBJTREE)/include/autoconf.mk

# Some architecture config.mk files need to know what CPUDIR is set to,
# so calculate CPUDIR before including ARCH/SOC/CPU config.mk files.
# Check if arch/$ARCH/cpu/$CPU exists, otherwise assume arch/$ARCH/cpu contains
# CPU-specific code.
CPUDIR=arch/$(ARCH)/cpu/$(CPU)
ifneq ($(SRCTREE)/$(CPUDIR),$(wildcard $(SRCTREE)/$(CPUDIR)))
CPUDIR=arch/$(ARCH)/cpu
endif

sinclude $(TOPDIR)/arch/$(ARCH)/config.mk	# include architecture dependend rules
sinclude $(TOPDIR)/$(CPUDIR)/config.mk		# include  CPU	specific rules

ifdef	SOC
sinclude $(TOPDIR)/$(CPUDIR)/$(SOC)/config.mk	# include  SoC	specific rules
endif
ifdef	VENDOR
BOARDDIR = $(VENDOR)/$(BOARD)
else
BOARDDIR = $(BOARD)
endif
ifdef	BOARD
sinclude $(TOPDIR)/$(RECOVERY_BLOCK)/board/$(BOARDDIR)/config.mk
endif

#########################################################################

# sinclude $(TOPDIR)/lib_$(ARCH)/config.mk

CROSS_COMPILE ?= arm-linux-

PLATFORM_CPPFLAGS += -DCONFIG_ARM -D__ARM__

# Explicitly specifiy 32-bit ARM ISA since toolchain default can be -mthumb:
PLATFORM_CPPFLAGS += $(call cc-option,-marm,)

# Try if EABI is supported, else fall back to old API,
# i. e. for example:
# - with ELDK 4.2 (EABI supported), use:
#	-mabi=aapcs-linux -mno-thumb-interwork
# - with ELDK 4.1 (gcc 4.x, no EABI), use:
#	-mabi=apcs-gnu -mno-thumb-interwork
# - with ELDK 3.1 (gcc 3.x), use:
#	-mapcs-32 -mno-thumb-interwork
PLATFORM_CPPFLAGS += $(call cc-option,\
				-mabi=aapcs-linux -mno-thumb-interwork,\
				$(call cc-option,\
					-mapcs-32,\
					$(call cc-option,\
						-mabi=apcs-gnu,\
					)\
				) $(call cc-option,-mno-thumb-interwork,)\
			)

# For EABI, make sure to provide raise()
ifneq (,$(findstring -mabi=aapcs-linux,$(PLATFORM_CPPFLAGS)))
# This file is parsed several times; make sure to add only once.
ifeq (,$(findstring lib_arm/eabi_compat.o,$(PLATFORM_LIBS)))
PLATFORM_LIBS += $(OBJTREE)/lib_arm/eabi_compat.o
endif
endif
LDSCRIPT := $(SRCTREE)/$(RECOVERY_BLOCK)/board/$(BOARDDIR)/recovery.lds

#########################################################################

# sinclude $(TOPDIR)/cpu/$(CPU)/config.mk
PLATFORM_RELFLAGS += -fno-strict-aliasing -fno-common -ffixed-r8 \
		     -msoft-float

# Make ARMv5 to allow more compilers to work, even though its v7a.
PLATFORM_CPPFLAGS += -march=armv5
# =========================================================================
#
# Supply options according to compiler version
#
# =========================================================================
PLATFORM_RELFLAGS +=$(call cc-option,-mshort-load-bytes,\
		    $(call cc-option,-malignment-traps,))

#########################################################################

ifneq (,$(findstring s,$(MAKEFLAGS)))
ARFLAGS = cr
else
ARFLAGS = crv
endif
RELFLAGS= $(PLATFORM_RELFLAGS)
DBGFLAGS= -g # -DDEBUG
OPTFLAGS= -Os #-fomit-frame-pointer
OBJCFLAGS += --gap-fill=0x00

gccincdir := $(shell $(CC) -print-file-name=include)

CPPFLAGS := $(DBGFLAGS) $(OPTFLAGS) $(RELFLAGS)		\
	-D__KERNEL__
ifneq ($(TEXT_BASE),)
CPPFLAGS += -DTEXT_BASE=$(TEXT_BASE)
endif

ifneq ($(RESET_VECTOR_ADDRESS),)
CPPFLAGS += -DRESET_VECTOR_ADDRESS=$(RESET_VECTOR_ADDRESS)
endif

ifneq ($(OBJTREE),$(SRCTREE))
CPPFLAGS += -I$(OBJTREE)/include2 -I$(OBJTREE)/include
endif

CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += -fno-builtin -ffreestanding -nostdinc	\
	-isystem $(gccincdir) -pipe $(PLATFORM_CPPFLAGS)

CPPFLAGS += -I$(TOPDIR)/$(RECOVERY_BLOCK)
CPPFLAGS += -DCONFIG_RECOVERY_BLOCK -D__HAVE_ARCH_MEMCPY32
CPPFLAGS += -DRECOVERY_DEBUG

ifdef BUILD_TAG
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes \
	-DBUILD_TAG='"$(BUILD_TAG)"'
else
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes
endif

CFLAGS += $(call cc-option,-fno-stack-protector)

# avoid trigraph warnings while parsing pci.h (produced by NIOS gcc-2.9)
# this option have to be placed behind -Wall -- that's why it is here
ifeq ($(ARCH),nios)
ifeq ($(findstring 2.9,$(shell $(CC) --version)),2.9)
CFLAGS := $(CPPFLAGS) -Wall -Wno-trigraphs
endif
endif

# $(CPPFLAGS) sets -g, which causes gcc to pass a suitable -g<format>
# option to the assembler.
AFLAGS_DEBUG :=

# turn jbsr into jsr for m68k
ifeq ($(ARCH),m68k)
ifeq ($(findstring 3.4,$(shell $(CC) --version)),3.4)
AFLAGS_DEBUG := -Wa,-gstabs,-S
endif
endif

AFLAGS := $(AFLAGS_DEBUG) -D__ASSEMBLY__ $(CPPFLAGS)

LDFLAGS += -Bstatic -T $(LDSCRIPT) $(PLATFORM_LDFLAGS)
ifneq ($(TEXT_BASE),)
LDFLAGS += -Ttext $(TEXT_BASE)
endif

#########################################################################

export	HOSTCC HOSTCFLAGS HOSTLDFLAGS PEDCFLAGS HOSTSTRIP CROSS_COMPILE \
	AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE
export	TEXT_BASE PLATFORM_CPPFLAGS PLATFORM_RELFLAGS CPPFLAGS CFLAGS AFLAGS

#########################################################################

# Allow boards to use custom optimize flags on a per dir/file basis
BCURDIR := $(notdir $(CURDIR))
$(obj)%.s:	%.S
	$(CPP) $(AFLAGS) $(AFLAGS_$(@F)) $(AFLAGS_$(BCURDIR)) -o $@ $<
$(obj)%.o:	%.S
	$(CC)  $(AFLAGS) $(AFLAGS_$(@F)) $(AFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.o:	%.c
	$(CC)  $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.i:	%.c
	$(CPP) $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.s:	%.c
	$(CC)  $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c -S

#########################################################################
