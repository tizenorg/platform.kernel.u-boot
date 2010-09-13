/* /drivers/video/s5p-dsim.c
 *
 * Samsung MIPI-DSIM driver.
 *
 * InKi Dae, <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <ubi_uboot.h>
#include <common.h>
#include <dsim.h>
#include <lcd.h>
#include <mipi_ddi.h>
#include <asm/errno.h>
#include <asm/arch/regs-dsim.h>
#include <asm/arch/power.h>
#include <linux/types.h>
#include <linux/list.h>
#include "s5p_dsim_lowlevel.h"


struct mipi_lcd_info {
	struct list_head	list;
	struct mipi_lcd_driver	*mipi_drv;
};

static LIST_HEAD(lcd_info_list);

extern int s3cfb_set_trigger(void);
extern int s3cfb_is_i80_frame_done(void);

struct dsim_global dsim;
static struct s5p_platform_dsim *dsim_pd;

static void s5p_dsim_long_data_wr(struct dsim_global *dsim, unsigned char data0,
	unsigned int data1)
{
	unsigned int data_cnt = 0, payload = 0;

	/* in case that data count is more then 4 */
	for (data_cnt = 0; data_cnt < data1; data_cnt += 4) {
		/*
		 * after sending 4bytes per one time,
		 * send remainder data less then 4.
		 */
		if ((data1 - data_cnt) < 4) {
			if ((data1 - data_cnt) == 3) {
				payload = *(unsigned char *)(data0 + data_cnt) |
				*(unsigned char *)(data0 + (data_cnt + 1)) << 8 |
				*(unsigned char *)(data0 + (data_cnt + 2)) << 16;
			} else if ((data1 - data_cnt) == 2) {
				payload = *(unsigned char *)(data0 + data_cnt) |
				*(unsigned char *)(data0 + (data_cnt + 1)) << 8;
			} else if ((data1 - data_cnt) == 1) {
				payload = *(unsigned char *)(data0 + data_cnt);
			}

			s5p_dsim_wr_tx_data(dsim->reg_base, payload);
		/* send 4bytes per one time. */
		} else {
			payload = *(unsigned char *)(data0 + data_cnt) |
			*(unsigned char *)(data0 + (data_cnt + 1)) << 8 |
			*(unsigned char *)(data0 + (data_cnt + 2)) << 16 |
			*(unsigned char *)(data0 + (data_cnt + 3)) << 24;

			s5p_dsim_wr_tx_data(dsim->reg_base, payload);
		}
	}
}

