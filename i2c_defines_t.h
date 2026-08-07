//i2c_defines.h
#ifndef _I2C_DEFINES_H_
#define _I2C_DEFINES_H_

// Pin select
#define SCL_0_2   (1<<4)
#define SDA_0_3   (1<<6)

// Clock settings
#define FOSC      12000000
#define CCLK      (5*FOSC)
#define PCLK      (CCLK/4)

#define I2C_SPEED 100000

#define I2C_DIVIDER ((PCLK/I2C_SPEED)/2)

// I2CONSET bits
#define AA_BIT  2
#define SI_BIT  3
#define STO_BIT 4
#define STA_BIT 5
#define I2EN_BIT 6

// I2CONCLR bits
#define AAC_BIT  2
#define SIC_BIT  3
#define STAC_BIT 5
#define I2ENC_BIT 6

#endif

