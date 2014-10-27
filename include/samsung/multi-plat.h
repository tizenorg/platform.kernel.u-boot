#ifndef __INCLUDE__MULTI_PLAT__H__
#define __INCLUDE__MULTI_PLAT__H__

#include <common.h>

#ifdef CONFIG_FDTDEC_MEMORY
struct memory_info {
	uint64_t addr[CONFIG_NR_DRAM_BANKS];
	uint64_t size[CONFIG_NR_DRAM_BANKS];
	uint32_t banks;
};
#endif
#endif
