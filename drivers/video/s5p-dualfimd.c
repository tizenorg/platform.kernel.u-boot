/*
 * S5PC100 and S5PC110 LCD Controller Specific driver.
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
#include <stdarg.h>
#include <linux/types.h>
#include <asm/io.h>
#include <lcd.h>

#include <asm/arch/cpu.h>
#include <asm/arch/regs-fb.h>
#include <asm/arch/hardware.h>
#include "s5p-fb.h"

#define MPLL 1

#define S5P_VFRAME_FREQ		60

static unsigned int ctrl_base;
static unsigned long *lcd_base_addr;
static vidinfo_t *pvid = NULL;

extern int dual_lcd;
extern unsigned long get_pll_clk(int pllreg);

void s5pc_fimd_lcd_init_mem(u_long screen_base, u_long fb_size, u_long palette_size)
{
	lcd_base_addr = (unsigned long *)screen_base;

	udebug("lcd_base_addr(framebuffer memory) = %x\n", lcd_base_addr);

	return;
}

void s5pc_c110_gpio_setup(void)
{
	/* set GPF0[0:7] for RGB Interface and Data lines (32bit) */
	writel(0x22222222, 0xE0200120);
	/* pull-up/down disable */
	writel(0x0, 0xE0200128);
	/* drive strength to max (24bit) */
	writel(0xffffff, 0xE020012C);

	/* set Data lines (32bit) */
	writel(0x22222222, 0xE0200140);
	writel(0x22222222, 0xE0200160);
	writel(readl(0xE0200180) & 0xFF0000, 0xE0200180);
	writel(readl(0xE0200180) | 0x002222, 0xE0200180);

	if (dual_lcd) {
		/* SUB_DISPLAY_PCLK (GPF3[4]) */
		writel(readl(0xE0200180) & 0xfff0ffff, 0xE0200180);
		writel(readl(0xE0200180) | 0x00020000, 0xE0200180);
		/* drive stength to max */
		writel(readl(0xE020018C) & 0xFCFF, 0xE020018C);
		writel(readl(0xE020018C) | 0x0300, 0xE020018C);
		/* set gpio configuration pin for SUBLCD_RST(MP0_2[1]) and SUBLCD_ON(MP0_2[0] */
		writel(readl(0xE0200300) & 0xffffff00, 0xE0200300); 
		writel(readl(0xE0200300) | 0x00000011, 0xE0200300); 
		/* set gpio confituration pin for SUB_DISPLAY_CS(MP0_1[2]) */
		writel(readl(0xE02002E0) & 0xfffff0ff, 0xE02002E0);
		writel(readl(0xE02002E0) | 0x00000100, 0xE02002E0);
		
		/* 
		 * DUALRGB INTERFACE SETTING REGISTER
		 * Line Split  mode, using VCLK, Display bypass Dual mode
		 */
		writel(0x0C801e01, 0xF800027C);
	}

	/* drive strength to max (24bit) */
	writel(0xffffff, 0xE020014C);
	writel(0xffffff, 0xE020016C);
	/* [11:0](drive stength level), [15:12](none), [21:16](Slew Rate) */
	writel(readl(0xE020018C) & 0x3FFF00, 0xE020018C);
	writel(readl(0xE020018C) | 0x0000FF, 0xE020018C);

	/* pull-up/down disable */
	writel(0x0, 0xE0200148);
	writel(0x0, 0xE0200168);
	writel(0x0, 0xE0200188);

	/* display output path selection (only [1:0] valid) */
	writel(0x2, 0xE0107008);

	/* set gpio configuration pin for MLCD_RST */
	writel(readl(0xE0200C20) & 0x0fffffff, 0xE0200C20);
	writel(readl(0xE0200C20) | 0x10000000, 0xE0200C20);

	/* set gpio configuration pin for MLCD_ON and then to LOW */
	writel(readl(0xE0200260) & 0xFFFF0FFF, 0xE0200260);
	writel(readl(0xE0200260) | 0x00001000, 0xE0200260);
	writel(readl(0xE0200264) & 0xf7, 0xE0200264);

	/* set gpio configuration pin for DISPLAY_CS, DISPLAY_CLK, DISPLSY_SI and LCD_ID */
	writel(readl(0xE02002E0) & 0xFFFFFF0F, 0xE02002E0);
	writel(readl(0xE02002E0) | 0x00000010, 0xE02002E0);
	writel(readl(0xE0200340) & 0xFFFF000F, 0xE0200340);
	writel(readl(0xE0200340) | 0x00001110, 0xE0200340);

	return;
}

