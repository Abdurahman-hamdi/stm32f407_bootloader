#include "gpio.h"
#include "stm32f4xx_rcc.h"
#include"stm32f4xx_gpio.h"


void USART3_GPIO_Config(void)
{
  /* Enable port b clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* Select alternate function mode */
    GPIOB->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
    GPIOB->MODER |= (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1);

    /* Select output type push-pull for Tx(Pb10) */
    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_10);

    /* Select output speed medium for Tx(Pb10) */
    GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR10);
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR10_0;

    /* Select pull up */
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR10 | GPIO_PUPDR_PUPDR11);
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPDR10_0 | GPIO_PUPDR_PUPDR11_0);

    //alternative function for portb, pins 10 and 11
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

}
void led_init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef GPIO_InitDef;
	GPIO_InitDef.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_15|GPIO_Pin_14;
	GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
	GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitDef.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitDef);
	}



