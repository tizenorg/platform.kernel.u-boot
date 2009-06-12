/*
 * (C) Copyright 2009
 * Samsung Electronics, <www.samsung.com/sec>
 * Heungjun Kim <riverful.kim@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef _CPU_H
#define _CPU_H

#include <asm/hardware.h>

#ifndef __S5PC100_H__
#define __S5PC100_H__

#define S5P_ADDR_BASE		(0xe0000000)
#define S5P_ADDR(x)		(S5P_ADDR_BASE + (x))

#define S5P_PA_CLK		S5P_ADDR(0x00100000)	/* Clock Base */
#define S5P_PA_PWR		S5P_ADDR(0x00108000)	/* Power Base */
#define S5P_PA_CLK_OTHERS	S5P_ADDR(0x00200000)	/* Clock Others Base */
#define S5P_PA_GPIO		S5P_ADDR(0x00300000)    /* GPIO Base */
#define S5P_PA_VIC0		S5P_ADDR(0x04000000)    /* Vector Interrupt Controller 0 */
#define S5P_PA_VIC1		S5P_ADDR(0x04100000)    /* Vector Interrupt Controller 1 */
#define S5P_PA_VIC2		S5P_ADDR(0x04200000)    /* Vector Interrupt Controller 2 */
#define S5P_PA_DMC		S5P_ADDR(0x06000000)    /* Dram Memory Controller */
#define S5P_PA_SROMC		S5P_ADDR(0x07000000)    /* SROM Controller */
#define S5P_PA_WATCHDOG		S5P_ADDR(0x0a200000)    /* Watchdog Timer */
#define S5P_PA_PWMTIMER		S5P_ADDR(0x0a000000)    /* PWM Timer */

/* 
 * Chip ID
 */
#define S5P_ID(x)		(S5P_PA_ID + (x))

#define S5P_PRO_ID		S5P_ID(0)
#define S5P_OMR			S5P_ID(4)

/*
 * Clock control
 */
#define S5P_CLKREG(x)		(S5P_PA_CLK + (x))

/* Clock Register */
#define S5P_APLL_LOCK		S5P_CLKREG(0x0)
#define S5P_MPLL_LOCK		S5P_CLKREG(0x4)
#define S5P_EPLL_LOCK		S5P_CLKREG(0x8)
#define S5P_HPLL_LOCK		S5P_CLKREG(0xc)

#define S5P_APLL_CON		S5P_CLKREG(0x100)
#define S5P_MPLL_CON		S5P_CLKREG(0x104)
#define S5P_EPLL_CON		S5P_CLKREG(0x108)
#define S5P_HPLL_CON		S5P_CLKREG(0x10c)

#define S5P_CLK_SRC0		S5P_CLKREG(0x200)
#define S5P_CLK_SRC1		S5P_CLKREG(0x204)
#define S5P_CLK_SRC2		S5P_CLKREG(0x208)
#define S5P_CLK_SRC3		S5P_CLKREG(0x20c)

#define S5P_CLK_DIV0		S5P_CLKREG(0x300)
#define S5P_CLK_DIV1		S5P_CLKREG(0x304)
#define S5P_CLK_DIV2		S5P_CLKREG(0x308)
#define S5P_CLK_DIV3		S5P_CLKREG(0x30c)
#define S5P_CLK_DIV4		S5P_CLKREG(0x310)

#define S5P_CLK_OUT		S5P_CLKREG(0x400)

#define S5P_CLK_GATE_D00	S5P_CLKREG(0x500)
#define S5P_CLK_GATE_D01	S5P_CLKREG(0x504)
#define S5P_CLK_GATE_D02	S5P_CLKREG(0x508)

#define S5P_CLK_GATE_D10	S5P_CLKREG(0x520)
#define S5P_CLK_GATE_D11	S5P_CLKREG(0x524)
#define S5P_CLK_GATE_D12	S5P_CLKREG(0x528)
#define S5P_CLK_GATE_D13	S5P_CLKREG(0x530)
#define S5P_CLK_GATE_D14	S5P_CLKREG(0x534)

