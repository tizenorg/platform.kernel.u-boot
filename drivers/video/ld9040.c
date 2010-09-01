/*
 * LCD panel driver for Board based on S5PC100 and S5PC110. 
 *
 * Author: Donghwa Lee  <dh09.lee@samsung.com>
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
#include <spi.h>

/* these machine specific platform data would be setting at universal.c */
struct spi_platform_data *ld9040;

static const unsigned char SEQ_SWRESET[] = {
	0x01, COMMAND_ONLY,
};

static const unsigned char SEQ_USER_SETTING[] = {
	0xF0, 0x5A,

	DATA_ONLY, 0x5A,
};

static const unsigned char SEQ_ELVSS[] = {
	0xB1, 0x0B,

	DATA_ONLY, 0x00,
	DATA_ONLY, 0x16,
};

static const unsigned char SEQ_GTCON[] = {
	0xF7, 0x09,

	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
};

static const unsigned char SEQ_GAMMA_SET1[] = {
	0xF9, 0x18,

	DATA_ONLY, 0x9A,
	DATA_ONLY, 0xB0,
	DATA_ONLY, 0xAB,
	DATA_ONLY, 0xC4,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xAA,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xA1,
	DATA_ONLY, 0xB5,
	DATA_ONLY, 0xB0,
	DATA_ONLY, 0xC7,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xC5,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0xA7,
	DATA_ONLY, 0xAC,
	DATA_ONLY, 0x9A,
	DATA_ONLY, 0xB6,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xE5,
};

static const unsigned char SEQ_GAMMA_CTRL[] = {
	0xFB, 0x00,

	DATA_ONLY, 0x5A,
};

static const unsigned char SEQ_APON[] = {
	0xF3, 0x00,

	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x0A,
	DATA_ONLY, 0x02,
};

static const unsigned char SEQ_DISPCTL[] = {
	0xF2, 0x02,

	DATA_ONLY, 0x03,
	DATA_ONLY, 0x1C,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x10,
};

static const unsigned char SEQ_SLPOUT[] = {
	0x11, COMMAND_ONLY,
};

static const unsigned char SEQ_SLPIN[] = {
	0x10, COMMAND_ONLY,
};

static const unsigned char SEQ_DISPON[] = {
	0x29, COMMAND_ONLY,
};

static const unsigned char SEQ_DISPOFF[] = {
	0x28, COMMAND_ONLY,
};

static const unsigned char SEQ_VCI1_1ST_EN[] = {
	0xF3, 0x10,

	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x02,
};

static const unsigned char SEQ_VL1_EN[] = {
	0xF3, 0x11,

	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x02,
};

static const unsigned char SEQ_VL2_EN[] = {
	0xF3, 0x13,

	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x02,
};

static const unsigned char SEQ_VCI1_2ND_EN[] = {
	0xF3, 0x33,

	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x02,
};

static const unsigned char SEQ_VL3_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x02,
};

static const unsigned char SEQ_VREG1_AMP_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0x01,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x02,
};

static const unsigned char SEQ_VGH_AMP_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0x11,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x02,
};

static const unsigned char SEQ_VGL_AMP_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0x31,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x02,
};

static const unsigned char SEQ_VMOS_AMP_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0xB1,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
};

static const unsigned char SEQ_VINT_AMP_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0xF1,
	/* DATA_ONLY, 0x71,	VMOS/VBL/VBH not used */
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
	/* DATA_ONLY, 0x02,	VMOS/VBL/VBH not used */
};

static const unsigned char SEQ_VBH_AMP_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0xF9,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
};

static const unsigned char SEQ_VBL_AMP_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0xFD,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
};

static const unsigned char SEQ_GAM_AMP_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0xFF,
	/* DATA_ONLY, 0x73,	VMOS/VBL/VBH not used */
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
	/* DATA_ONLY, 0x02,	VMOS/VBL/VBH not used */
};

static const unsigned char SEQ_SD_AMP_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0xFF,
	/* DATA_ONLY, 0x73,	VMOS/VBL/VBH not used */
	DATA_ONLY, 0x80,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
	/* DATA_ONLY, 0x02,	VMOS/VBL/VBH not used */
};

static const unsigned char SEQ_GLS_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0xFF,
	/* DATA_ONLY, 0x73,	VMOS/VBL/VBH not used */
	DATA_ONLY, 0x81,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
	/* DATA_ONLY, 0x02,	VMOS/VBL/VBH not used */
};

static const unsigned char SEQ_ELS_EN[] = {
	0xF3, 0x37,

	DATA_ONLY, 0xFF,
	/* DATA_ONLY, 0x73,	VMOS/VBL/VBH not used */
	DATA_ONLY, 0x83,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
	/* DATA_ONLY, 0x02,	VMOS/VBL/VBH not used */
};

