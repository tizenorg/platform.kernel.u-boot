/* linux/arm/arch/mach-s5pc110/include/mach/dsim.h
 *
 * Platform data header for Samsung MIPI-DSIM.
 *
 * Copyright (c) 2009 Samsung Electronics
 * InKi Dae <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <mipi_ddi.h>
#include <lcd.h>

#define DEBUG
#undef DEBUG
#ifdef DEBUG
#define udebug(args...)		printf(args)
#else
#define udebug(args...)		do { } while (0)
#endif

/* h/w configuration */
#define MIPI_FIN		24000000
#define DSIM_TIMEOUT_MS		5000
#define DSIM_NO_OF_INTERRUPT	26
#define DSIM_PM_STABLE_TIME	10

#define DSIM_TRUE		1
#define DSIM_FALSE		0

#define DSIM_HEADER_FIFO_SZ	16

/* should be '1' before enabling DSIM */
#define S5P_MIPI_M_RESETN	0xE0107200
#define S5P_MIPI_PHY_CON0	(1 << 0)

enum dsim_interface_type {
	DSIM_COMMAND = 0,
	DSIM_VIDEO = 1,
};

enum dsim_state {
	DSIM_STATE_RESET = 0,
	DSIM_STATE_INIT = 1,
	DSIM_STATE_STOP = 2,
	DSIM_STATE_HSCLKEN = 3,
	DSIM_STATE_ULPS = 4,
};

enum {
	DSIM_NONE_STATE = 0,
	DSIM_RESUME_COMPLETE = 1,
	DSIM_FRAME_DONE = 2,
};

enum dsim_virtual_ch_no {
	DSIM_VIRTUAL_CH_0 = 0,
	DSIM_VIRTUAL_CH_1 = 1,
	DSIM_VIRTUAL_CH_2 = 2,
	DSIM_VIRTUAL_CH_3 = 3,
};

enum dsim_video_mode_type {
	DSIM_NON_BURST_SYNC_EVENT = 0,
	DSIM_NON_BURST_SYNC_PULSE = 2,
	DSIM_BURST = 3,
	DSIM_NON_VIDEO_MODE = 4,
};

enum dsim_fifo_state {
	DSIM_RX_DATA_FULL = (1 << 25),
	DSIM_RX_DATA_EMPTY = (1 << 24),
	SFR_HEADER_FULL = (1 << 23),
	SFR_HEADER_EMPTY = (1 << 22),
	SFR_PAYLOAD_FULL = (1 << 21),
	SFR_PAYLOAD_EMPTY = (1 << 20),
	I80_HEADER_FULL = (1 << 19),
	I80_HEADER_EMPTY = (1 << 18),
	I80_PALOAD_FULL = (1 << 17),
	I80_PALOAD_EMPTY = (1 << 16),
	SUB_DISP_HEADER_FULL = (1 << 15),
	SUB_DISP_HEADER_EMPTY = (1 << 14),
	SUB_DISP_PAYLOAD_FULL = (1 << 13),
	SUB_DISP_PAYLOAD_EMPTY = (1 << 12),
	MAIN_DISP_HEADER_FULL = (1 << 11),
	MAIN_DISP_HEADER_EMPTY = (1 << 10),
	MAIN_DISP_PAYLOAD_FULL = (1 << 9),
	MAIN_DISP_PAYLOAD_EMPTY = (1 << 8),
};

enum dsim_no_of_data_lane {
	DSIM_DATA_LANE_1 = 0,
	DSIM_DATA_LANE_2 = 1,
	DSIM_DATA_LANE_3 = 2,
	DSIM_DATA_LANE_4 = 3,
};

enum dsim_byte_clk_src {
	DSIM_PLL_OUT_DIV8 = 0,
	DSIM_EXT_CLK_DIV8 = 1,
	DSIM_EXT_CLK_BYPASS = 2,
};