#define S5P_CLK_GATE_D20	S5P_CLKREG(0x540)

#define S5P_CLK_GATE_SCLK0	S5P_CLKREG(0x560)
#define S5P_CLK_GATE_SCLK1	S5P_CLKREG(0x564)

#ifndef __ASSEMBLY__
/* Clock Address */
#define S5P_APLL_LOCK_REG	__REG(S5P_APLL_LOCK)
#define S5P_MPLL_LOCK_REG	__REG(S5P_MPLL_LOCK)
#define S5P_EPLL_LOCK_REG	__REG(S5P_EPLL_LOCK)
#define S5P_HPLL_LOCK_REG	__REG(S5P_HPLL_LOCK)

#define S5P_APLL_CON_REG	__REG(S5P_APLL_CON)
#define S5P_MPLL_CON_REG	__REG(S5P_MPLL_CON)
#define S5P_EPLL_CON_REG	__REG(S5P_EPLL_CON)
#define S5P_HPLL_CON_REG	__REG(S5P_HPLL_CON)

#define S5P_CLK_SRC0_REG	__REG(S5P_CLK_SRC0)
#define S5P_CLK_SRC1_REG	__REG(S5P_CLK_SRC1)
#define S5P_CLK_SRC2_REG	__REG(S5P_CLK_SRC2)
#define S5P_CLK_SRC3_REG	__REG(S5P_CLK_SRC3)

#define S5P_CLK_DIV0_REG	__REG(S5P_CLK_DIV0)
#define S5P_CLK_DIV1_REG	__REG(S5P_CLK_DIV1)
#define S5P_CLK_DIV2_REG	__REG(S5P_CLK_DIV2)
#define S5P_CLK_DIV3_REG	__REG(S5P_CLK_DIV3)
#define S5P_CLK_DIV4_REG	__REG(S5P_CLK_DIV4)

#define S5P_CLK_OUT_REG		__REG(S5P_CLK_OUT)

#define S5P_CLK_GATE_D00_REG	__REG(S5P_CLK_GATE_D00)
#define S5P_CLK_GATE_D01_REG	__REG(S5P_CLK_GATE_D01)
#define S5P_CLK_GATE_D02_REG	__REG(S5P_CLK_GATE_D02)

#define S5P_CLK_GATE_D10_REG	__REG(S5P_CLK_GATE_D10)
#define S5P_CLK_GATE_D11_REG	__REG(S5P_CLK_GATE_D11)
#define S5P_CLK_GATE_D12_REG	__REG(S5P_CLK_GATE_D12)
#define S5P_CLK_GATE_D13_REG	__REG(S5P_CLK_GATE_D13)
#define S5P_CLK_GATE_D14_REG	__REG(S5P_CLK_GATE_D14)

#define S5P_CLK_GATE_D20_REG	__REG(S5P_CLK_GATE_D20)

#define S5P_CLK_GATE_SCLK0_REG	__REG(S5P_CLK_GATE_SCLK0)
#define S5P_CLK_GATE_SCLK1_REG	__REG(S5P_CLK_GATE_SCLK1)
#endif	/* __ASSENBLY__ */


/*
 * Power control
 */
#define S5P_PWRREG(x)			(S5P_PA_PWR + (x))

