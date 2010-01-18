/*
 * LCD panel driver for Board based on S5PC100 and S5PC110. 
 *
 * Author: InKi Dae  <inki.dae@samsung.com>
 *
 * Derived from drivers/video/omap/lcd-apollon.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <common.h>
#include <linux/types.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

#define PACKET_LEN		8

struct s6e63m0_platform_data {
	struct s5pc1xx_gpio_bank *bank;
	unsigned int num;
};

/* these machine specific platform data would be setting at universal.c */
struct s6e63m0_platform_data *spi_cs, *spi_clk, *spi_si, *spi_so;

void cs_low(void)
{
	gpio_set_value(spi_cs->bank, spi_cs->num, 0);
}

void cs_high(void)
{
	gpio_set_value(spi_cs->bank, spi_cs->num, 1);
}

void clk_low(void)
{
	gpio_set_value(spi_clk->bank, spi_clk->num, 0);
}

void clk_high(void)
{
	gpio_set_value(spi_clk->bank, spi_clk->num, 1);
}

void si_low(void)
{
	gpio_set_value(spi_si->bank, spi_si->num, 0);
}

void si_high(void)
{
	gpio_set_value(spi_si->bank, spi_si->num, 1);
}

char so_read(void)
{
	return gpio_get_value(spi_so->bank, spi_so->num);
}