static void s5pc_fimd_set_par(unsigned int win_id)
{
	unsigned int cfg = 0;

	/* set window control */
	cfg = readl(ctrl_base + S5P_WINCON(win_id));

	cfg &= ~(S5P_WINCON_BITSWP_ENABLE | S5P_WINCON_BYTESWP_ENABLE | \
		S5P_WINCON_HAWSWP_ENABLE | S5P_WINCON_WSWP_ENABLE | \
		S5P_WINCON_BURSTLEN_MASK | S5P_WINCON_BPPMODE_MASK | \
		S5P_WINCON_INRGB_MASK | S5P_WINCON_DATAPATH_MASK);

	/* DATAPATH is DMA */
	cfg |= S5P_WINCON_DATAPATH_DMA;

	/* bpp is 32 */
	cfg |= S5P_WINCON_WSWP_ENABLE;

	/* dma burst is 16 */
	cfg |= S5P_WINCON_BURSTLEN_16WORD;

	/* pixel format is unpacked RGB888 */
	cfg |= S5P_WINCON_BPPMODE_24BPP_888;

	writel(cfg, ctrl_base + S5P_WINCON(win_id));
	udebug("wincon%d = %x\n", win_id, cfg);

	/* set window position to x=0, y=0*/
	cfg = S5P_VIDOSD_LEFT_X(0) | S5P_VIDOSD_TOP_Y(0);
	writel(cfg, ctrl_base + S5P_VIDOSD_A(win_id));
	udebug("window postion left,top = %x\n", cfg);

	cfg = S5P_VIDOSD_RIGHT_X(pvid->vl_col - 1) |
		S5P_VIDOSD_BOTTOM_Y(pvid->vl_row - 1);
	writel(cfg, ctrl_base + S5P_VIDOSD_B(win_id));
	udebug("window postion right,bottom= %x\n", cfg);

	/* set window size for window0*/
	cfg = S5P_VIDOSD_SIZE(pvid->vl_col * pvid->vl_row);
	writel(cfg, ctrl_base + S5P_VIDOSD_C(win_id));
	udebug("vidosd_c%d= %x\n", win_id, cfg);

	return;
}

static void s5pc_fimd_set_buffer_address(unsigned int win_id)
{
	unsigned long start_addr, end_addr;

	start_addr = (unsigned long)lcd_base_addr;
	end_addr = start_addr + ((pvid->vl_col * (pvid->vl_bpix / 8))
		* pvid->vl_row);

	writel(start_addr, ctrl_base + S5P_VIDADDR_START0(win_id));
	writel(end_addr, ctrl_base + S5P_VIDADDR_END0(win_id));

	udebug("start addr = %x, end addr = %x\n", start_addr, end_addr);

	return;
}

static void s5pc_fimd_set_clock(void)
{
	unsigned int cfg = 0, div = 0, mpll_ratio = 0;
	unsigned long pixel_clock, src_clock, max_clock;

	max_clock = 66 * 1000000;

	pixel_clock = S5P_VFRAME_FREQ * (pvid->vl_hpw + pvid->vl_blw +
		pvid->vl_elw + pvid->vl_width) * (pvid->vl_vpw +
		    pvid->vl_bfw + pvid->vl_efw + pvid->vl_height);

	src_clock = get_pll_clk(MPLL);

	cfg = readl(ctrl_base + S5P_VIDCON0);
	cfg &= ~(S5P_VIDCON0_CLKSEL_MASK | S5P_VIDCON0_CLKVALUP_MASK | \
		S5P_VIDCON0_VCLKEN_MASK | S5P_VIDCON0_CLKDIR_MASK);
	cfg |= (S5P_VIDCON0_CLKSEL_HCLK | S5P_VIDCON0_CLKVALUP_ALWAYS | \
		S5P_VIDCON0_VCLKEN_NORMAL | S5P_VIDCON0_CLKDIR_DIVIDED);

	if (pixel_clock > max_clock)
		pixel_clock = max_clock;

	/* get mpll ratio */
	mpll_ratio = (readl(0xE0100300) & 0xf0000) >> 16;

	/* 
	 * It can get source clock speed as (mpll / mpll_ratio) 
	 * because lcd controller uses hclk_dsys.
	 * mpll is a parent of hclk_dsys.
	 */
	div = (unsigned int)((src_clock / (mpll_ratio + 1)) / pixel_clock);
	cfg |= S5P_VIDCON0_CLKVAL_F(div - 1);
	writel(cfg, ctrl_base + S5P_VIDCON0);

	udebug("mpll_ratio = %d, src_clock = %d, pixel_clock = %d, div = %d\n",
		mpll_ratio, src_clock, pixel_clock, div);

	return;
}

