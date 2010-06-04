
#include <config.h>
#include <version.h>
#include <asm/arch/cpu.h>

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r11, lr

#ifndef CONFIG_ONENAND_IPL
set_drvier_current:
	ldr	r1, =0x900D0078
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D007C
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D0080
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D0084
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D0088
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D008C
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D0090
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D0094
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D0098
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D009C
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D00A0
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D00A4
	ldr	r0, =0x43434343
	str	r0, [r1]
	ldr	r1, =0x900D00A8
	ldr	r0, =0x50504343
	str	r0, [r1]

set_system_clock:
	ldr	r1, =0x90090000
	ldr	r0, =0x00003000
	str	r0, [r1]
	ldr	r1, =0x90090008
	ldr	r0, =0xD6070814
	str	r0, [r1]

set_uart_clock:
	ldr	r1, =0x90070414
	ldr	r0, =0x1
	str	r0, [r1]
#endif

#ifdef CONFIG_ONENAND_IPL
set_system_memory:
	/* DQS Offset Value */
	ldr	r1, =0x80000000

	ldr	r0, =0x58585858
	str	r0, [r1, #0x20]
	ldr	r0, =0x03030303
	str	r0, [r1, #0x24]

	/* DDRPHY DLL Lock */
	ldr	r0, =0x200000F6
	str	r0, [r1, #0x1C]
	ldr	r0, =0x0010100A
	str	r0, [r1, #0x18]
	ldr	r0, =0x0010100B
	str	r0, [r1, #0x18]

	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b

#if 0
	ldr	r0, =0x200000F6
	str	r0, [r1, #0x1C]
#endif

	ldr	r0, =0xE3855731
	str	r0, [r1, #0x44]
	ldr	r0, =0xE3855733
	str	r0, [r1, #0x44]

	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b

	ldr	r0, =0x3FFF205A
	str	r0, [r1]
	ldr	r0, =0x00202440
	str	r0, [r1, #0x4]
	ldr	r0, =0xC0F01313
	str	r0, [r1, #0x8]
	ldr	r0, =0xC0001313
	str	r0, [r1, #0xC]
	ldr	r0, =0xFF000000
	str	r0, [r1, #0x14]
	ldr	r0, =0xFFFF00FF
	str	r0, [r1, #0x28]
	ldr	r0, =0x00000A28
	str	r0, [r1, #0x30]
	ldr	r0, =0x2B46550F
	str	r0, [r1, #0x34]
	ldr	r0, =0x35350304
	str	r0, [r1, #0x38]
	ldr	r0, =0x11C80232
	str	r0, [r1, #0x3C]

	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b

	ldr	r1, =0x80000010
	ldr	r0, =0x07000000
	str	r0, [r1]

	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b

	ldr	r0, =0x01000000
	str	r0, [r1]

	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b

	ldr	r0, =0x00020000
	str	r0, [r1]
	ldr	r0, =0x00030000
	str	r0, [r1]
	ldr	r0, =0x00010000
	str	r0, [r1]
	ldr	r0, =0x00000952
	str	r0, [r1]

	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b

	ldr	r0, =0x01000000
	str	r0, [r1]
	str	r0, [r1]
	ldr	r0, =0x05000000
	str	r0, [r1]
	str	r0, [r1]
	ldr	r0, =0x00000852
	str	r0, [r1]
	ldr	r0, =0x00010380
	str	r0, [r1]
	ldr	r0, =0x00010000
	str	r0, [r1]

	ldr	r1, =0x80000000
	ldr	r0, =0x3FFF207A
	str	r0, [r1]

	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b
#endif

#ifdef CONFIG_ONENAND_IPL
set_gpio:
	ldr	r1, =0x90040000
	ldr	r0, =0x0
	str	r0, [r1]
	ldr	r1, =0x90040004
	ldr	r0, =0x1
	str	r0, [r1]
#endif

done:
	mov	lr, r11
	mov	pc, lr
