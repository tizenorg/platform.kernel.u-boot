#
# Copyright (C) 2016 Samsung Electronics
# Lukasz Majewski <l.majewski@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	+= clock.o
ifndef CONFIG_TPL_TM2
obj-y	+= power.o soc.o system.o pinmux.o tzpc.o
endif

obj-$(CONFIG_EXYNOS5420)	+= sec_boot.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_EXYNOS5)	+= clock_init_exynos5.o
obj-$(CONFIG_EXYNOS5)	+= dmc_common.o dmc_init_ddr3.o
obj-$(CONFIG_EXYNOS4210)+= dmc_init_exynos4.o clock_init_exynos4.o
obj-y	+= spl_boot.o
obj-y	+= lowlevel_init.o
endif
