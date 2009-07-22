/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
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
 *
 */

#ifndef __ASM_ARCH_USB_HS_OTG_H_
#define __ASM_ARCH_USB_HS_OTG_H_

/*
 * USB2.0 HS OTG
 */
#define USBOTG_LINK_BASE	S5P_ADDR(0x0d200000)
#define USBOTG_PHY_BASE		S5P_ADDR(0x0d300000)

#define S5P_OTG_LINK(x)		(USBOTG_LINK_BASE + x)
#define S5P_OTG_PHY(x)		(USBOTG_PHY_BASE + x)

/* Core Global Registers */
#define S5P_OTG_GOTGCTL		S5P_OTG_LINK(0x000)
#define S5P_OTG_GOTGINT		S5P_OTG_LINK(0x004)
#define S5P_OTG_GAHBCFG		S5P_OTG_LINK(0x008)
#define S5P_OTG_GUSBCFG		S5P_OTG_LINK(0x00C)
#define S5P_OTG_GRSTCTL		S5P_OTG_LINK(0x010)
#define S5P_OTG_GINTSTS		S5P_OTG_LINK(0x014)
#define S5P_OTG_GINTMSK		S5P_OTG_LINK(0x018)
#define S5P_OTG_GRXSTSR		S5P_OTG_LINK(0x01C)
#define S5P_OTG_GRXSTSP		S5P_OTG_LINK(0x020)
#define S5P_OTG_GRXFSIZ		S5P_OTG_LINK(0x024)
#define S5P_OTG_GNPTXFSIZ	S5P_OTG_LINK(0x028)
#define S5P_OTG_GNPTXSTS	S5P_OTG_LINK(0x02C)

#define S5P_OTG_HPTXFSIZ	S5P_OTG_LINK(0x100)
#define S5P_OTG_DPTXFSIZ1	S5P_OTG_LINK(0x104)
#define S5P_OTG_DPTXFSIZ2	S5P_OTG_LINK(0x108)
#define S5P_OTG_DPTXFSIZ3	S5P_OTG_LINK(0x10C)
#define S5P_OTG_DPTXFSIZ4	S5P_OTG_LINK(0x110)
#define S5P_OTG_DPTXFSIZ5	S5P_OTG_LINK(0x114)
#define S5P_OTG_DPTXFSIZ6	S5P_OTG_LINK(0x118)
#define S5P_OTG_DPTXFSIZ7	S5P_OTG_LINK(0x11C
#define S5P_OTG_DPTXFSIZ8	S5P_OTG_LINK(0x120)
#define S5P_OTG_DPTXFSIZ9	S5P_OTG_LINK(0x124)
#define S5P_OTG_DPTXFSIZ10	S5P_OTG_LINK(0x128)
#define S5P_OTG_DPTXFSIZ11	S5P_OTG_LINK(0x12C)
#define S5P_OTG_DPTXFSIZ12	S5P_OTG_LINK(0x130)
#define S5P_OTG_DPTXFSIZ13	S5P_OTG_LINK(0x134)
#define S5P_OTG_DPTXFSIZ14	S5P_OTG_LINK(0x138)
#define S5P_OTG_DPTXFSIZ15	S5P_OTG_LINK(0x13C)

/* Host Global Registers */
#define S5P_OTG_HCFG		S5P_OTG_LINK(0x400)
#define S5P_OTG_HFIR		S5P_OTG_LINK(0x404)
#define S5P_OTG_HFNUM		S5P_OTG_LINK(0x408)
#define S5P_OTG_HPTXSTS		S5P_OTG_LINK(0x410)
#define S5P_OTG_HAINT		S5P_OTG_LINK(0x414)
#define S5P_OTG_HAINTMSK	S5P_OTG_LINK(0x418)

/* Host Port Control & Status Registers */
#define S5P_OTG_HPRT		S5P_OTG_LINK(0x440)

/* Host Channel-Specific Registers */
#define S5P_OTG_HCCHAR0		S5P_OTG_LINK(0x500)
#define S5P_OTG_HCSPLT0		S5P_OTG_LINK(0x504)
#define S5P_OTG_HCINT0		S5P_OTG_LINK(0x508)
#define S5P_OTG_HCINTMSK0	S5P_OTG_LINK(0x50C)
#define S5P_OTG_HCTSIZ0		S5P_OTG_LINK(0x510)
#define S5P_OTG_HCDMA0		S5P_OTG_LINK(0x514)

/* Device Global Registers */
#define S5P_OTG_DCFG		S5P_OTG_LINK(0x800)
#define S5P_OTG_DCTL		S5P_OTG_LINK(0x804)
#define S5P_OTG_DSTS		S5P_OTG_LINK(0x808)
#define S5P_OTG_DIEPMSK 	S5P_OTG_LINK(0x810)
#define S5P_OTG_DOEPMSK 	S5P_OTG_LINK(0x814)
#define S5P_OTG_DAINT		S5P_OTG_LINK(0x818)
#define S5P_OTG_DAINTMSK	S5P_OTG_LINK(0x81C)
#define S5P_OTG_DTKNQR1 	S5P_OTG_LINK(0x820)
#define S5P_OTG_DTKNQR2 	S5P_OTG_LINK(0x824)
#define S5P_OTG_DVBUSDIS	S5P_OTG_LINK(0x828)
#define S5P_OTG_DVBUSPULSE	S5P_OTG_LINK(0x82C)
#define S5P_OTG_DTKNQR3 	S5P_OTG_LINK(0x830)
#define S5P_OTG_DTKNQR4 	S5P_OTG_LINK(0x834)

/* Device Logical IN Endpoint-Specific Registers */
#define S5P_OTG_DIEPCTL0	S5P_OTG_LINK(0x900)
#define S5P_OTG_DIEPINT0	S5P_OTG_LINK(0x908)
#define S5P_OTG_DIEPTSIZ0	S5P_OTG_LINK(0x910)
#define S5P_OTG_DIEPDMA0	S5P_OTG_LINK(0x914)

/* Device Logical OUT Endpoint-Specific Registers */
#define S5P_OTG_DOEPCTL0	S5P_OTG_LINK(0xB00)
#define S5P_OTG_DOEPINT0	S5P_OTG_LINK(0xB08)
#define S5P_OTG_DOEPTSIZ0	S5P_OTG_LINK(0xB10)
#define S5P_OTG_DOEPDMA0	S5P_OTG_LINK(0xB14)

/* Power & clock gating registers */
#define S5P_OTG_PCGCCTRL	S5P_OTG_LINK(0xE00)

/* Endpoint FIFO address */
#define S5P_OTG_EP0_FIFO	S5P_OTG_LINK(0x1000)

/* OTG PHY CORE REGISTERS */
#define S5P_OTG_PHYPWR		S5P_OTG_PHY(0x00)
#define S5P_OTG_PHYCTRL		S5P_OTG_PHY(0x04)
#define S5P_OTG_RSTCON		S5P_OTG_PHY(0x08)

#endif
