#ifndef TIM_HANDLER_H
#define TIM_HANDLER_H

#include "stm32f103xb.h"
#include <stdint.h>

void TIM4_Init();
void delay_us(uint16_t time);
void delay_ms(uint16_t time);

#endif 