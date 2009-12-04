/*
 * Sleep command for S5PC110
 *
 *  Copyright (C) 2005-2008 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <i2c.h>

#include <asm/io.h>

#include <asm/arch/cpu.h>
#include <asm/arch/power.h>
#include <asm/arch/sys_proto.h>

#define CONFIG_CPU_S5PC110_EVT0_ERRATA

#include <common.h>
#include "sleep.h"

extern int s5pc110_cpu_save(unsigned long *saveblk);
extern void s5pc110_cpu_resume(void);
extern unsigned int s5pc110_sleep_return_addr;
extern unsigned int s5pc110_sleep_save_phys;

struct stack {
	u32 irq[3];
	u32 abt[3];
	u32 und[3];
} ____cacheline_aligned;

static struct stack stacks[1];

enum {
	SLEEP_WFI,
	SLEEP_REGISTER,
};

static void __board_sleep_init(void) { }

void board_sleep_init(void)
	__attribute__((weak, alias("__board_sleep_init")));

struct regs_to_save {
	unsigned int	start_address;
	unsigned int	size;
};
static struct regs_to_save core_save[] = {
	{ .start_address=0xE0100200, .size=7},
	{ .start_address=0xE0100280, .size=2},
	{ .start_address=0xE0100300, .size=8},
	{ .start_address=0xE0100460, .size=5},
	{ .start_address=0xE0100480, .size=3},
	{ .start_address=0xE0100500, .size=1},
	{ .start_address=0xE0107008, .size=1},
};
static unsigned int buf_core_save[7+2+8+5+3+1+1];
static struct regs_to_save gpio_save[] = {
	{ .start_address=0xE0200000, .size=6},
	{ .start_address=0xE0200020, .size=6},
	{ .start_address=0xE0200040, .size=6},
	{ .start_address=0xE0200060, .size=6},
	{ .start_address=0xE0200080, .size=6},
	{ .start_address=0xE02000A0, .size=6},
	{ .start_address=0xE02000C0, .size=6},
	{ .start_address=0xE02000E0, .size=6},
	{ .start_address=0xE0200100, .size=6},
	{ .start_address=0xE0200120, .size=6},
	{ .start_address=0xE0200140, .size=6},
	{ .start_address=0xE0200160, .size=6},
	{ .start_address=0xE0200180, .size=6},
	{ .start_address=0xE02001A0, .size=6},
	{ .start_address=0xE02001C0, .size=6},
	{ .start_address=0xE02001E0, .size=6},
	{ .start_address=0xE0200200, .size=6},
	{ .start_address=0xE0200220, .size=6},
	{ .start_address=0xE0200240, .size=6},
	{ .start_address=0xE0200260, .size=6},
	{ .start_address=0xE0200280, .size=6},
	{ .start_address=0xE02002A0, .size=6},
	{ .start_address=0xE02002C0, .size=6},
	{ .start_address=0xE02002E0, .size=6},
	{ .start_address=0xE0200300, .size=6},
	{ .start_address=0xE0200320, .size=6},
	{ .start_address=0xE0200340, .size=6},
	{ .start_address=0xE0200360, .size=6},
	{ .start_address=0xE0200380, .size=6},
	{ .start_address=0xE02003A0, .size=6},
	{ .start_address=0xE02003C0, .size=6},
	{ .start_address=0xE02003E0, .size=6},
	{ .start_address=0xE0200400, .size=6},
	{ .start_address=0xE0200420, .size=6},
	{ .start_address=0xE0200440, .size=6},
	{ .start_address=0xE0200460, .size=6},
	{ .start_address=0xE0200480, .size=6},
	{ .start_address=0xE02004A0, .size=6},
	{ .start_address=0xE02004C0, .size=6},
	{ .start_address=0xE02004E0, .size=6},
	{ .start_address=0xE0200500, .size=6},
	{ .start_address=0xE0200520, .size=6},
	{ .start_address=0xE0200540, .size=6},
	{ .start_address=0xE0200560, .size=6},
	{ .start_address=0xE0200580, .size=6},
	{ .start_address=0xE02005A0, .size=6},
	{ .start_address=0xE02005C0, .size=6},
	{ .start_address=0xE02005E0, .size=6},
	{ .start_address=0xE0200700, .size=22},
	{ .start_address=0xE0200900, .size=22},
	{ .start_address=0xE0200A00, .size=22},
};
static unsigned int buf_gpio_save[ 6*8*6 + 22*3 ];
static struct regs_to_save irq_save[] = {
	{ .start_address=0xF200000C, .size=2},
	{ .start_address=0xF2000018, .size=1},
	{ .start_address=0xF210000C, .size=2},
	{ .start_address=0xF2100018, .size=1},
	{ .start_address=0xF220000C, .size=2},
	{ .start_address=0xF2200018, .size=1},
	{ .start_address=0xF230000C, .size=2},
	{ .start_address=0xF2300018, .size=1},
};
static unsigned int buf_irq_save[ (2+1)*4 ];
static struct regs_to_save sromc_save[] = {
	{ .start_address=0xE8000000, .size=7},
};
static unsigned int buf_sromc_save[7];
static struct regs_to_save uart_save[] = {
	{ .start_address=0xE2900000, .size=4},
	{ .start_address=0xE2900028, .size=2},
	{ .start_address=0xE2900038, .size=1},
	{ .start_address=0xE2900400, .size=4},
	{ .start_address=0xE2900428, .size=2},
	{ .start_address=0xE2900438, .size=1},
	{ .start_address=0xE2900800, .size=4},
	{ .start_address=0xE2900828, .size=2},
	{ .start_address=0xE2900838, .size=1},
	{ .start_address=0xE2900C00, .size=4},
	{ .start_address=0xE2900C28, .size=2},
	{ .start_address=0xE2900C38, .size=1},
};
static unsigned int buf_uart_save[(4+2+1)*4];

static unsigned int reg_others;

void s5pc110_save_reg(struct regs_to_save * list, unsigned int * buf, int length)
{
	int i;
	int counter = 0;
	for (i=0; i<length; i++) {
		int j;
		for (j=0; j<list[i].size; j++) {
			(*buf) = readl((unsigned int *)(list[i].start_address+j*4));
			buf++;
			counter++;
		}
	}
}
void s5pc110_save_regs(void)
{
	reg_others = readl(S5PC110_OTHERS);
	s5pc110_save_reg(gpio_save, buf_gpio_save, ARRAY_SIZE(gpio_save));
	s5pc110_save_reg(irq_save, buf_irq_save, ARRAY_SIZE(irq_save));
	s5pc110_save_reg(core_save, buf_core_save, ARRAY_SIZE(core_save));
	s5pc110_save_reg(sromc_save, buf_sromc_save, ARRAY_SIZE(sromc_save));
	s5pc110_save_reg(uart_save, buf_uart_save, ARRAY_SIZE(uart_save));
}
void s5pc110_restore_reg(struct regs_to_save * list, unsigned int * buf, int length)
{
	int i;
#ifdef CONFIG_CPU_S5PC110_EVT0_ERRATA
	unsigned int tmp;
#endif
	for (i=0; i<length; i++) {
		int j;
		for (j=0; j<list[i].size; j++) {
			writel(*buf, (unsigned int *)(list[i].start_address+j*4));
#ifdef CONFIG_CPU_S5PC110_EVT0_ERRATA
			tmp = readl((unsigned int *)(list[i].start_address+j*4));
#endif
			buf++;
		}
	}

}
void s5pc110_restore_regs(void)
{
	s5pc110_restore_reg(uart_save, buf_uart_save, ARRAY_SIZE(uart_save));
	s5pc110_restore_reg(sromc_save, buf_sromc_save, ARRAY_SIZE(sromc_save));
	s5pc110_restore_reg(core_save, buf_core_save, ARRAY_SIZE(core_save));
	s5pc110_restore_reg(irq_save, buf_irq_save, ARRAY_SIZE(irq_save));
	s5pc110_restore_reg(gpio_save, buf_gpio_save, ARRAY_SIZE(gpio_save));
}

void s5pc110_wakeup(void)
{
	struct stack *stk = &stacks[0];
	__asm__ (
			"msr    cpsr_c, %1\n\t"
			"add    sp, %0, %2\n\t"
			"msr    cpsr_c, %3\n\t"
			"add    sp, %0, %4\n\t"
			"msr    cpsr_c, %5\n\t"
			"add    sp, %0, %6\n\t"
			"msr    cpsr_c, %7"
			:
			: "r" (stk),
			"I" (PSR_F_BIT | PSR_I_BIT | IRQ_MODE),
			"I" (offsetof(struct stack, irq[0])),
			"I" (PSR_F_BIT | PSR_I_BIT | ABT_MODE),
			"I" (offsetof(struct stack, abt[0])),
			"I" (PSR_F_BIT | PSR_I_BIT | UND_MODE),
			"I" (offsetof(struct stack, und[0])),
			"I" (PSR_F_BIT | PSR_I_BIT | SVC_MODE)
			: "r14");


	s5pc110_restore_regs();

	reg_others |= 1<<28;
	writel(reg_others, S5PC110_OTHERS);

	printf("%s: Waking up...\n", __func__);

	timer_init();
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE); /* init_func_i2c */
#endif
#if defined(CONFIG_CMD_ONENAND)
	onenand_init();
