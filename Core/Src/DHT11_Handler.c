#include "DHT11_Handler.h"
#include "stm32f1xx_hal.h"
#include "Tim_Handler.h"

void Set_PB5_Input(){
    GPIOB->CRL &= ~(0xF << 20);
    GPIOB->CRL |= (0x8 << 20);
    GPIOB->ODR |= (1 << 5);
}

void Set_PB5_Output(){
    GPIOB->CRL &= ~(0xF << 20);
    GPIOB->CRL |= (0x3 << 20);
}

void DHT11_Start(){
    Set_PB5_Output();
    TIM4_Init();
    GPIOB->BRR = (1 << 5);
    delay_ms(20);
    GPIOB->BSRR = (1 << 5);
    delay_us(40);
    Set_PB5_Input();
}

uint8_t DHT11_CheckResponse(){
    // Cài đặt ARR mức tối đa (65535) để đếm tự do không bị tràn sớm
    TIM4->ARR = 0xFFFF;
    TIM4->CNT = 0;
    TIM4->CR1 |= TIM_CR1_CEN; // Bật timer

    // Chờ DHT11 kéo chân xuống 0 (có timeout)
    while ((GPIOB->IDR & (1 << 5))) {
        if (TIM4->CNT > 100) { TIM4->CR1 &= ~TIM_CR1_CEN; return 0; }
    }

    TIM4->CNT = 0; // Reset đồng hồ
    
    // Chờ DHT11 kéo chân lên 1 (Response từ cảm biến)
    while (!(GPIOB->IDR & (1 << 5))) {
        if (TIM4->CNT > 100) { TIM4->CR1 &= ~TIM_CR1_CEN; return 0; }
    }

    TIM4->CNT = 0; // Reset đồng hồ
    
    // Chờ DHT11 kéo chân xuống 0 (Kết thúc Response, chuẩn bị gửi Data)
    while ((GPIOB->IDR & (1 << 5))) {
        if (TIM4->CNT > 100) { TIM4->CR1 &= ~TIM_CR1_CEN; return 0; }
    }

    TIM4->CR1 &= ~TIM_CR1_CEN; // Dừng Timer
    return 1;
}

uint8_t DHT11_ReadBit(){
    TIM4->ARR = 0xFFFF;
    TIM4->CNT = 0;
    TIM4->CR1 |= TIM_CR1_CEN; // Bật timer
    
    // Chờ chân lên mức 1 (Bắt đầu truyền 1 bit)
    while (!(GPIOB->IDR & (1 << 5))) {
        if(TIM4->CNT > 100) {
            TIM4->CR1 &= ~TIM_CR1_CEN;
            return 0; // Tránh kẹt
        }
    }
    
    TIM4->CNT = 0; // Bắt đầu tính thời gian chân ở mức 1
    
    // Chờ chân xuống mức 0 
    while ((GPIOB->IDR & (1 << 5))) {
        if(TIM4->CNT > 100) break; // Tránh kẹt nếu cảm biến lỗi
    }
    
    uint16_t time = TIM4->CNT; // Chốt lại thời gian duy trì mức 1
    TIM4->CR1 &= ~TIM_CR1_CEN; // Tắt timer
    
    // Theo datasheet DHT11: mức 1 dài ~70us, mức 0 dài ~26-28us
    if (time > 40) {
        return 1;
    }
    return 0;
}

uint8_t DHT11_Read_Byte(){
    uint8_t result = 0;
    for (int i = 0; i < 8; i++){
        result |= (DHT11_ReadBit() << (7 - i));
    }
    return result;
}

void DHT11_Data(uint8_t *temperature, uint8_t *humidity){
    uint8_t data[5] = {0};
    DHT11_Start();
    if (DHT11_CheckResponse() == 1){
        data[0] = DHT11_Read_Byte();
        data[1] = DHT11_Read_Byte();
        data[2] = DHT11_Read_Byte();
        data[3] = DHT11_Read_Byte();
        data[4] = DHT11_Read_Byte();

        if (((data[0] + data [1] + data[2] + data [3]) & 0xFF)  ==  data[4]){
            *humidity = data[0];
            *temperature = data[2];
        } else {
            *humidity = 0xFF; 
            *temperature = 0xFF;
        }
    } else {
        *humidity = 0x00;
        *temperature = 0x00;
    }
}
