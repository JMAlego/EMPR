#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_pwm.h"
#include "lpc17xx_rit.h"
#include <stdio.h>
#include <math.h>

#define STAGE5

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
#define UNKNOWN1 0x20
#define UNKNOWN2 0x50

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
void RIT_IRQHandler(void);

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

char KEYPAD_CheckKey(){
  char current_row = 0;
  char return_char = '\0';
  while(current_row < 4){
    KEYPAD_WriteRow(current_row);
    char col_result = KEYPAD_ReadCol();
    if(col_result == 4 && KEYPAD_ReadReset == current_row){
      KEYPAD_ReadReset = 4;
    }else if (KEYPAD_ReadReset == 4 && col_result != 4){
      return_char = keypad[(int) current_row][(int) col_result];
      KEYPAD_ReadReset = current_row;
    }
    current_row++;
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
  PinCfg.Pinnum = 24;
  PINSEL_ConfigPin(&PinCfg);

  ADC_Init(LPC_ADC, 200000);
  ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
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

void initPWM(uint8_t channel, int start, int reset){
  if(channel>6 || channel < 1){
      print("PWM channel must be >0 and <7\r\n");
      return;
  }
  PINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = 1;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;

  PinCfg.Portnum = 2;
  PinCfg.Pinnum = channel-1;
  PINSEL_ConfigPin(&PinCfg);

  PWM_TIMERCFG_Type PWMCfgDat;
  PWM_MATCHCFG_Type PWMMatchCfgDat;
  PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
  PWMCfgDat.PrescaleValue = 1;
  PWM_Init(LPC_PWM1, PWM_MODE_TIMER, (void *) &PWMCfgDat);
  PWM_MatchUpdate(LPC_PWM1, 0, reset, PWM_MATCH_UPDATE_NOW);
  PWMMatchCfgDat.IntOnMatch = DISABLE;
  PWMMatchCfgDat.MatchChannel = (channel-1)*2;
  PWMMatchCfgDat.ResetOnMatch = ENABLE;
  PWMMatchCfgDat.StopOnMatch = DISABLE;
  PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
  //PWM_ChannelConfig(LPC_PWM1, 1, PWM_CHANNEL_SINGLE_EDGE);
  PWM_MatchUpdate(LPC_PWM1, 1, start, PWM_MATCH_UPDATE_NOW);
  PWMMatchCfgDat.IntOnMatch = DISABLE;
  PWMMatchCfgDat.MatchChannel = ((channel-1)*2 + 1);
  PWMMatchCfgDat.ResetOnMatch = DISABLE;
  PWMMatchCfgDat.StopOnMatch = DISABLE;
  PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
  PWM_ChannelCmd(LPC_PWM1, 1, ENABLE);
  PWM_ResetCounter(LPC_PWM1);
  PWM_CounterCmd(LPC_PWM1, ENABLE);
  PWM_Cmd(LPC_PWM1, ENABLE);
}

void setPWMStart(uint8_t channel, int start){
  PWM_MATCHCFG_Type PWMMatchCfgDat;
  PWM_MatchUpdate(LPC_PWM1, 1, start, PWM_MATCH_UPDATE_NOW);
  PWMMatchCfgDat.IntOnMatch = DISABLE;
  PWMMatchCfgDat.MatchChannel = ((channel-1)*2 + 1);
  PWMMatchCfgDat.ResetOnMatch = DISABLE;
  PWMMatchCfgDat.StopOnMatch = DISABLE;
  PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
}

double ADC_To_Voltage(uint16_t adcOutput){
  return ((double) adcOutput)/((double) 4095) * 3;
}

uint16_t waveform_generator(double amplitude, double frequency){
  double peaktopeak = ((double) 2) * amplitude;
  int period = (int)(1000 * ((double) 1 / frequency));
  double count = ((double) (SysTickCnt % period)) / ((double) period);
  count = count * (double) 2 * PI;
  double x = sin(count);
  x = x + (double) 1;
  x = x / (double) 2;
  x = peaktopeak * x;
  return (uint16_t) (x * (1023 / 3.33));
}

double SENSOR_VoltageToDistance(double voltage){
  return (1.72955 * pow(voltage, 4)) - (19.0498 * pow(voltage, 3)) + (76.172 * pow(voltage, 2)) - (136.629 * voltage) + 104.683;
}

void SENSOR_Calibrate(double samples[]){
  print("CALIBRATION MODE SELECTED\r\n");
  print("When prompted, place an object at the distance specified\r\n");
  print("Then push any key on the keypad to continue\r\n");

  double raw_adc;
  double voltage;
  //uint8_t i;
  char output[20];

  print("80cm\r\n");
  KEYPAD_ReadKey();
  raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
  voltage = raw_adc / 4096 * 3.3;
  samples[0] = voltage;
  sprintf(output, "READ:%lfv\r\n", voltage);
  print(output);

  print("40cm\r\n");
  KEYPAD_ReadKey();
  raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
  voltage = raw_adc / 4096 * 3.3;
  samples[1] = voltage;
  sprintf(output, "READ:%lfv\r\n", voltage);
  print(output);

  print("10cm\r\n");
  KEYPAD_ReadKey();
  raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
  voltage = raw_adc / 4096 * 3.3;
  samples[2] = voltage;
  sprintf(output, "READ:%lfv\r\n", voltage);
  print(output);

  print("7cm\r\n");
  KEYPAD_ReadKey();
  raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
  voltage = raw_adc / 4096 * 3.3;
  samples[3] = voltage;
  sprintf(output, "READ:%lfv\r\n", voltage);
  print(output);

  print("5cm\r\n");
  KEYPAD_ReadKey();
  raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
  voltage = raw_adc / 4096 * 3.3;
  samples[4] = voltage;
  sprintf(output, "READ:%lfv\r\n", voltage);
  print(output);

  print("DONE!\r\n");


  /*for(i = 0; i < 16; i++){
    sprintf(output, "%d cm", (i+1)*5);
    KEYPAD_ReadKey();
    raw_adc = ADC_ChannelGetData(LPC_ADC, 1);
    voltage = raw_adc / 4096 * 3.3;
    samples[i] = voltage;
  }*/

}

double SENSOR_VoltageToDistance2(double samples[], double voltage){
  double m;
  double inv_distance;
  double c = 0;
  double distance;
  double distances[5] = {1/80.0, 1/40.0, 1/10.0, 1/7.0, 1/5.0};
  m = 0;

  if(voltage <= samples[0]){//80
    distance = 80;
  } else if (voltage > samples[4]){//5
    distance = 5;
  } else if (voltage >= samples[3] && voltage < samples[4]) {
    m = (samples[4]-samples[3])/(distances[4]-distances[3]);
    c = samples[4] - distances[4] * m;
  }else if (voltage >= samples[2] && voltage < samples[3]) {
    m = (samples[3]-samples[2])/(distances[3]-distances[2]);
    c = samples[3] - distances[3] * m;
  }else if (voltage >= samples[1] && voltage < samples[2]) {
    m = (samples[2]-samples[1])/(distances[2]-distances[1]);
    c = samples[2] - distances[2] * m;
  }else if (voltage >= samples[0] && voltage < samples[1]) {
    m = (samples[1]-samples[0])/(distances[1]-distances[0]);
    c = samples[1] - distances[1] * m;
  }
  if(m!=0){
    inv_distance = (voltage-c)/m;
    distance = 1/inv_distance;
  }
  return distance;
}

volatile unsigned int PWMCounter = 0;
volatile int8_t PWMDirection = 1;
volatile uint8_t RIT_Mode = 0;

void RIT_IRQHandler(void){
  RIT_GetIntStatus(LPC_RIT);
  if(RIT_Mode == 4){
    setPWMStart(1, PWMCounter);
    if(PWMCounter == 256){
      PWMDirection = -1;
    }
    if(PWMCounter == 0){
      PWMDirection = 1;
    }
    PWMCounter += PWMDirection;
  }
  if(RIT_Mode == 3){
    DAC_UpdateValue(LPC_DAC, ADC_ChannelGetData(LPC_ADC, 0) / 4 * 0.909);
  }
  if(RIT_Mode == 2){
    DAC_UpdateValue(LPC_DAC, waveform_generator(1, 1));
  }
}


int main(){
  SysTick_Config(SystemCoreClock/1000 - 1);
  GPIO_SetDir(1, ALL_LEDS, 1);
  initPrint();
  initI2C();
  initDAC();

  unsigned char data[2] = {0x00, 0x88};
  //, 0xFF, 0xFF, 0xFF, 0xFF};
  I2C_SendBytes(EIGHT_SEG_ADDRESS, data, 2);


  while(1);
  return 0;
}
