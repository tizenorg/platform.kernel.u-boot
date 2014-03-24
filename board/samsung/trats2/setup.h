#ifndef _BOARD_TRATS2_SETUP_
#define _BOARD_TRATS2_SETUP_

/* A/M PLL_CON0 */
#define SDIV(x)			((x) & 0x7)
#define PDIV(x)			((x & 0x3f) << 8)
#define MDIV(x)			((x & 0x3ff) << 16)
#define FSEL(x)			((x & 0x1) << 27)
#define PLL_LOCKED_BIT		(0x1 << 29)
#define PLL_ENABLE(x)		((x & 0x1) << 31)

/* CLK_SRC_CPU */
#define MUX_APLL_SEL(x)		((x) & 0x1)
#define MUX_CORE_SEL(x)		((x & 0x1) << 16)
#define MUX_HPM_SEL(x)		((x & 0x1) << 20)
#define MUX_MPLL_USER_SEL_C(x)	((x & 0x1) << 24)

/* CLK_MUX_STAT_CPU */
#define APLL_SEL(x)		((x) & 0x7)
#define CORE_SEL(x)		((x & 0x7) << 16)
#define HPM_SEL(x)		((x & 0x7) << 20)
#define MPLL_USER_SEL_C(x)	((x & 0x7) << 24)
#define MUX_STAT_CHANGING	0x100
#define MUX_STAT_CPU_CHANGING	(APLL_SEL(MUX_STAT_CHANGING) | \
				CORE_SEL(MUX_STAT_CHANGING) | \
				HPM_SEL(MUX_STAT_CHANGING) | \
				MPLL_USER_SEL_C(MUX_STAT_CHANGING))

/* CLK_DIV_CPU0 */
#define CORE_RATIO(x)		((x) & 0x7)
#define COREM0_RATIO(x)		((x & 0x7) << 4)
#define COREM1_RATIO(x)		((x & 0x7) << 8)
#define PERIPH_RATIO(x)		((x & 0x7) << 12)
#define ATB_RATIO(x)		((x & 0x7) << 16)
#define PCLK_DBG_RATIO(x)	((x & 0x7) << 20)
#define APLL_RATIO(x)		((x & 0x7) << 24)
#define CORE2_RATIO(x)		((x & 0x7) << 28)

/* CLK_DIV_STAT_CPU0 */
#define DIV_CORE(x)		((x) & 0x1)
#define DIV_COREM0(x)		((x & 0x1) << 4)
#define DIV_COREM1(x)		((x & 0x1) << 8)
#define DIV_PERIPH(x)		((x & 0x1) << 12)
#define DIV_ATB(x)		((x & 0x1) << 16)
#define DIV_PCLK_DBG(x)		((x & 0x1) << 20)
#define DIV_APLL(x)		((x & 0x1) << 24)
#define DIV_CORE2(x)		((x & 0x1) << 28)

#define DIV_STAT_CHANGING	0x1
#define DIV_STAT_CPU0_CHANGING	(DIV_CORE(DIV_STAT_CHANGING) | \
				DIV_COREM0(DIV_STAT_CHANGING) | \
				DIV_COREM1(DIV_STAT_CHANGING) | \
				DIV_PERIPH(DIV_STAT_CHANGING) | \
				DIV_ATB(DIV_STAT_CHANGING) | \
				DIV_PCLK_DBG(DIV_STAT_CHANGING) | \
				DIV_APLL(DIV_STAT_CHANGING) | \
				DIV_CORE2(DIV_STAT_CHANGING))

/* CLK_DIV_CPU1 */
#define COPY_RATIO(x)		((x) & 0x7)
#define HPM_RATIO(x)		((x & 0x7) << 4)
#define CORES_RATIO(x)		((x & 0x7) << 8)

/* CLK_DIV_STAT_CPU1 */
#define DIV_COPY(x)		((x) & 0x7)
#define DIV_HPM(x)		((x & 0x1) << 4)
#define DIV_CORES(x)		((x & 0x1) << 8)

#define DIV_STAT_CPU1_CHANGING	(DIV_COPY(DIV_STAT_CHANGING) | \
				DIV_HPM(DIV_STAT_CHANGING) | \
				DIV_CORES(DIV_STAT_CHANGING))

#endif /* _BOARD_TRATS2_SETUP_ */
