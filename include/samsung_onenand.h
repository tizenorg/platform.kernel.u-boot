/*
 *  Copyright (C) 2005-2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __SAMSUNG_ONENAND_H__
#define __SAMSUNG_ONENAND_H__

#include <asm/hardware.h>

/*
 * OneNAND Controller
 */
#define MEM_CFG_OFFSET		0x0000
#define BURST_LEN_OFFSET	0x0010
#define MEM_RESET_OFFSET	0x0020
#define INT_ERR_STAT_OFFSET	0x0030
#define INT_ERR_MASK_OFFSET	0x0040
#define INT_ERR_ACK_OFFSET	0x0050
#define ECC_ERR_STAT_OFFSET	0x0060
#define ECC_ERR_STAT_1_OFFSET	0x0060
#define MANUFACT_ID_OFFSET	0x0070
#define DEVICE_ID_OFFSET	0x0080
#define DATA_BUF_SIZE_OFFSET	0x0090
#define BOOT_BUF_SIZE_OFFSET	0x00A0
#define BUF_AMOUNT_OFFSET	0x00B0
#define TECH_OFFSET		0x00C0
#define FBA_WIDTH_OFFSET	0x00D0
#define FPA_WIDTH_OFFSET	0x00E0
#define FSA_WIDTH_OFFSET	0x00F0
#define SYNC_MODE_OFFSET	0x0130
#define TRANS_SPARE_OFFSET	0x0140
#define ERR_PAGE_ADDR_OFFSET	0x0180
#define INT_PIN_ENABLE_OFFSET	0x01A0
#define ACC_CLOCK_OFFSET	0x01C0
#define ERR_BLK_ADDR_OFFSET	0x01E0
#define FLASH_VER_ID_OFFSET	0x01F0
#define WATCHDOG_CNT_LOW_OFFSET	0x0260
#define WATCHDOG_CNT_HI_OFFSET	0x0270
#define SYNC_WRITE_OFFSET	0x0280
#define COLD_RESET_DELAY_OFFSET	0x02A0
#define DDP_DEVICE_OFFSET	0x02B0
#define MULTI_PLANE_OFFSET	0x02C0
#define TRANS_MODE_OFFSET	0x02E0
#define ECC_ERR_STAT_2_OFFSET	0x0300
#define ECC_ERR_STAT_3_OFFSET	0x0310
#define ECC_ERR_STAT_4_OFFSET	0x0320
#define DEV_PAGE_SIZE_OFFSET	0x0340
#define INT_MON_STATUS_OFFSET	0x0390

#define ONENAND_MEM_RESET_HOT	0x3
#define ONENAND_MEM_RESET_COLD	0x2
#define ONENAND_MEM_RESET_WARM	0x1

#define CACHE_OP_ERR    (1 << 13)
#define RST_CMP         (1 << 12)
#define RDY_ACT         (1 << 11)
#define INT_ACT         (1 << 10)
#define UNSUP_CMD       (1 << 9)
#define LOCKED_BLK      (1 << 8)
#define BLK_RW_CMP      (1 << 7)
#define ERS_CMP         (1 << 6)
#define PGM_CMP         (1 << 5)
#define LOAD_CMP        (1 << 4)
#define ERS_FAIL        (1 << 3)
#define PGM_FAIL        (1 << 2)
#define INT_TO          (1 << 1)
#define LD_FAIL_ECC_ERR (1 << 0)

#define TSRF		(1 << 0)

#endif
