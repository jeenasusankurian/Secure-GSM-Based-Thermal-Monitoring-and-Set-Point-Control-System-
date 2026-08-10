//i2c_eeprom.c                         // Source file containing I2C EEPROM communication functions

#include "i2c_t.h"                     // Header file containing I2C driver function declarations
#include "i2c_eeprom_t.h"              // Header file containing EEPROM related function declarations
#include "delay_t.h"                   // Header file containing delay function declarations



// Function to write a single byte of data into EEPROM
void i2c_eeprom_bytewrite(u8 slaveAddr, u16 memAddr, u8 data)
{
    i2c_start();                       // Generate I2C START condition to begin communication


    i2c_write(slaveAddr << 1);         // Send EEPROM slave address with Write bit (R/W = 0)


    i2c_write(memAddr >> 8);           // Send higher byte of memory address
                                       // Required for EEPROMs having 16-bit addressing


    i2c_write(memAddr & 0xFF);         // Send lower byte of memory address


    i2c_write(data);                   // Send the data byte to be written into EEPROM


    i2c_stop();                        // Generate I2C STOP condition to end communication


    delay_ms(10);                      // Wait for EEPROM internal write cycle completion
}




// Function to read a single byte from EEPROM at a specific memory address
u8 i2c_eeprom_randomread(u8 slaveAddr, u16 memAddr)
{
    u8 data;                           // Variable to store received EEPROM data


    i2c_start();                       // Generate START condition


    i2c_write(slaveAddr << 1);         // Send EEPROM slave address with Write operation
                                       // Write operation is required to send memory address


    i2c_write(memAddr >> 8);            // Send higher byte of EEPROM memory address


    i2c_write(memAddr & 0xFF);          // Send lower byte of EEPROM memory address


    i2c_restart();                     // Generate repeated START condition
                                       // Used to change direction from write to read


    i2c_write((slaveAddr << 1) | 1);   // Send EEPROM slave address with Read bit (R/W = 1)


    data = i2c_nack();                 // Read one byte from EEPROM and send NACK
                                       // NACK indicates end of reading


    i2c_stop();                        // Generate STOP condition


    return data;                       // Return received EEPROM data
}




// Function to write multiple bytes into EEPROM (Page Write)
void i2c_eeprom_pagewrite(u8 slaveAddr, u16 memAddr, char *data, u8 len)
{
    u8 i;                              // Loop counter


    i2c_start();                       // Generate START condition


    i2c_write(slaveAddr << 1);         // Send EEPROM slave address with Write bit


    i2c_write(memAddr >> 8);           // Send higher byte of EEPROM memory address


    i2c_write(memAddr & 0xFF);         // Send lower byte of EEPROM memory address


    for(i = 0; i < len; i++)           // Repeat until all bytes are transmitted
    {
        i2c_write(data[i]);             // Write each byte of data into EEPROM
    }


    i2c_stop();                        // Generate STOP condition


    delay_ms(10);                      // Wait for EEPROM write cycle completion
}




// Function to read multiple bytes sequentially from EEPROM
void i2c_eeprom_seqread(u8 slaveAddr, u16 memAddr, char *data, u8 len)
{
    u8 i;                              // Loop counter


    i2c_start();                       // Generate START condition


    i2c_write(slaveAddr << 1);         // Send EEPROM slave address with Write bit
                                       // Required to send memory address first


    i2c_write(memAddr >> 8);           // Send higher byte of starting memory address


    i2c_write(memAddr & 0xFF);         // Send lower byte of starting memory address


    i2c_restart();                     // Generate repeated START condition
                                       // Switch from write mode to read mode


    i2c_write((slaveAddr << 1) | 1);   // Send EEPROM slave address with Read bit


    for(i = 0; i < len - 1; i++)       // Read bytes except the last byte
    {
        data[i] = i2c_mack();          // Read byte and send ACK
                                       // ACK tells EEPROM that more data is required
    }


    data[i] = i2c_nack();              // Read final byte and send NACK
                                       // NACK indicates reading is completed


    i2c_stop();                        // Generate STOP condition and end communication
}