static int s5p_dsim_wr_data(void *dsim_data, unsigned int data_id,
	unsigned char data0, unsigned int data1)
{
	struct dsim_global *dsim = NULL;
	unsigned int timeout = 5000 * 2;
	unsigned long delay_val, delay;
	unsigned char check_rx_ack = 0;

	dsim = (struct dsim_global *)dsim_data;

	if (dsim == NULL) {
		udebug("dsim_data is NULL.\n");
		return -EFAULT;
	}

	if (dsim->state == DSIM_STATE_ULPS) {
		udebug("state is ULPS.\n");
		return -EINVAL;
	}

	delay_val = 1000000 / dsim->dsim_info->esc_clk;
	delay = 10 * delay_val;

	udelay(delay * 1000);

	/* only if transfer mode is LPDT, wait SFR becomes empty. */
	if (dsim->state == DSIM_STATE_STOP) {
		while (!(s5p_dsim_get_fifo_state(dsim->reg_base) &
				SFR_HEADER_EMPTY)) {
			if ((timeout--) > 0)
				udelay(1000);
			else {
				udebug("SRF header fifo is not empty.\n");
				return -EINVAL;
			}
		}
	}

	switch (data_id) {
	/* short packet types of packet types for command. */
	case GEN_SHORT_WR_NO_PARA:
	case GEN_SHORT_WR_1_PARA:
	case GEN_SHORT_WR_2_PARA:
	case DCS_WR_NO_PARA:
	case DCS_WR_1_PARA:
	case SET_MAX_RTN_PKT_SIZE:
		s5p_dsim_wr_tx_header(dsim->reg_base, (unsigned char) data_id,
			(unsigned char) data0, (unsigned char) data1);
		if (check_rx_ack)
			/* process response func should be implemented */
			return 0;
		else
			return -EINVAL;

	/* general command */
	case CMD_OFF:
	case CMD_ON:
	case SHUT_DOWN:
	case TURN_ON:
		s5p_dsim_wr_tx_header(dsim->reg_base, (unsigned char) data_id,
			(unsigned char) data0, (unsigned char) data1);
		if (check_rx_ack)
			/* process response func should be implemented. */
			return 0;
		else
			return -EINVAL;

	/* packet types for video data */
	case VSYNC_START:
	case VSYNC_END:
	case HSYNC_START:
	case HSYNC_END:
	case EOT_PKT:
		return 0;

	/* short and response packet types for command */
	case GEN_RD_1_PARA:
	case GEN_RD_2_PARA:
	case GEN_RD_NO_PARA:
	case DCS_RD_NO_PARA:
		s5p_dsim_clear_interrupt(dsim->reg_base, 0xffffffff);
		s5p_dsim_wr_tx_header(dsim->reg_base, (unsigned char) data_id,
			(unsigned char) data0, (unsigned char) data1);
		/* process response func should be implemented. */
		return 0;

	/* long packet type and null packet */
	case NULL_PKT:
	case BLANKING_PKT:
		return 0;
	case GEN_LONG_WR:
	case DCS_LONG_WR:
	{
		unsigned int size, data_cnt = 0, payload = 0;

		size = data1 * 4;

		/* if data count is less then 4, then send 3bytes data.  */
		if (data1 < 4) {
			payload = *(unsigned char *)(data0) |
			    *(unsigned char *)(data0 + 1) << 8 |
			    *(unsigned char *)(data0 + 2) << 16;

			s5p_dsim_wr_tx_data(dsim->reg_base, payload);

			udebug("count = %d payload = %x, %x %x %x\n",
				data1, payload,
				*(unsigned char *)(data0 + data_cnt),
				*(unsigned char *)(data0 + (data_cnt + 1)),
				*(unsigned char *)(data0 + (data_cnt + 2)));
		/* in case that data count is more then 4 */
		} else {
			s5p_dsim_long_data_wr(dsim, data0, data1);
		}
		/* put data into header fifo */
		s5p_dsim_wr_tx_header(dsim->reg_base, (unsigned char) data_id,
			(unsigned char) (((unsigned short) data1) & 0xff),
			(unsigned char) ((((unsigned short) data1) & 0xff00) >>
				8));

	}
	if (check_rx_ack)
		/* process response func should be implemented. */
		return 0;
	else
		return -EINVAL;

	/* packet typo for video data */
	case RGB565_PACKED:
	case RGB666_PACKED:
	case RGB666_LOOSLY:
	case RGB888_PACKED:
		if (check_rx_ack)
			/* process response func should be implemented. */
			return 0;
		else
			return -EINVAL;
	default:
		udebug("data id %x is not supported current DSI spec.\n",
			data_id);

		return -EINVAL;
	}

	return 0;
}

int s5p_dsim_init_header_fifo(struct dsim_global *dsim)
{
	unsigned int cnt;

	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return -EFAULT;
	}

	for (cnt = 0; cnt < DSIM_HEADER_FIFO_SZ; cnt++)
		dsim->header_fifo_index[cnt] = -1;
	return 0;
}

int s5p_dsim_pll_on(struct dsim_global *dsim, unsigned char enable)
{
	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return -EFAULT;
	}

	if (enable) {
		int sw_timeout = 1000;
		s5p_dsim_clear_interrupt(dsim->reg_base, DSIM_PLL_STABLE);
		s5p_dsim_enable_pll(dsim->reg_base, 1);
		while (1) {
			sw_timeout--;
			if (s5p_dsim_is_pll_stable(dsim->reg_base))
				return 0;
			if (sw_timeout == 0)
				return -EINVAL;
		}
	} else
		s5p_dsim_enable_pll(dsim->reg_base, 0);

	return 0;
}

