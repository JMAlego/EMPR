#ifndef EMPR_LIB_I2C_C
#define EMPR_LIB_I2C_C

#include "empr_lib_i2c.h"

void EL_I2C_Init(void){
  PINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = 3;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;

  PinCfg.Portnum = 0;
  PinCfg.Pinnum = 0;
  PINSEL_ConfigPin(&PinCfg);

  PinCfg.Pinnum = 1;
  PINSEL_ConfigPin(&PinCfg);

  I2C_Init(LPC_I2C1, EMPR_LIB_I2C_DATA_RATE);

  I2C_Cmd(LPC_I2C1, ENABLE);
}

Status EL_I2C_SendBytes(uint8_t address, uint8_t *data, size_t data_length)
{
  I2C_M_SETUP_Type transferSetup;
  transferSetup.rx_data = NULL;
  transferSetup.retransmissions_max = EMPR_LIB_I2C_RETRANSMISSIONS_MAX;
  transferSetup.rx_length = 0;
  transferSetup.sl_addr7bit = (uint32_t)address;
  transferSetup.tx_data = data; 
  transferSetup.tx_length = (uint32_t)data_length;
  return I2C_MasterTransferData(LPC_I2C1, &transferSetup, I2C_TRANSFER_POLLING);
}

Status EL_I2C_ReceiveBytes(uint8_t address, uint8_t *data, size_t data_length)
{
  I2C_M_SETUP_Type transferSetup;
  transferSetup.rx_data = data;
  transferSetup.retransmissions_max = EMPR_LIB_I2C_RETRANSMISSIONS_MAX;
  transferSetup.rx_length = (uint32_t)data_length;
  transferSetup.sl_addr7bit = (uint32_t)address;
  transferSetup.tx_data = NULL;
  transferSetup.tx_length = 0;
  return I2C_MasterTransferData(LPC_I2C1, &transferSetup, I2C_TRANSFER_POLLING);
}

#endif
