/*
 * S5PC100 LCD Controller driver.
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
#include <devices.h>
#include <lcd.h>

#include <asm/arch/regs-lcd.h>
#include <asm/arch/hardware.h>
#include "s5pcfb.h"

/* clock definitions */
#define SELECT_CLOCK_SOURCE2	0xE0100208
#define LCD_CLOCK_SOURCE		0x1000

#define CLOCK_DIV1				0xE0100304

/* LCD Panel definitions */
#define PANEL_WIDTH			480
#define PANEL_HEIGHT		800
#define S5PC_LCD_BPP		32	

#define S5PCFB_HFP			8
#define S5PCFB_HSW			4
#define S5PCFB_HBP			8	

#define S5PCFB_VFP			8
#define S5PCFB_VSW			4
#define S5PCFB_VBP			8

#define S5PCFB_HRES			480
#define S5PCFB_VRES			800

#define S5PCFB_HRES_VIRTUAL	480
#define S5PCFB_VRES_VIRTUAL	800

#define S5PCFB_HRES_OSD		480
#define S5PCFB_VRES_OSD		800

#define S5PC_VFRAME_FREQ	60

#define DEBUG 
#ifdef DEBUG
#define udebug printf
#else
#define udebug
#endif

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

static unsigned short makepixel565(char r, char g, char b)
{
    return (unsigned short)(((r>>3)<<11)|((g>>2)<<5)|(b>>3));
}

static unsigned int makepixel8888(char a, char r, char g, char b)
{
	return (unsigned int) ((a << 24) | (r << 16) | (g << 8)  | b);
}

static void read_image16(char* pImg, int x1pos, int y1pos, int x2pos, int y2pos, unsigned short pixel)
{
	unsigned short *pDst = (unsigned short *)pImg;
	unsigned int offset_s;
	int i, j;

	for(i = y1pos; i < y2pos; i++) {
		for(j = x1pos; j < x2pos; j++) {
			offset_s = i*PANEL_WIDTH+j;
			*(pDst+offset_s) = pixel;
		}
	}
}

static void read_image32(char* pImg, int x1pos, int y1pos, int x2pos, int y2pos, unsigned int pixel)
{
	unsigned int *pDst = (unsigned int *)pImg;
	unsigned int offset_s;
	int i, j;

	for(i = y1pos; i < y2pos; i++) {
		for(j = x1pos; j < x2pos; j++) {
			offset_s = i*PANEL_WIDTH+j;
			*(pDst+offset_s) = pixel;
		}
	}
}

vidinfo_t panel_info = {
		.vl_col		= PANEL_WIDTH,
		.vl_row		= PANEL_HEIGHT,
		.vl_width	= PANEL_WIDTH,
		.vl_height	= PANEL_HEIGHT,
		.vl_clkp	= CONFIG_SYS_HIGH,
		.vl_hsp		= CONFIG_SYS_LOW,
		.vl_vsp		= CONFIG_SYS_LOW,
		.vl_dp		= CONFIG_SYS_HIGH,
		.vl_bpix	= S5PC_LCD_BPP,
		.vl_lbw		= 0,
		.vl_splt	= 0,
		.vl_clor	= 0,
		.vl_tft		= 1,

		.vl_hpw		= 4,
		.vl_blw		= 8,
		.vl_elw		= 8,

		.vl_vpw		= 4,
		.vl_bfw		= 8,
		.vl_efw		= 8,
};