static const unsigned char SEQ_EL_ON[] = {
	0xF3, 0x37,

	DATA_ONLY, 0xFF,
	/* DATA_ONLY, 0x73,	VMOS/VBL/VBH not used */
	DATA_ONLY, 0x87,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
	/* DATA_ONLY, 0x02,	VMOS/VBL/VBH not used */
};

static void ld9040_spi_write(unsigned char address, unsigned char command)
{
	if (address != DATA_ONLY)
		spi_gpio_write(ld9040, (unsigned int) 0x0, (unsigned int) address);

	if (command != COMMAND_ONLY)
		spi_gpio_write(ld9040, (unsigned int) 0x1, (unsigned int) command);
}

static void ld9040_panel_send_sequence(const unsigned char *wbuf, unsigned int size_cmd)
{
	int i = 0;

	/* workaround */
	udelay(10);

	while (i < size_cmd) {
		ld9040_spi_write(wbuf[i], wbuf[i+1]);
		i += 2;
	}
}

void ld9040_cfg_ldo(void)
{
#if 1
	/* SMD power on sequence */
	ld9040_panel_send_sequence(SEQ_USER_SETTING, ARRAY_SIZE(SEQ_USER_SETTING));
	ld9040_panel_send_sequence(SEQ_ELVSS, ARRAY_SIZE(SEQ_ELVSS));
	ld9040_panel_send_sequence(SEQ_GTCON, ARRAY_SIZE(SEQ_GTCON));
	ld9040_panel_send_sequence(SEQ_GAMMA_SET1, ARRAY_SIZE(SEQ_GAMMA_SET1));
	ld9040_panel_send_sequence(SEQ_GAMMA_CTRL, ARRAY_SIZE(SEQ_GAMMA_CTRL));
	ld9040_panel_send_sequence(SEQ_SLPOUT, ARRAY_SIZE(SEQ_SLPOUT));
#endif

#if 0
	/* Auto power on sequence */
	ld9040_panel_send_sequence(SEQ_USER_SETTING, ARRAY_SIZE(SEQ_USER_SETTING));
	ld9040_panel_send_sequence(SEQ_ELVSS, ARRAY_SIZE(SEQ_ELVSS));
	ld9040_panel_send_sequence(SEQ_SLPOUT, ARRAY_SIZE(SEQ_SLPOUT));
#endif

#if 0
	/* Manual power on sequence */
	ld9040_panel_send_sequence(SEQ_SWRESET, ARRAY_SIZE(SEQ_SWRESET));
	ld9040_panel_send_sequence(SEQ_USER_SETTING, ARRAY_SIZE(SEQ_USER_SETTING));
	ld9040_panel_send_sequence(SEQ_ELVSS, ARRAY_SIZE(SEQ_ELVSS));
	ld9040_panel_send_sequence(SEQ_SLPOUT, ARRAY_SIZE(SEQ_SLPOUT));
	ld9040_panel_send_sequence(SEQ_VCI1_1ST_EN, ARRAY_SIZE(SEQ_VCI1_1ST_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VL1_EN, ARRAY_SIZE(SEQ_VL1_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VL2_EN, ARRAY_SIZE(SEQ_VL2_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VCI1_2ND_EN, ARRAY_SIZE(SEQ_VCI1_2ND_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VL3_EN, ARRAY_SIZE(SEQ_VL3_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VREG1_AMP_EN, ARRAY_SIZE(SEQ_VREG1_AMP_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VGH_AMP_EN, ARRAY_SIZE(SEQ_VGH_AMP_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VGL_AMP_EN, ARRAY_SIZE(SEQ_VGL_AMP_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VMOS_AMP_EN, ARRAY_SIZE(SEQ_VMOS_AMP_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VINT_AMP_EN, ARRAY_SIZE(SEQ_VINT_AMP_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VBH_AMP_EN, ARRAY_SIZE(SEQ_VBH_AMP_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_VBL_AMP_EN, ARRAY_SIZE(SEQ_VBL_AMP_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_GAM_AMP_EN, ARRAY_SIZE(SEQ_GAM_AMP_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_SD_AMP_EN, ARRAY_SIZE(SEQ_SD_AMP_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_GLS_EN, ARRAY_SIZE(SEQ_GLS_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_ELS_EN, ARRAY_SIZE(SEQ_ELS_EN));
	udelay(6000);
	ld9040_panel_send_sequence(SEQ_EL_ON, ARRAY_SIZE(SEQ_EL_ON));
	udelay(6000);
#endif

	udelay(10);
}

void ld9040_enable_ldo(unsigned int onoff)
{
	if (onoff) {
		ld9040_panel_send_sequence(SEQ_DISPON, ARRAY_SIZE(SEQ_DISPON));
	}
}

/* this function would be called at universal.c */
void ld9040_set_platform_data(struct spi_platform_data *pd)
{
	if (pd == NULL) {
		printf("pd is NULL.\n");
		return;
	}

	ld9040 = pd;
}