enum dsim_lane {
	DSIM_LANE_DATA0 = (1 << 0),
	DSIM_LANE_DATA1 = (1 << 1),
	DSIM_LANE_DATA2 = (1 << 2),
	DSIM_LANE_DATA3 = (1 << 3),
	DSIM_LANE_DATA_ALL = 0xf,
	DSIM_LANE_CLOCK = (1 << 4),
	DSIM_LANE_ALL = DSIM_LANE_CLOCK | DSIM_LANE_DATA_ALL,
};

enum dsim_pixel_format {
	DSIM_CMD_3BPP = 0,
	DSIM_CMD_8BPP = 1,
	DSIM_CMD_12BPP = 2,
	DSIM_CMD_16BPP = 3,
	DSIM_VID_16BPP_565 = 4,
	DSIM_VID_18BPP_666PACKED = 5,
	DSIM_18BPP_666LOOSELYPACKED = 6,
	DSIM_24BPP_888 = 7,
};

enum dsim_lane_state {
	DSIM_LANE_STATE_HS_READY,
	DSIM_LANE_STATE_ULPS,
	DSIM_LANE_STATE_STOP,
	DSIM_LANE_STATE_LPDT,
};

enum dsim_transfer {
	DSIM_TRANSFER_NEITHER	= 0,
	DSIM_TRANSFER_BYCPU	= (1 << 7),
	DSIM_TRANSFER_BYLCDC	= (1 << 6),
	DSIM_TRANSFER_BOTH	= (0x3 << 6)
};

enum dsim_lane_change {
	DSIM_NO_CHANGE = 0,
	DSIM_DATA_LANE_CHANGE = 1,
	DSIM_CLOCK_NALE_CHANGE = 2,
	DSIM_ALL_LANE_CHANGE = 3,
};

enum dsim_int_src {
	DSIM_ALL_OF_INTR = 0xffffffff,
	DSIM_PLL_STABLE = (1 << 31),
};

enum dsim_data_id {
	/* short packet types of packet types for command */
	GEN_SHORT_WR_NO_PARA	= 0x03,
	GEN_SHORT_WR_1_PARA	= 0x13,
	GEN_SHORT_WR_2_PARA	= 0x23,
	GEN_RD_NO_PARA		= 0x04,
	GEN_RD_1_PARA		= 0x14,
	GEN_RD_2_PARA		= 0x24,
	DCS_WR_NO_PARA		= 0x05,
	DCS_WR_1_PARA		= 0x15,
	DCS_RD_NO_PARA		= 0x06,
	SET_MAX_RTN_PKT_SIZE	= 0x37,

	/* long packet types of packet types for command */
	NULL_PKT		= 0x09,
	BLANKING_PKT		= 0x19,
	GEN_LONG_WR		= 0x29,
	DCS_LONG_WR		= 0x39,

	/* short packet types of generic command */
	CMD_OFF			= 0x02,
	CMD_ON			= 0x12,
	SHUT_DOWN		= 0x22,
	TURN_ON			= 0x32,

	/* short packet types for video data */
	VSYNC_START		= 0x01,
	VSYNC_END		= 0x11,
	HSYNC_START		= 0x21,
	HSYNC_END		= 0x31,
	EOT_PKT			= 0x08,

	/* long packet types for video data */
	RGB565_PACKED		= 0x0e,
	RGB666_PACKED		= 0x1e,
	RGB666_LOOSLY		= 0x2e,
	RGB888_PACKED		= 0x3e,
};