unsigned long s5p_dsim_change_pll(struct dsim_global *dsim,
	unsigned char pre_divider, unsigned short main_divider,
	unsigned char scaler)
{
	unsigned long dfin_pll, dfvco, dpll_out;
	unsigned char freq_band;

	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return 0;
	}

	dfin_pll = (MIPI_FIN / pre_divider);

	if (dfin_pll < 6 * 1000 * 1000 || dfin_pll > 12 * 1000 * 1000) {
		udebug("warning!!\n");
		udebug("fin_pll range is 6MHz ~ 12MHz\n");
		udebug("fin_pll of mipi dphy pll is %luMHz\n",
			(dfin_pll / 1000000));

		s5p_dsim_enable_afc(dsim->reg_base, 0, 0);
	} else {
		if (dfin_pll < 7 * 1000000)
			s5p_dsim_enable_afc(dsim->reg_base, 1, 0x1);
		else if (dfin_pll < 8 * 1000000)
			s5p_dsim_enable_afc(dsim->reg_base, 1, 0x0);
		else if (dfin_pll < 9 * 1000000)
			s5p_dsim_enable_afc(dsim->reg_base, 1, 0x3);
		else if (dfin_pll < 10 * 1000000)
			s5p_dsim_enable_afc(dsim->reg_base, 1, 0x2);
		else if (dfin_pll < 11 * 1000000)
			s5p_dsim_enable_afc(dsim->reg_base, 1, 0x5);
		else
			s5p_dsim_enable_afc(dsim->reg_base, 1, 0x4);
	}

	dfvco = dfin_pll * main_divider;
	udebug("dfvco = %lu, dfin_pll = %lu, main_divider = %d\n",
		dfvco, dfin_pll, main_divider);
	if (dfvco < 500000000 || dfvco > 1000000000) {
		udebug("Caution!!\n");
		udebug("fvco range is 500MHz ~ 1000MHz\n");
		udebug("fvco of mipi dphy pll is %luMHz\n",
			(dfvco / 1000000));
	}

	dpll_out = dfvco / (1 << scaler);
	udebug("dpll_out = %lu, dfvco = %lu, scaler = %d\n",
		dpll_out, dfvco, scaler);
	if (dpll_out < 100 * 1000000)
		freq_band = 0x0;
	else if (dpll_out < 120 * 1000000)
		freq_band = 0x1;
	else if (dpll_out < 170 * 1000000)
		freq_band = 0x2;
	else if (dpll_out < 220 * 1000000)
		freq_band = 0x3;
	else if (dpll_out < 270 * 1000000)
		freq_band = 0x4;
	else if (dpll_out < 320 * 1000000)
		freq_band = 0x5;
	else if (dpll_out < 390 * 1000000)
		freq_band = 0x6;
	else if (dpll_out < 450 * 1000000)
		freq_band = 0x7;
	else if (dpll_out < 510 * 1000000)
		freq_band = 0x8;
	else if (dpll_out < 560 * 1000000)
		freq_band = 0x9;
	else if (dpll_out < 640 * 1000000)
		freq_band = 0xa;
	else if (dpll_out < 690 * 1000000)
		freq_band = 0xb;
	else if (dpll_out < 770 * 1000000)
		freq_band = 0xc;
	else if (dpll_out < 870 * 1000000)
		freq_band = 0xd;
	else if (dpll_out < 950 * 1000000)
		freq_band = 0xe;
	else
		freq_band = 0xf;

	udebug("freq_band = %d\n", freq_band);

	s5p_dsim_pll_freq(dsim->reg_base, pre_divider, main_divider, scaler);


	    unsigned char temp0, temp1;

	    temp0 = 0;
	    s5p_dsim_hs_zero_ctrl(dsim->reg_base, temp0);
	    temp1 = 0;
	    s5p_dsim_prep_ctrl(dsim->reg_base, temp1);


	/* Freq Band */
	s5p_dsim_pll_freq_band(dsim->reg_base, freq_band);

	/* Stable time */
	s5p_dsim_pll_stable_time(dsim->reg_base,
		dsim->dsim_info->pll_stable_time);

	/* Enable PLL */
	udebug("FOUT of mipi dphy pll is %luMHz\n",
		(dpll_out / 1000000));

	return dpll_out;
}

