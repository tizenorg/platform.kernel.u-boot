/* include/asm/arch/map-base.h
 *
 * Author: InKi Dae <inki.dae@samsung.com> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef ___ASM_ARCH_MAP_BASE_H
#define ___ASM_ARCH_MAP_BASE_H

#define S5PC_ADDR_BASE	(0xE1F00000)

#define S5PC_ADDR(x)	(S5PC_ADDR_BASE + (x))

#define S5PC_LCD_BASE	S5PC_ADDR(0xC100000)	/* Display Controller */

#endif
