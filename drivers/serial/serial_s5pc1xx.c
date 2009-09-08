/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 *
 * based on drivers/serial/s3c64xx.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/clk.h>

#if defined(CONFIG_SERIAL_MULTI)
#include <serial.h>

/* Multi serial device functions */
#define DECLARE_S5P_SERIAL_FUNCTIONS(port) \
    int  s5p_serial##port##_init (void) {\
	return serial_init_dev(port);}\
    void s5p_serial##port##_setbrg (void) {\
	serial_setbrg_dev(port);}\
    int  s5p_serial##port##_getc (void) {\
	return serial_getc_dev(port);}\
    int  s5p_serial##port##_tstc (void) {\
	return serial_tstc_dev(port);}\
    void s5p_serial##port##_putc (const char c) {\
	serial_putc_dev(port, c);}\
    void s5p_serial##port##_puts (const char *s) {\
	serial_puts_dev(port, s);}

#define INIT_S5P_SERIAL_STRUCTURE(port,name,bus) {\
	name,\
	bus,\
	s5p_serial##port##_init,\
	s5p_serial##port##_setbrg,\
	s5p_serial##port##_getc,\
	s5p_serial##port##_tstc,\
	s5p_serial##port##_putc,\
	s5p_serial##port##_puts, }

#else

#ifdef CONFIG_SERIAL0
#define UART_NR	S5PC1XX_UART0
#elif defined(CONFIG_SERIAL1)
#define UART_NR	S5PC1XX_UART1
#elif defined(CONFIG_SERIAL2)
#define UART_NR	S5PC1XX_UART2
#elif defined(CONFIG_SERIAL3)
#define UART_NR	S5PC1XX_UART3
#else
#error "Bad: you didn't configure serial ..."
#endif

#endif /* CONFIG_SERIAL_MULTI */

static inline s5pc1xx_uart_t *s5pc1xx_get_base_uart(enum s5pc1xx_uarts_nr nr)
{
	if (cpu_is_s5pc100())
		return (s5pc1xx_uart_t *)(S5PC100_PA_UART + (nr * 0x400));
	else
		return (s5pc1xx_uart_t *)(S5PC110_PA_UART + (nr * 0x400));
}

/*
 * The coefficient, used to calculate the baudrate on S5PC1XX UARTs is
 * calculated as
 * C = UBRDIV * 16 + number_of_set_bits_in_UDIVSLOT
 * however, section 31.6.11 of the datasheet doesn't recomment using 1 for 1,
 * 3 for 2, ... (2^n - 1) for n, instead, they suggest using these constants:
 */
static const int udivslot[] = {
	0,
	0x0080,
	0x0808,
	0x0888,
	0x2222,
	0x4924,
	0x4a52,
	0x54aa,
	0x5555,
	0xd555,
	0xd5d5,
	0xddd5,
	0xdddd,
	0xdfdd,
	0xdfdf,
	0xffdf,
};

void _serial_setbrg(const int dev_index)
{
	DECLARE_GLOBAL_DATA_PTR;
	s5pc1xx_uart_t *const uart = s5pc1xx_get_base_uart(dev_index);
	u32 pclk = get_pclk();
	u32 baudrate = gd->baudrate;
	u32 val;

	val = pclk / baudrate;

	writel(val / 16 - 1, &uart->UBRDIV);
	writel(udivslot[val % 16], &uart->UDIVSLOT);
}

#if defined(CONFIG_SERIAL_MULTI)
static inline void
serial_setbrg_dev(unsigned int dev_index)
{
	_serial_setbrg(dev_index);
}
#else
void serial_setbrg(void)
{
	_serial_setbrg(UART_NR);
}
#endif

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
int serial_init_dev(const int dev_index)
{
	s5pc1xx_uart_t *const uart = s5pc1xx_get_base_uart(dev_index);

	/* reset and enable FIFOs, set triggers to the maximum */
	writel(0, &uart->UFCON);
	writel(0, &uart->UMCON);
	/* 8N1 */
	writel(0x3, &uart->ULCON);
	/* No interrupts, no DMA, pure polling */
	writel(0x245, &uart->UCON);

	_serial_setbrg(dev_index);

	return 0;
}

