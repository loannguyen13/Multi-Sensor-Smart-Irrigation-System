#include "GPIO.h"
#include "I2C_Handler.h"
#include "Tim_Handler.h"


void GPIO_Init(){
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    GPIOA->CRL &= ~((0xF << 0) | (0xF << 16)); // setup PA0, PA4 lam input
    GPIOA->CRL |= (0x8 << 0) | (0x8 << 16);
    GPIOA->ODR |= (1 << 0) | (1 << 4);

    GPIOA->CRH &= ~(0xF << 0);                  // setup PA8 lam input
    GPIOA->CRH |= (0x8 << 0);
    GPIOA->ODR |= (1 << 8);

    GPIOA->CRL &= ~((0xF << 4) | (0xF << 8));   // setup PA1, PA2 lam output dieu khien led
    GPIOA->CRL |= (0x2 << 4) | (0x2 << 8);

    GPIOA->CRH &= ~(0xF << 4);                  // setup PA9 lam output dieu khien bom
    GPIOA->CRH |= (0x2 << 4);

    AFIO->EXTICR[0] &= ~(0xF << 0);             // setup ngat ngoai 0
    AFIO->EXTICR[0] |= (0x0 << 0);
    EXTI->IMR |= (0x1 << 0);
    EXTI->FTSR |= (0x1 << 0);
    NVIC_EnableIRQ(EXTI0_IRQn);

    AFIO->EXTICR[1] &= ~(0xF << 0);              // setup ngat ngoai 1
    AFIO->EXTICR[1] |= (0x0 << 0);
    EXTI->IMR |= (0x1 << 4);
    EXTI->FTSR |= (0x1 << 4);
    NVIC_EnableIRQ(EXTI4_IRQn);

    AFIO->EXTICR[2] &= ~(0xF << 0);               // setup ngat ngoai 2
    AFIO->EXTICR[2] |= (0x0 << 0);
    EXTI->IMR |= (0x1 << 8);
    EXTI->FTSR |= (0x1 << 8);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void EXTI0_IRQHandler()
{
    if ((EXTI->PR & EXTI_PR_PR0) != 0)
    {
        EXTI->PR |= EXTI_PR_PR0;
        mode++;
        if (mode > 2)
        {
            mode = 1;
        }
    }
}

void EXTI4_IRQHandler()
{
    if ((EXTI->PR & EXTI_PR_PR4) != 0)
    {
        EXTI->PR |= EXTI_PR_PR4;
        if (mode == 2)
        {
            manual = 1;
        }
    }
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR8)
    {
        EXTI->PR = EXTI_PR_PR8;
        if (mode == 2)
        {
            if (pump == 0) pump = 1;
            else pump = 0;
        }
    }
}