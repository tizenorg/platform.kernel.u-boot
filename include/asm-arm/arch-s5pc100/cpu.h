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


/* UART (see manual chapter 11) */
#ifndef __ASSEMBLY__
typedef struct s5pc1xx_uart {
	volatile unsigned long	ULCON;
	volatile unsigned long	UCON;
	volatile unsigned long	UFCON;
	volatile unsigned long	UMCON;
	volatile unsigned long	UTRSTAT;
	volatile unsigned long	UERSTAT;
	volatile unsigned long	UFSTAT;
	volatile unsigned long	UMSTAT;
#ifdef __BIG_ENDIAN
	volatile unsigned char	res1[3];
	volatile unsigned char	UTXH;
	volatile unsigned char	res2[3];
	volatile unsigned char	URXH;
#else /* Little Endian */
	volatile unsigned char	UTXH;
	volatile unsigned char	res1[3];
	volatile unsigned char	URXH;
	volatile unsigned char	res2[3];
#endif
	volatile unsigned long	UBRDIV;
#ifdef __BIG_ENDIAN
	volatile unsigned char     res3[2];
	volatile unsigned short    UDIVSLOT;
#else
	volatile unsigned short    UDIVSLOT;
	volatile unsigned char     res3[2];
#endif
} s5pc1xx_uart_t;

enum s5pc1xx_uarts_nr {
	S5PC1XX_UART0,
	S5PC1XX_UART1,
	S5PC1XX_UART2,
	S5PC1XX_UART3,
};
#else
#endif

#ifndef __ASSEMBLY__
typedef struct s5pc1x0_timer {
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
} s5pc1xx_timers;
#else
#endif


#endif /* _CPU_H */