int s5p_dsim_set_clock(struct dsim_global *dsim,
	unsigned char byte_clk_sel, unsigned char enable)
{
	unsigned int esc_div;
	unsigned long esc_clk_error_rate;

	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return -EINVAL;
	}

	if (enable) {
		dsim->e_clk_src = byte_clk_sel;

		/* Escape mode clock and byte clock source */
		s5p_dsim_set_byte_clock_src(dsim->reg_base, byte_clk_sel);

		/* DPHY, DSIM Link : D-PHY clock out */
		if (byte_clk_sel == DSIM_PLL_OUT_DIV8) {
			dsim->hs_clk = s5p_dsim_change_pll(dsim,
				dsim->dsim_info->p, dsim->dsim_info->m,
				dsim->dsim_info->s);
			if (dsim->hs_clk == 0) {
				udebug("failed to get hs clock.\n");
				return -EINVAL;
			}

			dsim->byte_clk = dsim->hs_clk / 8;
			s5p_dsim_enable_pll_bypass(dsim->reg_base, 0);
			s5p_dsim_pll_on(dsim, 1);
		/* DPHY : D-PHY clock out, DSIM link : external clock out */
		} else if (byte_clk_sel == DSIM_EXT_CLK_DIV8)
			udebug("this project is not support \
				external clock source for MIPI DSIM\n");
		else if (byte_clk_sel == DSIM_EXT_CLK_BYPASS)
			udebug("this project is not support \
				external clock source for MIPI DSIM\n");

		/* escape clock divider */
		esc_div = dsim->byte_clk / (dsim->dsim_info->esc_clk);
		udebug("esc_div = %d, byte_clk = %lu, esc_clk = %lu\n",
			esc_div, dsim->byte_clk, dsim->dsim_info->esc_clk);
		if ((dsim->byte_clk / esc_div) >= 20000000 ||
			(dsim->byte_clk / esc_div) > dsim->dsim_info->esc_clk)
			esc_div += 1;

		dsim->escape_clk = dsim->byte_clk / esc_div;
		udebug("escape_clk = %lu, byte_clk = %lu, esc_div = %d\n",
			dsim->escape_clk, dsim->byte_clk, esc_div);

		/*
		 * enable escclk on lane
		 *
		 * in case of evt0, DSIM_TRUE is enable and
		 * DSIM_FALSE is enable for evt1.
		 */
		if (dsim->dsim_pd->platform_rev == 1)
			s5p_dsim_enable_byte_clock(dsim->reg_base, DSIM_FALSE);
		else
			s5p_dsim_enable_byte_clock(dsim->reg_base, DSIM_TRUE);

		/* enable byte clk and escape clock */
		s5p_dsim_set_esc_clk_prs(dsim->reg_base, 1, esc_div);
		/* escape clock on lane */
		s5p_dsim_enable_esc_clk_on_lane(dsim->reg_base,
			(DSIM_LANE_CLOCK | dsim->data_lane), 1);

		udebug("byte clock is %luMHz\n",
			(dsim->byte_clk / 1000000));
		udebug("escape clock that user's need is %lu\n",
			(dsim->dsim_info->esc_clk / 1000000));
		udebug("escape clock divider is %x\n", esc_div);
		udebug("escape clock is %luMHz\n",
			((dsim->byte_clk / esc_div) / 1000000));

		if ((dsim->byte_clk / esc_div) > dsim->escape_clk) {
			esc_clk_error_rate = dsim->escape_clk /
				(dsim->byte_clk / esc_div);
			udebug("error rate is %lu over.\n",
				(esc_clk_error_rate / 100));
		} else if ((dsim->byte_clk / esc_div) < (dsim->escape_clk)) {
			esc_clk_error_rate = (dsim->byte_clk / esc_div) /
				dsim->escape_clk;
			udebug("error rate is %lu under.\n",
				(esc_clk_error_rate / 100));
		}
	} else {
		s5p_dsim_enable_esc_clk_on_lane(dsim->reg_base,
			(DSIM_LANE_CLOCK | dsim->data_lane), 0);
		s5p_dsim_set_esc_clk_prs(dsim->reg_base, 0, 0);

		/*
		 * in case of evt0, DSIM_FALSE is disable and
		 * DSIM_TRUE is disable for evt1.
		 */
		if (dsim->dsim_pd->platform_rev == 1)
			s5p_dsim_enable_byte_clock(dsim->reg_base, DSIM_TRUE);
		else
			s5p_dsim_enable_byte_clock(dsim->reg_base, DSIM_FALSE);

		if (byte_clk_sel == DSIM_PLL_OUT_DIV8)
			s5p_dsim_pll_on(dsim, 0);
	}

	return 0;
}

