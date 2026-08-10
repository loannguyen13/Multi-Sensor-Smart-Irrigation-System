#include "stm32f103xb.h"
#include "main.h"
#include "I2C_Handler.h"





int main()
{
    // 1. Cap xung cho GPIOB va I2C1
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    // 2. PB6/PB7 -> Alternate Function OPEN-DRAIN 50 MHz (nibble 0xF)
    GPIOB->CRL &= ~(0xFFu << 24);
    GPIOB->CRL |= (0xFFu << 24);
    HAL_Init();
    I2C_Init();
    LCD_Init();

    LCD_String("Bye");

    while (1)
    {
        
    }
}
/*
void i2c_init(){
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    // Tắt I2C1 trước khi cấu hình (xóa bit PE)
    I2C1->CR1 &= ~I2C_CR1_PE;

    // Thiết lập tần số PCLK1 = 8 MHz trong thanh ghi CR2
    I2C1->CR2 &= ~I2C_CR2_FREQ;
    I2C1->CR2 |= 8; // FREQ = 8 (8MHz)

    // Cấu hình thanh ghi CCR cho tốc độ 100 kHz (Standard Mode)
    I2C1->CCR = 40; // CCR = 8MHz / (2 * 100kHz) = 40

    // Cấu hình thời gian sườn lên (TRISE)
    I2C1->TRISE = 9; // TRISE = 8 + 1 = 9

    // Bật lại ngoại vi I2C1
    I2C1->CR1 |= I2C_CR1_PE;
}

void i2c_write(uint8_t address, uint8_t *data, uint8_t len){
    I2C1->CR1 |= I2C_CR1_START;
    while (! (I2C1->SR1 & I2C_SR1_SB));
    I2C1->DR = LCD_Address | 0;

    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void) I2C1->SR1; 
    (void) I2C1->SR2;
    for (uint8_t i = 0; i < len; i++)
    {
        while (!(I2C1->SR1 & I2C_SR1_TXE));

        I2C1->DR = data[i];
    }
    while (!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
}

void LCD_Command(unsigned char command){
    uint8_t nibble_high, nibble_low;
    uint8_t table[4];

    nibble_high = (command & 0xF0);
    nibble_low = ((command & 0x0F) << 4);
    table[0] = nibble_high | EN | BlackLight;
    table[1] = nibble_high | BlackLight;
    table[2] = nibble_low | EN | BlackLight;
    table[3] = nibble_low | BlackLight;

    i2c_write(LCD_Address, table, 4);
}

void LCD_Data(unsigned char data){
    uint8_t nibble_high, nibble_low;
    uint8_t table[4];

    nibble_high = (data & 0xF0);
    nibble_low = ((data & 0x0F) << 4);
    table[0] = nibble_high | EN | BlackLight | RS;
    table[1] = nibble_high | BlackLight | RS;
    table[2] = nibble_low | EN | BlackLight | RS; 
    table[3] = nibble_low | BlackLight | RS;

    i2c_write(LCD_Address, table, 4);
}

void LCD_Init(){
    HAL_Delay(50);

    LCD_Command(0x33);
    HAL_Delay(2);
    LCD_Command(0x32);
    HAL_Delay(2);

    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x01);
    HAL_Delay(2);
    LCD_Command(0x06);
}

void LCD_String(const char *string){
    while (*string)
    {
        LCD_Data(*string);
        string++;
    }
    
}
*/