#endif  
#ifdef CONFIG_SERIAL_MULTI
	serial_initialize();
#endif
	stdio_init_resume ();   /* get the devices list going. */

	return;
}

static int s5pc110_sleep(int mode)
{
	static int counter;
	unsigned long regs_save[16];
	unsigned int value;
	int i;

	printf("Entering s5pc110_sleep();\n");

	board_sleep_init();

	value = readl(S5PC110_WAKEUP_MASK);
	value |= (1 << 15);
	value |= (1 << 14);
	value |= (1 << 13);
	value |= (1 << 12);
	value |= (1 << 11);
	value |= (1 << 10);
	value |= (1 << 9);
	value |= (1 << 5);
	value |= (1 << 4);
	value |= (1 << 3);
	value |= (1 << 2);
	value |= (1 << 1);

	writel(value, S5PC110_WAKEUP_MASK);

	value = readl(S5PC110_EINT_WAKEUP_MASK);
	value = 0xFFFFFFFF;
	value &= ~(1 << 7);       /* AP_PMIC_IRQ */
	value &= ~(1 << 22);      /* nPOWER */
	/*      value &= ~(1 << 23);*/    /* JACK_nINT */
	value &= ~(1 << 24);      /* KBR(0) */
	value &= ~(1 << 25);      /* KBR(1) */
	/*      value &= ~(1 << 28);*/    /* T-Flash */
	writel(value, S5PC110_EINT_WAKEUP_MASK);
#ifdef CONFIG_CPU_S5PC110_EVT0_ERRATA
	value = readl(S5PC110_EINT_WAKEUP_MASK);
        for (i = 0; i < 4; i++)
		value = readl(0xE0200F40+i*4);
#endif

	s5pc110_save_regs();
	writel((unsigned long) s5pc110_cpu_resume, S5PC110_INFORM0);

#ifdef CONFIG_CPU_S5PC110_EVT0_ERRATA
	value = readl(S5PC110_SLEEP_CFG);
	value &= ~(1 << 0); /* OSC_EN off */
	value &= ~(1 << 1); /* USBOSC_EN off */
	writel(value, S5PC110_SLEEP_CFG);
#endif

	value = readl(S5PC110_PWR_CFG);
	value &= ~S5PC110_CFG_STANDBYWFI_MASK;
	if (mode == SLEEP_WFI)
#ifdef CONFIG_CPU_S5PC110_EVT0_ERRATA
	{
		printf("ERRATA MODE\n");
		value |= S5PC110_CFG_STANDBYWFI_IGNORE;
	}
#else
		value |= S5PC110_CFG_STANDBYWFI_SLEEP;
#endif
	else
		value |= S5PC110_CFG_STANDBYWFI_IGNORE;
	writel(value, S5PC110_PWR_CFG);

	/* F2000000: VICIRQSTATUS-irq0 */
	writel(0xffffffff, 0xF2000000 + 0x14); /* VIC_INT_ENABLE_CLEAR */
	writel(0xffffffff, 0xF2100000 + 0x14);
	writel(0xffffffff, 0xF2200000 + 0x14);
	writel(0xffffffff, 0xF2300000 + 0x14);
	writel(0xffffffff, 0xF2000000 + 0x1c); /* VIC_INT_SOFT_CLEAR */
	writel(0xffffffff, 0xF2100000 + 0x1c);
	writel(0xffffffff, 0xF2200000 + 0x1c);
	writel(0xffffffff, 0xF2300000 + 0x1c);

	/* Clear all EINT PENDING bit */
	writel(0xff, 0xE0200000 + 0xF40);
	value = readl(0xE0200000 + 0xF40);
	writel(0xff, 0xE0200000 + 0xF44);
	value = readl(0xE0200000 + 0xF44);
	writel(0xff, 0xE0200000 + 0xF48);
	value = readl(0xE0200000 + 0xF48);
	writel(0xff, 0xE0200000 + 0xF4C);
	value = readl(0xE0200000 + 0xF4C);

	value = readl(S5PC110_WAKEUP_STAT);
	writel(value, S5PC110_WAKEUP_STAT);


	value = readl(S5PC110_OTHERS);
	value |= S5PC110_OTHERS_SYSCON_INT_DISABLE;
	writel(value, S5PC110_OTHERS);

	s5pc110_sleep_save_phys = (unsigned int) regs_save;

	value = readl(S5PC110_OTHERS);
	value |= 1;
	writel(value, S5PC110_OTHERS);

	/* cache flush */
	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (0)); 
	l2_cache_disable();
	invalidate_dcache(get_device_type());

	if (s5pc110_cpu_save(regs_save) == 0) {
		/* cache flush */
		asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (0)); 
		l2_cache_disable();
		invalidate_dcache(get_device_type());

		if (mode == SLEEP_WFI) {
#ifdef CONFIG_CPU_S5PC110_EVT0_ERRATA
			printf("Warn: Entering SLEEP_WFI mode with EVT0_ERRATA. \n");
			printf("Warn: This sleep will probably fail. \n");
#endif
			value = readl(S5PC110_PWR_CFG);
			value &= ~S5PC110_CFG_STANDBYWFI_MASK;
			value |= S5PC110_CFG_STANDBYWFI_SLEEP;
			writel(value, S5PC110_PWR_CFG);

			asm("b	1f\n\t"
					".align 5\n\t"
					"1:\n\t"
					"mcr p15, 0, %0, c7, c10, 5\n\t"
					"mcr p15, 0, %0, c7, c10, 4\n\t"
					".word 0xe320f003" :: "r" (value));

		} else { /* SLEEP_REGISTER */
			value = (1 << 2);
			writel(value, S5PC110_PWR_MODE);
			while(1);
		}
	}


	s5pc110_wakeup();

	writel(0, S5PC110_EINT_WAKEUP_MASK);
	readl(S5PC110_EINT_WAKEUP_MASK);
	for (i = 0; i < 4; i++)
		readl(0xE0200F40+i*4);
	value = readl(S5PC110_WAKEUP_STAT);
	writel(0xFFFF & value, S5PC110_WAKEUP_STAT);

	printf("Wakeup Source: 0x%08x\n", value);
	value = readl(S5PC110_WAKEUP_STAT);

	show_hw_revision();

	counter++;
	board_sleep_resume();
	return 0;
}

int do_sleep(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int mode = SLEEP_WFI;

	if (argc >= 2)
		mode = SLEEP_REGISTER;

	if (cpu_is_s5pc110())
		return s5pc110_sleep(mode);

	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	sleep,		CONFIG_SYS_MAXARGS,	1, do_sleep,
	"S5PC110 sleep",
	"[sleep mode]\n# sleep: Sleep with SLEEP_WFI mode\n# sleep 1: Sleep with SLEEP_REGISTER mode\n"
);

