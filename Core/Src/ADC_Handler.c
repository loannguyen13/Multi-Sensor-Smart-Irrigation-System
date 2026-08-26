#include "Tim_Handler.h"
#include "ADC_Handler.h"

void ADC_Init(){
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Clock cho ADC1

    // Cấu hình PB0 là Analog Input
    GPIOB->CRL &= ~(0xF << 0); 

    // Cấu hình ADC1
    ADC1->CR2 &= ~ADC_CR2_CONT;      // Chọn chế độ Single conversion (CONT = 0)
    ADC1->CR2 |= ADC_CR2_ADON;       // Bật ADC (lần 1)
    delay_us(50);

    RCC->CFGR &= ~RCC_CFGR_ADCPRE;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2;

    // 4. Chọn kênh 8 (PB0) cho chuỗi chuyển đổi thường
    // SQ1 là vị trí đầu tiên, lưu giá trị 8 vào bit [4:0]
    ADC1->SQR3 = 8;                  

    // thời gian lấy mẫu
    ADC1->SMPR2 |= (2 << (3 * 8)); 

    // Calib ADC giam sai số
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL); // Chờ hiệu chuẩn xong
}

uint16_t ADC1_Read(void) {
    // Bật bit chuyên đổi
    ADC1->CR2 |= ADC_CR2_ADON;
    
    // chờ cờ eoc
    while (!(ADC1->SR & ADC_SR_EOC));
    
    // Đọc dữ liệu
    return ADC1->DR;
}

//  Hàm đọc trung bình 10 mẫu để lọc nhiễu
uint16_t ADC1_Read_Average(void) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < 10; i++) {
        sum += ADC1_Read();
    }
    return (uint16_t)(sum / 10);
}

// Hàm quy đổi giá trị ADC sang % Độ ẩm đất (0% - 100%)
uint8_t Moisture_Percent(void) {
    // Đọc trung bình 10 mẫu ADC
    uint16_t adc_val = ADC1_Read_Average(); 

    const uint16_t ADC_DRY = 3800; // Ngưỡng đất khô (0%)
    const uint16_t ADC_WET = 1400; // Ngưỡng đất ướt (100%)

    // Giới hạn biên
    if (adc_val >= ADC_DRY) return 0;
    if (adc_val <= ADC_WET) return 100;

    // Công thức tính % độ ẩm (đảo ngược)
    uint32_t moisture_percent = ((uint32_t)(ADC_DRY - adc_val) * 100) / (ADC_DRY - ADC_WET);

    return (uint8_t)moisture_percent;
}