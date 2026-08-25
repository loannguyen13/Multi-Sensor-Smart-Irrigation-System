#ifndef GPIO_H
#define GPIO_H

#include "stm32f103xb.h"
#include <stdint.h>

extern volatile uint8_t mode;
extern volatile uint8_t pump;
extern volatile uint8_t manual;

void GPIO_Init();

#endif