#include <stddef.h>
#include "stm32f4xx.h"
#include "uart.h"
#include "stm32f4xx_gpio.h"
static int i=0;
uint8_t RxBuffer[MAX_BUFFER_LENGTH + 1];
static char RxChar = 0;
USART3_cmd_StatusType current_cmd_Status = USART3_NO_cmd;//cmd receiving status
void strTransmit(const char data)
{

	while(!(USART3->SR & USART_SR_TXE))
	      {
	         //Wait for transmission buffer empty flag
	      }

	      // Write data into transmit data register
	      USART3->DR = data;

}


void USART3_IRQ_Callback(void)
{
	if((USART3->SR & USART_SR_IDLE) == USART_SR_IDLE)
        {   /* Read data register to clear idle line flag "cmd receiving has finished "*/
		(void)USART3->DR;
		//end of rx buffer
		RxBuffer[i]=0;
		i=USART3->DR;
		i=0;//clean to receive new cmd
		// set led and Set CMD status
		GPIO_SetBits(GPIOD,GPIO_Pin_12);
		current_cmd_Status = USART3_CMD_RECEIVED;
		return;
         }

       /* Check USART receiver */
        if((USART3->SR & USART_SR_RXNE) == USART_SR_RXNE)
	{
               // Read character
		RxChar = USART3->DR;
                //FILL RECIVER BUFFER
                RxBuffer[i++]=RxChar;


       }
       else
        {
       //  No new data received
        }
}
void USART3_Init(void)
{

	 /* Enable USART3 clock */
	  RCC->APB1ENR = RCC_APB1ENR_USART3EN;

	  /* Select oversampling by 8 mode */
	  USART3->CR1 |= USART_CR1_OVER8;

	  /* Select one sample bit method */
	  USART3->CR3 |= USART_CR3_ONEBIT;

	  /* Select 1 Start bit, 8 Data bits, n Stop bit */
	  USART3->CR1 &= ~USART_CR1_M;

	  /* Select 1 stop bit */
	  USART3->CR2 &= ~USART_CR2_STOP;

	  /* disable parity control */
	  USART3->CR1 &= ~USART_CR1_PCE;

	  //set baudrate at 38.4kbs ,see page 986 at reference manual
	  /*USARTDIV = Fpclk / (16 * baud_rate)
	     *          = 42000000 / (16 * 38400) = 68.375
	     *
	     * DIV_Fraction = 16 * 0.375 = 0x6
	     * DIV_Mantissa = 68 = 0x44
	     *
	     * BRR          = 0x446 */
	      /* Write to USART BRR register */
	      USART3->BRR = (uint16_t)0x446;

	  //USART2->BRR = 0x2d8;
}
void USART3_Enable(void)
{

 /* Enable USART3 */
  USART3->CR1 |= USART_CR1_UE;

  /* Enable transmitter */
  USART3->CR1 |= USART_CR1_TE;

  /* Enable receiver */
  USART3->CR1 |= USART_CR1_RE;

  /* Enable idle line detection interrupt */
  USART3->CR1 |= USART_CR1_IDLEIE;

  /* Enable reception buffer not empty flag interrupt */
 USART3->CR1 |= USART_CR1_RXNEIE;



}
void NVIC_Int(void)
{
  /* Set priority group to 3
   * bits[3:0] are the sub-priority,
   * bits[7:4] are the pre-empt priority (0-15) */
  NVIC_SetPriorityGrouping(3);

  NVIC_SetPriority(USART3_IRQn, 1);

  NVIC_EnableIRQ(USART3_IRQn);



}