#if !defined(CONFIG_SERIAL_MULTI)
int serial_init (void)
{
	return serial_init_dev(UART_NR);
}
#endif

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int _serial_getc(const int dev_index)
{
	s5pc1xx_uart_t *const uart = s5pc1xx_get_base_uart(dev_index);

	/* wait for character to arrive */
	while (!(readl(&uart->UTRSTAT) & 0x1))
		;

	return (int)(readl(&uart->URXH) & 0xff);
}

#if defined(CONFIG_SERIAL_MULTI)
static inline int serial_getc_dev(unsigned int dev_index)
{
	return _serial_getc(dev_index);
}
#else
int serial_getc (void)
{
	return _serial_getc(UART_NR);
}
#endif

#ifdef CONFIG_MODEM_SUPPORT
static int be_quiet;
void disable_putc(void)
{
	be_quiet = 1;
}

void enable_putc(void)
{
	be_quiet = 0;
}
#endif

/*
 * Output a single byte to the serial port.
 */
void _serial_putc(const char c, const int dev_index)
{
	s5pc1xx_uart_t *const uart = s5pc1xx_get_base_uart(dev_index);

#ifdef CONFIG_MODEM_SUPPORT
	if (be_quiet)
		return;
#endif

	/* wait for room in the tx FIFO */
	while (!(readl(&uart->UTRSTAT) & 0x2))
		;

	writel(c, &uart->UTXH);

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc('\r');
}

#if defined(CONFIG_SERIAL_MULTI)
static inline void serial_putc_dev(unsigned int dev_index, const char c)
{
	_serial_putc(c, dev_index);
}
#else
void serial_putc(const char c)
{
	_serial_putc(c, UART_NR);
}
#endif

/*
 * Test whether a character is in the RX buffer
 */
int _serial_tstc(const int dev_index)
{
	s5pc1xx_uart_t *const uart = s5pc1xx_get_base_uart(dev_index);

	return (int)(readl(&uart->UTRSTAT) & 0x1);
}

#if defined(CONFIG_SERIAL_MULTI)
static inline int serial_tstc_dev(unsigned int dev_index)
{
	return _serial_tstc(dev_index);
}
#else
int serial_tstc(void)
{
	return _serial_tstc(UART_NR);
}
#endif

void _serial_puts(const char *s, const int dev_index)
{
	while (*s)
		_serial_putc(*s++, dev_index);
}

#if defined(CONFIG_SERIAL_MULTI)
static inline void serial_puts_dev(int dev_index, const char *s)
{
	_serial_puts(s, dev_index);
}
#else
void serial_puts (const char *s)
{
	_serial_puts(s, UART_NR);
}
#endif

#ifdef CONFIG_HWFLOW
static int hwflow;             /* turned off by default */
int hwflow_onoff(int on)
{
	switch (on) {
	case 1:
		hwflow = 1;     /* turn on */
		break;
	case -1:
		hwflow = 0;     /* turn off */
		break;
	}
	return hwflow;
}
#endif

#if defined(CONFIG_SERIAL_MULTI)
DECLARE_S5P_SERIAL_FUNCTIONS(0);
struct serial_device s5pc1xx_serial0_device =
	INIT_S5P_SERIAL_STRUCTURE(0, "s5pser0", "S5PUART0");
DECLARE_S5P_SERIAL_FUNCTIONS(1);
struct serial_device s5pc1xx_serial1_device =
	INIT_S5P_SERIAL_STRUCTURE(1, "s5pser1", "S5PUART1");
DECLARE_S5P_SERIAL_FUNCTIONS(2);
struct serial_device s5pc1xx_serial2_device =
	INIT_S5P_SERIAL_STRUCTURE(2, "s5pser2", "S5PUART2");
DECLARE_S5P_SERIAL_FUNCTIONS(3);
struct serial_device s5pc1xx_serial3_device =
	INIT_S5P_SERIAL_STRUCTURE(3, "s5pser3", "S5PUART3");
#endif /* CONFIG_SERIAL_MULTI */
