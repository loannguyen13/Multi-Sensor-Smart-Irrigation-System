#include "Tim_Handler.h"

void TIM4_Init() {
    // 1. Cấp xung Clock cho ngoại vi TIM2 trên bus APB1
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    // 2. Cấu hình Prescaler (PSC) và Auto-Reload (ARR)
    TIM4->PSC = 8 - 1;   // Chia 8MHz xuống 1MHz (1 tick = 1us)
    // 3. Tạo sự kiện cập nhật để nạp giá trị PSC và ARR vào Shadow Registers
    TIM4->EGR |= TIM_EGR_UG;
    // 4. Xóa cờ ngắt tràn (UIF) vừa tạo ra do bước trên
    TIM4->SR &= ~TIM_SR_UIF;
}

void delay_us(uint16_t time) {
    TIM4->CNT = 0;              // Reset bộ đếm CNT về 0
    TIM4->ARR = time - 1;       
    TIM4->EGR |= TIM_EGR_UG;
    TIM4->SR &= ~TIM_SR_UIF;    // Xóa cờ báo tràn UIF
    TIM4->CR1 |= TIM_CR1_CEN;   // Bật Timer (Counter Enable)
    // Chờ cho đến khi cờ UIF được kéo lên 1
    while (!(TIM4->SR & TIM_SR_UIF));
    TIM4->CR1 &= ~TIM_CR1_CEN;  // Dừng Timer
    TIM4->SR &= ~TIM_SR_UIF;    // Xóa cờ báo tràn UIF để chuẩn bị cho lần sau
}

void delay_ms(uint16_t ms)
{
    TIM4->CNT = 0;
    TIM4->ARR = (ms * 1000) - 1;

    TIM4->EGR |= TIM_EGR_UG;
    TIM4->SR &= ~TIM_SR_UIF;

    TIM4->CR1 |= TIM_CR1_CEN;

    while (!(TIM4->SR & TIM_SR_UIF));

    TIM4->CR1 &= ~TIM_CR1_CEN;
    TIM4->SR &= ~TIM_SR_UIF;
}