/**
 * struct dsim_config - interface for configuring mipi-dsi controller.
 *
 * @auto_flush: enable or disable Auto flush of MD FIFO using VSYNC pulse.
 * @eot_disable: enable or disable EoT packet in HS mode.
 * @auto_vertical_cnt: specifies auto vertical count mode.
 * 	in Video mode, the vertical line transition uses line counter
 * 	configured by VSA, VBP, and Vertical resolution.
 * 	If this bit is set to '1', the line counter does not use VSA and VBP
 * 	registers.(in command mode, this variable is ignored)
 * @hse: set horizontal sync event mode.
 * 	In VSYNC pulse and Vporch area, MIPI DSI master transfers only HSYNC
 * 	start packet to MIPI DSI slave at MIPI DSI spec1.1r02.
 * 	this bit transfers HSYNC end packet in VSYNC pulse and Vporch area
 * 	(in mommand mode, this variable is ignored)
 * @hfp: specifies HFP disable mode.
 * 	if this variable is set, DSI master ignores HFP area in VIDEO mode.
 * 	(in command mode, this variable is ignored)
 * @hbp: specifies HBP disable mode.
 * 	if this variable is set, DSI master ignores HBP area in VIDEO mode.
 * 	(in command mode, this variable is ignored)
 * @hsa: specifies HSA disable mode.
 * 	if this variable is set, DSI master ignores HSA area in VIDEO mode.
 * 	(in command mode, this variable is ignored)
 * @e_no_data_lane: specifies data lane count to be used by Master.
 * @e_byte_clk: select byte clock source. (it must be DSIM_PLL_OUT_DIV8)
 * 	DSIM_EXT_CLK_DIV8 and DSIM_EXT_CLK_BYPASSS are not supported.
 * @pll_stable_time: specifies the PLL Timer for stability of the ganerated
 * 	clock(System clock cycle base)
 * 	if the timer value goes to 0x00000000, the clock stable bit of status
 * 	and interrupt register is set.
 * @esc_clk: specifies escape clock frequency for getting the escape clock
 * 	prescaler value.
 * @stop_holding_cnt: specifies the interval value between transmitting
 * 	read packet(or write "set_tear_on" command) and BTA request.
 * 	after transmitting read packet or write "set_tear_on" command,
 * 	BTA requests to D-PHY automatically. this counter value specifies
 * 	the interval between them.
 * @bta_timeout: specifies the timer for BTA.
 * 	this register specifies time out from BTA request to change
 * 	the direction with respect to Tx escape clock.
 * @rx_timeout: specifies the timer for LP Rx mode timeout.
 * 	this register specifies time out on how long RxValid deasserts,
 * 	after RxLpdt asserts with respect to Tx escape clock.
 * 	- RxValid specifies Rx data valid indicator.
 * 	- RxLpdt specifies an indicator that D-PHY is under RxLpdt mode.
 * 	- RxValid and RxLpdt specifies signal from D-PHY.
 * @e_lane_swap: swaps Dp/Dn channel of Clock lane or Data lane.
 * 	if this bit is set, Dp and Dn channel would be swapped each other.
 */
struct dsim_config {
	unsigned char auto_flush;
	unsigned char eot_disable;

	unsigned char auto_vertical_cnt;
	unsigned char hse;
	unsigned char hfp;
	unsigned char hbp;
	unsigned char hsa;

	enum dsim_no_of_data_lane e_no_data_lane;
	enum dsim_byte_clk_src e_byte_clk;

	/*
	 * ===========================================
	 * |    P    |    M    |    S    |    MHz    |
	 * -------------------------------------------
	 * |    3    |   100   |    3    |    100    |
	 * |    3    |   100   |    2    |    200    |
	 * |    3    |    63   |    1    |    252    |
	 * |    4    |   100   |    1    |    300    |
	 * |    4    |   110   |    1    |    330    |
	 * |   12    |   350   |    1    |    350    |
	 * |    3    |   100   |    1    |    400    |
	 * |    4    |   150   |    1    |    450    |
	 * |    3    |   118   |    1    |    472    |
	 * |   12    |   250   |    0    |    500    |
	 * |    4    |   100   |    0    |    600    |
	 * |    3    |    81   |    0    |    648    |
	 * |    3    |    88   |    0    |    704    |
	 * |    3    |    90   |    0    |    720    |
	 * |    3    |   100   |    0    |    800    |
	 * |   12    |   425   |    0    |    850    |
	 * |    4    |   150   |    0    |    900    |
	 * |   12    |   475   |    0    |    950    |
	 * |    6    |   250   |    0    |   1000    |
	 * -------------------------------------------
	 */
	unsigned char p;
	unsigned short m;
	unsigned char s;

