//i2c_peripheral.c
#include <LPC21xx.h>
#include "types_t.h"
#include "i2c_defines_t.h"

void init_i2c(void)
{
    PINSEL0 |= SCL_0_2 | SDA_0_3;

    I2SCLH = I2C_DIVIDER;
    I2SCLL = I2C_DIVIDER;

    I2CONSET = (1 << I2EN_BIT);
}

void i2c_start(void)
{
    I2CONSET = (1 << STA_BIT);

    while(((I2CONSET >> SI_BIT) & 1) == 0);

    I2CONCLR = (1 << STAC_BIT);
}

void i2c_restart(void)
{
    I2CONSET = (1 << STA_BIT);
    I2CONCLR = (1 << SIC_BIT);

    while(((I2CONSET >> SI_BIT) & 1) == 0);

    I2CONCLR = (1 << STAC_BIT);
}

void i2c_stop(void)
{
    I2CONSET = (1 << STO_BIT);
    I2CONCLR = (1 << SIC_BIT);
}

void i2c_write(u8 dat)
{
    I2DAT = dat;

    I2CONCLR = (1 << SIC_BIT);

    while(((I2CONSET >> SI_BIT) & 1) == 0);
}

u8 i2c_nack(void)
{
    I2CONCLR = (1 << SIC_BIT);

    while(((I2CONSET >> SI_BIT) & 1) == 0);

    return I2DAT;
}

u8 i2c_mack(void)
{
    I2CONSET = (1 << AA_BIT);

    I2CONCLR = (1 << SIC_BIT);

    while(((I2CONSET >> SI_BIT) & 1) == 0);

    I2CONCLR = (1 << AA_BIT);

    return I2DAT;
}

