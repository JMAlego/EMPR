#ifndef EMPR_LIB_LCD_H
#define EMPR_LIB_LCD_H

#include "empr_lib_common.h"
#include "empr_lib_i2c_addresses.h"
#include "empr_lib_i2c.h"
#include "empr_lib_utilities.h"

Status EL_LCD_Init(void);
Status EL_LCD_WriteChar(unsigned char character);
Status EL_LCD_WriteChars(uint8_t * characters, size_t length);
Status EL_LCD_WriteAddress(uint8_t address);
Status EL_LCD_ClearDisplay(void);
uint8_t EL_LCD_EncodeASCII(uint8_t character);
void EL_LCD_EncodeASCIIString(uint8_t * string);

#endif
