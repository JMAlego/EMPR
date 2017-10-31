#ifndef EMPR_LIB_SERIAL_H
#define EMPR_LIB_SERIAL_H

#include "empr_lib_common.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>

void EL_SERIAL_Init(void);
size_t EL_SERIAL_SizeOfString(uint8_t string[]);
void EL_SERIAL_Print(uint8_t string[]);

#endif
