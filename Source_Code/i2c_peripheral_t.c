//i2c_peripheral.c

#include <LPC21xx.h>          // LPC21xx microcontroller register definitions
#include "types_t.h"           // User defined data types (u8, u16 etc.)
#include "i2c_defines_t.h"     // I2C pin definitions and bit positions


//================ I2C INITIALIZATION =================
// Function : init_i2c()
// Purpose  : Configure I2C pins and enable I2C peripheral
void init_i2c(void)
{
    // Configure P0.2 as SCL0 and P0.3 as SDA0 using PINSEL0 register
    PINSEL0 |= SCL_0_2 | SDA_0_3;

    // Set I2C clock high period value
    I2SCLH = I2C_DIVIDER;

    // Set I2C clock low period value
    I2SCLL = I2C_DIVIDER;

    // Enable I2C peripheral
    I2CONSET = (1 << I2EN_BIT);
}


//================ I2C START CONDITION =================
// Function : i2c_start()
// Purpose  : Generate START condition on I2C bus
void i2c_start(void)
{
    // Set START bit to generate start condition
    I2CONSET = (1 << STA_BIT);

    // Wait until START condition is transmitted
    while(((I2CONSET >> SI_BIT) & 1) == 0);

    // Clear START bit after transmission
    I2CONCLR = (1 << STAC_BIT);
}


//================ I2C RESTART CONDITION =================
// Function : i2c_restart()
// Purpose  : Generate repeated START condition
void i2c_restart(void)
{
    // Set START bit again for repeated start
    I2CONSET = (1 << STA_BIT);

    // Clear interrupt flag before starting operation
    I2CONCLR = (1 << SIC_BIT);

    // Wait until repeated START is completed
    while(((I2CONSET >> SI_BIT) & 1) == 0);

    // Clear START condition bit
    I2CONCLR = (1 << STAC_BIT);
}


//================ I2C STOP CONDITION =================
// Function : i2c_stop()
// Purpose  : Release I2C bus by generating STOP condition
void i2c_stop(void)
{
    // Set STOP bit to generate stop condition
    I2CONSET = (1 << STO_BIT);

    // Clear interrupt flag
    I2CONCLR = (1 << SIC_BIT);
}


//================ I2C BYTE WRITE =================
// Function : i2c_write()
// Purpose  : Send one byte of data/address through I2C bus
void i2c_write(u8 dat)
{
    // Load data into I2C data register
    I2DAT = dat;

    // Clear SI flag to start transmission
    I2CONCLR = (1 << SIC_BIT);

    // Wait until transmission is completed
    while(((I2CONSET >> SI_BIT) & 1) == 0);
}


//================ I2C BYTE READ WITH NACK =================
// Function : i2c_nack()
// Purpose  : Receive last byte from slave
void i2c_nack(void)
{
    // Clear SI flag to start receiving
    I2CONCLR = (1 << SIC_BIT);

    // Wait until reception is completed
    while(((I2CONSET >> SI_BIT) & 1) == 0);

    // Return received data byte
    return I2DAT;
}


//================ I2C BYTE READ WITH ACK =================
// Function : i2c_mack()
// Purpose  : Receive byte and send ACK for next byte
u8 i2c_mack(void)
{
    // Enable acknowledge bit
    I2CONSET = (1 << AA_BIT);

    // Clear SI flag
    I2CONCLR = (1 << SIC_BIT);

    // Wait until data reception is completed
    while(((I2CONSET >> SI_BIT) & 1) == 0);

    // Disable acknowledge after receiving required byte
    I2CONCLR = (1 << AA_BIT);

    // Return received byte
    return I2DAT;
}