s5pcfb_fimd_info_t s5pcfb_fimd = {
	.vidcon0 = S5PC_VIDCON0_INTERLACE_F_PROGRESSIVE | S5PC_VIDCON0_VIDOUT_RGB_IF | \
			   S5PC_VIDCON0_L1_DATA16_SUB_16PLUS8_MODE | S5PC_VIDCON0_L0_DATA16_MAIN_16PLUS8_MODE | \
			   S5PC_VIDCON0_PNRMODE_RGB_P | S5PC_VIDCON0_CLKVALUP_ALWAYS | \
			   S5PC_VIDCON0_CLKDIR_DIVIDED | S5PC_VIDCON0_CLKSEL_F_HCLK | \
			   S5PC_VIDCON0_ENVID_DISABLE | S5PC_VIDCON0_ENVID_F_DISABLE,

	.vidcon1 = S5PC_VIDCON1_IHSYNC_INVERT | S5PC_VIDCON1_IVSYNC_INVERT | 
		S5PC_VIDCON1_IVDEN_INVERT | S5PC_VIDCON1_IVCLK_RISE_EDGE,

	.vidtcon0 = S5PC_VIDTCON0_VBPD(S5PCFB_VBP - 1) | S5PC_VIDTCON0_VFPD(S5PCFB_VFP - 1) | \
				S5PC_VIDTCON0_VSPW(S5PCFB_VSW - 1),
	.vidtcon1 = S5PC_VIDTCON1_HBPD(S5PCFB_HBP - 1) | S5PC_VIDTCON1_HFPD(S5PCFB_HFP - 1) | \
				S5PC_VIDTCON1_HSPW(S5PCFB_HSW - 1),
	.vidtcon2 = S5PC_VIDTCON2_LINEVAL(S5PCFB_VRES - 1) | S5PC_VIDTCON2_HOZVAL(S5PCFB_HRES - 1),
	.vidosd0a = S5PC_VIDOSDxA_OSD_LTX_F(0) | S5PC_VIDOSDxA_OSD_LTY_F(0),
	.vidosd0b = S5PC_VIDOSDxB_OSD_RBX_F(S5PCFB_HRES - 1) | S5PC_VIDOSDxB_OSD_RBY_F(S5PCFB_VRES - 1),

	.vidosd1a = S5PC_VIDOSDxA_OSD_LTX_F(0) | S5PC_VIDOSDxA_OSD_LTY_F(0),
	.vidosd1b = S5PC_VIDOSDxB_OSD_RBX_F(S5PCFB_HRES_OSD - 1) | S5PC_VIDOSDxB_OSD_RBY_F(S5PCFB_VRES_OSD - 1),

	.width = PANEL_WIDTH,
	.height = PANEL_HEIGHT,
	.xres = PANEL_WIDTH,
	.yres = PANEL_HEIGHT,

	.dithmode = (S5PC_DITHMODE_RDITHPOS_5BIT | S5PC_DITHMODE_GDITHPOS_6BIT | \
			S5PC_DITHMODE_BDITHPOS_5BIT ) & S5PC_DITHMODE_DITHERING_DISABLE,

	.wincon0 = S5PC_WINCONx_HAWSWP_DISABLE | S5PC_WINCONx_BURSTLEN_16WORD | S5PC_WINCONx_BPPMODE_F_24BPP_888,

	.bpp = S5PC_LCD_BPP,
	.bytes_per_pixel = 4,
	.wpalcon = S5PC_WPALCON_W0PAL_24BIT,

	.vidintcon0 = S5PC_VIDINTCON0_FRAMESEL0_VSYNC | S5PC_VIDINTCON0_FRAMESEL1_NONE | \
				  S5PC_VIDINTCON0_INTFRMEN_DISABLE | S5PC_VIDINTCON0_FIFOSEL_WIN0 | \
				  S5PC_VIDINTCON0_FIFOLEVEL_25 | S5PC_VIDINTCON0_INTFIFOEN_DISABLE | \
				  S5PC_VIDINTCON0_INTEN_ENABLE,
	.vidintcon1 = 0,
	.xoffset = 0,
	.yoffset = 0,
};


static void s5pc_lcd_clock_enable(void)
{
	__REG(SELECT_CLOCK_SOURCE2) = LCD_CLOCK_SOURCE;
}

static void s5pc_lcd_init_mem(void *lcdbase, vidinfo_t *vid)
{
	u_long palette_mem_size;
	unsigned int fb_size = vid->vl_row * vid->vl_col * (vid->vl_bpix / 8);

	s5pcfb_fimd.screen = (u_long)lcdbase;

	s5pcfb_fimd.palette_size = NBITS(vid->vl_bpix) == 8 ? 256 : 16;
	palette_mem_size = s5pcfb_fimd.palette_size * sizeof(u32);

	s5pcfb_fimd.palette = (u_long)lcdbase + fb_size + PAGE_SIZE - palette_mem_size;

	udebug("fb_size=%d, screen_base=%x, palette_size=%d, palettle_mem_size=%d, palette=%x\n",
			fb_size, s5pcfb_fimd.screen, s5pcfb_fimd.palette_size, palette_mem_size,
			s5pcfb_fimd.palette);
}

