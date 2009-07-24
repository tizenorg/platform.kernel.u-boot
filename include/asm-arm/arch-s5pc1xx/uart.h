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

#ifndef __ASM_ARCH_UART_H_
#define __ASM_ARCH_UART_H_

/* 
 * UART
 */
/* uart base address */
#define S5PC100_PA_UART		0xEC000000
#define S5PC110_PA_UART		0xE2900000

/* uart offset */
#define ULCON_OFFSET		0x00
#define UCON_OFFSET		0x04
#define UFCON_OFFSET		0x08
#define UMCON_OFFSET		0x0C
#define UTRSTAT_OFFSET		0x10
#define UERSTAT_OFFSET		0x14
#define UFSTAT_OFFSET		0x18
#define UMSTAT_OFFSET		0x1C
#define UTXH_OFFSET		0x20
#define URXH_OFFSET		0x24
#define UBRDIV_OFFSET		0x28
#define UDIVSLOT_OFFSET		0x2C
#define UINTP_OFFSET		0x30
#define UINTSP_OFFSET		0x34
#define UINTM_OFFSET		0x38

#define UTRSTAT_TX_EMPTY	(1 << 2)
#define UTRSTAT_RX_READY	(1 << 0)
#define UART_ERR_MASK		0xF

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
#endif	/* __ASSEMBLY__ */

#endif
