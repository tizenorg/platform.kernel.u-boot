/*
 * S5PC100 and S5PC110 LCD Controller driver.
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <lcd.h>

#include "s5p-fb.h"
#include "logo.h"
/*
#include "logo_rgb24_wvga_portrait.h"
#include "opening_logo_rgb24_143_44.h"
*/

extern void tl2796_panel_power_on(void);
extern void tl2796_panel_enable(void);
extern void tl2796_panel_init(void);

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;
void *lcd_console_address;

short console_col;
short console_row;

static unsigned int panel_width, panel_height;

static unsigned short makepixel565(char r, char g, char b)
{
    return (unsigned short)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

static unsigned int makepixel8888(char a, char r, char g, char b)
{
	return (unsigned int)((a << 24) | (r << 16) | (g << 8)  | b);
}

static void read_image16(char* pImg, int x1pos, int y1pos, int x2pos,
		int y2pos, unsigned short pixel)
{
	unsigned short *pDst = (unsigned short *)pImg;
	unsigned int offset_s;
	int i, j;

	for(i = y1pos; i < y2pos; i++) {
		for(j = x1pos; j < x2pos; j++) {
			offset_s = i * panel_width + j;
			*(pDst + offset_s) = pixel;
		}
	}
}

static void read_image32(char* pImg, int x1pos, int y1pos, int x2pos,
		int y2pos, unsigned int pixel)
{
	unsigned int *pDst = (unsigned int *)pImg;
	unsigned int offset_s;
	int i, j;

	for(i = y1pos; i < y2pos; i++) {
		for(j = x1pos; j < x2pos; j++) {
			offset_s = i * panel_width + j;
			*(pDst+offset_s) = pixel;
		}
	}
}

/* LCD Panel data */
vidinfo_t panel_info = {
		.vl_lbw		= 0,
		.vl_splt	= 0,
		.vl_clor	= 1,
		.vl_tft		= 1,
};

static void s5pc_lcd_init_mem(void *lcdbase, vidinfo_t *vid)
{
	unsigned long palette_size, palette_mem_size;
	unsigned int fb_size;

	fb_size = vid->vl_row * vid->vl_col * (vid->vl_bpix / 8);

	lcd_base = lcdbase;

	palette_size = NBITS(vid->vl_bpix) == 8 ? 256 : 16;
	palette_mem_size = palette_size * sizeof(u32);

	s5pc_fimd_lcd_init_mem((unsigned long)lcd_base, (unsigned long)fb_size, palette_size);

	udebug("fb_size=%d, screen_base=%x, palette_size=%d, palettle_mem_size=%d\n",
			fb_size, (unsigned int)lcd_base, (int)palette_size, (int)palette_mem_size);
}

static void s5pc_gpio_setup(void)
{
	if (cpu_is_s5pc100())
		s5pc_c100_gpio_setup();
	else
		s5pc_c110_gpio_setup();
}

static void s5pc_lcd_init(vidinfo_t *vid)
{
	s5pc_fimd_lcd_init(vid);
}

static void lcd_test(void)
{
	/* red */
	read_image32((char *)lcd_base, 0, 0, 480, 200,
			makepixel8888(0, 255, 0, 0));
	/* green */
	read_image32((char *)lcd_base, 0, 200, 480, 400,
			makepixel8888(0, 0, 255, 0));
	/* blue */
	read_image32((char *)lcd_base, 0, 400, 480, 600,
			makepixel8888(0, 0, 0, 255));
	/* write */
	read_image32((char *)lcd_base, 0, 600, 480, 800,
			makepixel8888(0, 255, 255, 255));
}

int conv_rgb565_to_rgb888(unsigned short rgb565, unsigned int sw)
{
	char red, green, blue;
	unsigned int threshold = 150;

	red = (rgb565 & 0xF800) >> 11;
	green = (rgb565 & 0x7E0) >> 5;
	blue = (rgb565 & 0x1F);

	red = red << 3;
	green = green << 2;
	blue = blue << 3;

	/* correct error pixels of samsung logo. */
	if (sw) {
		if (red > threshold)
			red = 255;
		if (green > threshold)
			green = 255;
		if (blue > threshold)
			blue = 255;
	}

	return (red << 16 | green << 8 | blue);
}

void draw_bitmap(void *lcdbase, int x, int y, int w, int h, unsigned long *bmp)
{
	int i, j;
	short k = 0;
	unsigned long *fb = (unsigned  long*)lcdbase;

	for (j = y; j < (y + h); j++) {
		for (i = x; i < (x + w); i++)
			*(fb + (j * panel_width) + i) = *(bmp + k++);
	}
}

void _draw_samsung_logo(void *lcdbase, int x, int y, int w, int h, unsigned short *bmp)
{
	int i, j, error_range = 40;
	short k = 0;
	unsigned int pixel;
	unsigned long *fb = (unsigned  long*)lcdbase;

	for (j = y; j < (y + h); j++) {
		for (i = x; i < (x + w); i++) {
			pixel = (*(bmp + k++));

			/* 40 lines under samsung logo image are error range. */
			if (j > h + y - error_range)
				*(fb + (j * panel_width) + i) =
					conv_rgb565_to_rgb888(pixel, 1);
			else
				*(fb + (j * panel_width) + i) =
					conv_rgb565_to_rgb888(pixel, 0);
		}
	}
}

static void draw_samsung_logo(void* lcdbase)
{
	int x, y;

	x = (panel_width - 298) / 2;
	y = (panel_height - 78) / 2 - 5;

	_draw_samsung_logo(lcdbase, x, y, 298, 78, (unsigned short *) logo);
}

static void s5pc_init_panel_info(vidinfo_t *vid)
{
#if 1
	vid->vl_col	= 480,
	vid->vl_row	= 800,
	vid->vl_width	= 480,
	vid->vl_height	= 800,
	vid->vl_clkp	= CONFIG_SYS_HIGH,
	vid->vl_hsp	= CONFIG_SYS_LOW,
	vid->vl_vsp	= CONFIG_SYS_LOW,
	vid->vl_dp	= CONFIG_SYS_HIGH,
	vid->vl_bpix	= 32,

	/* S6E63M0 LCD Panel */
	vid->vl_hpw	= 2,	/* HLW */
	vid->vl_blw	= 16,	/* HBP */
	vid->vl_elw	= 16,	/* HFP */

	vid->vl_vpw	= 2,	/* VLW */
	vid->vl_bfw	= 3,	/* VBP */
	vid->vl_efw	= 28,	/* VFP */
#endif
#if 0
	vid->vl_col	= 480,
	vid->vl_row	= 800,
	vid->vl_width	= 480,
	vid->vl_height	= 800,
	vid->vl_clkp	= CONFIG_SYS_HIGH,
	vid->vl_hsp	= CONFIG_SYS_LOW,
	vid->vl_vsp	= CONFIG_SYS_LOW,
	vid->vl_dp	= CONFIG_SYS_HIGH,
	vid->vl_bpix	= 32,

	/* tl2796 panel. */
	vid->vl_hpw	= 4,
	vid->vl_blw	= 8,
	vid->vl_elw	= 8,

	vid->vl_vpw	= 4,
	vid->vl_bfw	= 8,
	vid->vl_efw	= 8,
#endif
#if 0
	vid->vl_col	= 1024,
	vid->vl_row	= 600,
	vid->vl_width	= 1024,
	vid->vl_height	= 600,
	vid->vl_clkp	= CONFIG_SYS_HIGH,
	vid->vl_hsp	= CONFIG_SYS_HIGH,
	vid->vl_vsp	= CONFIG_SYS_HIGH,
	vid->vl_dp	= CONFIG_SYS_LOW,
	vid->vl_bpix	= 32,

	/* AMS701KA AMOLED Panel. */
	vid->vl_hpw	= 30,
	vid->vl_blw	= 114,
	vid->vl_elw	= 48,

	vid->vl_vpw	= 2,
	vid->vl_bfw	= 6,
	vid->vl_efw	= 8,
#endif

	panel_width = vid->vl_col;
	panel_height = vid->vl_row;
}

static void lcd_panel_on(void)
{
	lcd_panel_init();
	lcd_panel_power_on();

	lcd_panel_enable();
}

void lcd_ctrl_init(void *lcdbase)
{
	char *option;

	s5pc_lcd_init_mem(lcdbase, &panel_info);

	/* initialize parameters which is specific to panel. */
	s5pc_init_panel_info(&panel_info);

	option = getenv("lcd");

	/*
	if (strcmp(option, "test") == 0) {
		memset(lcdbase, 0, PANEL_WIDTH*PANEL_HEIGHT*S5P_LCD_BPP >> 3);
		lcd_test();
	} else if (strcmp(option, "image") == 0)
		memcpy(lcdbase, LOGO_RGB24, PANEL_WIDTH*PANEL_HEIGHT*S5P_LCD_BPP >> 3);
	else {
		memset(lcdbase, 0, PANEL_WIDTH*PANEL_HEIGHT*S5P_LCD_BPP >> 3);
		draw_samsung_logo(lcdbase);
	}
	*/

	memset(lcdbase, 0, panel_width * panel_height * (32 >> 3));
	draw_samsung_logo(lcdbase);

	s5pc_gpio_setup();

	s5pc_lcd_init(&panel_info);

	/* font test */
	/*
	init_font();
	set_font_color(FONT_WHITE);
	fb_printf("Test\n");
	exit_font();
	*/
}


void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blud)
{
	return;
}

void lcd_enable(void)
{
	lcd_panel_on();
}

ulong calc_fbsize(void)
{
	return s5pc_fimd_calc_fbsize();
}

void s5pc1xxfb_test(void *lcdbase)
{
	lcd_ctrl_init(lcdbase);
	lcd_enable();
}
