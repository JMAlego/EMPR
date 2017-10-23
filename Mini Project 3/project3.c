#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>
#include <math.h>

#define STAGE2

#define NIBBLE_TO_BINARY(byte)  \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define ALL_LEDS 0xB40000
#define LED1 0x040000
#define LED2 0x100000
#define LED3 0x200000
#define LED4 0x800000

#define LCD_ADDRESS 0x3b
#define EIGHT_SEG_ADDRESS 0x38
#define KEYPAD_ADDRESS 0x21

#define PI 3.1415926535

const char keypad[4][4] = {
  {'D','#','0','*'},
  {'C','9','8','7'},
  {'B','6','5','4'},
  {'A','3','2','1'}
};

unsigned char KEYPAD_ReadReset = 4;

volatile unsigned long SysTickCnt;

void SysTick_Handler (void);

void Delay (unsigned long tick);

void SysTick_Handler (void) {
  SysTickCnt++;
}

void Delay (unsigned long tick) {
  unsigned long systickcnt;
  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}

void setLedsWithChar(char led_vals){
  GPIO_ClearValue(1, ALL_LEDS);
  led_vals = led_vals & 0xf;
  if(led_vals & 0x1) GPIO_SetValue(1, LED1);
  if(led_vals & 0x2) GPIO_SetValue(1, LED2);
  if(led_vals & 0x4) GPIO_SetValue(1, LED3);
  if(led_vals & 0x8) GPIO_SetValue(1, LED4);
}

void initPrint(void){
  PINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = 1;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;

  PinCfg.Portnum = 0;
  PinCfg.Pinnum = 2;
  PINSEL_ConfigPin(&PinCfg);

  PinCfg.Pinnum = 3;
  PINSEL_ConfigPin(&PinCfg);

  UART_CFG_Type UARTCfg;
  UART_FIFO_CFG_Type FIFOCfg;
  UART_ConfigStructInit(&UARTCfg);
  UART_FIFOConfigStructInit(&FIFOCfg);

  UART_FIFOConfig(LPC_UART0, &FIFOCfg);
  UART_Init(LPC_UART0, &UARTCfg);

  UART_TxCmd(LPC_UART0, ENABLE);
}

unsigned int sizeofStr(char * string){
  unsigned int length = 0;
  while(string[length] != '\0')
    length++;
  return length + 1;
}

void print(char string[]){
  UART_Send(LPC_UART0, (uint8_t*) string, sizeofStr(string), BLOCKING);
}

Status I2C_SendBytes(unsigned char address, unsigned char * data, unsigned int data_length){
  I2C_M_SETUP_Type transferSetup;
  transferSetup.rx_data = NULL;
  transferSetup.retransmissions_max = 0;
  transferSetup.rx_length = 0;
  transferSetup.sl_addr7bit = (uint32_t) address;
  transferSetup.tx_data = data;
  transferSetup.tx_length = (uint32_t) data_length;
  return I2C_MasterTransferData(LPC_I2C1, &transferSetup, I2C_TRANSFER_POLLING);
}

Status I2C_ReceiveBytes(unsigned char address, unsigned char * data, unsigned int data_length){
  I2C_M_SETUP_Type transferSetup;
  transferSetup.rx_data = data;
  transferSetup.retransmissions_max = 0;
  transferSetup.rx_length = (uint32_t) data_length;
  transferSetup.sl_addr7bit = (uint32_t) address;
  transferSetup.tx_data = NULL;
  transferSetup.tx_length = 0;
  return I2C_MasterTransferData(LPC_I2C1, &transferSetup, I2C_TRANSFER_POLLING);
}

unsigned char KEYPAD_SeparateCols(uint8_t data){
  return (0xf0 & data) >> 4;
}

unsigned char KEYPAD_SeparateRows(uint8_t data){
  return 0x0f & data;
}

unsigned char KEYPAD_1HotToChar(uint8_t data){
  if(data == 0xe) return 0;
  else if(data == 0xd) return 1;
  else if(data == 0xb) return 2;
  else if(data == 0x7) return 3;
  else return 4;
}

void KEYPAD_WriteRow(char row){
  unsigned char data[1];
  row = row & 0x0f;
  if(row == 0) data[0] = 0xfe;
  else if(row == 1) data[0] = 0xfd;
  else if(row == 2) data[0] = 0xfb;
  else if(row == 3) data[0] = 0xf7;
  else data[0] = 0xff;
  I2C_SendBytes(KEYPAD_ADDRESS, data, 1);
}

char KEYPAD_ReadCol(){
  unsigned char data[1];
  I2C_ReceiveBytes(KEYPAD_ADDRESS, data, 1);
  return KEYPAD_1HotToChar(KEYPAD_SeparateCols(data[0]));
}

char KEYPAD_ReadKey(){
  char current_row = 0;
  char return_char = '\0';
  while(return_char == '\0'){
    KEYPAD_WriteRow(current_row);
    char col_result = KEYPAD_ReadCol();
    if(col_result == 4 && KEYPAD_ReadReset == current_row){
      KEYPAD_ReadReset = 4;
    }else if (KEYPAD_ReadReset == 4 && col_result != 4){
      return_char = keypad[(int) current_row][(int) col_result];
      KEYPAD_ReadReset = current_row;
    }
    current_row = (current_row + 1) % 4;
  }
  return return_char;
}

void initI2C(void){
  PINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = 3;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;

  PinCfg.Portnum = 0;
  PinCfg.Pinnum = 0;
  PINSEL_ConfigPin(&PinCfg);

  PinCfg.Pinnum = 1;
  PINSEL_ConfigPin(&PinCfg);

  I2C_Init(LPC_I2C1, 100000);

  I2C_Cmd(LPC_I2C1, ENABLE);
}

