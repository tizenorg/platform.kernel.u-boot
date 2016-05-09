/*
 * (C) Copyright 2016 Samsung
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <asm/armv8/mmu.h>
#include <asm/arch/dwmmc.h>

DECLARE_GLOBAL_DATA_PTR;

/* Information about a serial port */
struct s5p_serial_platdata {
	struct s5p_uart *reg;  /* address of registers in physical memory */
	u8 port_id;     /* uart port number */
};

static const struct s5p_serial_platdata serial1_platdata = {
	.reg = (struct s5p_uart *)0x14C20000,
	.port_id = 1,
};

U_BOOT_DEVICE(serial1) = {
	.name	= "serial_s5p",
	.platdata = &serial1_platdata,
};

static struct mm_region draco_mem_map[] = {
	{
		.base = 0x00000000UL,
		.size = 0x20000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.base = 0x20000000UL,
		.size = 0xC0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = draco_mem_map;


int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}

void reset_cpu(ulong addr)
{
	writel(0x1, (void *)0x105C0400);
}

void lowlevel_init(void)
{
}

void apply_core_errata(void)
{
}

void smp_kick_all_cpus(void)
{
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int ret;
	/* dwmmc initializattion for available channels */
	ret = exynos_dwmmc_init(gd->fdt_blob);
	if (ret)
		debug("dwmmc init failed\n");

	return ret;
}
#endif

int checkboard(void)
{
	const char *board_info;

	board_info = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	printf("Board: %s\n", board_info ? board_info : "unknown");

	return 0;
}
