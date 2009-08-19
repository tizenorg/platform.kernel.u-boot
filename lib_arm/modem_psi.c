#include <asm/arch/gpio.h>
#include <asm/io.h>
#include <command.h>
#include <serial.h>
#include "psi_ram.h"

#define OneDRAM_B_BANK_BASEADDR		0x35000000

#define MODEM_OS_BASEADDR		OneDRAM_B_BANK_BASEADDR
#define MODEM_EEP_STATIC_BASEADDR	(MODEM_OS_BASEADDR + 0x870000)
#define MODEM_EEP_CAL_BAK_BASEADDR	(MODEM_OS_BASEADDR + 0x880000)
#define MODEM_EEP_DYNAMIC_BASEADDR	(MODEM_OS_BASEADDR + 0x890000)
#define MODEM_EEP_SECURITY_BASEADDR	(MODEM_OS_BASEADDR + 0x8A0000)

#define OneDRAMREG_BASEADDR		(OneDRAM_B_BANK_BASEADDR + 0xFFF800)

#define rOneDRAM_SEMAPHORE		*(volatile int*)(OneDRAMREG_BASEADDR)
#define rOneDRAM_MAILBOX_AB		*(volatile int*)(OneDRAMREG_BASEADDR + 0x08)
#define rOneDRAM_MAILBOX_BA		*(volatile int*)(OneDRAMREG_BASEADDR + 0x10)

//primitive IPC on OneDRAM
#define IPC_CP_READY_FOR_LOADING	0x21
#define IPC_CP_IMG_LOADED		0x22
#define IPC_CP_READY			0x23
#define IPC_BOOT_DONE			0x24

#define CP_BOOT_MODE_NORMAL		0x0a
#define CP_BOOT_MODE_PTEST		0x0b

#define CP_HAS_SEM			0x0
#define AP_HAS_SEM			0x1

extern int uart_serial_setbrg(unsigned int baudrate, int port);
extern int uart_serial_pollc(int retry, int port);
extern void uart_serial_putc(const char c, int port);
extern void uart_serial_puts(const char *s, int port);

int do_modem(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned int con,dat,pud,exit=0;
	unsigned int pin;
	char *s;
	int nTCnt, nATCnt, nCnt;
	char * pDataPSI;
	int nCoreVer, nCode, nSizePSI, nCRC;
	int ack, ind;	
	int i, tmp;
	int port = 3;

// 1. Modem gpio init
	// Phone_on
	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_J1_OFFSET);

	con = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	con &= ~(0xf << 0); /* 0 = 0 * 4 */
	con |= (0x1 << 0); /* Set to output (0x1) */
	writel(con, pin + S5PC1XX_GPIO_CON_OFFSET);

	pud = readl(pin + S5PC1XX_GPIO_PULL_OFFSET);
	pud &= ~(0x3 << 0); /* 0 = 0 * 2 */
	pud |= (0x0 << 0); /* Pull-up/down disabled */
	writel(pud, pin + S5PC1XX_GPIO_PULL_OFFSET);

	// CP_Reset
	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H3_OFFSET);

	con = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	con &= ~(0xf << 28); /* 28 = 7 * 4 */
	con |= (0x1 << 28); /* Set to output (0x1) */
	writel(con, pin + S5PC1XX_GPIO_CON_OFFSET);

	pud = readl(pin + S5PC1XX_GPIO_PULL_OFFSET);
	pud &= ~(0x3 << 14); /* 14 = 7 * 2 */
	pud |= (0x0 << 14); /* Pull-up/down disabled */
	writel(pud, pin + S5PC1XX_GPIO_PULL_OFFSET);

	// nINT_ONEDRAN_AP
	pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET);

	con = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
	con &= ~(0xf << 12); /* 12 = 3 * 4 */
	con |= (0x0 << 12); /* Set to input (0x0) */
	writel(con, pin + S5PC1XX_GPIO_CON_OFFSET);

	pud = readl(pin + S5PC1XX_GPIO_PULL_OFFSET);
	pud &= ~(0x3 << 6); /* 6 = 3 * 2 */
	pud |= (0x0 << 6); /* Pull-up/down disabled */
	writel(pud, pin + S5PC1XX_GPIO_PULL_OFFSET);

// 2. Uart3 init
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
	writel(0x0,0xE2900C08);
	writel(0x0,0xE2900C0C);
	/* 8N1 */
	writel(0x3,0xE2900C00); 
	/* No interrupts, no DMA, pure polling */
	writel(0x245,0xE2900C04); 
	uart_serial_setbrg(115200, port);

