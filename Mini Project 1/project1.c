#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>

#define NIBBLE_TO_BINARY(byte)  \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define DEMO

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
#if defined(STAGE1) || defined(STAGE2)
  char counter = 0;
  char i = 0;
#endif

  SysTick_Config(SystemCoreClock/1000 - 1);

  GPIO_SetDir(1, ALL_LEDS, 1);

#ifdef STAGE1
  //STAGE1
  for(counter = 0; counter < 2; counter++){
    for(i = 0; i < 4; i++){
      GPIO_SetValue(1, leds[i]);
      Delay(1000);
      GPIO_ClearValue(1, leds[i]);
    }
  }Delay(1000);
#endif
#ifdef STAGE2
  //STAGE2
  for(counter = 0; counter < 2; counter++){
    for(i = 0; i < 16; i++){
      setLedsWithChar(i);ART
      Delay(1000);
    }
  }
#endif
#ifdef STAGE3
  initPrint();
  print("Hello World");
#endif

#ifdef DEMO
  initPrint();

  char buffer[50];
  char i;
  print("Starting count\r\n");
  for(i = 0; i < 16; i++){
    setLedsWithChar(i);
    sprintf(buffer, "Decimal: %i, Hex: 0x%x, Binary: %c%c%c%c\r\n", i, i, NIBBLE_TO_BINARY(i));
    print(buffer);
    Delay(1000);
  }
  print("Finished count\r\n");
#endif

  return 0;
}