static int s5p_dsim_init_dsim(struct dsim_global *dsim)
{
	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return -EFAULT;
	}

	if (dsim->dsim_pd->init_d_phy)
		dsim->dsim_pd->init_d_phy(dsim);

	dsim->state = DSIM_STATE_RESET;

	switch (dsim->dsim_info->e_no_data_lane) {
	case DSIM_DATA_LANE_1:
		dsim->data_lane = DSIM_LANE_DATA0;
		break;
	case DSIM_DATA_LANE_2:
		dsim->data_lane = DSIM_LANE_DATA0 | DSIM_LANE_DATA1;
		break;
	case DSIM_DATA_LANE_3:
		dsim->data_lane = DSIM_LANE_DATA0 | DSIM_LANE_DATA1 |
			DSIM_LANE_DATA2;
		break;
	case DSIM_DATA_LANE_4:
		dsim->data_lane = DSIM_LANE_DATA0 | DSIM_LANE_DATA1 |
			DSIM_LANE_DATA2 | DSIM_LANE_DATA3;
		break;
	default:
		udebug("data lane is invalid.\n");
		return -EINVAL;
	};

	s5p_dsim_init_header_fifo(dsim);
	s5p_dsim_sw_reset(dsim->reg_base);
	s5p_dsim_dp_dn_swap(dsim->reg_base, dsim->dsim_info->e_lane_swap);

	return 0;
}

int s5p_dsim_enable_frame_done_int(struct dsim_global *dsim, int enable)
{
	/* enable only frame done interrupt */
	s5p_dsim_set_interrupt_mask(dsim->reg_base,
					DSIM_INTMSK_FRAME_DONE, enable);

	return 0;
}

int s5p_dsim_set_display_mode(struct dsim_global *dsim)
{
	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return -EFAULT;
	}

	if (dsim->dsim_pd->pvid != NULL) {
		s5p_dsim_set_main_disp_resol(dsim->reg_base,
				dsim->dsim_pd->pvid->vl_height,
				dsim->dsim_pd->pvid->vl_width);
	} else {
		udebug("main lcd is NULL.\n");
		return -EFAULT;
	}
#if 0
	/* in case of VIDEO MODE (RGB INTERFACE) */
	if (dsim->dsim_lcd_info->e_interface == (u32) DSIM_VIDEO) {

		main_timing = &main_lcd_panel_info->timing;
		if (main_timing == NULL) {
			udebug("main_timing is NULL.\n");
			return -EFAULT;
		}

		if (dsim->dsim_info->auto_vertical_cnt == DSIM_FALSE) {
			s5p_dsim_set_main_disp_vporch(dsim->reg_base,
				vid->vl_vfpd, vid->vl_vbpd);
			s5p_dsim_set_main_disp_hporch(dsim->reg_base,
				vid->vl_hfpd, vid->vl_hbpd);
			s5p_dsim_set_main_disp_sync_area(dsim->reg_base,
				vid->vl_vspw, vid->vl_hspw);
		}
#endif
	s5p_dsim_display_config(dsim->reg_base, dsim->dsim_lcd_info, NULL);

	return 0;
}