static const unsigned short SEQ_PANEL_CONDITION_SET[] = {
	0xF8, 0x01,
	DATA_ONLY, 0x27,
	DATA_ONLY, 0x27,
	DATA_ONLY, 0x07,
	DATA_ONLY, 0x07,
	DATA_ONLY, 0x54,
	DATA_ONLY, 0x9f,
	DATA_ONLY, 0x63,
	DATA_ONLY, 0x86,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x0d,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_DISPLAY_CONDITION_SET[] = {
	0xf2, 0x02,
	DATA_ONLY, 0x03,
	DATA_ONLY, 0x1c,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x10,

#if 0
	0xf7, 0x03,
#endif
	0xf7, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_GAMMA_SETTING[] = {
	0xfa, 0x00,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x05,
	DATA_ONLY, 0x1b,
	DATA_ONLY, 0x62,
	DATA_ONLY, 0x54,
	DATA_ONLY, 0x3f,
	DATA_ONLY, 0xb6,
	DATA_ONLY, 0xb9,
	DATA_ONLY, 0xaa,
	DATA_ONLY, 0xaf,
	DATA_ONLY, 0xb4,
	DATA_ONLY, 0xa5,
	DATA_ONLY, 0xc0,
	DATA_ONLY, 0xc3,
	DATA_ONLY, 0xb9,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x96,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x99,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xce,
	0xfa, 0x01,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_ETC_CONDITION_SET[] = {
	0xf6, 0x00,
	DATA_ONLY, 0x8c,
	DATA_ONLY, 0x07,

	/* added for panel rev 0.1*/
	0xb3, 0xc,

	0xb5, 0x2c,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x0c,
	DATA_ONLY, 0x0a,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0e,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x13,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x2a,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1b,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x17,

	DATA_ONLY, 0x2b,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x3a,
	DATA_ONLY, 0x34,
	DATA_ONLY, 0x30,
	DATA_ONLY, 0x2c,
	DATA_ONLY, 0x29,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x25,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x1e,
	DATA_ONLY, 0x1e,

	0xb6, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,

	DATA_ONLY, 0x55,
	DATA_ONLY, 0x55,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,

	0xb7, 0x2c,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x0c,
	DATA_ONLY, 0x0a,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0e,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x13,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x2a,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1b,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x17,

	DATA_ONLY, 0x2b,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x3a,
	DATA_ONLY, 0x34,
	DATA_ONLY, 0x30,
	DATA_ONLY, 0x2c,
	DATA_ONLY, 0x29,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x25,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x1e,
	DATA_ONLY, 0x1e,

	0xb8, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,

	DATA_ONLY, 0x55,
	DATA_ONLY, 0x55,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,

	0xb9, 0x2c,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x0c,
	DATA_ONLY, 0x0a,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0e,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x13,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x2a,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1b,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x17,

	DATA_ONLY, 0x2b,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x3a,
	DATA_ONLY, 0x34,
	DATA_ONLY, 0x30,
	DATA_ONLY, 0x2c,
	DATA_ONLY, 0x29,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x25,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x1e,
	DATA_ONLY, 0x1e,

	0xba, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,

	DATA_ONLY, 0x55,
	DATA_ONLY, 0x55,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,

	ENDDEF, 0x0000
};


static const unsigned short SEQ_STAND_BY_OFF[] = {
	0x11, COMMAND_ONLY,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_STAND_BY_ON[] = {
	0x10, COMMAND_ONLY,

	ENDDEF, 0x0000
};

/* added for panel rev 0.1*/
static const unsigned short SEQ_DISPLAY_ON[] = {
	0x29, COMMAND_ONLY,

	ENDDEF, 0x0000
};

unsigned char DELAY=1;

static void s6e63m0_c110_spi_write_byte(unsigned char address, unsigned char command)
{
	int     j;
	unsigned short data;

	data = (address << 8) + command;

	cs_high();
	si_high();
	clk_high();
	udelay(DELAY);

	cs_low();
	udelay(DELAY);

	for (j = PACKET_LEN; j >= 0; j--)
	{
		clk_low();

		/* data high or low */
		if ((data >> j) & 0x0001)
			si_high();
		else
			si_low();

		udelay(DELAY);

		clk_high();
		udelay(DELAY);
	}

	cs_high();
	udelay(DELAY);
}


static unsigned char s6e63m0_c110_spi_read_byte(unsigned char select, unsigned char address)
{
	int     j;
	static unsigned int first = 1;
	unsigned char DELAY=1;
	unsigned short data = 0;
	char command = 0;

	data = (select << 8) + address;

	cs_high();
	si_high();
	clk_high();
	udelay(DELAY);

	clk_low();
	udelay(DELAY);

	for (j = PACKET_LEN + 8; j >= 0; j--)
	{

		if (j > 7) {
			clk_low();

			/* data high or low */
			if ((data >> (j - 8)) & 0x0001)
				si_high();
			else
				si_low();

			udelay(DELAY);
			clk_high();
		} else {
			if (first) {
				gpio_cfg_pin(spi_so->bank, spi_so->num, GPIO_INPUT);
				first = 0;
			}

			clk_low();

			if (so_read() & 0x1)
				command |= 1 << j;
			else
				command |= 0 << j;

			udelay(DELAY);
			clk_high();
		}

		udelay(DELAY);
	}

	cs_high();
	udelay(DELAY);

	gpio_cfg_pin(spi_so->bank, spi_so->num, GPIO_OUTPUT);

	return command;
}

static void s6e63m0_spi_write(unsigned char address, unsigned char command)
{
	if (address != DATA_ONLY)
		s6e63m0_c110_spi_write_byte(0x0, address);

	if (command != COMMAND_ONLY)
		s6e63m0_c110_spi_write_byte(0x1, command);
}

static void s6e63m0_panel_send_sequence(const unsigned short *wbuf)
{
	int i = 0;

	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC)
			s6e63m0_spi_write(wbuf[i], wbuf[i+1]);
		else
			udelay(wbuf[i+1]*1000);
		i += 2;
	}
}

void s6e63m0_cfg_ldo(void)
{
	/*
	data = s6e63m0_c110_spi_read_byte(0x0, 0xdd);
	printf("data = %d, %x\n", data, &data);
	*/

	s6e63m0_panel_send_sequence(SEQ_PANEL_CONDITION_SET);
	s6e63m0_panel_send_sequence(SEQ_DISPLAY_CONDITION_SET);
	s6e63m0_panel_send_sequence(SEQ_GAMMA_SETTING);
	s6e63m0_panel_send_sequence(SEQ_ETC_CONDITION_SET);
}

void s6e63m0_enable_ldo(unsigned int onoff)
{
	if (onoff) {
		s6e63m0_panel_send_sequence(SEQ_STAND_BY_OFF);
		s6e63m0_panel_send_sequence(SEQ_DISPLAY_ON);
	} else {
		s6e63m0_panel_send_sequence(SEQ_STAND_BY_ON);
	}
}

/* this function would be called at universal.c */
void s6e63m0_set_spi_interface(struct s6e63m0_platform_data *cs,
	struct s6e63m0_platform_data *clk, struct s6e63m0_platform_data *si,
	struct s6e63m0_platform_data *so)
{
	if (cs == NULL || clk == NULL || si == NULL) {
		printf("gpio bank is NULL.\n");
		return;
	}

	spi_cs = cs;
	spi_clk = clk;
	spi_si = si;

	/* so could be NULL. */
	spi_so = so;
}
