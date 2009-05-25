/*
 * (C) Copyright 2003
 * David MÃŒller ELSOFT AG Switzerland. d.mueller@elsoft.ch
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 */

/************************************************
 * NAME	    : S3C64XX.h
 * Version  : 31.3.2003
 *
 * common stuff for SAMSUNG S3C64XX SoC
 ************************************************/

#ifndef __S5PC1XX_H__
#define __S5PC1XX_H__

#if defined(CONFIG_SYNC_MODE) && defined(CONFIG_S3C6400)
#error CONFIG_SYNC_MODE unavailable on S3C6400, please, fix your configuration!
#endif

#include <asm/types.h>

/* UART (see manual chapter 11) */
typedef struct {
	volatile u32	ULCON;
	volatile u32	UCON;
	volatile u32	UFCON;
	volatile u32	UMCON;
	volatile u32	UTRSTAT;
	volatile u32	UERSTAT;
	volatile u32	UFSTAT;
	volatile u32	UMSTAT;
#ifdef __BIG_ENDIAN
	volatile u8	res1[3];
	volatile u8	UTXH;
	volatile u8	res2[3];
	volatile u8	URXH;
#else /* Little Endian */
	volatile u8	UTXH;
	volatile u8	res1[3];
	volatile u8	URXH;
	volatile u8	res2[3];
#endif
	volatile u32	UBRDIV;
#ifdef __BIG_ENDIAN
	volatile u8     res3[2];
	volatile u16    UDIVSLOT;
#else
	volatile u16    UDIVSLOT;
	volatile u8     res3[2];
#endif
} s5pc1xx_uart;

typedef struct {
	volatile u32	TCFG0;
	volatile u32	TCFG1;
	volatile u32	TCON;
	volatile u32	TCNTB0;
	volatile u32	TCMPB0;
	volatile u32	TCNTO0;
	volatile u32	TCNTB1;
	volatile u32	TCMPB1;
	volatile u32	TCNTO1;
	volatile u32	TCNTB2;
	volatile u32	TCMPB2;
	volatile u32	TCNTO2;
	volatile u32	TCNTB3;
	volatile u32	res1;
	volatile u32	TCNTO3;
	volatile u32	TCNTB4;
	volatile u32	TCNTO4;
	volatile u32	TINTCSTAT;
} s5pc1xx_timers;

#endif
