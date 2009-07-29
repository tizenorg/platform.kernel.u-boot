/*
 * S5PC100 LCD Controller Specific driver.
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 */

#include <config.h>
#include <common.h>
#include <stdarg.h>
#include <linux/types.h>
#include <asm/io.h>
#include <lcd.h>

#include <asm/arch/regs-lcd.h>
#include <asm/arch/hardware.h>
#include "s5pcfb.h"

#define MPLL 1

/* clock definitions */
#define SELECT_CLOCK_SOURCE2	0xE0100208
#define LCD_CLOCK_SOURCE	0x1000

#define CLOCK_DIV1		0xE0100304

/* LCD Panel definitions */
#define PANEL_WIDTH		480
#define PANEL_HEIGHT		800

/* LCD Panel definitions */
#define PANEL_WIDTH		480
#define PANEL_HEIGHT		800
#define S5P_LCD_BPP		32	

#define S5PCFB_HFP		8
#define S5PCFB_HSW		4
#define S5PCFB_HBP		8

#define S5PCFB_VFP		8
#define S5PCFB_VSW		4
#define S5PCFB_VBP		8

#define S5PCFB_HRES		480
#define S5PCFB_VRES		800

#define S5PCFB_HRES_VIRTUAL	480
#define S5PCFB_VRES_VIRTUAL	800

#define S5PCFB_HRES_OSD		480
#define S5PCFB_VRES_OSD		800

#define S5P_VFRAME_FREQ		60

/* LCD Controller data */
s5pcfb_fimd_info_t s5pcfb_fimd = {
	.vidcon0 = S5P_VIDCON0_INTERLACE_F_PROGRESSIVE | S5P_VIDCON0_VIDOUT_RGB_IF | \
			   S5P_VIDCON0_L1_DATA16_SUB_16PLUS8_MODE | \
			   S5P_VIDCON0_L0_DATA16_MAIN_16PLUS8_MODE | \
			   S5P_VIDCON0_PNRMODE_RGB_P | S5P_VIDCON0_CLKVALUP_ALWAYS | \
			   S5P_VIDCON0_CLKDIR_DIVIDED | S5P_VIDCON0_CLKSEL_F_HCLK | \
			   S5P_VIDCON0_ENVID_DISABLE | S5P_VIDCON0_ENVID_F_DISABLE,

	.vidcon1 = S5P_VIDCON1_IHSYNC_NORMAL | S5P_VIDCON1_IVSYNC_NORMAL | 
		S5P_VIDCON1_IVDEN_INVERT | S5P_VIDCON1_IVCLK_RISE_EDGE,

	.vidtcon0 = S5P_VIDTCON0_VBPD(S5PCFB_VBP - 1) | \
		    S5P_VIDTCON0_VFPD(S5PCFB_VFP - 1) | \
		    S5P_VIDTCON0_VSPW(S5PCFB_VSW - 1),

	.vidtcon1 = S5P_VIDTCON1_HBPD(S5PCFB_HBP - 1) | \
		    S5P_VIDTCON1_HFPD(S5PCFB_HFP - 1) | \
		    S5P_VIDTCON1_HSPW(S5PCFB_HSW - 1),

	.vidtcon2 = S5P_VIDTCON2_LINEVAL(S5PCFB_VRES - 1) | \
		    S5P_VIDTCON2_HOZVAL(S5PCFB_HRES - 1),

	.vidosd0a = S5P_VIDOSDxA_OSD_LTX_F(0) | S5P_VIDOSDxA_OSD_LTY_F(0),
	.vidosd0b = S5P_VIDOSDxB_OSD_RBX_F(S5PCFB_HRES - 1) | \
		    S5P_VIDOSDxB_OSD_RBY_F(S5PCFB_VRES - 1),

	.vidosd1a = S5P_VIDOSDxA_OSD_LTX_F(0) | S5P_VIDOSDxA_OSD_LTY_F(0),
	.vidosd1b = S5P_VIDOSDxB_OSD_RBX_F(S5PCFB_HRES_OSD - 1) | \
		    S5P_VIDOSDxB_OSD_RBY_F(S5PCFB_VRES_OSD - 1),

	.width = PANEL_WIDTH,
	.height = PANEL_HEIGHT,
	.xres = PANEL_WIDTH,
	.yres = PANEL_HEIGHT,

	.dithmode = (S5P_DITHMODE_RDITHPOS_5BIT | S5P_DITHMODE_GDITHPOS_6BIT | \
			S5P_DITHMODE_BDITHPOS_5BIT ) & S5P_DITHMODE_DITHERING_DISABLE,

	.wincon0 = S5P_WINCONx_HAWSWP_DISABLE | S5P_WINCONx_BURSTLEN_16WORD | \
		   S5P_WINCONx_BPPMODE_F_24BPP_888,

	.bpp = S5P_LCD_BPP,
	.bytes_per_pixel = 4,
	.wpalcon = S5P_WPALCON_W0PAL_24BIT,

	.vidintcon0 = S5P_VIDINTCON0_FRAMESEL0_VSYNC | S5P_VIDINTCON0_FRAMESEL1_NONE | \
		      S5P_VIDINTCON0_INTFRMEN_DISABLE | S5P_VIDINTCON0_FIFOSEL_WIN0 | \
		      S5P_VIDINTCON0_FIFOLEVEL_25 | S5P_VIDINTCON0_INTFIFOEN_DISABLE | \
		      S5P_VIDINTCON0_INTEN_ENABLE,
	.vidintcon1 = 0,
	.xoffset = 0,
	.yoffset = 0,
};