char charToNumChar(char numchar){
  if(numchar == '1') return 1;
  else if(numchar == '2') return 2;
  else if(numchar == '3') return 3;
  else if(numchar == '4') return 4;
  else if(numchar == '5') return 5;
  else if(numchar == '6') return 6;
  else if(numchar == '7') return 7;
  else if(numchar == '8') return 8;
  else if(numchar == '9') return 9;
  return 0;
}

long long stringToLongLong(char * string){
  int count = 0;
  long long output = 0;
  while(string[count] != '\0')
    count++;
  int i = 0;
  while(i < count){
    output += pow(10, count - i - 1) * charToNumChar(string[i]);
    i++;
  }
  return output;
}

void strcpy(char * to_string, char * from_string){
  int i = 0;
  while(from_string[i] != '\0'){
    to_string[i] = from_string[i];
    i++;
  }
  to_string[i] = '\0';
}

Status LCD_Init(void){
  unsigned char data[11] = {0x00,0x34,0x0c,0x06,0x35,0x04,0x10,0x42,0x9f,0x34,0x02};
  Status result = I2C_SendBytes(LCD_ADDRESS, data, 11);
  Delay(200);
  return result;
}

Status LCD_Write_Char(unsigned char character){
  unsigned char data[2];
  data[0] = 0x40;
  data[1] = character;
  return I2C_SendBytes(LCD_ADDRESS, data, 2);
}

Status LCD_Write_Chars(unsigned char * characters, int length){
  unsigned char data[length*2];
  int i;
  for(i = 0; i < length; i++){
    if(i == length - 1){
      data[i*2] = 0x40;
    }else{
      data[i*2] = 0xC0;
    }
    data[i*2 + 1] = characters[i];
  }
  return I2C_SendBytes(LCD_ADDRESS, data, length * 2);
}

Status LCD_Write_Address(unsigned char address){
  unsigned char data[2];
  data[0] = 0x00;
  data[1] = 0x80 | address;
  return I2C_SendBytes(LCD_ADDRESS, data, 2);
}

Status LCD_Clear_Display(void){
  unsigned char data[2];
  Status result = SUCCESS;
  data[0] = 0x00;
  data[1] = 0x08;
  if (!I2C_SendBytes(LCD_ADDRESS, data, 2))
    result = ERROR;
  //data[0] = 0x00;
  //data[1] = 0x06;
  //if (!I2C_SendBytes(LCD_ADDRESS, data, 2))
  //  result = ERROR;
  if (!LCD_Write_Address(0x00))
    result = ERROR;
  unsigned char chars[20] = {0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0};
  if (!LCD_Write_Chars(chars, 20))
    result = ERROR;
  if (!LCD_Write_Address(0x40))
    result = ERROR;
  if (!LCD_Write_Chars(chars, 20))
    result = ERROR;
  if (!LCD_Write_Address(0x00))
    result = ERROR;
  data[0] = 0x00;
  data[1] = 0x0C;
  if (!I2C_SendBytes(LCD_ADDRESS, data, 2))
    result = ERROR;
  Delay(200);
  return result;
}

char LCD_EncodeASCII(char character){
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

void LCD_EncodeASCIIString(char * string){
  int i = 0;
  while(string[i] != '\0'){
    string[i] = LCD_EncodeASCII(string[i]);
    i++;
  }
}

void initADC(){
  PINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = 1;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;

  PinCfg.Portnum = 0;
  PinCfg.Pinnum = 23;
  PINSEL_ConfigPin(&PinCfg);

  ADC_Init(LPC_ADC, 200000);
  ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
  ADC_BurstCmd(LPC_ADC,ENABLE);
  Delay(20);
}

void initDAC(){
  PINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = 2;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;

  PinCfg.Portnum = 0;
  PinCfg.Pinnum = 26;
  PINSEL_ConfigPin(&PinCfg);

  DAC_Init(LPC_DAC);
  Delay(20);
}

double ADC_To_Voltage(uint16_t adcOutput){
  return ((double) adcOutput)/((double) 4095) * 3;
}

uint16_t waveform_generator(double amplitude, uint32_t frequency){
  double peaktopeak = ((double) 2) * amplitude;
  double count = ((double) (SysTickCnt % 1000)) / ((double) 1000);
  count = count * (double) 2 * PI;
  double x = sin(count);
  x = x + (double) 1;
  x = x / (double) 2;
  x = peaktopeak * x;
  return (uint16_t) (x * (1023 / peaktopeak));
}

int main(){
  SysTick_Config(SystemCoreClock/1000 - 1);
  GPIO_SetDir(1, ALL_LEDS, 1);
  initPrint();
  initI2C();
  LCD_Init();

  #ifdef STAGE1

  initADC();
  double result;
  char toprint[50];
  result = ADC_To_Voltage(ADC_ChannelGetData(LPC_ADC, 0));
  sprintf(toprint, "%lf Volts\r\n", result);
  LCD_Clear_Display();
  print(toprint);
  LCD_EncodeASCIIString(toprint);
  LCD_Write_Chars((unsigned char *) toprint, sizeofStr(toprint) - 3);
  Delay(20);
  #endif

  #ifdef STAGE2

  initDAC();
  while(1){
    DAC_UpdateValue(LPC_DAC, waveform_generator(1.5, 1000));
    Delay(10);
  }

  #endif

  return 0;
}
