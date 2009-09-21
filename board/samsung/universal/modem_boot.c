/*
 * need copyright
 */

#include <common.h>
#include <command.h>
#include <serial.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>
#include "psi_ram.h"

#define OneDRAM_B_BANK_BASEADDR		0x35000000

#define MODEM_OS_BASEADDR		OneDRAM_B_BANK_BASEADDR
#define MODEM_EEP_STATIC_BASEADDR	(MODEM_OS_BASEADDR + 0x870000)
#define MODEM_EEP_CAL_BAK_BASEADDR	(MODEM_OS_BASEADDR + 0x880000)
#define MODEM_EEP_DYNAMIC_BASEADDR	(MODEM_OS_BASEADDR + 0x890000)
#define MODEM_EEP_SECURITY_BASEADDR	(MODEM_OS_BASEADDR + 0x8A0000)

#define OneDRAMREG_BASEADDR		(OneDRAM_B_BANK_BASEADDR + 0xFFF800)

#define rOneDRAM_SEMAPHORE	(*(volatile int*)(OneDRAMREG_BASEADDR))
#define rOneDRAM_MAILBOX_AB	(*(volatile int*)(OneDRAMREG_BASEADDR + 0x20))
#define rOneDRAM_MAILBOX_BA	(*(volatile int*)(OneDRAMREG_BASEADDR + 0x40))

/* primitive IPC on OneDRAM */
#define IPC_CP_READY_FOR_LOADING	0x12341234
#define IPC_CP_IMG_LOADED		0x45674567
#define IPC_CP_READY			0x23
#define IPC_BOOT_DONE			0x89EF89EF

#define CP_BOOT_MODE_NORMAL		0x0a
#define CP_BOOT_MODE_PTEST		0x0b

#define CP_HAS_SEM			0x0
#define AP_HAS_SEM			0x1

#define RETRY				3

#define IND				0x30
#define VERS				0xf0
#define CRC_OK				0x01
#define CRC_ERR				0xff

#define GPIO_CON_MASK			0xF
#define GPIO_DAT_MASK			0x1
#define GPIO_PUL_MASK			0x3

extern struct serial_device s5pc1xx_serial3_device;

int uart_serial_pollc(struct serial_device *uart, int retry)
{
	int i = 0;

	for (i = 0; i < retry; i++) {
		if (uart->tstc())
			return uart->getc();
		udelay(1000);
	}

	return -1;
}

enum {
	MACH_UNIVERSAL,
	MACH_TICKERTAPE,
	MACH_AQUILA,
};

DECLARE_GLOBAL_DATA_PTR;

static int get_machine_id(void)
{
	if (cpu_is_s5pc100())
		return -1;

	return gd->bd->bi_arch_number - 3100;
}

static int machine_is_aquila(void)
{
	return get_machine_id() == MACH_AQUILA;
}

static int machine_is_tickertape(void)
{
	return get_machine_id() == MACH_TICKERTAPE;
}

