#ifndef EMPR_LIB_UTILITIES_C
#define EMPR_LIB_UTILITIES_C

#include "empr_lib_utilities.h"

void SysTick_Handler(void)
{
  SysTickCnt++;
}

void Delay(uint32_t tick)
{
  uint64_t systickcnt;
  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick)
    ;
}

uint8_t EL_UTIL_ASCIINumberCharacterToNumber(uint8_t numchar)
{
  if (numchar == '1')
    return 1;
  else if (numchar == '2')
    return 2;
  else if (numchar == '3')
    return 3;
  else if (numchar == '4')
    return 4;
  else if (numchar == '5')
    return 5;
  else if (numchar == '6')
    return 6;
  else if (numchar == '7')
    return 7;
  else if (numchar == '8')
    return 8;
  else if (numchar == '9')
    return 9;
  return 0;
}

uint32_t EL_UTIL_StringToUint32(uint8_t *string)
{
  size_t count = 0;
  uint32_t output = 0;
  while (string[count] != '\0')
    count++;
  size_t i = 0;
  while (i < count)
  {
    output += pow(10, count - i - 1) * (double)EL_UTIL_ASCIINumberCharacterToNumber(string[i]);
    i++;
  }
  return output;
}

uint64_t EL_UTIL_StringToUint64(uint8_t *string)
{
  size_t count = 0;
  uint64_t output = 0;
  while (string[count] != '\0')
    count++;
  size_t i = 0;
  while (i < count)
  {
    output += pow(10, count - i - 1) * (double)EL_UTIL_ASCIINumberCharacterToNumber(string[i]);
    i++;
  }
  return output;
}

void EL_UTIL_ByteCopyWithNullTerminator(uint8_t *to_string, const uint8_t *from_string)
{
  size_t i = 0;
  while (from_string[i] != '\0')
  {
    to_string[i] = from_string[i];
    i++;
  }
  to_string[i] = '\0';
}

#endif
