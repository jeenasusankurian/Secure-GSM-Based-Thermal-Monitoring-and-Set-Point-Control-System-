//i2c_eeprom.h
#ifndef _I2C_EEPROM_H_
#define _I2C_EEPROM_H_

#include "types_t.h"

void i2c_eeprom_bytewrite(u8 slaveAddr, u16 memAddr, u8 data);

u8 i2c_eeprom_randomread(u8 slaveAddr, u16 memAddr);

void i2c_eeprom_pagewrite(u8 slaveAddr, u16 memAddr, char *data, u8 len);

void i2c_eeprom_seqread(u8 slaveAddr, u16 memAddr, char *data, u8 len);

#endif