static s5pc_fimd_lcd_on(unsigned int win_id)
{
	int cfg = 0;

	/* display on */
	cfg = readl(ctrl_base + S5P_VIDCON0);
	cfg |= (S5P_VIDCON0_ENVID_ENABLE | S5P_VIDCON0_ENVID_F_ENABLE);
	writel(cfg, ctrl_base + S5P_VIDCON0);
	udebug("vidcon0 = %x\n", cfg);

	/* enable window */
	cfg = readl(ctrl_base + S5P_WINCON(win_id));
	cfg |= S5P_WINCON_ENWIN_ENABLE;
	writel(cfg, ctrl_base + S5P_WINCON(win_id));
	udebug("wincon%d=%x\n", win_id, cfg);
}

void s5pc_fimd_lcd_init(vidinfo_t *vid)
{
	unsigned int cfg = 0, rgb_mode, win_id = 0;

	/* store panel info to global variable */
	pvid = vid;

	/* select register base according to cpu type */
	ctrl_base = 0xF8000000;

	/* set output to BGR for 3.1inch TL2796 LCD Panel */
	rgb_mode = MODE_BGR_P;

	/* 3.5inch TL2796 LCD Panel */
	//rgb_mode = MODE_RGB_P;

	cfg = readl(ctrl_base + S5P_VIDCON0);	
	cfg &= ~S5P_VIDCON0_VIDOUT_MASK;

	/* clock source is HCLK */
	cfg |= 0 << 2;

	cfg |= S5P_VIDCON0_VIDOUT_RGB;
	writel(cfg, ctrl_base + S5P_VIDCON0);

	/* set display mode */
	cfg = readl(ctrl_base + S5P_VIDCON0);
	cfg &= ~S5P_VIDCON0_PNRMODE_MASK;
	cfg |= (rgb_mode << S5P_VIDCON0_PNRMODE_SHIFT);
	writel(cfg, ctrl_base + S5P_VIDCON0);

	/* set polarity */
	cfg = 0;
	cfg |= S5P_VIDCON1_IVDEN_INVERT | S5P_VIDCON1_IVCLK_RISING_EDGE;
	writel(cfg, ctrl_base + S5P_VIDCON1);

	/* set timing */
	cfg = 0;
	cfg |= S5P_VIDTCON0_VBPD(pvid->vl_bfw - 1);
	cfg |= S5P_VIDTCON0_VFPD(pvid->vl_efw - 1);
	cfg |= S5P_VIDTCON0_VSPW(pvid->vl_vpw - 1);
	writel(cfg, ctrl_base + S5P_VIDTCON0);
	udebug("vidtcon0 = %x\n", cfg);

	cfg = 0;
	cfg |= S5P_VIDTCON1_HBPD(pvid->vl_blw - 1);
	cfg |= S5P_VIDTCON1_HFPD(pvid->vl_elw - 1);
	cfg |= S5P_VIDTCON1_HSPW(pvid->vl_hpw - 1);

	writel(cfg, ctrl_base + S5P_VIDTCON1);
	udebug("vidtcon1 = %x\n", cfg);

	/* set lcd size */
	cfg = 0;
	cfg |= S5P_VIDTCON2_HOZVAL(pvid->vl_col - 1);
	cfg |= S5P_VIDTCON2_LINEVAL(pvid->vl_row - 1);
	
	writel(cfg, ctrl_base + S5P_VIDTCON2);
	udebug("vidtcon2 = %x\n", cfg);

	/* set par */
	s5pc_fimd_set_par(win_id);

	/* set memory address */
	s5pc_fimd_set_buffer_address(win_id);

	/* set buffer size */
	cfg = S5P_VIDADDR_PAGEWIDTH(pvid->vl_col * pvid->vl_bpix / 8);
	writel(cfg, ctrl_base + S5P_VIDADDR_SIZE(win_id));
	udebug("vidaddr_pagewidth = %d\n", cfg);

	/* set clock */
	s5pc_fimd_set_clock();

	/* display on */
	s5pc_fimd_lcd_on(win_id);

	udebug("lcd controller init completed.\n");

	return;
}

ulong s5pc_fimd_calc_fbsize(void)
{
	return (pvid->vl_col * pvid->vl_row * (pvid->vl_bpix / 8));
}