int s5p_dsim_init_link(struct dsim_global *dsim)
{
	unsigned int time_out = 100;

	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return -EFAULT;
	}

	switch (dsim->state) {
	case DSIM_STATE_RESET:
		s5p_dsim_sw_reset(dsim->reg_base);
	case DSIM_STATE_INIT:
		s5p_dsim_init_fifo_pointer(dsim->reg_base, 0x1f);

		/* dsi configuration */
		s5p_dsim_init_config(dsim->reg_base, dsim->dsim_lcd_info,
			NULL, dsim->dsim_info);
		s5p_dsim_enable_lane(dsim->reg_base, DSIM_LANE_CLOCK, 1);
		s5p_dsim_enable_lane(dsim->reg_base, dsim->data_lane, 1);

		/* set clock configuration */
		s5p_dsim_set_clock(dsim, dsim->dsim_info->e_byte_clk,
			1);

		/* check clock and data lane state is stop state */
		while (!(s5p_dsim_is_lane_state(dsim->reg_base, DSIM_LANE_CLOCK)
			    == DSIM_LANE_STATE_STOP) &&
			!(s5p_dsim_is_lane_state(dsim->reg_base,
				dsim->data_lane) == DSIM_LANE_STATE_STOP)) {
			time_out--;
			if (time_out == 0) {
				udebug("DSI Master is not stop state.\n");
				udebug("Check initialization process\n");

				return -EINVAL;
			}
		}

		if (time_out != 0) {
			udebug("initialization of DSI Master is successful\n");
			udebug("DSI Master state is stop state\n");
		}

		dsim->state = DSIM_STATE_STOP;

		/* BTA sequence counters */
		s5p_dsim_set_stop_state_counter(dsim->reg_base,
			dsim->dsim_info->stop_holding_cnt);
		s5p_dsim_set_bta_timeout(dsim->reg_base,
			dsim->dsim_info->bta_timeout);
		s5p_dsim_set_lpdr_timeout(dsim->reg_base,
			dsim->dsim_info->rx_timeout);

		/* default LPDT by both cpu and lcd controller */
		s5p_dsim_set_data_mode(dsim->reg_base, DSIM_TRANSFER_BOTH,
			DSIM_STATE_STOP);

		return 0;
	default:
		udebug("DSI Master is already init.\n");
		return 0;
	}

	return 0;
}

int s5p_dsim_set_hs_enable(struct dsim_global *dsim)
{
	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return -EFAULT;
	}

	if (dsim->state == DSIM_STATE_STOP) {
		if (dsim->e_clk_src != DSIM_EXT_CLK_BYPASS) {
			dsim->state = DSIM_STATE_HSCLKEN;
			s5p_dsim_set_data_mode(dsim->reg_base,
				DSIM_TRANSFER_BOTH, DSIM_STATE_HSCLKEN);
			s5p_dsim_enable_hs_clock(dsim->reg_base, 1);

			return 0;
		} else
			udebug("clock source is external bypass.\n");
	} else
		udebug("DSIM is not stop state.\n");

	return 0;
}

int s5p_dsim_set_data_transfer_mode(struct dsim_global *dsim,
	unsigned char data_path, unsigned char hs_enable)
{
	int ret = -1;

	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return -EFAULT;
	}

	if (hs_enable) {
		if (dsim->state == DSIM_STATE_HSCLKEN) {
			s5p_dsim_set_data_mode(dsim->reg_base, data_path,
				DSIM_STATE_HSCLKEN);
			ret = 0;
		} else {
			udebug("HS Clock lane is not enabled.\n");
			ret = -EINVAL;
		}
	} else {
		if (dsim->state == DSIM_STATE_INIT || dsim->state ==
			DSIM_STATE_ULPS) {
			udebug("DSI Master is not STOP or HSDT state.\n");
			ret = -EINVAL;
		} else {
			s5p_dsim_set_data_mode(dsim->reg_base, data_path,
				DSIM_STATE_STOP);
			ret = 0;
		}
	}

	return ret;
}

