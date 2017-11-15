#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>
#include <math.h>

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

#define LCD_ADDRESS 0x3b
#define EIGHT_SEG_ADDRESS 0x38
#define KEYPAD_ADDRESS 0x21


const int leds[4] = {LED1, LED2, LED3, LED4};

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

  I2C_Init(LPC_I2C1, 100000);

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

  print("Start LCD\r\n");
  LCD_Init();
  LCD_Clear_Display();
  unsigned char stringHelloWorld[12] = {0xC8, 0xC5, 0xCC, 0xCC, 0xCF, 0xA0, 0xD7, 0xCF, 0xD2, 0xCC, 0xC4, 0xA1};
  LCD_Write_Chars(stringHelloWorld, 12);
  Delay(1000);
  LCD_Clear_Display();
  Delay(1000);
  unsigned char stringHello[5] = {0xC8, 0xC5, 0xCC, 0xCC, 0xCF};
  unsigned char stringWorld[5] = {0xD7, 0xCF, 0xD2, 0xCC, 0xC4};
  LCD_Write_Chars(stringHello, 5);
  LCD_Write_Address(0x40);
  LCD_Write_Chars(stringWorld, 5);
  Delay(1000);
  LCD_Clear_Display();
  print("End LCD\r\n");

  #endif

  #ifdef STAGE3
  LCD_Init();
  LCD_Clear_Display();
  print("Start Keypad\r\n");
  char toprint[4] = " \r\n";
  int i;
  for(i = 0; i < 16; i++){
    toprint[0] = KEYPAD_ReadKey();
    print(toprint);
    unsigned char forLCD[1];
    forLCD[0] = LCD_EncodeASCII(toprint[0]);
    LCD_Write_Chars(forLCD, 1);
  }
  print("Finished Keypad\r\n");
  #endif






  #ifdef STAGE4
  print("------------------------------------------------------\r\n");
  print("Start Calculator\r\n");
  print("A -> +\r\n");
  print("B -> -\r\n");
  print("C -> /\r\n");
  print("D -> Clear All\r\n");
  print("# -> =\r\n");
  print("Enter a number, then operation\r\n");
  print("Screen will display last key press \r\n");
  print("------------------------------------------------------\r\n");
  LCD_Init();
  long long answer = 0;
  char operator = '+';
  char buffer[17] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                     '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
  unsigned char buffer_pointer = 0;
  char input = '\0';
  char last_input_was_number = 0;
  char error_string[17] = "";
  LCD_Clear_Display();

  while(1){
    input = KEYPAD_ReadKey();
    if(input < 58 && input > 47 && buffer_pointer < 16){
      if(last_input_was_number == 0)
        LCD_Clear_Display();
      last_input_was_number = 1;
      buffer[buffer_pointer] = input;
      buffer_pointer++;
      unsigned char print_character[1];
      print_character[0] = LCD_EncodeASCII(input);
      LCD_Write_Chars(print_character, 1);
    } else {
      if (input == 'D') {
        answer = 0;
        unsigned char i;
        for (i = 0; i < 17; i++) buffer[i] = '\0';
        buffer_pointer = 0;
        operator = '+';
        LCD_Clear_Display();
      } else {
        if(operator == '+'){
          answer += stringToLongLong(buffer);
        }else if(operator == '-'){
          answer -= stringToLongLong(buffer);
        }else if(operator == '/'){
          if(stringToLongLong(buffer) != 0){
            answer /= stringToLongLong(buffer);
          }else{
            answer = 0;
            strcpy(error_string, "DIV BY 0 ERROR");
          }
        }else if(operator == '*'){
          answer *= stringToLongLong(buffer);
        }

        if (input == 'A'){
          operator = '+';
        } else if (input == 'B'){
          operator = '-';
        } else if (input == 'C'){
          operator = '/';
        } else if (input == 'D'){
          answer = 0;
          operator = '+';
        } else if (input == '*'){
          operator = '*';
        } else if (input == '#'){
          operator = '=';
        }
        unsigned char i;
        for (i = 0; i < 17; i++) buffer[i] = '\0';
        buffer_pointer = 0;
        LCD_Clear_Display();
        unsigned char print_operator[1];
        print_operator[0] = LCD_EncodeASCII(operator);
        LCD_Write_Chars(print_operator, 1);
        Delay(300);
        LCD_Clear_Display();
        char print_answer[20];
        sprintf(print_answer, "%lld", answer);
        LCD_EncodeASCIIString(print_answer);
        LCD_Write_Chars((unsigned char *) print_answer, sizeofStr(print_answer) - 1);
        if(operator == '='){
          if(error_string[0] != '\0'){
            LCD_Write_Address(0x40);
            LCD_EncodeASCIIString(error_string);
            LCD_Write_Chars((unsigned char *) error_string, sizeofStr(error_string) - 1);
            LCD_Write_Address(0x00);
            error_string[0] = '\0';
          }
        }
        last_input_was_number = 0;
      }
    }
  }

  #endif


  #ifdef DEMO

  print("Starting DEMO\r\n");
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

  Delay(1000);

  print("Start LCD\r\n");
  LCD_Init();
  LCD_Clear_Display();
  unsigned char stringHelloWorld[12] = {0xC8, 0xC5, 0xCC, 0xCC, 0xCF, 0xA0, 0xD7, 0xCF, 0xD2, 0xCC, 0xC4, 0xA1};
  LCD_Write_Chars(stringHelloWorld, 12);
  Delay(1000);
  LCD_Clear_Display();
  Delay(1000);
  unsigned char stringHello[5] = {0xC8, 0xC5, 0xCC, 0xCC, 0xCF};
  unsigned char stringWorld[5] = {0xD7, 0xCF, 0xD2, 0xCC, 0xC4};
  LCD_Write_Chars(stringHello, 5);
  LCD_Write_Address(0x40);
  LCD_Write_Chars(stringWorld, 5);
  Delay(1000);
  LCD_Clear_Display();
  print("End LCD\r\n");

  Delay(1000);

  LCD_Clear_Display();
  print("Start Keypad\r\n");
  char toprint[4] = " \r\n";
  int i;
  for(i = 0; i < 16; i++){
    toprint[0] = KEYPAD_ReadKey();
    print(toprint);
    unsigned char forLCD[1];
    forLCD[0] = LCD_EncodeASCII(toprint[0]);
    LCD_Write_Chars(forLCD, 1);
  }
  Delay(1000);
  LCD_Clear_Display();
  print("Finished Keypad\r\n");
  print("Finished DEMO\r\n");

  #endif

  return 0;

}
