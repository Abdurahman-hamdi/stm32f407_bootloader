#include "stm32f4xx_crc.h"
#include "Sys_init.h"


/* static void sys_init (void)
 * used to initialize the system_state as if it comes from reset
 *
 * */


static inline oper_statues check_MSP(uint32_t*app_start_addess){

	return ((0x20000000 == (*(volatile uint32_t* )(app_start_addess) & 0x20000000)) ? (__SUUCESS):(__FAILED));

                     }
static void sys_init (void){

	                  /* Reset GPIOB */
					  RCC->AHB1RSTR = (RCC_AHB1RSTR_GPIOBRST );
					  RCC->AHB1RSTR = (RCC_AHB1RSTR_GPIODRST );

					  /* Release reset */
					  RCC->AHB1RSTR = 0;

					  /* Reset USART3 */
					  RCC->APB1RSTR = RCC_APB1RSTR_USART3RST;

					  /* Release reset */
					  RCC->APB1RSTR = 0;

					    /* Reset GPIOD "GPIO_Pin_12,14 and 15" */
					    RCC->AHB1RSTR = (RCC_AHB1RSTR_GPIODRST );
					     /* Release reset */
					    RCC->AHB1RSTR = 0;
	                                     __disable_irq();
	                                  /* Reset RCC */
					  /* Set HSION bit to the reset value */
					  RCC->CR |= RCC_CR_HSION;

					  /* Wait till HSI is ready */
					  while(RCC_CR_HSIRDY != (RCC_CR_HSIRDY & RCC->CR))
					  {
					/* Waiting */
					  }

					  /* Set HSITRIM[4:0] bits to the reset value */
					  RCC->CR |= RCC_CR_HSITRIM_4;

					  /* Reset CFGR register */
					  RCC->CFGR = 0;

					  /* Wait till clock switch is ready and
					   * HSI oscillator selected as system clock */
					  while(0 != (RCC_CFGR_SWS & RCC->CFGR))
					  {
					/* Waiting */
					  }

					  /* Clear HSEON, HSEBYP and CSSON bits */
					  RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_HSEBYP | RCC_CR_CSSON);

					  /* Wait till HSE is disabled */
					  while(0 != (RCC_CR_HSERDY & RCC->CR))
					  {
					/* Waiting */
					  }

					  /* Clear PLLON bit */
					  RCC->CR &= ~RCC_CR_PLLON;

					  /* Wait till PLL is disabled */
					  while(0 != (RCC_CR_PLLRDY & RCC->CR))
					  {
					/* Waiting */
					  }

					  /* Reset PLLCFGR register to default value */
					  RCC->PLLCFGR = RCC_PLLCFGR_PLLM_4 | RCC_PLLCFGR_PLLN_6
					 | RCC_PLLCFGR_PLLN_7 | RCC_PLLCFGR_PLLQ_2;

					  /* Reset SysTick */
					  SysTick->CTRL = 0;
					  SysTick->LOAD = 0;
					  SysTick->VAL = 0;
					  __DMB();



}


/*jump_to_exist_app jumps to an exist firmware in cease of there is no upgrade req
 *
 */
void jump_to_exist_app(uint32_t*app_start_addess){
			sys_init();
	       SCB->VTOR = (uint32_t)app_start_addess;
			 __DSB();
			 void (*jump_to_bl)(void) = (void *)(*((uint32_t *)(app_start_addess + 4)));//Set fun_pointer pointing to image reset handler

	   /* Set stack pointer */
	        __set_MSP(*(volatile uint32_t*)app_start_addess);
	         jump_to_bl();//de-reference fun_ptr to execute image reset handler

}


/* Architecture specific oper_statues jump_to_new_app(uin32_t*app_start_addess,void (*ACK))  return the statues of jump operation
 * This function two arguments:
 * arg1:app_start_addess which is image start address of new firmware
 * arg2: void(*ACK) pointer to function called when MSP met a condition of it points to legal address
 *  it first checks the validity of stack pointer(msp) which must point to an address included with SRAM space
 * if msp met ,ACK is sent and  perform system initialization and then branch to new firmware
 *
 * */
oper_statues jump_to_new_app(uint32_t *app_start_addess, void (* ACKNOWLEDGE)()){
	             /* Check if it has valid stack pointer in the RAM */
			    if(check_MSP(app_start_addess))
			    {
			    	ACKNOWLEDGE(__ACK);
			    	sys_init();
			    	SCB->VTOR = (uint32_t)app_start_addess;
			    	__DSB();
			    	void (*jump_address)(void) = (void *)(*((uint32_t *)(app_start_addess + 4)));

			    	/* Set stack pointer */
			    	__set_MSP(*(uint32_t*)(app_start_addess));

			    	/* Jump */
			    	jump_address();

			     }
			    else
			    {
			    	ACKNOWLEDGE(__NACK);
			    	return __FAILED;

			     }






}
