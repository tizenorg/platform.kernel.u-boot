
#include <config.h>
#include <version.h>
#include <asm/arch/cpu.h>

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r11, lr

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

set_gpio:
	ldr	r1, =0x90040000
	ldr	r0, =0x0
	str	r0, [r1]
	ldr	r1, =0x90040004
	ldr	r0, =0x1
	str	r0, [r1]

done:
	mov	lr, r11
	mov	pc, lr
