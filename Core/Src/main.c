#include "stm32f103xb.h"
#include "main.h"
#include "I2C_Handler.h"
#include "DHT11_Handler.h"
#include "Tim_Handler.h"


void Int_To_String(int num, char *str); //Ham de hien thi so len LCD

int main()
{
    TIM4_Init();    //Khoi tao timer 4
    HAL_Init();
    I2C_Init();     //Khoi tao i2c
    LCD_Init();     //khoi tao lcd

    LCD_Setcusor(1, 1);     //Ham de setup vi tri tren lcd (dong, cot)
    LCD_String("Mode: ");
    LCD_Setcusor(2, 1);
    LCD_String("Soil Moisture: ");
    LCD_Setcusor(3, 1);
    LCD_String("Light intensity: ");
    LCD_Setcusor(4, 1);
    LCD_String("Bump: ");

    BH1750_Init();      // Khoi tao cam bien anh sang
    delay_ms(60);       // Cho phep do dau tien hoan tat (toi da ~180ms theo datasheet)
    delay_ms(60);
    delay_ms(60);

    //Bien nhiet do va do am khong khi (dinh dung them 1 con LCD 1602 de hien thi)
    uint8_t temperature = 0, humidity = 0; 

    char buffer[10]; //Bang de chua ki tu so de doi len lcd
    /*Vi du:
        DHT11_Data(&temperature, &humidity);
        Int_To_String(temperature, buffer);
        LCD_Setcusor(4,14);
        LCD_String((const char*)buffer);
        LCD_String("%   ");
    */
    while (1)
    {
    
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


