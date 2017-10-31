#ifndef EMPR_LIB_UTILITIES_H
#define EMPR_LIB_UTILITIES_H

#include "empr_lib_common.h"
#include <math.h>

#define NIBBLE_TO_BINARY(byte)   \
  (byte & 0x08 ? '1' : '0'),     \
      (byte & 0x04 ? '1' : '0'), \
      (byte & 0x02 ? '1' : '0'), \
      (byte & 0x01 ? '1' : '0')

volatile uint64_t SysTickCnt;

void SysTick_Handler(void);
void Delay(uint32_t tick);
uint8_t EL_UTIL_ASCIINumberCharacterToNumber(uint8_t numchar);
uint32_t EL_UTIL_StringToUint32(uint8_t *string);
void EL_UTIL_ByteCopyWithNullTerminator(uint8_t *to_string, const uint8_t *from_string);

#endif