extern unsigned long get_pll_clk(int pllreg);

void s5pc_fimd_lcd_clock_enable(void)
{
	__REG(SELECT_CLOCK_SOURCE2) = LCD_CLOCK_SOURCE;
}

void s5pc_fimd_lcd_init_mem(u_long screen_base, u_long fb_size, u_long palette_size)
{
	u_long palette_mem_size;

	s5pcfb_fimd.screen = screen_base;

	s5pcfb_fimd.palette_size = palette_size;
	palette_mem_size = palette_size * sizeof(u32);

	s5pcfb_fimd.palette = screen_base + fb_size + PAGE_SIZE - palette_mem_size;

	udebug("palette=%x\n", (unsigned int)s5pcfb_fimd.palette);
}

void s5pc_fimd_gpio_setup(void)
{
	/* set GPF0[0:7] for RGB Interface and Data lines */
	writel(0x22222222, 0xE03000E0);

	/* set Data lines */
	writel(0x22222222, 0xE0300100);
	writel(0x22222222, 0xE0300120);
	writel(0x2222, 0xE0300140);

	/* set gpio configuration pin for MLCD_RST */
	writel(0x10000000, 0xE0300C20);

	/* set gpio configuration pin for MLCD_ON */
	writel(0x1000, 0xE0300220);
	writel(readl(0xE0300224) & 0xf7, 0xE0300224);

	/* set gpio configuration pin for DISPLAY_CS, DISPLAY_CLK and DISPLSY_SI */
	writel(0x11100000, 0xE0300300);
}

