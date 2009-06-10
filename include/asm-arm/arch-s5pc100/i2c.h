
/* I2C */
#define S5P_PA_I2C		S5P_ADDR(0x0c100000)
#define I2Cx_OFFSET(x)		(S5P_PA_I2C + x * 0x100000)
#define S5P_I2C_BASE		S5P_PA_I2C

#ifndef __ASSEMBLY__
typedef struct s5pc1xx_i2c {
	volatile unsigned long	IICCON;
	volatile unsigned long	IICSTAT;
	volatile unsigned long	IICADD;
	volatile unsigned long	IICDS;
} s5pc1xx_i2c_t;
#endif

