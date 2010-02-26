#include <common.h>
#include <asm/arch/gpio.h>
#include "s5p-spi.h"


static void cs_low(struct spi_platform_data *spi)
{
	gpio_set_value(spi->cs_bank, spi->cs_num, 0);
}

static void cs_high(struct spi_platform_data *spi)
{
	gpio_set_value(spi->cs_bank, spi->cs_num, 1);
}

static void clk_low(struct spi_platform_data *spi)
{
	gpio_set_value(spi->clk_bank, spi->clk_num, 0);
}

static void clk_high(struct spi_platform_data *spi)
{
	gpio_set_value(spi->clk_bank, spi->clk_num, 1);
}

static void si_low(struct spi_platform_data *spi)
{
	gpio_set_value(spi->si_bank, spi->si_num, 0);
}

static void si_high(struct spi_platform_data *spi)
{
	gpio_set_value(spi->si_bank, spi->si_num, 1);
}

/*
static char so_read(struct spi_platform_data *spi)
{
	return gpio_get_value(spi->so_bank, spi->so_num);
}
*/

void spi_write_byte(struct spi_platform_data *spi, unsigned char address, unsigned char command)
{
	int     j;
	unsigned short data;
	unsigned char DELAY = 1;

	data = (address << 8) + command;

	cs_high(spi);
	si_high(spi);
	clk_high(spi);
	udelay(DELAY);

	cs_low(spi);
	udelay(DELAY);

	for (j = PACKET_LEN; j >= 0; j--) {
		clk_low(spi);

		/* data high or low */
		if ((data >> j) & 0x1)
			si_high(spi);
		else
			si_low(spi);

		udelay(DELAY);

		clk_high(spi);
		udelay(DELAY);
	}

	cs_high(spi);
	udelay(DELAY);
}

#ifdef UNUSED_FUNCTIONS
unsigned char spi_read_byte(struct spi_platform_data *spi, unsigned char select, unsigned char address)
{
	int     j;
	static unsigned int first = 1;
	unsigned char DELAY = 1;
	unsigned short data = 0;
	char command = 0;

	data = (select << 8) + address;

	cs_high(spi);
	si_high(spi);
	clk_high(spi);
	udelay(DELAY);

	clk_low(spi);
	udelay(DELAY);

	for (j = PACKET_LEN + 8; j >= 0; j--) {

		if (j > 7) {
			clk_low(spi);

			/* data high or low */
			if ((data >> (j - 8)) & 0x1)
				si_high(spi);
			else
				si_low(spi);

			udelay(DELAY);
			clk_high(spi);
		} else {
			if (first) {
				gpio_cfg_pin(spi->so_bank, spi->so_num, GPIO_INPUT);
				first = 0;
			}

			clk_low(spi);

			if (so_read(spi) & 0x1)
				command |= 1 << j;
			else
				command |= 0 << j;

			udelay(DELAY);
			clk_high(spi);
		}

		udelay(DELAY);
	}

	cs_high(spi);
	udelay(DELAY);

	gpio_cfg_pin(spi->so_bank, spi->so_num, GPIO_OUTPUT);

	return command;
}
#endif