void s5pc_fimd_lcd_init(vidinfo_t *vid)
{
	unsigned int mpll_ratio, offset, fb_size, page_width;

	s5pcfb_fimd.bytes_per_pixel = vid->vl_bpix / 8;
	page_width = s5pcfb_fimd.xres * s5pcfb_fimd.bytes_per_pixel;
	offset = 0;

	/* calculate LCD Pixel clock */
	s5pcfb_fimd.pixclock = (S5P_VFRAME_FREQ *
			(vid->vl_hpw + vid->vl_blw + vid->vl_elw + vid->vl_width) *
			(vid->vl_vpw + vid->vl_bfw + vid->vl_efw + vid->vl_height));

	/* initialize the fimd specific */
	s5pcfb_fimd.vidintcon0 &= ~S5P_VIDINTCON0_FRAMESEL0_MASK;
	s5pcfb_fimd.vidintcon0 |= S5P_VIDINTCON0_FRAMESEL0_VSYNC;
	s5pcfb_fimd.vidintcon0 |= S5P_VIDINTCON0_INTFRMEN_ENABLE;

	writel(s5pcfb_fimd.vidintcon0, S5P_VIDINTCON0);

	/* set configuration register for VCLK */
	s5pcfb_fimd.vidcon0 = s5pcfb_fimd.vidcon0 &
			~(S5P_VIDCON0_ENVID_ENABLE | S5P_VIDCON0_ENVID_F_ENABLE);
	writel(s5pcfb_fimd.vidcon0, S5P_VIDCON0);

	mpll_ratio = (readl(CLOCK_DIV1) & 0x000000f0) >> 4;
	s5pcfb_fimd.vidcon0 |= S5P_VIDCON0_CLKVAL_F((int)(((get_pll_clk(MPLL) / mpll_ratio)
					/ s5pcfb_fimd.pixclock) - 1));

	udebug("mpll_ratio = %d, MCLK = %d, pixclock=%d, vidcon0 = %d\n",
			mpll_ratio, (unsigned int)get_pll_clk(MPLL), (unsigned int)s5pcfb_fimd.pixclock,
			(unsigned int)s5pcfb_fimd.vidcon0);

	/* set window size */
	s5pcfb_fimd.vidosd0c = S5P_VIDOSD0C_OSDSIZE(PANEL_WIDTH * PANEL_HEIGHT);

	/* set wondow position */
	writel(S5P_VIDOSDxA_OSD_LTX_F(0) | S5P_VIDOSDxA_OSD_LTY_F(0), S5P_VIDOSD0A);
	writel(S5P_VIDOSDxB_OSD_RBX_F(PANEL_WIDTH - 1 + s5pcfb_fimd.xoffset) |
		S5P_VIDOSDxB_OSD_RBY_F(PANEL_HEIGHT - 1 + s5pcfb_fimd.yoffset), S5P_VIDOSD0B);

	/* set framebuffer start address */
	writel(s5pcfb_fimd.screen, S5P_VIDW00ADD0B0);

	/* set framebuffer end address  */
	writel((readl(S5P_VIDW00ADD0B0) + (page_width + offset) * s5pcfb_fimd.yres),
		S5P_VIDW00ADD1B0);
	
	/* set framebuffer size */
	fb_size = S5P_VIDWxxADD2_OFFSIZE_F(offset) |
			(S5P_VIDWxxADD2_PAGEWIDTH_F(page_width));

	writel(fb_size, S5P_VIDW00ADD2);

	udebug("fb_size at s5pc_lcd_init=%d, page_width=%d\n", fb_size, page_width);

	/* set window0 conguration register */
	s5pcfb_fimd.wincon0 = S5P_WINCONx_WSWP_ENABLE |
			S5P_WINCONx_BURSTLEN_16WORD |
			S5P_WINCONx_BPPMODE_F_24BPP_888;

	s5pcfb_fimd.bpp = S5P_LCD_BPP;
	s5pcfb_fimd.bytes_per_pixel = s5pcfb_fimd.bpp / 8;

	/* set registers */
	writel(s5pcfb_fimd.wincon0, S5P_WINCON0);
	writel(s5pcfb_fimd.vidcon0, S5P_VIDCON0);
	writel(s5pcfb_fimd.vidcon1, S5P_VIDCON1);
	writel(s5pcfb_fimd.vidtcon0, S5P_VIDTCON0);
	writel(s5pcfb_fimd.vidtcon1, S5P_VIDTCON1);
	writel(s5pcfb_fimd.vidtcon2, S5P_VIDTCON2);
	writel(s5pcfb_fimd.vidintcon0, S5P_VIDINTCON0);
	writel(s5pcfb_fimd.vidintcon1, S5P_VIDINTCON1);

	writel(s5pcfb_fimd.vidosd0a, S5P_VIDOSD0A);
	writel(s5pcfb_fimd.vidosd0b, S5P_VIDOSD0B);
	writel(s5pcfb_fimd.vidosd0c, S5P_VIDOSD0C);
	writel(s5pcfb_fimd.wpalcon, S5P_WPALCON);

	/* enable window0 */
	writel((readl(S5P_WINCON0) | S5P_WINCONx_ENWIN_F_ENABLE), S5P_WINCON0);
	writel((readl(S5P_VIDCON0) | S5P_VIDCON0_ENVID_ENABLE | S5P_VIDCON0_ENVID_F_ENABLE),
		S5P_VIDCON0);
} 

ulong s5pc_fimd_calc_fbsize(void)
{
	return (s5pcfb_fimd.xres * s5pcfb_fimd.yres * s5pcfb_fimd.bytes_per_pixel);
}