#define S5P_PWR_CFG			S5P_PWRREG(0x0)
#define S5P_EINT_WAKEUP_MASK		S5P_PWRREG(0x04)
#define S5P_NORMAL_CFG			S5P_PWRREG(0x10)
#define S5P_STOP_CFG			S5P_PWRREG(0x14)
#define S5P_SLEEP_CFG			S5P_PWRREG(0x18)
#define S5P_STOP_MEM_CFG		S5P_PWRREG(0x1c)
#define S5P_OSC_FREQ			S5P_PWRREG(0x100)
#define S5P_OSC_STABLE			S5P_PWRREG(0x104)
#define S5P_PWR_STABLE			S5P_PWRREG(0x108)
#define S5P_INTERNAL_PWR_STABLE		S5P_PWRREG(0x110)
#define S5P_CLAMP_STABLE		S5P_PWRREG(0x114)
#define S5P_OTHERS			S5P_PWRREG(0x200)
#define S5P_RST_STAT			S5P_PWRREG(0x300)
#define S5P_WAKEUP_STAT			S5P_PWRREG(0x304)
#define S5P_BLK_PWR_STAT		S5P_PWRREG(0x308)
#define S5P_INFORM0			S5P_PWRREG(0x400)
#define S5P_INFORM1			S5P_PWRREG(0x404)
#define S5P_INFORM2			S5P_PWRREG(0x408)
#define S5P_INFORM3			S5P_PWRREG(0x40c)
#define S5P_INFORM4			S5P_PWRREG(0x410)
#define S5P_INFORM5			S5P_PWRREG(0x414)
#define S5P_INFORM6			S5P_PWRREG(0x418)
#define S5P_INFORM7			S5P_PWRREG(0x41c)
#define S5P_DCGIDX_MAP0			S5P_PWRREG(0x500)
#define S5P_DCGIDX_MAP1			S5P_PWRREG(0x504)
#define S5P_DCGIDX_MAP2			S5P_PWRREG(0x508)
#define S5P_DCGPERF_MAP0		S5P_PWRREG(0x50c)
#define S5P_DCGPERF_MAP1		S5P_PWRREG(0x510)
#define S5P_DVCIDX_MAP			S5P_PWRREG(0x514)
#define S5P_FREQ_CPU			S5P_PWRREG(0x518)
#define S5P_FREQ_DPM			S5P_PWRREG(0x51c)
#define S5P_DVSEMCLK_EN			S5P_PWRREG(0x520)
#define S5P_APLL_CON_L8			S5P_PWRREG(0x600)
#define S5P_APLL_CON_L7			S5P_PWRREG(0x604)
#define S5P_APLL_CON_L6			S5P_PWRREG(0x608)
#define S5P_APLL_CON_L5			S5P_PWRREG(0x60c)
#define S5P_APLL_CON_L4			S5P_PWRREG(0x610)
#define S5P_APLL_CON_L3			S5P_PWRREG(0x614)
#define S5P_APLL_CON_L2			S5P_PWRREG(0x618)
#define S5P_APLL_CON_L1			S5P_PWRREG(0x61c)
#define S5P_EM_CONTROL			S5P_PWRREG(0x620)

#define S5P_CLKDIV_IEM_L8		S5P_PWRREG(0x700)
#define S5P_CLKDIV_IEM_L7		S5P_PWRREG(0x704)
#define S5P_CLKDIV_IEM_L6		S5P_PWRREG(0x708)
#define S5P_CLKDIV_IEM_L5		S5P_PWRREG(0x70c)
#define S5P_CLKDIV_IEM_L4		S5P_PWRREG(0x710)
#define S5P_CLKDIV_IEM_L3		S5P_PWRREG(0x714)
#define S5P_CLKDIV_IEM_L2		S5P_PWRREG(0x718)
#define S5P_CLKDIV_IEM_L1		S5P_PWRREG(0x71c)

#define S5P_IEM_HPMCLK_DIV		S5P_PWRREG(0x724)


/* 
 * Vector Interrupt Controller
 * : VIC0, VIC1, VIC2
 */
/* VIC0 */
#define S5P_VIC0_BASE(x)		(S5P_PA_VIC0 + (x))
#define S5P_VIC1_BASE(x)		(S5P_PA_VIC1 + (x))
#define S5P_VIC2_BASE(x)		(S5P_PA_VIC2 + (x))

/* Vector Interrupt Offset */
#define VIC_IRQSTATUS_OFFSET		0x0	/* IRQ Status Register */
#define VIC_FIQSTATUS_OFFSET		0x4	/* FIQ Status Register */
#define VIC_RAWINTR_OFFSET		0x8	/* Raw Interrupt Status Register */
#define VIC_INTSELECT_OFFSET		0xc	/* Interrupt Select Register */
#define VIC_INTENABLE_OFFSET		0x10	/* Interrupt Enable Register */
#define VIC_INTENCLEAR_OFFSET		0x14	/* Interrupt Enable Clear Register */
#define VIC_SOFTINT_OFFSET		0x18	/* Software Interrupt Register */
#define VIC_SOFTINTCLEAR_OFFSET		0x1c	/* Software Interrupt Clear Register */
#define VIC_PROTECTION_OFFSET		0x20	/* Protection Enable Register */
#define VIC_SWPRIORITYMASK_OFFSET	0x24	/* Software Priority Mask Register */
#define VIC_PRIORITYDAISY_OFFSET	0x28	/* Vector Priority Register for Daisy Chain */
#define VIC_INTADDRESS_OFFSET		0xf00	/* Vector Priority Register for Daisy Chain */


