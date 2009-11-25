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

#include <linux/types.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>

struct s5pc110_gpio *gpio = (struct s5pc110_gpio *) S5PC110_GPIO_BASE;

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

#define S5PCFB_C110_CS_LOW	gpio_set_value(&gpio->gpio_mp0_1, 1, 0)
#define S5PCFB_C110_CS_HIGH	gpio_set_value(&gpio->gpio_mp0_1, 1, 1)
#define S5PCFB_C110_CLK_LOW	gpio_set_value(&gpio->gpio_mp0_4, 1, 0)
#define S5PCFB_C110_CLK_HIGH	gpio_set_value(&gpio->gpio_mp0_4, 1, 1)
#define S5PCFB_C110_SDA_LOW	gpio_set_value(&gpio->gpio_mp0_4, 3, 0)
#define S5PCFB_C110_SDA_HIGH	gpio_set_value(&gpio->gpio_mp0_4, 3, 1)

#define S5PCFB_C110_SDO_READ	gpio_get_value(&gpio->gpio_mp0_4, 4)

const unsigned short GAMMA_SETTING[] = {
	0x3f, 0x0000,	/* gamma setting : ams701ka */

	/* High Red Gamma */
	0x40, 0x0720,
	0x41, 0xCBA7,
	0x42, 0xAB92,
	0x43, 0x0088,

	/* High Green Gamma */
	0x44, 0x7804,
	0x45, 0xCEBE,
	0x46, 0xB39E,
	0x47, 0x00A4,

	/* High Blue Gamma */
	0x48, 0x0009,
	0x49, 0xC6B9,
	0x4a, 0xA996,
	0x4b, 0x00C7,

#if 0
	/* Low Red Gamma */
	0x4c, 0x005f,
	0x4d, 0x0001,
	0x4e, 0x0009,
	0x4f, 0x0063,

	/* Low Green Gamma */
	0x50, 0x007f,
	0x51, 0x005f,
	0x52, 0x0005,
	0x53, 0x005f,

	/* Low Blue Gamma */
	0x54, 0x0002,
	0x55, 0x0002,
	0x56, 0x0002,
	0x57, 0x0002,
#endif

	0xb0, 0x0002,
	0xb1, 0x0068,
	0xb2, 0x007f,
	0xb3, 0x005f,
	0xb4, 0x0003,
	0xb5, 0x005f,
	0xb6, 0x0000,

	0xb7, 0x0004,
	0xb8, 0x005c,
	0xb9, 0x007f,
	0xba, 0x005f,
	0xbb, 0x0004,
	0xbc, 0x005f,
	0xbd, 0x0001,

	0xbe, 0x0009,
	0xbf, 0x0063,
	0xc0, 0x007f,
	0xc1, 0x005f,
	0xc2, 0x0005,
	0xc3, 0x005f,
	0xc4, 0x0002,

	ENDDEF, 0x0000


};

const unsigned short SEQ_SLEEP_OUT[] = {
	0x02, 0x2300,	/* Sleep Out */
	SLEEPMSEC, 1,
	ENDDEF, 0x0000
};

const unsigned short MANUAL_POWER_ON_SETTING[] = {
	0x06, 0x4000,
	0x12, 0x0040,
	ENDDEF, 0x0000
};


const unsigned short MANUAL_LTPS_SETTING[] = {
	0x06, 0x0000,
	0x06, 0x0001,
	SLEEPMSEC, 1,
	0x06, 0x0003,
	SLEEPMSEC, 5,
	0x06, 0x0007,
	SLEEPMSEC, 10,
	0x06, 0x000f,
	SLEEPMSEC, 50,
	0x06, 0x001f,
	SLEEPMSEC, 5,
	0x06, 0x003f,
	SLEEPMSEC, 5,
	0x06, 0x007f,
	SLEEPMSEC, 5,
	0x06, 0x00ff,
	SLEEPMSEC, 5,
	0x06, 0x08ff,
	SLEEPMSEC, 10,

	0x03, 0x134A,	/* ETC Register setting */
	0x04, 0x86a4,	/* LTPS Power on setting VCIR=2.7V */
	0x32, 0x0002,
	0x3f, 0x0004,

	0x14, 0x0808,	/* VFP, VBP Register setting */
	0x15, 0x3090,	/* HSW,HFP,HBP Register setting */
	ENDDEF, 0x0000

};

