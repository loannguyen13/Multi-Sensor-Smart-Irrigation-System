#ifndef DHT11_HANDLER_H
#define DHT11_HANDLER_H

#include "stm32f103xb.h"
#include <stdint.h>

void Set_PB5_Input();
void Set_PB5_Output();
void DHT11_Start();
uint8_t DHT11_CheckResponse();
uint8_t DHT11_ReadBit();
uint8_t DHT11_Read_Byte();
void DHT11_Data(uint8_t *temperature, uint8_t *humidity);

#endif