	unsigned int pll_stable_time;
	unsigned long esc_clk;

	unsigned short stop_holding_cnt;
	unsigned char bta_timeout;
	unsigned short rx_timeout;
	enum dsim_video_mode_type e_lane_swap;
};

/**
 * struct dsim_lcd_config - interface for configuring mipi-dsi based lcd panel.
 *
 * @e_interface: specifies interface to be used.(CPU or RGB interface)
 * @parameter[0]: specifies virtual channel number
 * 	that main or sub diaplsy uses.
 * @parameter[1]: specifies pixel stream format for main or sub display.
 * @parameter[2]: selects Burst mode in Video mode.
 * 	in Non-burst mode, RGB data area is filled with RGB data and NULL
 * 	packets, according to input bandwidth of RGB interface.
 * 	In Burst mode, RGB data area is filled with RGB data only.
 * @lcd_panel_info: pointer for lcd panel specific structure.
 * 	this structure specifies width, height, timing and polarity and so on.
 * @mipi_ddi_pd: pointer for lcd panel platform data.
 */
struct dsim_lcd_config {
	enum dsim_interface_type e_interface;
	unsigned int parameter[3];

	void *lcd_panel_info;
	void *mipi_ddi_pd;
};

struct dsim_global;

/**
 * struct s5p_platform_dsim - interface to platform data for mipi-dsi driver.
 *
 * @clk_name: specifies clock name for mipi-dsi.
 * @lcd_panel_name: specifies lcd panel name registered to mipi-dsi driver.
 * 	lcd panel driver searched would be actived.
 * @platfrom_rev: specifies platform revision number.
 * 	revision number should become 1.
 * @dsim_config: pointer of structure for configuring mipi-dsi controller.
 * @dsim_lcd_info: pointer to structure for configuring
 * 	mipi-dsi based lcd panel. 
 * @mipi_power: callback pointer for enabling or disabling mipi power.
 * @part_reset: callback pointer for reseting mipi phy.
 * @init_d_phy: callback pointer for enabing d_phy of dsi master.
 * @get_fb_frame_done: callback pointer for getting frame done status of the
 * 	display controller(FIMD).
 * @trigger: callback pointer for triggering display controller(FIMD)
 * 	in case of CPU mode.
 * @delay_for_stabilization: specifies stable time.
 * 	this delay needs when writing data on SFR
 * 	after mipi mode became LP mode.
 */
struct s5p_platform_dsim {
	char	*clk_name;
	char	lcd_panel_name[64];
	unsigned int platform_rev;

	struct dsim_config *dsim_info;
	struct dsim_lcd_config *dsim_lcd_info;

unsigned int delay_for_stabilization;

	void (*mipi_power) (void);
	int (*part_reset) (struct dsim_global *dsim);
	int (*init_d_phy) (struct dsim_global *dsim);
	int (*get_fb_frame_done) (void);
	void (*trigger) (void);
	void (*mipi_set_link)(struct mipi_ddi_platform_data *ddi_pd);
	void (*mipi_panel_init)(void);
	void (*mipi_display_on)(void);
	vidinfo_t *pvid;
};