static int aquila_infineon_modem_on(void)
{
	unsigned int con, dat, pud, exit = 0;
	unsigned int pin;
	unsigned int nInfoSize;
	unsigned char *pDataPSI;
	int nCnt;
	int nCoreVer, nSizePSI, nCRC;
	int ack;
	int i, tmp;

	/* Infineon modem */
	/* 1. Modem gpio init */
	/* Phone_on */
	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_J1_OFFSET);

	con = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	con &= ~(0xf << 0); /* 0 = 0 * 4 */
	con |= (0x1 << 0); /* Set to output (0x1) */
	writel(con, pin + S5PC1XX_GPIO_CON_OFFSET);

	pud = readl(pin + S5PC1XX_GPIO_PULL_OFFSET);
	pud &= ~(0x3 << 0); /* 0 = 0 * 2 */
	pud |= (0x0 << 0); /* Pull-up/down disabled */
	writel(pud, pin + S5PC1XX_GPIO_PULL_OFFSET);

	/* CP_Reset */
	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H3_OFFSET);

	con = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	con &= ~(0xf << 28); /* 28 = 7 * 4 */
	con |= (0x1 << 28); /* Set to output (0x1) */
	writel(con, pin + S5PC1XX_GPIO_CON_OFFSET);

	pud = readl(pin + S5PC1XX_GPIO_PULL_OFFSET);
	pud &= ~(0x3 << 14); /* 14 = 7 * 2 */
	pud |= (0x0 << 14); /* Pull-up/down disabled */
	writel(pud, pin + S5PC1XX_GPIO_PULL_OFFSET);

	/* nINT_ONEDRAN_AP */
	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET);

	con = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	con &= ~(0xf << 12); /* 12 = 3 * 4 */
	con |= (0x0 << 12); /* Set to input (0x0) */
	writel(con, pin + S5PC1XX_GPIO_CON_OFFSET);

	pud = readl(pin + S5PC1XX_GPIO_PULL_OFFSET);
	pud &= ~(0x3 << 6); /* 6 = 3 * 2 */
	pud |= (0x0 << 6); /* Pull-up/down disabled */
	writel(pud, pin + S5PC1XX_GPIO_PULL_OFFSET);

	/* 2. Uart3 init */
	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_A1_OFFSET);

	con = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	con &= ~(0xf << 8); /* 8 = 2 * 4 */
	con |= (0x2 << 8); /* Set to UART3_RXD */
	con &= ~(0xf << 12); /* 12 = 3 * 4 */
	con |= (0x2 << 12); /* Set to UART3_TXD */
	writel(con, pin + S5PC1XX_GPIO_CON_OFFSET);

	pud = readl(pin + S5PC1XX_GPIO_PULL_OFFSET);
	pud &= ~(0x3 << 4); /* 4 = 2 * 2 */
	pud |= (0x0 << 4); /* Pull-up/down disabled */
	pud &= ~(0x3 << 6); /* 6 = 3 * 2 */
	pud |= (0x0 << 6); /* Pull-up/down disabled */
	writel(pud, pin + S5PC1XX_GPIO_PULL_OFFSET);

	/* reset and enable FIFOs, set triggers to the maximum */
	writel(0x0, 0xE2900C08);
	writel(0x0, 0xE2900C0C);
	/* 8N1 */
	writel(0x3, 0xE2900C00);
	/* No interrupts, no DMA, pure polling */
	writel(0x245, 0xE2900C04);
	s5pc1xx_serial3_device.setbrg();

	/* 3. Modem reset */
	while (1) {
		/* PHONE_ON -> low */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_J1_OFFSET);
		dat = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
		dat &= ~(0x1 << 0);
		writel(dat, pin + S5PC1XX_GPIO_DAT_OFFSET);

		/* CP_RESET -> low */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H3_OFFSET);
		dat = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
		dat &= ~(0x1 << 7);
		writel(dat, pin + S5PC1XX_GPIO_DAT_OFFSET);

		udelay(200*1000);	/* 200ms */

		/* CP_RESET -> high */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H3_OFFSET);
		dat = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
		dat |= (0x1 << 7);
		writel(dat, pin + S5PC1XX_GPIO_DAT_OFFSET);

		/* PHONE_ON -> high */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_J1_OFFSET);
		dat = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
		dat |= (0x1 << 0);
		writel(dat, pin + S5PC1XX_GPIO_DAT_OFFSET);

		udelay(27*1000);	/* > 26.6ms */

		udelay(40*1000);	/* Power stabilization */

		if (++exit > RETRY)
			break;

		printf("********************************\n");
		printf(" Reset and try to send PSI : #%d\n", exit);

		/* Drain Rx Serial */
		tmp = 0;
		while (tmp != -1)
			tmp = uart_serial_pollc(&s5pc1xx_serial3_device, 5);

		/* Sending "AT" in ASCII */
		for (nCnt = 0; nCnt < 20; nCnt++) {
			s5pc1xx_serial3_device.puts("AT");
			nCoreVer =
				uart_serial_pollc(&s5pc1xx_serial3_device, 5);

			if (nCoreVer == VERS)
				break;

			/*
			 * Send "AT" at 50ms internals till the bootcore
			 * version is received
			 */
			udelay(50*1000);	/* 50mec */
		}

		/* if fail to receive Modem core version restart all process */
		if (nCnt == 20)
			continue;

		nInfoSize = uart_serial_pollc(&s5pc1xx_serial3_device, 5);
		printf("Got Bootcore version and related info!!!\n"
			" - nCoreVer = 0x%x \n - nInfoSize = 0x%x\n",
			nCoreVer, nInfoSize);

		/* Drain Rx Serial */
		tmp = 0;
		while (tmp != -1)
			tmp = uart_serial_pollc(&s5pc1xx_serial3_device, 5);

		/* INDICATION BYTE */
		s5pc1xx_serial3_device.putc(IND);

		/* 16 bit length in little endian format */
		nSizePSI = sizeof(g_tblBin);
		s5pc1xx_serial3_device.putc(nSizePSI & 0xff);
		s5pc1xx_serial3_device.putc(nSizePSI >> 8);

		printf("Sending PSI data!!!\n - Len = %d\n", nSizePSI);

		/* Data bytes of PSI */
		pDataPSI = g_tblBin;
		nCRC = 0;
		for (nCnt = 0; nCnt < nSizePSI ; nCnt++) {
			s5pc1xx_serial3_device.putc(*pDataPSI);
			nCRC ^= *pDataPSI++;
		}

		/* CRC of PSI */
		s5pc1xx_serial3_device.putc(nCRC);

		udelay(10*1000);	/* 10mec */

		/* Getting ACK */
		ack = uart_serial_pollc(&s5pc1xx_serial3_device, 5);

		if (ack == CRC_OK) {
			printf("PSI sending was sucessful\n");
		} else {
			printf("PSI sending was NOT sucessful\n"
				" - ack(0x%x)\n - nCRC(0x%x)\n", ack, nCRC);
			continue;
		}

		/* check Modem reaction */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET);
		do {
			dat = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
			dat &= (0x1 << 3);
		} while (dat);

		if (rOneDRAM_MAILBOX_AB != IPC_CP_READY_FOR_LOADING) {
			printf("OneDRAM is NOT initialized for Modem\n");
			printf("Modem downloading is failed!!!\n");
			return 1;
		}

		printf("Modem is ready for loading\n");

		/*
		 * Do not support full-booting sequence,
		 * Full support will be done by Kernel
		 */
		return 0;

		printf("OneDRAM is mailbox(expecting 0x21), "
			"rOneDRAM_MAILBOX_AB=%x\n",
			rOneDRAM_MAILBOX_AB);
		while (rOneDRAM_SEMAPHORE == CP_HAS_SEM) {
			printf("OneDRAM semaphore is NOT available");
			udelay(100*1000);
		}

		/* load cp image */
		printf("Now, load CP image and static EEP at B-bank of OneDRAM\n");
		printf("By pausing and load .fls[0x51000000] and .eep[0x51870000]\n");
		printf("images to memory through EJTAG\n");

		for (i = 0; i < 60; i++) {
			udelay(100 * 1000);
			printf("*");
		}
		printf("\n");

