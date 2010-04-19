#
# Copyright (C) 2010 # Samsung Elecgtronics
#

# Recovery block' size should be 1 block (256K)
# Recovery block includes the onenand_ipl(16K), so actual size is 240K
TEXT_BASE 	= 0x34000000
# 256K - 16K(IPL Size)
TEXT_BASE_256K 	= 0x3403C000

AFLAGS += -DCONFIG_RECOVERY_BLOCK -g -UTEXT_BASE -DTEXT_BASE=$(TEXT_BASE)
CFLAGS += -DCONFIG_RECOVERY_BLOCK -g -D__HAVE_ARCH_MEMCPY32

