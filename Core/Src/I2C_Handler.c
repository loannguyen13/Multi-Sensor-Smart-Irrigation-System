#include "I2C_Handler.h"
#include "stm32f1xx_hal.h"
#include "Tim_Handler.h"

void I2C_Init()
{
    // 1. Cap xung cho GPIOB va I2C1
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // 2. PB6/PB7 -> Alternate Function Open-Drain 50MHz
    GPIOB->CRL &= ~(0xFFu << 24);
    GPIOB->CRL |= (0xFFu << 24);
    
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
    I2C1->CR1 |= I2C_CR1_PE | I2C_CR1_ACK;
}

void I2C_Trans(uint8_t address, uint8_t *data, uint8_t len)
{
    I2C1->CR1 |= I2C_CR1_START;

    while (!(I2C1->SR1 & I2C_SR1_SB));
    I2C1->DR = address;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    for (uint8_t i = 0; i < len; i++)
    {
        while (!(I2C1->SR1 & I2C_SR1_TXE));
        I2C1->DR = data[i];
    }

    while (!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
}

void LCD_Command(unsigned char command)
{
    uint8_t nibble_high, nibble_low;
    uint8_t table[4];
    nibble_high = (command & 0xF0);
    nibble_low = ((command & 0x0F) << 4);

    table[0] = nibble_high | EN | BlackLight;
    table[1] = nibble_high | BlackLight;
    table[2] = nibble_low | EN | BlackLight;
    table[3] = nibble_low | BlackLight;

    I2C_Trans(LCD_Address, table, 4);
}

void LCD_Data(unsigned char data)
{
    uint8_t nibble_high, nibble_low;
    uint8_t table[4];
    nibble_high = (data & 0xF0);
    nibble_low = ((data & 0x0F) << 4);

    table[0] = nibble_high | EN | BlackLight | RS;
    table[1] = nibble_high | BlackLight | RS;
    table[2] = nibble_low | EN | BlackLight | RS;
    table[3] = nibble_low | BlackLight | RS;

    I2C_Trans(LCD_Address, table, 4);
}

void LCD_Init()
{
    delay_ms(50);

    LCD_Command(0x33);
    delay_ms(2);
    LCD_Command(0x32);
    delay_ms(2);

    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x01);
    delay_ms(2);
    LCD_Command(0x06);
}

void LCD_String(const char *string)
{
    while (*string)
    {
        LCD_Data(*string);
        string++;
    }
}

void LCD_Setcusor(uint8_t row, uint8_t column){
    switch (row)
    {
    case 1:
        LCD_Command(0x80 + column - 1);
        break;

    case 2:
        LCD_Command(0xC0 + column - 1);
        break;

    case 3:
        LCD_Command(0x94 + column - 1);
        break;

    case 4:
        LCD_Command(0xD4 + column - 1);
        break;

    default:
        break;
    }
}

// Gui lenh Power On + Continuous High Res Mode (0x10) mot lan duy nhat
void BH1750_Init(void)
{
    uint32_t timeout;

    // --- Power On (0x01) ---
    timeout = 100000;
    while (I2C1->SR2 & I2C_SR2_BUSY) { if (--timeout == 0) return; }

    I2C1->CR1 |= I2C_CR1_START;
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_SB)) { if (--timeout == 0) return; }

    I2C1->DR = (BH1750_ADDR << 1);
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
        if (I2C1->SR1 & I2C_SR1_AF) { I2C1->SR1 &= ~I2C_SR1_AF; I2C1->CR1 |= I2C_CR1_STOP; return; }
        if (--timeout == 0) return;
    }
    (void)I2C1->SR1; (void)I2C1->SR2;

    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = 0x01;
    while (!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;

    delay_ms(10);

    // --- Continuous High Res Mode (0x10) ---
    timeout = 100000;
    while (I2C1->SR2 & I2C_SR2_BUSY) { if (--timeout == 0) return; }

    I2C1->CR1 |= I2C_CR1_START;
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_SB)) { if (--timeout == 0) return; }

    I2C1->DR = (BH1750_ADDR << 1);
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
        if (I2C1->SR1 & I2C_SR1_AF) { I2C1->SR1 &= ~I2C_SR1_AF; I2C1->CR1 |= I2C_CR1_STOP; return; }
        if (--timeout == 0) return;
    }
    (void)I2C1->SR1; (void)I2C1->SR2;

    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = 0x10;
    while (!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
}

// Chi lam MOT viec: doc 2 byte tu chip. Khong dung co POS.
uint16_t BH1750_ReadRaw(void)
{
    uint8_t MSB = 0, LSB = 0;
    uint32_t timeout;

    // 1. Cho bus thuc su ranh truoc khi START
    timeout = 100000;
    while (I2C1->SR2 & I2C_SR2_BUSY) { if (--timeout == 0) return 0xFFFF; }

    // DỌN DẸP TRẠNG THÁI: Bật ACK mặc định và TẮT cờ POS lỗi
    I2C1->CR1 |= I2C_CR1_ACK;
    I2C1->CR1 &= ~I2C_CR1_POS;

    // 2. Gui START
    I2C1->CR1 |= I2C_CR1_START;
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_SB)) { if (--timeout == 0) return 0xFFFF; }

    // 3. Gui dia chi doc (Read)
    I2C1->DR = (BH1750_ADDR << 1) | 0x01;
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
        if (I2C1->SR1 & I2C_SR1_AF) { 
            I2C1->SR1 &= ~I2C_SR1_AF; 
            I2C1->CR1 |= I2C_CR1_STOP; 
            return 0xFFFF; 
        }
        if (--timeout == 0) return 0xFFFF;
    }

    // 4. Xóa cờ ADDR để bắt đầu nhận dữ liệu (Tuyệt đối KHÔNG chạm vào ACK hay POS ở đây)
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    // 5. Chờ nhận xong Byte 1 (MSB) báo qua cờ RXNE
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_RXNE)) { 
        if (--timeout == 0) { I2C1->CR1 |= I2C_CR1_STOP; return 0xFFFF; } 
    }

    // --- CRITICAL SECTION ---
    // Ngay khi Byte 1 vừa vào DR, Byte 2 đang bắt đầu dịch vào Shift Register.
    // Lúc này ta ngắt ACK (để chip gửi NACK cho Byte 2) và báo STOP.
    I2C1->CR1 &= ~I2C_CR1_ACK;
    I2C1->CR1 |= I2C_CR1_STOP;

    // 6. Lấy Byte 1 (MSB) ra
    MSB = I2C1->DR;

    // 7. Chờ nhận xong Byte 2 (LSB) báo qua cờ RXNE
    timeout = 100000;
    while (!(I2C1->SR1 & I2C_SR1_RXNE)) { 
        if (--timeout == 0) { I2C1->CR1 |= I2C_CR1_STOP; return 0xFFFF; } 
    }

    // 8. Lấy Byte 2 (LSB) ra
    LSB = I2C1->DR;

    // 9. Phục hồi lại ACK cho vòng lặp tiếp theo
    I2C1->CR1 |= I2C_CR1_ACK;

    return ((uint16_t)MSB << 8) | LSB;
}

// Thuan tuy phep tinh, khong dung toi I2C
uint16_t BH1750_RawToLux(uint16_t raw)
{
    if (raw == 0xFFFF) return 0; // gia tri loi, tra ve 0 de tranh hien thi rac
    return (uint16_t)((float)raw / 1.2f);
}