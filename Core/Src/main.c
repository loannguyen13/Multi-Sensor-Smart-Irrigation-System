#include "stm32f103xb.h"
#include "main.h"
#include "I2C_Handler.h"
#include "DHT11_Handler.h"
#include "Tim_Handler.h"
#include "ADC_Handler.h"
#include "GPIO.h"

void Int_To_String(int num, char *str); // Ham de hien thi so len LCD
void func();

// main.c
#include "main.h"

// Khai báo biến toàn cục ở đây
volatile uint8_t mode = 0;
volatile uint8_t manual = 0;
volatile uint8_t pump = 0;
volatile uint8_t tim_flag = 0;

// Bien nhiet do va do am khong khi (dinh dung them 1 con LCD 1602 de hien thi)
uint8_t temperature = 0, humidity = 0, moisture = 0;
// Bien anh sang
uint16_t raw = 0, lux = 0;
// Bang de chua ki tu so de doi len lcd
char buffer[10];

int main()
{
    GPIO_Init();
    TIM4_Init(); // Khoi tao timer 4 delay
    TIM2_Init(); // Khoi tao timer 2 phuc vu ngat
    ADC_Init();

    I2C_Init(); // Khoi tao i2c
    LCD_Init(); // khoi tao lcd

    LCD_Setcusor(1, 1); // Ham de setup vi tri tren lcd (dong, cot)
    LCD_String("Mode: ");
    LCD_Setcusor(2, 1);
    LCD_String("Soil: ");
    LCD_Setcusor(2, 12);
    LCD_String("Pump: ");
    LCD_Setcusor(3, 1);
    LCD_String("Lux: ");
    LCD_Setcusor(4, 1);
    LCD_String("Air: ");
    LCD_Setcusor(4, 12);
    LCD_String("t: ");

    BH1750_Init(); // Khoi tao cam bien anh sang
    delay_ms(60);  // Cho phep do dau tien hoan tat (toi da ~180ms theo datasheet)
    delay_ms(60);
    delay_ms(60);

    // Cap nhat gia tri hien thi len lcd
    uint8_t last_mode = 0;
    /*Vi du:
        DHT11_Data(&temperature, &humidity);
        Int_To_String(temperature, buffer);
        LCD_Setcusor(4,14);
        LCD_String((const char*)buffer);
        LCD_String("%   ");
    */
    while (1)
    {
        if ((moisture < 40) || (moisture > 80) || (temperature < 15) || (lux < 100)){
            GPIOA->BSRR = (1 << 2);
            GPIOA->BRR = (1 << 1);
        } else {
            GPIOA->BSRR = (1 << 1);
            GPIOA->BRR = (1 << 2);
        }
        // 1. Luôn xử lý và reset tim_flag để tránh treo cờ
        if (tim_flag == 1)
        {
            tim_flag = 0; // Reset cờ ngay lập tức bất kể đang ở Mode nào!

            // Chỉ đọc cảm biến khi ở Mode Auto (Mode 1)
            if (mode == 1)
            {
                func();
                if (moisture < 40){
                    pump = 1;
                }
                else {
                    pump = 0;
                }
                LCD_Setcusor(2, 18);
                if (pump == 1)
                {
                    GPIOA->ODR |= (1 << 9);
                    LCD_String("On ");
                }
                else
                {
                    GPIOA->ODR &= ~(1 << 9);
                    LCD_String("Off");
                }
            }
        }
        if (mode == 2)
        {
            LCD_Setcusor(2, 18);
            if (pump == 1)
            {
                GPIOA->ODR |= (1 << 9);
                LCD_String("On ");
            }
            else
            {
                GPIOA->ODR &= ~(1 << 9);
                LCD_String("Off");
            }
            if (manual == 1){
                manual = 0;
                func();
            }
        }
        if (mode != last_mode)
        {
            last_mode = mode;
            LCD_Setcusor(1, 7);
            if (mode == 1)
            {
                LCD_String("Auto  ");
            }
            else
            {
                LCD_String("Manual");
            }
        }
    }
}

void Int_To_String(int num, char *str)
{
    int i = 0;
    int isNegative = 0;

    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0)
    {
        isNegative = 1;
        num = -num;
    }

    while (num != 0)
    {
        int rem = num % 10;
        str[i++] = rem + '0';
        num = num / 10;
    }

    if (isNegative)
    {
        str[i++] = '-';
    }

    str[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end)
    {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void func()
{
    moisture = Moisture_Percent();
    Int_To_String(moisture, buffer);
    LCD_Setcusor(2, 7);
    LCD_String((const char *)buffer);
    LCD_String("%  ");

    raw = BH1750_ReadRaw();
    lux = BH1750_RawToLux(raw);
    Int_To_String(lux, buffer);
    LCD_Setcusor(3, 6);
    LCD_String((const char *)buffer);
    LCD_String("  ");

    DHT11_Data(&temperature, &humidity);
    Int_To_String(temperature, buffer);
    LCD_Setcusor(4, 15);
    LCD_String((const char *)buffer);
    LCD_Data(0xDF);
    LCD_String("C  ");

    Int_To_String(humidity, buffer);
    LCD_Setcusor(4, 6);
    LCD_String((const char *)buffer);
    LCD_String("%  ");
}