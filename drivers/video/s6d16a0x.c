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
#include "s5p-spi.h"

/* these machine specific platform data would be setting at universal.c */
struct spi_platform_data *s6d16a0x;

void cs_low_(void)
{
	gpio_set_value(s6d16a0x->cs_bank, s6d16a0x->cs_num, 0);
}

void cs_high_(void)
{
	gpio_set_value(s6d16a0x->cs_bank, s6d16a0x->cs_num, 1);
}

void clk_low_(void)
{
	gpio_set_value(s6d16a0x->clk_bank, s6d16a0x->clk_num, 0);
}

void clk_high_(void)
{
	gpio_set_value(s6d16a0x->clk_bank, s6d16a0x->clk_num, 1);
}

void si_low_(void)
{
	gpio_set_value(s6d16a0x->si_bank, s6d16a0x->si_num, 0);
}

void si_high_(void)
{
	gpio_set_value(s6d16a0x->si_bank, s6d16a0x->si_num, 1);
}

char so_read_(void)
{
	return gpio_get_value(s6d16a0x->so_bank, s6d16a0x->so_num);
}

static const unsigned short SEQ_PASSWD2_SET[] = {
	0xF1, 0x5A,
	DATA_ONLY, 0x5A,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_DISCTL_SET[] = {
	0xf2, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x62,
	DATA_ONLY, 0x62,
	DATA_ONLY, 0x57,
	DATA_ONLY, 0x57,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_PWRCTL_SET[] = {
	0xf3, 0x00,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x25,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0x2D,
	DATA_ONLY, 0x2D,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x2D,
	DATA_ONLY, 0x14,
	DATA_ONLY, 0x14,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x62,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_VCMCTL_SET[] = {
	0xf4, 0x00,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x4E,
	DATA_ONLY, 0x5A,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x4E,
	DATA_ONLY, 0x5A,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_SRCCTL_SET[] = {
	0xf5, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x57,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0x14,
	DATA_ONLY, 0x14,
	DATA_ONLY, 0x09,
	DATA_ONLY, 0x09,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_PANELCTL1_SET[] = {
	0xf6, 0x02,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x80,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x00,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_PANELCTL2_SET[] = {
	0xf7, 0x00,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xF2,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x1F,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x07,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x4B,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x8C,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_PANELCTL3_SET[] = {
	0xf8, 0x00,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xF2,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x1F,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x07,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x4B,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x8C,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_PANELCTL4_SET[] = {
	0xf9, 0x00,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x05,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x04,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x0C,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x0F,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x1F,
	DATA_ONLY, 0xFF,
	DATA_ONLY, 0xE0,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_PGAMMACTL_SET[] = {
	0xfa, 0x1E,
	DATA_ONLY, 0x35,
	DATA_ONLY, 0x2C,
	DATA_ONLY, 0x0C,
	DATA_ONLY, 0x0E,
	DATA_ONLY, 0x0F,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x04,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x1B,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x18,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x1F,
	DATA_ONLY, 0x1E,
	DATA_ONLY, 0x1E,
	DATA_ONLY, 0x35,
	DATA_ONLY, 0x2C,
	DATA_ONLY, 0x0C,
	DATA_ONLY, 0x0E,
	DATA_ONLY, 0x0F,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x04,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0x1B,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x18,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x1F,
	DATA_ONLY, 0x1E,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x35,
	DATA_ONLY, 0x2C,
	DATA_ONLY, 0x0C,
	DATA_ONLY, 0x0E,
	DATA_ONLY, 0x0F,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x04,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x1B,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x19,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x22,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_NGAMMACTL_SET[] = {
	0xFB, 0x01,
	DATA_ONLY, 0x1C,
	DATA_ONLY, 0x1D,
	DATA_ONLY, 0x1C,
	DATA_ONLY, 0x3B,
	DATA_ONLY, 0x43,
	DATA_ONLY, 0x4A,
	DATA_ONLY, 0x3F,
	DATA_ONLY, 0x3A,
	DATA_ONLY, 0x54,
	DATA_ONLY, 0x4B,
	DATA_ONLY, 0x4A,
	DATA_ONLY, 0x48,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x0D,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0x1C,
	DATA_ONLY, 0x1D,
	DATA_ONLY, 0x1C,
	DATA_ONLY, 0x3B,
	DATA_ONLY, 0x43,
	DATA_ONLY, 0x4A,
	DATA_ONLY, 0x3F,
	DATA_ONLY, 0x39,
	DATA_ONLY, 0x54,
	DATA_ONLY, 0x4B,
	DATA_ONLY, 0x4A,
	DATA_ONLY, 0x48,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x0D,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x18,
	DATA_ONLY, 0x19,
	DATA_ONLY, 0x39,
	DATA_ONLY, 0x43,
	DATA_ONLY, 0x4A,
	DATA_ONLY, 0x3E,
	DATA_ONLY, 0x3A,
	DATA_ONLY, 0x54,
	DATA_ONLY, 0x4B,
	DATA_ONLY, 0x4A,
	DATA_ONLY, 0x48,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x0D,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_CLKCTL3_SET[] = {
	0xB7, 0x00,

	DATA_ONLY, 0x11,
	DATA_ONLY, 0x11,
	ENDDEF, 0x0000
};

static const unsigned short SEQ_HOSTCTL1_SET[] = {
	0xB8, 0x31,

	DATA_ONLY, 0x11,
	ENDDEF, 0x0000
};

static const unsigned short SEQ_HOSTCTL2_SET[] = {
	0xB9, 0x00,

	DATA_ONLY, 0x06,
	ENDDEF, 0x0000
};

static const unsigned short SEQ_TEON_SET[] = {
	COMMAND_ONLY, 0x35,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_CASET[] = {
	0x2A, 0x00,

	DATA_ONLY, 0x00,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0x3F,
	ENDDEF, 0x0000
};

static const unsigned short SEQ_PASET[] = {
	0x2B, 0x00,

	DATA_ONLY, 0x00,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0xDF,
	ENDDEF, 0x0000
};

static const unsigned short SEQ_COLMOD[] = {
	0x3A, 0x77,
	/* 0x36, 0xC4, */	/* MADCTL : Reverse display */

	ENDDEF, 0x0000
};

static const unsigned short SEQ_WRCTRLD[] = {
	0x53, 0x00,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_SLPOUT[] = {
	0x11, COMMAND_ONLY,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_DISPON[] = {
	0x29, COMMAND_ONLY,

	ENDDEF, 0x0000
};

static const unsigned short SEQ_DISPOFF[] = {
	0x28, COMMAND_ONLY,

	ENDDEF, 0x0000
};

unsigned char Delay=1;

static void s6d16a0x_c110_spi_write_byte(unsigned char address, unsigned char command)
{
	int     j;
	unsigned short data;
	data = (address << 8) + command;

	cs_high_();
	si_high_();
	clk_high_();
	udelay(Delay);

	cs_low_();
	udelay(Delay);

	for (j = PACKET_LEN; j >= 0; j--)
	{
		clk_low_();

		/* data high or low */
		if ((data >> j) & 0x0001)
			si_high_();
		else
			si_low_();

		udelay(Delay);

		clk_high_();
		udelay(Delay);
	}

	cs_high_();
	udelay(Delay);
}

#ifdef UNUSED_FUNCTIONS
static unsigned char s6d16a0x_c110_spi_read_byte(unsigned char select, unsigned char address)
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
				gpio_cfg_pin(s6d16a0x->so_bank, s6d16a0x->so_num, GPIO_INPUT);
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

	gpio_cfg_pin(s6d16a0x->so_bank, s6d16a0x->so_num, GPIO_OUTPUT);

	return command;
}
#endif

static void s6d16a0x_spi_write(unsigned char address, unsigned char command)
{
	if (address != DATA_ONLY)
		s6d16a0x_c110_spi_write_byte(0x0, address);

	if (command != COMMAND_ONLY)
		s6d16a0x_c110_spi_write_byte(0x1, command);
}

static void s6d16a0x_panel_send_sequence(const unsigned short *wbuf)
{
	int i = 0;

	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC)
			s6d16a0x_spi_write(wbuf[i], wbuf[i+1]);
		else
			udelay(wbuf[i+1]*1000);
		i += 2;
	}
}

void s6d16a0x_cfg_ldo(void)
{
	s6d16a0x_panel_send_sequence(SEQ_PASSWD2_SET);
	s6d16a0x_panel_send_sequence(SEQ_DISCTL_SET);
	s6d16a0x_panel_send_sequence(SEQ_PWRCTL_SET);
	s6d16a0x_panel_send_sequence(SEQ_VCMCTL_SET);
	s6d16a0x_panel_send_sequence(SEQ_SRCCTL_SET);
	s6d16a0x_panel_send_sequence(SEQ_PANELCTL1_SET);
	s6d16a0x_panel_send_sequence(SEQ_PANELCTL2_SET);
	s6d16a0x_panel_send_sequence(SEQ_PANELCTL3_SET);
	s6d16a0x_panel_send_sequence(SEQ_PANELCTL4_SET);
	s6d16a0x_panel_send_sequence(SEQ_PGAMMACTL_SET);
	s6d16a0x_panel_send_sequence(SEQ_NGAMMACTL_SET);
	s6d16a0x_panel_send_sequence(SEQ_CLKCTL3_SET);
	s6d16a0x_panel_send_sequence(SEQ_HOSTCTL1_SET);
	s6d16a0x_panel_send_sequence(SEQ_HOSTCTL2_SET);
	s6d16a0x_panel_send_sequence(SEQ_TEON_SET);
	s6d16a0x_panel_send_sequence(SEQ_PASSWD2_SET);
	s6d16a0x_panel_send_sequence(SEQ_CASET);
	s6d16a0x_panel_send_sequence(SEQ_PASET);
	s6d16a0x_panel_send_sequence(SEQ_COLMOD);
	s6d16a0x_panel_send_sequence(SEQ_WRCTRLD);
	s6d16a0x_panel_send_sequence(SEQ_SLPOUT);
	udelay(120000);
}

void s6d16a0x_enable_ldo(unsigned int onoff)
{
	if (onoff) {
		s6d16a0x_panel_send_sequence(SEQ_DISPON);
	}
}

/* this function would be called at universal.c */
void s6d16a0x_set_platform_data(struct spi_platform_data *pd)
{
	if (pd == NULL) {
		printf("pd is NULL.\n");
		return;
	}

	s6d16a0x = pd;
}