static void s5pc_gpio_setup(void)
{
	/* set GPF0[0:7] for RGB Interface and Data lines */
	__REG(0xE03000E0) = 0x22222222;

	/* set Data lines */
	__REG(0xE0300100) = 0x22222222;
	__REG(0xE0300120) = 0x22222222;
	__REG(0xE0300140) = 0x2222;

	/* set gpio configuration pin for MLCD_RST */
	__REG(0xE0300C20) = 0x10000000;

	/* set gpio configuration pin for MLCD_ON */
	__REG(0xE0300220) = 0x1000;
	__raw_writel(__raw_readl(0xE0300224) & 0xf7, 0xE0300224);

	/* set gpio configuration pin for DISPLAY_CS, DISPLAY_CLK and DISPLSY_SI */
	__REG(0xE0300300) = 0x11100000;
}

static void s5pc_lcd_init(vidinfo_t *vid)
{
	unsigned long page_width, offset;
	unsigned int mpll_ratio, fb_size;

	s5pcfb_fimd.bytes_per_pixel = vid->vl_bpix / 8;
	page_width = s5pcfb_fimd.xres * s5pcfb_fimd.bytes_per_pixel;
	offset = 0;

	/* calculate LCD Pixel clock */
	s5pcfb_fimd.pixclock = (S5PC_VFRAME_FREQ * (vid->vl_hpw + vid->vl_blw + vid->vl_elw + vid->vl_width)
			* (vid->vl_vpw + vid->vl_bfw + vid->vl_efw + vid->vl_height));

	/* initialize the fimd specific */
	s5pcfb_fimd.vidintcon0 &= ~S5PC_VIDINTCON0_FRAMESEL0_MASK;
	s5pcfb_fimd.vidintcon0 |= S5PC_VIDINTCON0_FRAMESEL0_VSYNC;
	s5pcfb_fimd.vidintcon0 |= S5PC_VIDINTCON0_INTFRMEN_ENABLE;

	__REG(S5PC_VIDINTCON0) = s5pcfb_fimd.vidintcon0;

	/* set configuration register for VCLK*/
	s5pcfb_fimd.vidcon0 = s5pcfb_fimd.vidcon0 & ~(S5PC_VIDCON0_ENVID_ENABLE | S5PC_VIDCON0_ENVID_F_ENABLE);
	__REG(S5PC_VIDCON0) = s5pcfb_fimd.vidcon0;

	mpll_ratio = (__raw_readl(CLOCK_DIV1) & 0x000000f0) >> 4;
	s5pcfb_fimd.vidcon0 |= S5PC_VIDCON0_CLKVAL_F((int)(((get_MCLK() / mpll_ratio) / s5pcfb_fimd.pixclock) - 1));

	udebug("mpll_ratio = %d, MCLK = %d, pixclock=%d, vidcon0 = %d\n", mpll_ratio, get_MCLK(),
			s5pcfb_fimd.pixclock, s5pcfb_fimd.vidcon0);


	/* set window size */
	s5pcfb_fimd.vidosd0c = S5PC_VIDOSD0C_OSDSIZE(PANEL_WIDTH * PANEL_HEIGHT);

	/* set wondow position */
	__REG(S5PC_VIDOSD0A) = S5PC_VIDOSDxA_OSD_LTX_F(0) | S5PC_VIDOSDxA_OSD_LTY_F(0);
	__REG(S5PC_VIDOSD0B) = S5PC_VIDOSDxB_OSD_RBX_F(PANEL_WIDTH - 1 + s5pcfb_fimd.xoffset) |
		S5PC_VIDOSDxB_OSD_RBY_F(PANEL_HEIGHT - 1 + s5pcfb_fimd.yoffset);

	/* set framebuffer start address */
	__REG(S5PC_VIDW00ADD0B0) = s5pcfb_fimd.screen;

	/* set framebuffer end address  */
	__REG(S5PC_VIDW00ADD1B0) = (__raw_readl(S5PC_VIDW00ADD0B0) + (page_width + offset) * s5pcfb_fimd.yres);
	
	/* set framebuffer size */
	fb_size = S5PC_VIDWxxADD2_OFFSIZE_F(offset) | (S5PC_VIDWxxADD2_PAGEWIDTH_F(page_width));
	__REG(S5PC_VIDW00ADD2) = fb_size;

	udebug("fb_size at s5pc_lcd_init=%d, page_width=%d\n", fb_size, page_width);

	/* set window0 conguration register */
	//s5pcfb_fimd.wincon0 = S5PC_WINCONx_HAWSWP_ENABLE | S5PC_WINCONx_BURSTLEN_16WORD |
	//	S5PC_WINCONx_BPPMODE_F_16BPP_565;
	s5pcfb_fimd.wincon0 = S5PC_WINCONx_WSWP_ENABLE | S5PC_WINCONx_BURSTLEN_16WORD | S5PC_WINCONx_BPPMODE_F_24BPP_888;
	s5pcfb_fimd.bpp = S5PC_LCD_BPP;
	s5pcfb_fimd.bytes_per_pixel = s5pcfb_fimd.bpp / 8;

	/* set registers */
	__REG(S5PC_WINCON0) = s5pcfb_fimd.wincon0;
	__REG(S5PC_VIDCON0) = s5pcfb_fimd.vidcon0;
	__REG(S5PC_VIDCON1) = s5pcfb_fimd.vidcon1;
	__REG(S5PC_VIDTCON0) = s5pcfb_fimd.vidtcon0;
	__REG(S5PC_VIDTCON1) = s5pcfb_fimd.vidtcon1;
	__REG(S5PC_VIDTCON2) = s5pcfb_fimd.vidtcon2;
	__REG(S5PC_VIDINTCON0) = s5pcfb_fimd.vidintcon0;
	__REG(S5PC_VIDINTCON1) = s5pcfb_fimd.vidintcon1;

	__REG(S5PC_VIDOSD0A) = s5pcfb_fimd.vidosd0a;
	__REG(S5PC_VIDOSD0B) = s5pcfb_fimd.vidosd0b;
	__REG(S5PC_VIDOSD0C) = s5pcfb_fimd.vidosd0c;
	__REG(S5PC_WPALCON) = s5pcfb_fimd.wpalcon;

	/* enable window0 */
	__REG(S5PC_WINCON0) = (__raw_readl(S5PC_WINCON0) | S5PC_WINCONx_ENWIN_F_ENABLE);
	__REG(S5PC_VIDCON0) = (__raw_readl(S5PC_VIDCON0) | S5PC_VIDCON0_ENVID_ENABLE |
			S5PC_VIDCON0_ENVID_F_ENABLE);
}

