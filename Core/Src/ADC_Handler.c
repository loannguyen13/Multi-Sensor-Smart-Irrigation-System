#include "Tim_Handler.h"
#include "ADC_Handler.h"

void ADC_Init(){
    // 1. Bật Clock cho ADC1
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Clock cho ADC1

    // 2. Cấu hình PB0 là Analog Input
    // Với GPIO_CRL, chân 0 tương ứng với CRL[3:0]
    // Mode = 00 (Input), CNF = 00 (Analog)
    GPIOB->CRL &= ~(0xF << 0); 

    // 3. Cấu hình ADC1
    ADC1->CR2 &= ~ADC_CR2_CONT;      // Chọn chế độ Single conversion (CONT = 0)
    ADC1->CR2 |= ADC_CR2_ADON;       // Bật ADC (lần 1)
    delay_us(50);

    RCC->CFGR &= ~RCC_CFGR_ADCPRE;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2; // tường minh dù đang là default

    // 4. Chọn kênh 8 (PB0) cho chuỗi chuyển đổi thường
    // SQ1 là vị trí đầu tiên, lưu giá trị 8 vào bit [4:0]
    ADC1->SQR3 = 8;                  

    // 5. Thiết lập thời gian lấy mẫu cho kênh 8 (SMPR2 quản lý kênh 0-9)
    // Chọn 13.5 chu kỳ (giá trị 001)
    ADC1->SMPR2 |= (2 << (3 * 8)); 

    // 6. Calib ADC (tốt cho độ chính xác)
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL); // Chờ hiệu chuẩn xong
}

uint16_t ADC1_Read(void) {
    // Bắt đầu chuyển đổi (lần 2 - ghi vào bit ADON)
    ADC1->CR2 |= ADC_CR2_ADON;
    
    // Đợi cờ End Of Conversion (EOC) lên 1
    while (!(ADC1->SR & ADC_SR_EOC));
    
    // Đọc dữ liệu (tự động xóa cờ EOC khi đọc xong)
    return ADC1->DR;
}

// 1. Hàm đọc trung bình N mẫu để lọc nhiễu
uint16_t ADC1_Read_Average(void) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < 10; i++) {
        sum += ADC1_Read(); // Gọi lại hàm đọc 1 lần đã viết
    }
    return (uint16_t)(sum / 10);
}

// 2. Hàm quy đổi giá trị ADC sang % Độ ẩm đất (0% - 100%)
uint8_t Moisture_Percent(void) {
    // Đọc trung bình 10 mẫu ADC
    uint16_t adc_val = ADC1_Read_Average(); 

    // Giá trị đã qua hiệu chuẩn thực tế (Thay số của bạn vào đây)
    const uint16_t ADC_DRY = 3800; // Ngưỡng đất khô (0%)
    const uint16_t ADC_WET = 1400; // Ngưỡng đất ướt (100%)

    // Giới hạn biên tránh bị xé hình / âm %
    if (adc_val >= ADC_DRY) return 0;
    if (adc_val <= ADC_WET) return 100;

    // Công thức tính % độ ẩm (đảo ngược)
    uint32_t moisture_percent = ((uint32_t)(ADC_DRY - adc_val) * 100) / (ADC_DRY - ADC_WET);

    return (uint8_t)moisture_percent;
}