const unsigned short SEQ_MANUAL_DISPLAY_ON[] = {
	0x12, 0x0001,
	SLEEPMSEC, 1,
	0x12, 0x0003,
	SLEEPMSEC, 1,
	0x12, 0x0007,
	SLEEPMSEC, 1,
	0x12, 0x000f,
	SLEEPMSEC, 1,
	0x2e, 0x0605,
	SLEEPMSEC, 1,
	0x30, 0x0f11,
	0x31, 0x0709,
	SLEEPMSEC, 5,
	0x12, 0x001f,
	SLEEPMSEC, 10,

	ENDDEF, 0x0000
};

const unsigned short ACL_ON_DISPLAY_SETTING[] = {
	0x5b, 0x0013,
	0x5c, 0x0000,
	0x5d, 0x03ff,
	0x5e, 0x0000,
	0x5f, 0x0257,
	ENDDEF, 0x0000
};

const unsigned short ACL_ON_WINDOW_SETTING[] = {
	0x5b, 0x0013,
	0x5c, 0x0200,
	0x5d, 0x03ff,
	0x5e, 0x0000,
	0x5f, 0x0257,
	ENDDEF, 0x0000
};

const unsigned short SEQ_MANUAL_DISPLAY_OFF[] = {
	0x12, 0x000f,
	SLEEPMSEC, 1,
	0x2e, 0x0604,
	SLEEPMSEC, 1,
	0x12, 0x0001,
	SLEEPMSEC, 1,
	0x12, 0x0000,
	SLEEPMSEC, 2,

	ENDDEF, 0x0000
};

const unsigned short SEQ_STANDBY_ON[] = {
	0x02, 0x2301,

	ENDDEF, 0x0000
};

const unsigned short SEQ_STANDBY_OFF[] = {
	0x02, 0x2300,

	ENDDEF, 0x0000
};

const unsigned short SEQ_MANUAL_LTPS_OFF[] = {
	0x06, 0x08ff,
	SLEEPMSEC, 1,
	0x06, 0x03ff,
	SLEEPMSEC, 5,
	0x06, 0x007f,
	SLEEPMSEC, 5,
	0x06, 0x003f,
	SLEEPMSEC, 5,
	0x06, 0x001f,
	SLEEPMSEC, 5,
	0x06, 0x000f,
	SLEEPMSEC, 5,
	0x06, 0x0007,
	SLEEPMSEC, 5,
	0x06, 0x0003,
	SLEEPMSEC, 5,
	0x06, 0x0001,
	SLEEPMSEC, 5,
	0x06, 0x0000,

	ENDDEF, 0x0000

};
static void ams701ka_c110_spi_write_byte(unsigned char address, unsigned short command)
{
	int     j;
	unsigned char DELAY=1;
	unsigned int data;

	data = (address << 16) + command;

	S5PCFB_C110_CS_HIGH;
	S5PCFB_C110_SDA_HIGH;
	S5PCFB_C110_CLK_HIGH;
	udelay(DELAY);

	S5PCFB_C110_CS_LOW;
	udelay(DELAY);

	for (j = 23; j >= 0; j--)
	{
		S5PCFB_C110_CLK_LOW;

		/* data high or low */
		if ((data >> j) & 0x1) {
			S5PCFB_C110_SDA_HIGH;
		} else {
			S5PCFB_C110_SDA_LOW;
		}

		udelay(DELAY);

		S5PCFB_C110_CLK_HIGH;
		udelay(DELAY);
	}

	S5PCFB_C110_CS_HIGH;
	udelay(DELAY);
}

