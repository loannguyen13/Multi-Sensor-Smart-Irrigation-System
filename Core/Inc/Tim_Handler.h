#ifndef TIM_HANDLER_H
#define TIM_HANDLER_H

#include "stm32f103xb.h"
#include <stdint.h>

extern volatile uint8_t tim_flag;

void TIM4_Init();
void delay_us(uint16_t time);
void delay_ms(uint16_t time);

void TIM2_Init();

#endif 