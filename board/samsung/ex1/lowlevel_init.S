
#include <config.h>
#include <version.h>
#include <asm/arch/cpu.h>

#define PD_YEAR		0x5555
#define PD_MON		0xa
#define PD_DATE		0x35
#define PD_WEEK		0x7

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r11, lr

#ifndef CONFIG_ONENAND_IPL
rtc_set_prevent_powerup:
	ldr	r1, =DRIME3_RTC_BASE

	ldr	r0, =0x03
	str	r0, [r1]

	ldr	r0, =PD_YEAR
	ldr	r2, [r1, #0x20]		@ load RTC_AYEAR
	cmp	r0, r2
	bne	wakeup_off

	ldr	r0, =PD_MON
	ldr	r2, [r1, #0x1c]		@ load RTC_AMON
	cmp	r0, r2
	bne	wakeup_off

	ldr	r0, =PD_WEEK
	ldr	r2, [r1, #0x18]		@ load RTC_AWEEK
	cmp	r0, r2
	bne	wakeup_off

	ldr	r0, =PD_DATE
	ldr	r2, [r1, #0x14]		@ load RTC_ADATE
	cmp	r0, r2
	bne	wakeup_off
	b	set_drvier_current

wakeup_off:
	ldr	r0, =0x0
	str	r0, [r1, #0x4c]		@ store RTC_INTCLR

	ldr	r0, =PD_YEAR
	str	r0, [r1, #0x20]		@ store RTC_AYEAR

	ldr	r0, =PD_MON
	str	r0, [r1, #0x1c]		@ store RTC_AMON

	ldr	r0, =PD_WEEK
	str	r0, [r1, #0x18]		@ store RTC_AWEEK

	ldr	r0, =PD_DATE
	str	r0, [r1, #0x14]		@ store RTC_ADATE

	ldr	r0, =0x0
	str	r0, [r1]		@ store RTC_CTRL

	ldr	r1, =DRIME3_PCU_BASE
	ldr	r0, [r1]
	ldr	r2, =0xffffffdf
	and	r0, r0, r2
	str	r0, [r1]
	ldr	r2, =0xffffffbf
	ldr	r0, [r1]
	and	r0, r0, r2
	str	r0, [r1]

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

set_clock_gating:
	/* Enable All Clocks */
	ldr	r1, =0x90070000

	/* AUDIO */
	ldr	r0, =0x1
	str	r0, [r1, #0x400]

	/* I2C 0-2 */
	ldr	r0, =0x7
	str	r0, [r1, #0x40C]

	/* SDMMC/SDIO */
	ldr	r0, =0x3
	str	r0, [r1, #0x410]

	/* UART 0-1 */
	ldr	r0, =0x3
	str	r0, [r1, #0x414]

	/* SPI 0-7 */
	ldr	r0, =0xff
	str	r0, [r1, #0x418]

	/* PWM 0-2 */
	ldr	r0, =0x7
	str	r0, [r1, #0x41C]

	/* GPIO 0-10 */
	ldr	r0, =0x7ff
	str	r0, [r1, #0x420]
#endif

#ifdef CONFIG_ONENAND_IPL
set_system_memory:
	/*
	 * register usage
	 * r1 : DRAM base (???)
	 */
	ldr	r1, =0x80000000

	/* DQS Offset Value */
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

#if 1
	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b
#endif

#if 1
	ldr	r0, =0x200000F6
	str	r0, [r1, #0x1C]
#endif

	/* DDR ZQ Calibration */
	ldr	r0, =0xE3855731
	str	r0, [r1, #0x44]
	ldr	r0, =0xE3855733
	str	r0, [r1, #0x44]

#if 0
	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b
#endif

	/* LPCON setting for 333MHz */
	ldr	r0, =0x3FFF205A
	str	r0, [r1, #0x0]
	ldr	r0, =0x00202440
	str	r0, [r1, #0x4]
	ldr	r0, =0xC0F01313
	str	r0, [r1, #0x8]
	ldr	r0, =0xC0F00313
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

	/* Memory Device Initialization */
	ldr	r0, =0x07000000
	str	r0, [r1, #0x10]

#if 1
	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b
#endif

	ldr	r0, =0x01000000
	str	r0, [r1, #0x10]

#if 0
	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b
#endif

	ldr	r0, =0x00020000
	str	r0, [r1, #0x10]
	ldr	r0, =0x00030000
	str	r0, [r1, #0x10]
	ldr	r0, =0x00010000
	str	r0, [r1, #0x10]
	ldr	r0, =0x00000952
	str	r0, [r1, #0x10]

	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b

	ldr	r0, =0x01000000
	str	r0, [r1, #0x10]
	str	r0, [r1, #0x10]
	ldr	r0, =0x05000000
	str	r0, [r1, #0x10]
	str	r0, [r1, #0x10]
	ldr	r0, =0x00000852
	str	r0, [r1, #0x10]
	ldr	r0, =0x00010380
	str	r0, [r1, #0x10]
	ldr	r0, =0x00010000
	str	r0, [r1, #0x10]

	/* LPCON normal refresh start */
	ldr	r0, =0x3FFF207A
	str	r0, [r1]

#if 0
	mov	r2, #0x10000
1:	subs	r2, r2, #1
	bne	1b
#endif
#endif

#ifndef CONFIG_ONENAND_IPL
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