static unsigned short ams701ka_c110_spi_read_byte(unsigned char address)
{
	int     j;
	unsigned char DELAY=1;
	unsigned int data = 0;
	unsigned short command = 0;

	data = address << 16;

	S5PCFB_C110_CS_HIGH;
	S5PCFB_C110_SDA_HIGH;
	S5PCFB_C110_CLK_HIGH;
	udelay(DELAY);

	S5PCFB_C110_CS_LOW;
	udelay(DELAY);

	for (j = 23; j >= 0; j--)
	{
		S5PCFB_C110_CLK_LOW;

		if (j > 15) {
			/* data high or low */
			if ((data >> j) & 0x1)
				S5PCFB_C110_SDA_HIGH;
			else
				S5PCFB_C110_SDA_LOW;
		} else {
			if (S5PCFB_C110_SDO_READ == 1)
				command |= 1 << j;
			else
				command |= 0 << j;
		}

		udelay(DELAY);

		S5PCFB_C110_CLK_HIGH;
		udelay(DELAY);
	}

	S5PCFB_C110_CS_HIGH;
	udelay(DELAY);

	return command;
}

static void ams701ka_spi_write(unsigned char address, unsigned short command)
{
	if (cpu_is_s5pc110()) {
		if(address != DATA_ONLY)
			ams701ka_c110_spi_write_byte(0x70, address);	/* FIXME */

		ams701ka_c110_spi_write_byte(0x72, command);		/* FIXME */
	}
}

static void ams701ka_spi_read(unsigned char address)
{
	unsigned short data;

	ams701ka_c110_spi_write_byte(0x70, address);		/* FIXME */
	data = ams701ka_c110_spi_read_byte(0x71);		/* FIXME */
	printf("[status read] 0x%x : 0x%x\n", address, data);
}

static void ams701ka_panel_send_sequence(const unsigned short *wbuf)
{
	int i = 0;

	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC)
			ams701ka_spi_write(wbuf[i], wbuf[i+1]);
		else
			udelay(wbuf[i+1]*1000);
		i += 2;
	}
}

/* test */


void lcd_panel_power_on(void)
{
	/* set gpio data for MLCD_ON to HIGH */

	/* set gpio data for MLCD_RST to HIGH */
	gpio_set_value(&gpio->gpio_mp0_5, 5, 1);
	gpio_set_value(&gpio->gpio_mp0_5, 5, 0);
	udelay(100000);
	gpio_set_value(&gpio->gpio_mp0_5, 5, 1);

	/* set gpio data for MLCD_ON to HIGH */
	gpio_set_value(&gpio->gpio_j1, 3, 1);
	udelay(100000);

	udelay(10000);


	ams701ka_panel_send_sequence(GAMMA_SETTING);
	ams701ka_panel_send_sequence(MANUAL_POWER_ON_SETTING);
	ams701ka_panel_send_sequence(SEQ_SLEEP_OUT);
	ams701ka_panel_send_sequence(MANUAL_LTPS_SETTING);
	ams701ka_panel_send_sequence(SEQ_MANUAL_DISPLAY_ON);

	udelay(90000);
	ams701ka_panel_send_sequence(ACL_ON_DISPLAY_SETTING);
}


static inline void ams701ka_c110_panel_hw_reset(void)
{
	/* set gpio pin for MLCD_RST to LOW */
	gpio_set_value(&gpio->gpio_mp0_5, 5, 0);
	udelay(1);	/* Shorter than 5 usec */

	/* set gpio pin for MLCD_RST to HIGH */
	gpio_set_value(&gpio->gpio_mp0_5, 5, 1);
	udelay(10000);
}

void lcd_panel_enable(void)
{
	ams701ka_panel_send_sequence(SEQ_MANUAL_DISPLAY_ON);
}

static void ams701ka_panel_disable(void)
{
	ams701ka_panel_send_sequence(SEQ_MANUAL_DISPLAY_OFF);
}


void lcd_panel_init(void)
{
	/* set gpio pin for DISPLAY_CS to HIGH */
	gpio_set_value(&gpio->gpio_mp0_1, 1, 1);
	/* set gpio pin for DISPLAY_CLK to HIGH */
	gpio_set_value(&gpio->gpio_mp0_4, 1, 1);
	/* set gpio pin for DISPLAY_SI to HIGH */
	gpio_set_value(&gpio->gpio_mp0_4, 3, 1);
}
