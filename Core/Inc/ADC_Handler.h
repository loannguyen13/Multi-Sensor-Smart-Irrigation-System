#ifndef ADC_HANDLER_H
#define ADC_HANDLER_H

#include "stm32f103xb.h"
#include <stdint.h>



void I2C_Init();
void ADC_Init(void);
uint16_t ADC1_Read(void);
uint16_t ADC1_Read_Average(void);
uint8_t Moisture_Percent(void);

#endif // ADC_HANDLER_H