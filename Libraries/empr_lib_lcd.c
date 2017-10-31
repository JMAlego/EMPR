#ifndef EMPR_LIB_LCD_C
#define EMPR_LIB_LCD_C

#include "empr_lib_lcd.h"

Status EL_LCD_Init(void){
  uint8_t data[11] = {0x00,0x34,0x0c,0x06,0x35,0x04,0x10,0x42,0x9f,0x34,0x02};
  Status result = EL_I2C_SendBytes(EMPR_LIB_I2C_ADDRESS_LCD, data, 11);
  Delay(200);
  return result;
}

Status EL_LCD_WriteChar(uint8_t character){
  uint8_t data[2];
  data[0] = 0x40;
  data[1] = character;
  return EL_I2C_SendBytes(EMPR_LIB_I2C_ADDRESS_LCD, data, 2);
}

Status EL_LCD_WriteChars(uint8_t * characters, size_t length){
  uint8_t data[length*2];
  size_t i;
  for(i = 0; i < length; i++){
    if(i == length - 1){
      data[i*2] = 0x40;
    }else{
      data[i*2] = 0xC0;
    }
    data[i*2 + 1] = characters[i];
  }
  return EL_I2C_SendBytes(EMPR_LIB_I2C_ADDRESS_LCD, data, length * 2);
}

Status EL_LCD_WriteAddress(uint8_t address){
  uint8_t data[2];
  data[0] = 0x00;
  data[1] = 0x80 | address;
  return EL_I2C_SendBytes(EMPR_LIB_I2C_ADDRESS_LCD, data, 2);
}

Status EL_LCD_ClearDisplay(void){
  uint8_t data[2];
  Status result = SUCCESS;
  data[0] = 0x00;
  data[1] = 0x08;
  if (!EL_I2C_SendBytes(EMPR_LIB_I2C_ADDRESS_LCD, data, 2))
    result = ERROR;
  if (!EL_LCD_WriteAddress(0x00))
    result = ERROR;
  unsigned char chars[20] = {0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0};
  if (!EL_LCD_WriteChars(chars, 20))
    result = ERROR;
  if (!EL_LCD_WriteAddress(0x40))
    result = ERROR;
  if (!EL_LCD_WriteChars(chars, 20))
    result = ERROR;
  if (!EL_LCD_WriteAddress(0x00))
    result = ERROR;
  data[0] = 0x00;
  data[1] = 0x0C;
  if (!EL_I2C_SendBytes(EMPR_LIB_I2C_ADDRESS_LCD, data, 2))
    result = ERROR;
  Delay(200);
  return result;
}

uint8_t EL_LCD_EncodeASCII(uint8_t character){
  if(character > 31 && character < 64){
    return character + 128;
  }
  if(character == '@') return 0x80;
  if(character > 64 && character < 91){
    return character + 128;
  }
  if(character > 96 && character < 123){
    return character + 128;
  }

  return 0x56;
}

void EL_LCD_EncodeASCIIString(uint8_t * string){
  size_t i = 0;
  while(string[i] != '\0'){
    string[i] = EL_LCD_EncodeASCII(string[i]);
    i++;
  }
}

#endif