/*
 * SROMC Controller
 */
/* DRAM Memory Controller */
#define S5P_DMC_BASE(x)		(S5P_PA_DMC + (x))
/* SROMC Base */
#define S5P_SROMC_BASE(x)	(S5P_PA_SROMC + (x))
/* SROMC offset */
#define CONCONTROL_OFFSET	0x0	/* Controller Control Register */
#define MEMCONTROL_OFFSET	0x04	/* Memory Control Register */
#define MEMCONFIG0_OFFSET	0x08	/* Memory Chip0 Configuration Register */
#define MEMCONFIG1_OFFSET	0x0c	/* Memory Chip1 Configuration Register */
#define DIRECTCMD_OFFSET	0x10	/* Memory Direct Command Register */
#define PRECHCONFIG_OFFSET	0x14	/* Precharge Policy Configuration Register */
#define PHYCONTROL0_OFFSET	0x18	/* PHY Control0 Register */
#define PHYCONTROL1_OFFSET	0x1c	/* PHY Control1 Register */
#define PHYCONTROL2_OFFSET	0x20	/* PHY Control2 Register */
#define PWRDNCONFIG_OFFSET	0x28	/* Dynamic Power Down Configuration Register */
#define TIMINGAREF_OFFSET	0x30	/* AC Timing Register for SDRAM Auto Refresh */
#define TIMINGROW_OFFSET	0x34	/* AC Timing Register for SDRAM Row */
#define TIMINGDATA_OFFSET	0x38	/* AC Timing Register for SDRAM Data */
#define TIMINGPOWER_OFFSET	0x3c	/* AC Timing Register for Power Mode of SDRAM */
#define PHYSTATUS0_OFFSET	0x40	/* PHY Status Register 0 */
#define PHYSTATUS1_OFFSET	0x44	/* PHY Status Register 1 */
#define CHIP0STATUS_OFFSET	0x48	/* Memory Chip0 Status Register */
#define CHIP1STATUS_OFFSET	0x4c	/* Memory Chip1 Status Register */
#define AREFSTATUS_OFFSET	0x50	/* Counter status Register for Auto Refresh */
#define MRSTATUS_OFFSET		0x54	/* Memory Mode Registers Status Register */
#define PHYTEST0_OFFSET		0x58	/* PHY Test Register 0 */
#define PHYTEST1_OFFSET		0x5c	/* PHY Test Register 1 */

