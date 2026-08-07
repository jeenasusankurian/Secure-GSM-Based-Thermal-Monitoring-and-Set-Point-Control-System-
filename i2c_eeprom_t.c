//i2c_eeprom.c

#include "i2c_t.h"
#include "i2c_eeprom_t.h"
#include "delay_t.h"

void i2c_eeprom_bytewrite(u8 slaveAddr, u16 memAddr, u8 data)
{
    i2c_start();

    i2c_write(slaveAddr << 1);

    i2c_write(memAddr >> 8);
    i2c_write(memAddr & 0xFF);

    i2c_write(data);

    i2c_stop();

    delay_ms(10);
}

u8 i2c_eeprom_randomread(u8 slaveAddr, u16 memAddr)
{
    u8 data;

    i2c_start();

    i2c_write(slaveAddr << 1);

    i2c_write(memAddr >> 8);
    i2c_write(memAddr & 0xFF);

    i2c_restart();

    i2c_write((slaveAddr << 1) | 1);

    data = i2c_nack();

    i2c_stop();

    return data;
}

void i2c_eeprom_pagewrite(u8 slaveAddr, u16 memAddr, char *data, u8 len)
{
    u8 i;

    i2c_start();

    i2c_write(slaveAddr << 1);

    i2c_write(memAddr >> 8);
    i2c_write(memAddr & 0xFF);

    for(i = 0; i < len; i++)
    {
        i2c_write(data[i]);
    }

    i2c_stop();

    delay_ms(10);
}

void i2c_eeprom_seqread(u8 slaveAddr, u16 memAddr, char *data, u8 len)
{
    u8 i;

    i2c_start();

    i2c_write(slaveAddr << 1);

    i2c_write(memAddr >> 8);
    i2c_write(memAddr & 0xFF);

    i2c_restart();

    i2c_write((slaveAddr << 1) | 1);

    for(i = 0; i < len - 1; i++)
    {
        data[i] = i2c_mack();
    }

    data[i] = i2c_nack();

    i2c_stop();
}


