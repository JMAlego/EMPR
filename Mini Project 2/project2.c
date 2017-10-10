#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>

#define NIBBLE_TO_BINARY(byte)  \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define STAGE1

#define ALL_LEDS 0xB40000
#define LED1 0x040000
#define LED2 0x100000
#define LED3 0x200000
#define LED4 0x800000

#define LCD_ADDRESS 0x00
#define KEYPAD_ADDRESS 0x21

const int leds[4] = {LED1, LED2, LED3, LED4};

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

void initPrint(void)
{
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

void initI2C(void)
{
  PINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = 3;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;

  PinCfg.Portnum = 0;
  PinCfg.Pinnum = 0;
  PINSEL_ConfigPin(&PinCfg);

  PinCfg.Pinnum = 1;
  PINSEL_ConfigPin(&PinCfg);

  I2C_Init(LPC_I2C1, 100);

  I2C_Cmd(LPC_I2C1, ENABLE);
}

unsigned int sizeofStr(char * string)
{
  unsigned int length = 0;
  while(string[length] != '\0')
    length++;
  return length + 1;
}

void print(char string[])
{
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

Status LCD_Init(void){
  unsigned char data[11] = {0x00,0x34,0x0c,0x06,0x35,0x04,0x10,0x42,0x9f,0x34,0x02};
  return I2C_SendBytes(LCD_ADDRESS, data, 11);
}

Status LCD_Clear_Display(void){
  unsigned char data[2];
  data[0] = 0x00;
  data[1] = 0x01;
  Status result = I2C_SendBytes(LCD_ADDRESS, data, 2);
  Delay(200);
  return result;
}

Status LCD_Write_Char(unsigned char character){
  unsigned char data[2];
  data[0] = 0x40;
  data[1] = character;
  return I2C_SendBytes(LCD_ADDRESS, data, 2);
}

Status LCD_Write_Address(unsigned char address){
  unsigned char data[2];
  data[0] = 0x00;
  data[1] = address;
  return I2C_SendBytes(LCD_ADDRESS, data, 2);
}

int main(void)
{
  SysTick_Config(SystemCoreClock/1000 - 1);
  GPIO_SetDir(1, ALL_LEDS, 1);
  initPrint();
  initI2C();

  #ifdef STAGE1

  print("Scanning I2C...\r\n");

  unsigned char nullByte[1] = "\0";
  unsigned char successfulAddresses[128];
  unsigned char addressCounter;
  unsigned char successes = 0;

  for(addressCounter = 0; addressCounter < 128; addressCounter++){
    Status result = I2C_SendBytes(addressCounter, nullByte, 1);
    if(result == SUCCESS){
      successfulAddresses[successes] = addressCounter;
      successes++;
    }
  }

  char strDevicesConnected[36];
  sprintf(strDevicesConnected, "%d devices connected to i2c bus\r\n", successes);
  print(strDevicesConnected);

  unsigned char index;
  for(index = 0; index < successes; index++){
    sprintf(strDevicesConnected, "I2C device at address 0x%x\r\n", successfulAddresses[index]);
    print(strDevicesConnected);
  }

  #endif

  #ifdef STAGE2

  LCD_Init();
  LCD_Clear_Display();
  LCD_Write_Address(0x80);
  LCD_Write_Char(0x64);

  #endif

  return 0;
}
