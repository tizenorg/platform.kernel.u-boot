
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

#define PACKET_LEN		8

struct spi_platform_data {
	struct s5pc1xx_gpio_bank *cs_bank;
	struct s5pc1xx_gpio_bank *clk_bank;
	struct s5pc1xx_gpio_bank *si_bank;
	struct s5pc1xx_gpio_bank *so_bank;

	unsigned int cs_num;
	unsigned int clk_num;
	unsigned int si_num;
	unsigned int so_num;

	unsigned int set_rev;
};