#define S5P_CONCONTROL		S5P_DMC_BASE(CONCONTROL_OFFSET)
#define S5P_MEMCONTROL		S5P_DMC_BASE(MEMCONTROL_OFFSET)
#define S5P_MEMCONFIG0		S5P_DMC_BASE(MEMCONFIG0_OFFSET)
#define S5P_MEMCONFIG1		S5P_DMC_BASE(MEMCONFIG1_OFFSET)
#define S5P_DIRECTCMD		S5P_DMC_BASE(DIRECTCMD_OFFSET)
#define S5P_PRECHCONFIG		S5P_DMC_BASE(PRECHCONFIG_OFFSET)
#define S5P_PHYCONTROL0		S5P_DMC_BASE(PHYCONTROL0_OFFSET)
#define S5P_PHYCONTROL1		S5P_DMC_BASE(PHYCONTROL1_OFFSET)
#define S5P_PHYCONTROL2		S5P_DMC_BASE(PHYCONTROL2_OFFSET)
#define S5P_PWRDNCONFIG		S5P_DMC_BASE(PWRDNCONFIG_OFFSET)
#define S5P_TIMINGAREF		S5P_DMC_BASE(TIMINGAREF_OFFSET)
#define S5P_TIMINGROW		S5P_DMC_BASE(TIMINGROW_OFFSET)
#define S5P_TIMINGDATA		S5P_DMC_BASE(TIMINGDATA_OFFSET)
#define S5P_TIMINGPOWER		S5P_DMC_BASE(TIMINGPOWER_OFFSET)
#define S5P_PHYSTATUS0		S5P_DMC_BASE(PHYSTATUS0_OFFSET)
#define S5P_PHYSTATUS1		S5P_DMC_BASE(PHYSTATUS1_OFFSET)
#define S5P_CHIP0STATUS		S5P_DMC_BASE(CHIP0STATUS_OFFSET)
#define S5P_CHIP1STATUS		S5P_DMC_BASE(CHIP1STATUS_OFFSET)
#define S5P_AREFSTATUS		S5P_DMC_BASE(AREFSTATUS_OFFSET)
#define S5P_MRSTATUS		S5P_DMC_BASE(MRSTATUS_OFFSET)
#define S5P_PHYTEST0		S5P_DMC_BASE(PHYTEST0_OFFSET)
#define S5P_PHYTEST1		S5P_DMC_BASE(PHYTEST1_OFFSET)


/*
 * PWM Timer 
 */
#define S5P_PWMTIMER_BASE(x)	(S5P_PA_PWMTIMER + (x))

/* PWM timer offset */
#define PWM_TCFG0_OFFSET	0x0
#define PWM_TCFG1_OFFSET	0x04
#define PWM_TCON_OFFSET		0x08
#define PWM_TCNTB0_OFFSET	0x0c
#define PWM_TCMPB0_OFFSET	0x10
#define PWM_TCNTO0_OFFSET	0x14
#define PWM_TCNTB1_OFFSET	0x18
#define PWM_TCMPB1_OFFSET	0x1c
#define PWM_TCNTO1_OFFSET	0x20
#define PWM_TCNTB2_OFFSET	0x24
#define PWM_TCMPB2_OFFSET	0x28
#define PWM_TCNTO2_OFFSET	0x2c
#define PWM_TCNTB3_OFFSET	0x30
#define PWM_TCNTO3_OFFSET	0x38
#define PWM_TCNTB4_OFFSET	0x3c
#define PWM_TCNTO4_OFFSET	0x40
#define PWM_TINT_CSTAT_OFFSET	0x44

/* PWM timer register */
#define S5P_PWM_TCFG0		S5P_PWMTIMER_BASE(PWM_TCFG0_OFFSET)
#define S5P_PWM_TCFG1		S5P_PWMTIMER_BASE(PWM_TCFG1_OFFSET)
#define S5P_PWM_TCON		S5P_PWMTIMER_BASE(PWM_TCON_OFFSET)
#define S5P_PWM_TCNTB0		S5P_PWMTIMER_BASE(PWM_TCNTB0_OFFSET)
#define S5P_PWM_TCMPB0		S5P_PWMTIMER_BASE(PWM_TCMPB0_OFFSET)
#define S5P_PWM_TCNTO0		S5P_PWMTIMER_BASE(PWM_TCNTO0_OFFSET)
#define S5P_PWM_TCNTB1		S5P_PWMTIMER_BASE(PWM_TCNTB1_OFFSET)
#define S5P_PWM_TCMPB1		S5P_PWMTIMER_BASE(PWM_TCMPB1_OFFSET)
#define S5P_PWM_TCNTO1		S5P_PWMTIMER_BASE(PWM_TCNTO1_OFFSET)
#define S5P_PWM_TCNTB2		S5P_PWMTIMER_BASE(PWM_TCNTB2_OFFSET)
#define S5P_PWM_TCMPB2		S5P_PWMTIMER_BASE(PWM_TCMPB2_OFFSET)
#define S5P_PWM_TCNTO2		S5P_PWMTIMER_BASE(PWM_TCNTO2_OFFSET)
#define S5P_PWM_TCNTB3		S5P_PWMTIMER_BASE(PWM_TCNTB3_OFFSET)
#define S5P_PWM_TCNTO3		S5P_PWMTIMER_BASE(PWM_TCNTO3_OFFSET)
#define S5P_PWM_TCNTB4		S5P_PWMTIMER_BASE(PWM_TCNTB4_OFFSET)
#define S5P_PWM_TCNTO4		S5P_PWMTIMER_BASE(PWM_TCNTO4_OFFSET)
#define S5P_PWM_TINT_CSTAT	S5P_PWMTIMER_BASE(PWM_TINT_CSTAT_OFFSET)

