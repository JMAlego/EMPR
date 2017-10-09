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
    I2C_M_SETUP_Type transferSetup;
    transferSetup.rx_data = NULL;
    transferSetup.retransmissions_max = 0;
    transferSetup.rx_length = 0;
    transferSetup.sl_addr7bit = (uint32_t) addressCounter;
    transferSetup.tx_data = nullByte;
    transferSetup.tx_length = 1;
    Status result = I2C_MasterTransferData(LPC_I2C1, &transferSetup, I2C_TRANSFER_POLLING);
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

  return 0;
}