/**
 * struct dsim_global - global interface for mipi-dsi driver.
 *
 * @dev: driver model representation of the device.
 * @clock: pointer to MIPI-DSI clock of clock framework.
 * @irq: interrupt number to MIPI-DSI controller.
 * @reg_base: base address to memory mapped SRF of MIPI-DSI controller.
 * 	(virtual address)
 * @r_mipi_1_1v: pointer to regulator for MIPI 1.1v power. 
 * @r_mipi_1_8v: pointer to regulator for MIPI 1.8v power.
 * @pd: pointer to MIPI-DSI driver platform data.
 * @dsim_lcd_info: pointer to structure for configuring
 * 	mipi-dsi based lcd panel. 
 * @lcd_panel_info: pointer for lcd panel specific structure.
 * 	this structure specifies width, height, timing and polarity and so on.
 * @mipi_ddi_pd: pointer for lcd panel platform data.
 * @mipi_drv: pointer to driver structure for mipi-dsi based lcd panel.
 * @s3cfb_notif: kernel notifier structure to be registered
 * 	by device specific framebuffer driver.
 * 	this notifier could be used by fb_blank of device specifiec
 * 	framebuffer driver.
 * @state: specifies status of MIPI-DSI controller.
 * 	the status could be RESET, INIT, STOP, HSCLKEN and ULPS.
 * @data_lane: specifiec enabled data lane number.
 * 	this variable would be set by driver according to e_no_data_lane
 * 	automatically.
 * @e_clk_src: select byte clock source.
 * 	this variable would be set by driver according to e_byte_clock
 * 	automatically.
 * @hs_clk: HS clock rate.
 * 	this variable would be set by driver automatically.
 * @byte_clk: Byte clock rate.
 * 	this variable would be set by driver automatically.
 * @escape_clk: ESCAPE clock rate.
 * 	this variable would be set by driver automatically.
 * @freq_band: indicates Bitclk frequency band for D-PHY global timing.
 * 	Serial Clock(=ByteClk X 8)		FreqBand[3:0]
 * 		~ 99.99 MHz 				0000
 * 		100 ~ 119.99 MHz 			0001
 * 		120 ~ 159.99 MHz			0010
 * 		160 ~ 199.99 MHz 			0011
 * 		200 ~ 239.99 MHz 			0100
 * 		140 ~ 319.99 MHz 			0101
 * 		320 ~ 389.99 MHz 			0110
 * 		390 ~ 449.99 MHz 			0111
 * 		450 ~ 509.99 MHz 			1000
 * 		510 ~ 559.99 MHz 			1001
 * 		560 ~ 639.99 MHz 			1010
 * 		640 ~ 689.99 MHz 			1011
 * 		690 ~ 769.99 MHz 			1100
 * 		770 ~ 869.99 MHz 			1101
 * 		870 ~ 949.99 MHz 			1110
 * 		950 ~ 1000 MHz 				1111
 * 	this variable would be calculated by driver automatically.
 *
 * @header_fifo_index: specifies header fifo index.
 * 	this variable is not used yet.
 */
struct dsim_global {
	struct clk *clock;
	unsigned int irq;
	unsigned int reg_base;

	struct s5p_platform_dsim *dsim_pd;
	struct dsim_config *dsim_info;
	struct dsim_lcd_config *dsim_lcd_info;
	struct mipi_ddi_platform_data *mipi_ddi_pd;
	struct mipi_lcd_driver *mipi_drv;

	unsigned char state;
	unsigned int data_lane;
	enum dsim_byte_clk_src e_clk_src;
	unsigned long hs_clk;
	unsigned long byte_clk;
	unsigned long escape_clk;
	unsigned char freq_band;

	char header_fifo_index[DSIM_HEADER_FIFO_SZ];
};

/*
 * driver structure for mipi-dsi based lcd panel.
 *
 * this structure should be registered by lcd panel driver.
 * mipi-dsi driver seeks lcd panel registered through name field
 * and calls these callback functions in appropriate time.
 */
struct mipi_lcd_driver {
	s8	name[64];

	s32	(*init)(void);
	void	(*display_on)(void);
	s32	(*set_link)(struct mipi_ddi_platform_data *pd);
};

/*
 * register mipi_lcd_driver object defined by lcd panel driver
 * to mipi-dsi driver.
 */
extern int s5p_dsim_start(void);
//extern int s5p_dsim_start(vidinfo_t *vid);

/* reset MIPI PHY through MIPI PHY CONTROL REGISTER. */
extern int s5p_dsim_part_reset(struct dsim_global *dsim);
/* enable MIPI D-PHY and DSI Master block. */
extern int s5p_dsim_init_d_phy(struct dsim_global *dsim);
