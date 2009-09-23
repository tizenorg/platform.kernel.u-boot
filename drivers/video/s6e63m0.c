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

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

#define PACKET_LEN		8

#define S5PCFB_C110_CS_LOW	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_1_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET)) & 0xfd, S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_1_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET))
#define S5PCFB_C110_CS_HIGH	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_1_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET)) | 0x02, S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_1_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET))
#define S5PCFB_C110_CLK_LOW	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET)) & 0xfd, S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET))
#define S5PCFB_C110_CLK_HIGH	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET)) | 0x02, S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET))
#define S5PCFB_C110_SDA_LOW	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET)) & 0xf7, S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET))
#define S5PCFB_C110_SDA_HIGH	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET)) | 0x08, S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+\
		S5PC1XX_GPIO_DAT_OFFSET))

const unsigned short SEQ_PANEL_CONDITION_SET[] = {
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

const unsigned short SEQ_DISPLAY_CONDITION_SET[] = {
	0xf2, 0x02,
	DATA_ONLY, 0x03,
	DATA_ONLY, 0x1c,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x10,

	0xf7, 0x03,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,

	ENDDEF, 0x0000
};

const unsigned short SEQ_GAMMA_SETTING[] = {
	0xfa, 0x00,
	DATA_ONLY, 0x18,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x5f,
	DATA_ONLY, 0x50,
	DATA_ONLY, 0x2d,
	DATA_ONLY, 0xb6,
	DATA_ONLY, 0xb9,
	DATA_ONLY, 0xa7,
	DATA_ONLY, 0xad,
	DATA_ONLY, 0xb1,
	DATA_ONLY, 0x9f,
	DATA_ONLY, 0xbe,
	DATA_ONLY, 0xc0,
	DATA_ONLY, 0xb5,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xa0,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xa4,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xdb,

	0xfa, 0x01,

	ENDDEF, 0x0000
};

const unsigned short SEQ_ETC_CONDITION_SET[] = {
	0xf6, 0x00,
	DATA_ONLY, 0x8c,
	DATA_ONLY, 0x07,

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


const unsigned short SEQ_STAND_BY_OFF[] = {
	0x11, COMMAND_ONLY,

	ENDDEF, 0x0000
};

unsigned char DELAY=1;

static void s6e63m0_c110_spi_write_byte(unsigned char address, unsigned char command)
{
	int     j;
	unsigned short data;

	data = (address << 8) + command;

	S5PCFB_C110_CS_HIGH;
	S5PCFB_C110_SDA_HIGH;
	S5PCFB_C110_CLK_HIGH;
	udelay(DELAY);

	S5PCFB_C110_CS_LOW;
	udelay(DELAY);

	for (j = PACKET_LEN; j >= 0; j--)
	{
		S5PCFB_C110_CLK_LOW;

		/* data high or low */
		if ((data >> j) & 0x0001)
			S5PCFB_C110_SDA_HIGH;
		else
			S5PCFB_C110_SDA_LOW;

		udelay(DELAY);

		S5PCFB_C110_CLK_HIGH;
		udelay(DELAY);
	}

	S5PCFB_C110_CS_HIGH;
	udelay(DELAY);
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

void lcd_panel_power_on(void)
{
	udelay(25000);

	/* set gpio data for MLCD_RST to HIGH */
	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_5_OFFSET+
			S5PC1XX_GPIO_DAT_OFFSET)) | 0x20,
		S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_5_OFFSET+
		    S5PC1XX_GPIO_DAT_OFFSET));
	/* set gpio data for MLCD_ON to HIGH */
	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_J1_OFFSET+
			S5PC1XX_GPIO_DAT_OFFSET)) | 0x8,
		S5PC110_GPIO_BASE(S5PC110_GPIO_J1_OFFSET+
		    S5PC1XX_GPIO_DAT_OFFSET));

	/* set gpio data for MLCD_RST to LOW */
	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_5_OFFSET+
			S5PC1XX_GPIO_DAT_OFFSET)) & 0xdf,
		S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_5_OFFSET+
		    S5PC1XX_GPIO_DAT_OFFSET));
	udelay(20);
	/* set gpio data for MLCD_RST to HIGH */
	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_5_OFFSET+
			S5PC1XX_GPIO_DAT_OFFSET)) | 0x20,
		S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_5_OFFSET+
		    S5PC1XX_GPIO_DAT_OFFSET));

	udelay(120000);

	s6e63m0_panel_send_sequence(SEQ_PANEL_CONDITION_SET);
	s6e63m0_panel_send_sequence(SEQ_DISPLAY_CONDITION_SET);
	s6e63m0_panel_send_sequence(SEQ_GAMMA_SETTING);
	s6e63m0_panel_send_sequence(SEQ_ETC_CONDITION_SET);
}

static inline void s6e63m0_c110_panel_hw_reset(void)
{
	/* set gpio pin for MLCD_RST to LOW */
	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET+
			S5PC1XX_GPIO_DAT_OFFSET)) & 0x7f,
		S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET+
		    S5PC1XX_GPIO_DAT_OFFSET));
	udelay(1);	/* Shorter than 5 usec */
	/* set gpio pin for MLCD_RST to HIGH */
	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET+
			S5PC1XX_GPIO_DAT_OFFSET)) | 0x80,
		S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET+
		    S5PC1XX_GPIO_DAT_OFFSET));
	udelay(10000);
}

void lcd_panel_enable(void)
{
	s6e63m0_panel_send_sequence(SEQ_STAND_BY_OFF);
}

static void s6e63m0_panel_disable(void)
{
}

void lcd_panel_init(void)
{
	/* set gpio pin for DISPLAY_CS to HIGH */
	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_1_OFFSET+
			S5PC1XX_GPIO_DAT_OFFSET)) | 0x02,
		S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_1_OFFSET+
		    S5PC1XX_GPIO_DAT_OFFSET));
	/* set gpio pin for DISPLAY_CLK to HIGH */
	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+
			S5PC1XX_GPIO_DAT_OFFSET)) | 0x02,
		S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+
		    S5PC1XX_GPIO_DAT_OFFSET));
	/* set gpio pin for DISPLAY_SI to HIGH */
	writel(readl(S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+
			S5PC1XX_GPIO_DAT_OFFSET)) | 0x08,
		S5PC110_GPIO_BASE(S5PC110_GPIO_MP0_4_OFFSET+
		    S5PC1XX_GPIO_DAT_OFFSET));
}