int s5p_dsim_get_frame_done_status(void *dsim_data)
{
	struct dsim_global *dsim = NULL;

	dsim = (struct dsim_global *)dsim_data;

	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return -EFAULT;
	}

	return _s5p_dsim_get_frame_done_status(dsim->reg_base);
}

int s5p_dsim_clear_frame_done(void *dsim_data)
{
	struct dsim_global *dsim = NULL;

	dsim = (struct dsim_global *)dsim_data;

	if (dsim == NULL) {
		udebug("dsim_global pointer is NULL.\n");
		return -EFAULT;
	}

	_s5p_dsim_clear_frame_done(dsim->reg_base);

	return 0;
}

int s5p_dsim_register_lcd_driver(struct mipi_lcd_driver *lcd_drv)
{
	struct mipi_lcd_info *lcd_info = NULL;


	lcd_info = kmalloc(sizeof(struct mipi_lcd_info), GFP_KERNEL);
	if (lcd_info == NULL)
		return -ENOMEM;

	lcd_info->mipi_drv = kmalloc(sizeof(struct mipi_lcd_driver),
				GFP_KERNEL);

	if (lcd_info->mipi_drv == NULL)
		return -ENOMEM;


	memcpy(lcd_info->mipi_drv, lcd_drv, sizeof(struct mipi_lcd_driver));

	mutex_lock(&mipi_lock);
	list_add_tail(&lcd_info->list, &lcd_info_list);
	mutex_unlock(&mipi_lock);

	udebug("registered lcd panel driver(%s) to mipi-dsi driver.\n",
		lcd_drv->name);

	return 0;
}

/*
 * This function is wrapper for changing transfer mode.
 * It is used to in panel driver before and after changing gamma value.
 */
int s5p_dsim_change_transfer_mode(int mode)
{
	if (mode < 0 || mode > 1) {
		udebug("mode range should be 0 or 1.\n");
		return -1;
	}

	if (mode == 0)
		s5p_dsim_set_data_transfer_mode(&dsim,
			DSIM_TRANSFER_BYCPU, mode);
	else
		s5p_dsim_set_data_transfer_mode(&dsim,
			DSIM_TRANSFER_BYLCDC, mode);

	return 0;
}

int s5p_dsim_enable_d_phy(struct dsim_global *dsim, unsigned int enable)
{
	unsigned int reg;

	if (dsim == NULL) {
		udebug("dsim is NULL.\n");
		return -EFAULT;
	}
#ifdef CONFIG_S5PC110
	reg = (readl(S5PC110_MIPI_DPHY_CONTROL)) & ~(1 << 0);
	reg |= (enable << 0);
	writel(reg, S5PC110_MIPI_DPHY_CONTROL);
#endif

	return 0;
}

int s5p_dsim_enable_dsi_master(struct dsim_global *dsim,
	unsigned int enable)
{
	unsigned int reg;

	if (dsim == NULL) {
		udebug("dsim is NULL.\n");
		return -EFAULT;
	}

#ifdef CONFIG_S5PC110
	reg = (readl(S5PC110_MIPI_DPHY_CONTROL)) & ~(1 << 2);
	reg |= (enable << 2);
	writel(reg, S5PC110_MIPI_DPHY_CONTROL);
#endif

	return 0;
}

int s5p_dsim_part_reset(struct dsim_global *dsim)
{
	if (dsim == NULL) {
		udebug("dsim is NULL.\n");
		return -EFAULT;
	}

	writel(S5P_MIPI_M_RESETN, S5P_MIPI_PHY_CON0);

	return 0;
}