// 3. Modem reset
	while(1)	{
		char ch;

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

		/* PHONE_ON -> high */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_J1_OFFSET);
		dat = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
		dat |= (0x1 << 0);
		writel(dat, pin + S5PC1XX_GPIO_DAT_OFFSET);

		udelay(27*1000);	/* > 26.6ms */

		/* CP_RESET -> high */
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H3_OFFSET);
		dat = readl(pin + S5PC1XX_GPIO_DAT_OFFSET);
		dat |= (0x1 << 7);
		writel(dat, pin + S5PC1XX_GPIO_DAT_OFFSET);

		/* Power is stable, 40msec after CP_RESET release - experimental */
		udelay(40*1000);	/* 40ms */

		exit++;
		printf("exit = %x\n",exit);
		if(exit >= 0x10)
			break;

		/* Drain Rx Serial */
		tmp = 0;
		while (tmp != -1)
			tmp = uart_serial_pollc(5, port);

		for(nCnt = 0;nCnt < 20;nCnt++){
			uart_serial_puts("AT", port);
			nCoreVer = uart_serial_pollc(5, port);

			//if(nCoreVer >= 0xD1 && nCoreVer <= 0xD4)
			if(nCoreVer != -1)
				break;

			/* Send "AT" at 50ms internals till the bootcore version
			 * is received
			 */
			udelay(50*1000);	/* 50mec */
		}

		//if fail to receive Modem core version restart all process		
		if(nCnt == 20) continue;

		nCode = uart_serial_pollc(5, port);	//link establish code
		printf("Got Version code!! nCoreVer=%x nCode=%x\n,",nCoreVer,nCode);
		
		/* Drain Rx Serial */
		tmp = 0;
		while (tmp != -1)
			tmp = uart_serial_pollc(5, port);

		nSizePSI = sizeof(g_tblBin);

		/* INDICATION BYTE (0x30) */
		uart_serial_putc(0x30, port);

		/* 16 bit length in little endian format */
		uart_serial_putc(nSizePSI & 0xff, port);
		uart_serial_putc(nSizePSI >> 8, port);
		
		nCRC = 0;
		pDataPSI = g_tblBin;
		/* Data bytes of PSI */
		for(nCnt = 0; nCnt < nSizePSI ; nCnt++)
			{
			uart_serial_putc(*pDataPSI, port);
			nCRC ^= *pDataPSI++;
			}

		/* CRC of PSI */
		uart_serial_putc(nCRC, port);

		udelay(10*1000);	/* 10mec */
		ack = uart_serial_pollc(5, port);

		if( ack == 0x01) {
			printf("VALID CRC nCRC=%x\n\n",nCRC);
		}
		else {
			printf("CRC NOT VALID!!\n");
			continue;
		}
		
		//check Modem reaction
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET);
		do{
			dat = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
			dat &= (0x1 << 3);
		}while (dat);

		if(rOneDRAM_MAILBOX_AB != IPC_CP_READY_FOR_LOADING){
			printf("OneDRAM is NOT initialized for Modem\n");
			printf("Modem downloading is failed!!!\n");
			return 1;
		}

		printf("Modem is ready for loading\n");
		printf("OneDRAM is mailbox(expecting 0x21), rOneDRAM_MAILBOX_AB=%x\n",
			rOneDRAM_MAILBOX_AB);
		while(rOneDRAM_SEMAPHORE == CP_HAS_SEM) {
			printf("OneDRAM semaphore is NOT available");
			udelay(100*1000);
		}
			
		//load cp image
		printf("Now, load CP image and static EEP at B-bank of OneDRAM\n");
		printf("By pausing and load .fls[0x51000000] and .eep[0x51870000]\n");
		printf("images to memory through EJTAG\n");

		for(i=0;i<60;i++) {
			udelay(100*1000);
			printf("*");
		}
		printf("\n");
			
//		break;
//		LoadCPImage();
//
		printf("Thank you for loading\n");

		rOneDRAM_SEMAPHORE = CP_HAS_SEM;
		rOneDRAM_MAILBOX_BA = IPC_CP_IMG_LOADED;

		//check Modem reaction
		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET);
		do{
			dat = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
			dat &= (0x1 << 3);
		}while (dat);

		if(rOneDRAM_MAILBOX_AB != IPC_CP_READY){
			printf("Modem is NOT ready to boot\n");
			printf("Modem booting is failed!!!\n");
			return 1;
		}

		printf("Modem is done copying CP images\n");
		printf("OneDRAM is mailbox message(expecting 0x23), rOneDRAM_MAILBOX_AB=%x\n",
			rOneDRAM_MAILBOX_AB);
		while(rOneDRAM_SEMAPHORE == CP_HAS_SEM) {
			printf("OneDRAM semaphore is NOT available");
			udelay(100*1000);
		}
			
		rOneDRAM_SEMAPHORE = CP_HAS_SEM;

		rOneDRAM_MAILBOX_BA = IPC_BOOT_DONE;
		printf("Modem is now running\n");

		for(i=0;i<50;i++) {
			udelay(100*1000);
			if(rOneDRAM_SEMAPHORE == AP_HAS_SEM)
				printf("S");
			else
				printf("*");
		}
		printf("\n");

		rOneDRAM_MAILBOX_BA = 0x05; //FIXME

		for(i=0;i<50;i++) {
			udelay(100*1000);
			if(rOneDRAM_SEMAPHORE == AP_HAS_SEM)
				printf("S");
			else
				printf("*");
		}
		printf("\n");
		printf("Waiting for 5 secs for modem to respond\n");	

		pin = S5PC110_GPIO_BASE(S5PC110_GPIO_H1_OFFSET);
		for(i=0;i<50000;i++){
			dat = readl(pin + S5PC1XX_GPIO_CON_OFFSET);
			dat &= (0x1 << 3);
			if(dat == 0)
				break;
			udelay(100);
		}

		if( tmp == 1)
			printf("No response from Modem!!!\n");

		break;
	}
}
void LoadCPImage()
{
	// load CP image
//	LoadPartition(PARTITION_ID_MODEM_OS , (UINT8 *)MODEM_OS_BASEADDR, 0);
	// load CP EEP data
//	LoadValidEEP();
}

U_BOOT_CMD(modem, 6, 1, do_modem,
	"Infineon Modem init\n",
	"info [l[ayout]]"
		" - Display volume and ubi layout information\n"
);

