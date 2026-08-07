//i2c.h
#ifndef _I2C_H_
#define _I2C_H_

#include "types_t.h"

void init_i2c(void);

void i2c_start(void);
void i2c_restart(void);
void i2c_stop(void);

void i2c_write(u8 dat);

u8 i2c_nack(void);
u8 i2c_mack(void);

#endif