static void fill_fb(void)
{
	/* red */
	read_image32((char *)s5pcfb_fimd.screen, 0, 0, 480, 200, makepixel8888(0, 255, 0, 0));
	/* green */
	read_image32((char *)s5pcfb_fimd.screen, 0, 200, 480, 400, makepixel8888(0, 0, 255, 0));
	/* blue */
	read_image32((char *)s5pcfb_fimd.screen, 0, 400, 480, 600, makepixel8888(0, 0, 0, 255));
	/* write */
	read_image32((char *)s5pcfb_fimd.screen, 0, 600, 480, 800, makepixel8888(0, 255, 255, 255));
}

static void lcd_panel_init(void)
{
	tl2796_panel_init();
	tl2796_panel_power_on();
	tl2796_panel_enable();
}

void lcd_ctrl_init(void *lcdbase)
{
	s5pc_lcd_clock_enable();

	s5pc_lcd_init_mem(lcdbase, &panel_info);

	fill_fb();

	s5pc_gpio_setup();

	s5pc_lcd_init(&panel_info);
}

void lcd_setcolreg(short regno, ushort red, ushort green, ushort blud)
{
	return;
}

void lcd_enable(void)
{
	lcd_panel_init();
}

ulong calc_fbsize(void)
{
	return (s5pcfb_fimd.xres * s5pcfb_fimd.yres * s5pcfb_fimd.bytes_per_pixel);
}
