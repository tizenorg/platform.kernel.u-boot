
/* 
 * UART
 */
/* uart base address */
#define S5P_PA_UART		S5P_ADDR(0x0c000000)    /* UART */
#define UARTx_OFFSET(x)		(S5P_PA_UART + x * 0x400)
#define S5P_UART_BASE		(S5P_PA_UART)
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

