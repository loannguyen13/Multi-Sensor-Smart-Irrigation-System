#ifndef I2C_HANDLER_H
#define I2C_HANDLER_H

#include "stm32f103xb.h"
#include <stdint.h>

#define RS 0x01
#define EN 0x04
#define BlackLight 0x08
#define LCD_Address (0x27 << 1)

void I2C_Init();
void I2C_Trans(uint8_t address, uint8_t *data, uint8_t len);

void LCD_Command(unsigned char command);
void LCD_Data(unsigned char data);
void LCD_Init();
void LCD_String(const char *string);

#endif // I2C_HANDLER_H