/*
		break;
		LoadCPImage();
*/

		printf("Thank you for loading\n");

		rOneDRAM_SEMAPHORE = CP_HAS_SEM;
		rOneDRAM_MAILBOX_BA = IPC_CP_IMG_LOADED;

		/* check Modem reaction */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET);
		do {
			dat = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
			dat &= (0x1 << 3);
		} while (dat);

		if (rOneDRAM_MAILBOX_AB != IPC_CP_READY) {
			printf("Modem is NOT ready to boot\n");
			printf("Modem booting is failed!!!\n");
			return 1;
		}

		printf("Modem is done copying CP images\n");
		printf("OneDRAM is mailbox message(expecting 0x23), "
			"rOneDRAM_MAILBOX_AB=%x\n",
			rOneDRAM_MAILBOX_AB);
		while (rOneDRAM_SEMAPHORE == CP_HAS_SEM) {
			printf("OneDRAM semaphore is NOT available");
			udelay(100*1000);
		}

		rOneDRAM_SEMAPHORE = CP_HAS_SEM;

		rOneDRAM_MAILBOX_BA = IPC_BOOT_DONE;
		printf("Modem is now running\n");

		for (i = 0; i < 50; i++) {
			udelay(100 * 1000);
			if (rOneDRAM_SEMAPHORE == AP_HAS_SEM)
				printf("S");
			else
				printf("*");
		}
		printf("\n");

		rOneDRAM_MAILBOX_BA = 0x05; /* FIXME */

		for (i = 0; i < 50; i++) {
			udelay(100*1000);
			if (rOneDRAM_SEMAPHORE == AP_HAS_SEM)
				printf("S");
			else
				printf("*");
		}
		printf("\n");
		printf("Waiting for 5 secs for modem to respond\n");

		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET);
		for (i = 0; i < 50000; i++) {
			dat = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
			dat &= (0x1 << 3);
			if (dat == 0)
				break;
			udelay(100);
		}

		if (tmp == 1)
			printf("No response from Modem!!!\n");

		break;
	}

	return 0;
}

