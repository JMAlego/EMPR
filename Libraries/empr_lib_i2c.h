#ifndef EMPR_LIB_I2C_H
#define EMPR_LIB_I2C_H

#include "empr_lib_common.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"

#define EMPR_LIB_I2C_RETRANSMISSIONS_MAX 0
#define EMPR_LIB_I2C_DATA_RATE 100000

void EL_I2C_Init(void);
Status EL_I2C_SendBytes(uint8_t address, uint8_t *data, size_t data_length);
Status EL_I2C_ReceiveBytes(uint8_t address, uint8_t *data, size_t data_length);

#endif