/* PWM timer addressing */
#define S5P_TIMER_BASE		S5P_PWMTIMER_BASE(0x0)
#define S5P_PWMTIMER_BASE_REG	__REG(S5P_PWMTIMER_BASE(0x0))
#define S5P_PWM_TCFG0_REG	__REG(S5P_PWM_TCFG0)
#define S5P_PWM_TCFG1_REG	__REG(S5P_PWM_TCFG1)
#define S5P_PWM_TCON_REG	__REG(S5P_PWM_TCON)
#define S5P_PWM_TCNTB0_REG	__REG(S5P_PWM_TCNTB0)
#define S5P_PWM_TCMPB0_REG	__REG(S5P_PWM_TCMPB0)
#define S5P_PWM_TCNTO0_REG	__REG(S5P_PWM_TCNTO0)
#define S5P_PWM_TCNTB1_REG	__REG(S5P_PWM_TCNTB1)
#define S5P_PWM_TCMPB1_REG	__REG(S5P_PWM_TCMPB1)
#define S5P_PWM_TCNTO1_REG	__REG(S5P_PWM_TCNTO1)
#define S5P_PWM_TCNTB2_REG	__REG(S5P_PWM_TCNTB2)
#define S5P_PWM_TCMPB2_REG	__REG(S5P_PWM_TCMPB2)
#define S5P_PWM_TCNTO2_REG	__REG(S5P_PWM_TCNTO2)
#define S5P_PWM_TCNTB3_REG	__REG(S5P_PWM_TCNTB3)
#define S5P_PWM_TCNTO3_REG	__REG(S5P_PWM_TCNTO3)
#define S5P_PWM_TCNTB4_REG	__REG(S5P_PWM_TCNTB4)
#define S5P_PWM_TCNTO4_REG	__REG(S5P_PWM_TCNTO4)
#define S5P_PWM_TINT_CSTAT_REG	__REG(S5P_PWM_TINT_CSTAT)

/* PWM timer value */
#define S5P_TCON4_AUTO_RELOAD	(1 << 22)  /* Interval mode(Auto Reload) of PWM Timer 4 */
#define S5P_TCON4_UPDATE	(1 << 21)  /* Update TCNTB4 */
#define S5P_TCON4_ON		(1 << 20)  /* start bit of PWM Timer 4 */

#ifndef __ASSEMBLY__
typedef struct s5pc1xx_timer {
	volatile unsigned long	TCFG0;
	volatile unsigned long	TCFG1;
	volatile unsigned long	TCON;
	volatile unsigned long	TCNTB0;
	volatile unsigned long	TCMPB0;
	volatile unsigned long	TCNTO0;
	volatile unsigned long	TCNTB1;
	volatile unsigned long	TCMPB1;
	volatile unsigned long	TCNTO1;
	volatile unsigned long	TCNTB2;
	volatile unsigned long	TCMPB2;
	volatile unsigned long	TCNTO2;
	volatile unsigned long	TCNTB3;
	volatile unsigned long	res1;
	volatile unsigned long	TCNTO3;
	volatile unsigned long	TCNTB4;
	volatile unsigned long	TCNTO4;
	volatile unsigned long	TINTCSTAT;
} s5pc1xx_timers_t;
#endif	/* __ASSEMBLY__ */

#include <asm/arch/uart.h>
#include <asm/arch/watchdog.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clock-others.h>
#include <asm/arch/usb-hs-otg.h>
#include <asm/arch/i2c.h>

#endif	/* __S5PC100_H__ */

#endif	/* _CPU_H */