int s5p_dsim_init_d_phy(struct dsim_global *dsim)
{
	if (dsim == NULL) {
		udebug("dsim is NULL.\n");
		return -EFAULT;
	}

	/* enable D-PHY */
	s5p_dsim_enable_d_phy(dsim, 1);

	/* enable DSI master block */
	s5p_dsim_enable_dsi_master(dsim, 1);

	return 0;
}

struct mipi_lcd_driver *scan_mipi_driver(const char *name)
{
	struct mipi_lcd_info *lcd_info;
	struct mipi_lcd_driver *mipi_drv = NULL;

	mutex_lock(&mipi_lock);

	list_for_each_entry(lcd_info, &lcd_info_list, list) {
		mipi_drv = lcd_info->mipi_drv;

		if ((strcmp(mipi_drv->name, name)) == 0) {
			mutex_unlock(&mipi_lock);
			udebug("found!!!(%s).\n", mipi_drv->name);
			return mipi_drv;
		}
	}

	mutex_unlock(&mipi_lock);

	return NULL;
}

static struct mipi_ddi_platform_data dsim_mipi_ddi_pd = {
	.resume_complete = 0,
	.cmd_write = s5p_dsim_wr_data,
	.cmd_read = NULL,
	.get_dsim_frame_done = s5p_dsim_get_frame_done_status,
	.clear_dsim_frame_done = s5p_dsim_clear_frame_done,
	.change_dsim_transfer_mode = s5p_dsim_change_transfer_mode,
	.get_fb_frame_done = s3cfb_is_i80_frame_done,
	.trigger = s3cfb_set_trigger,
};

int s5p_dsim_start(void)
{
	struct mipi_ddi_platform_data *ddi_pd = NULL;
	int ret = 0;

	dsim.dsim_pd = dsim_pd;
	dsim.dsim_info = dsim.dsim_pd->dsim_info;
	/* set dsim config data, dsim lcd config data and lcd panel data. */
	dsim.dsim_lcd_info = dsim.dsim_pd->dsim_lcd_info;
	dsim.mipi_ddi_pd = &dsim_mipi_ddi_pd;

	dsim.reg_base = 0xfa500000;
	dsim.mipi_ddi_pd->dsim_data = (void *)&dsim;

	dsim.mipi_drv = scan_mipi_driver(dsim.dsim_pd->lcd_panel_name);

	if (dsim.mipi_drv == NULL) {
		udebug("mipi_drv is NULL.\n");
		return 0;
	}
	/* it is used for MIPI-DSI based lcd panel driver. */

	ret = dsim.mipi_drv->set_link(dsim.mipi_ddi_pd);
	if (ret < 0) {
		udebug("failed to set link.\n");
		return 0;
	}

	s5p_dsim_init_dsim(&dsim);
	s5p_dsim_init_link(&dsim);

	s5p_dsim_set_hs_enable(&dsim);
	s5p_dsim_set_data_transfer_mode(&dsim, DSIM_TRANSFER_BYCPU, 1);

	/* it needs delay for stabilization */
	udelay(dsim.dsim_pd->delay_for_stabilization);

	/* initialize lcd panel */
	dsim.mipi_drv->init();
	dsim.mipi_drv->display_on();

	s5p_dsim_set_display_mode(&dsim);

	s5p_dsim_set_data_transfer_mode(&dsim, DSIM_TRANSFER_BYLCDC, 1);

	/* in case of command mode, trigger. */
	if (dsim.dsim_lcd_info->e_interface == DSIM_COMMAND)
		if (dsim.dsim_pd->trigger)
			dsim.dsim_pd->trigger();

	udebug("mipi-dsi driver(%s mode) has been probed.\n",
		(dsim.dsim_lcd_info->e_interface == DSIM_COMMAND) ?
			"CPU" : "RGB");

	return 0;
}

void s5p_set_dsim_platform_data(struct s5p_platform_dsim *pd)
{
	if (pd == NULL) {
		udebug("pd is NULL\n");
		return;
	}

	dsim_pd = pd;
}