static void gpio_direction_output(int base, int offset, int value)
{
	int tmp;

	tmp = readl(base + S5PC1XX_GPIO_CON_OFFSET);
	tmp &= ~(GPIO_CON_MASK << (offset << 2));
	tmp |= (1 << (offset << 2));
	writel(tmp, base + S5PC1XX_GPIO_CON_OFFSET);

	tmp = readl(base + S5PC1XX_GPIO_DAT_OFFSET);
	tmp &= ~(GPIO_DAT_MASK << (offset << 0));
	tmp |= (value << (offset << 0));
	writel(tmp, base + S5PC1XX_GPIO_DAT_OFFSET);
}

static void gpio_set_value(int base, int offset, int value)
{
	int tmp;

	tmp = readl(base + S5PC1XX_GPIO_DAT_OFFSET);
	tmp &= ~(GPIO_DAT_MASK << (offset << 0));
	tmp |= (value << (offset << 0));
	writel(tmp, base + S5PC1XX_GPIO_DAT_OFFSET);
}

static void mdelay(int msec)
{
	int i;
	for (i = 0; i < msec; i++)
		udelay(1000);
}

static void tickertape_modem_on(void)
{
	int gpio_phone_on_base, gpio_phone_on;
	int gpio_cp_rst_base, gpio_cp_rst;

	printf("Tickertape phone on\n");

	gpio_phone_on_base = S5PC110_GPIO_BASE(S5PC110_GPIO_J1_OFFSET);
	gpio_phone_on = 0;
	gpio_cp_rst_base = S5PC110_GPIO_BASE(S5PC110_GPIO_H3_OFFSET);
	gpio_cp_rst = 7;

	gpio_direction_output(gpio_phone_on_base, gpio_phone_on, 0);
	gpio_direction_output(gpio_cp_rst_base, gpio_cp_rst, 0);

	gpio_set_value(gpio_cp_rst_base, gpio_cp_rst, 0);
	gpio_set_value(gpio_phone_on_base, gpio_phone_on, 1);
	mdelay(100);
	gpio_set_value(gpio_cp_rst_base, gpio_cp_rst, 1);
	mdelay(900);
	gpio_set_value(gpio_phone_on_base, gpio_phone_on, 0);
}

int do_modem(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *cmd;

	cmd = argv[1];

	switch (argc) {
	case 0:
	case 1:
		goto usage;

	case 2:
		if (strncmp(cmd, "on", 2) == 0) {
			if (machine_is_aquila())
				aquila_infineon_modem_on();
			if (machine_is_tickertape())
				tickertape_modem_on();
			return 0;
		}
		break;
	}

	return 0;
usage:
	cmd_usage(cmdtp);
	return 1;
}

void LoadCPImage(void)
{
#if 0
	/* load CP image */
	LoadPartition(PARTITION_ID_MODEM_OS , (UINT8 *)MODEM_OS_BASEADDR, 0);
	/* load CP EEP data */
	LoadValidEEP();
#endif
}

U_BOOT_CMD(modem,	CONFIG_SYS_MAXARGS,	1,	do_modem,
	"Modem init\n",
	"info [l[ayout]]"
		" - Display volume and ubi layout information\n"
);
