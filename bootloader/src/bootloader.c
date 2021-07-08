#include "bootloader.h"
#include "uart.h"
#include "stm32f4xx_crc.h"
#include <string.h>

extern USART3_cmd_StatusType current_cmd_Status;//CMD received status which is defined in uart.c
extern uint8_t RxBuffer[MAX_BUFFER_LENGTH + 1];//cmd buffer which is defined in uart.c

void boot_process(){
	  NVIC_Int();
	  USART3_GPIO_Config();
	  USART3_Init();
	  RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;//enable crc
	  CRC_ResetDR();//reset crc
	  USART3_Enable();
	  while(1){
		  switch(current_cmd_Status){
	  	  case USART3_CMD_RECEIVED:
		  switch(RxBuffer[0]){
			case 's': // start session
			current_cmd_Status = USART3_NO_cmd;
			break;

			case CMD_ERASE:
			CRC_ResetDR();
			cmdErase(RxBuffer);
			CRC_ResetDR();
			 break;

			case CMD_WRITE:
			CRC_ResetDR();
			cmdWrite(RxBuffer);
			CRC_ResetDR();
            		break;
		
			case CMD_Jump:
			CRC_ResetDR();
			cmdjump(RxBuffer);
			CRC_ResetDR();
        	        break;
		        case CMD_wr_p:
			CRC_ResetDR();
			wr_prot(RxBuffer);
			CRC_ResetDR();
			break;
				  
		        case RE_wr_pr:
			CRC_ResetDR();
			Re_wr_prot(RxBuffer);
			CRC_ResetDR();
			break;

		        case up_date:
			CRC_ResetDR();
			update(RxBuffer);
			CRC_ResetDR();
		        break;


		                    }

		case USART3_NO_cmd:
      		GPIO_SetBits(GPIOD,GPIO_Pin_15);
                GPIO_ResetBits(GPIOD,GPIO_Pin_12);
		break;

	}

}}
void cmdErase(uint8_t *pucData){
	current_cmd_Status = USART3_NO_cmd;
	uint32_t crc_val=0;
	uint32_t pulCmd[] = { pucData[0], pucData[1] };
	memcpy(&crc_val, pucData + 2, sizeof(uint32_t));
	if(crc_val==CRC_CalcBlockCRC(pulCmd, 2))
	{
		FLASH_Unlock();
		if(RxBuffer[1]==0xff)
		{	FLASH_EraseAllSectors(VoltageRange_3);
		}
		else
		{	FLASH_EraseSector(RxBuffer[1], VoltageRange_3);
		}
		FLASH_Lock();
    		strTransmit(ACK);
	}
	else
	{

		strTransmit(NACK);
		return;
	}

}
void cmdWrite(uint8_t*pucData){
	current_cmd_Status = USART3_NO_cmd;
	uint32_t ulSaddr = 0, ulCrc = 0,val;

	memcpy(&ulSaddr, pucData + 1, sizeof(uint32_t));
	 memcpy(&ulCrc, pucData + 5, sizeof(uint32_t));
	 uint32_t pulData[5];
	 uint32_t pullData[16]={0};
	 for(int i = 0; i < 5; i++)
	 	pulData[i] = pucData[i];
	 val=CRC_CalcBlockCRC(pulData, 5);

	 if(ulCrc==val)
	 {
         	CRC_ResetDR();
		memset(RxBuffer, 0, 20*sizeof(char));
		strTransmit(ACK);
      		while(current_cmd_Status==USART3_NO_cmd);//wait for the rest of command
       		current_cmd_Status = USART3_NO_cmd;
	       memcpy(&ulCrc, pucData + 16, sizeof(uint32_t));
	       for(int i = 0; i < 16; i++)
			{
			 pullData[i] = pucData[i];
			}
	       val=CRC_CalcBlockCRC((uint32_t*)pullData, 16);
	       if(ulCrc==val)
		{

			FLASH_Unlock();

			for (uint8_t i = 0; i < 16; i++) 
			{
				FLASH_ProgramByte(ulSaddr,pucData[i]);
				ulSaddr += 1;
			}
			FLASH_Lock();
			strTransmit(ACK);
		 }
	       else 
	       {
			strTransmit(NACK);
			return;


		}
		 }

		 else {
			   strTransmit(NACK);
			   return;

			 }

}
void cmdjump(uint8_t*p){

	uint32_t crc=0,val=0;
	current_cmd_Status = USART3_NO_cmd;
	memcpy(&crc, p + 5, sizeof(uint32_t));
	uint32_t pulData[5];
	for(int i = 0; i < 5; i++)
	pulData[i] = p[i];
	val=CRC_CalcBlockCRC(pulData, 5);
	if(crc==val)
	{

                  /* Get jump address */
		    uint32_t address =  *(uint32_t *)(p+1);
		    uint32_t val = * (uint32_t *) address;

		    /* Check if it has valid stack pointer in the RAM */
		    if(0x20000000 == (val & 0x20000000))
		    {   crc=0xfffff;
			strTransmit(ACK);
			while(crc--);
			      /* Disable all interrupts */
		      __disable_irq();

		      /* Reset GPIOB */
		      RCC->AHB1RSTR = (RCC_AHB1RSTR_GPIOBRST );
		      RCC->AHB1RSTR = (RCC_AHB1RSTR_GPIODRST );

		      /* Release reset */
		      RCC->AHB1RSTR = 0;

		      /* Reset USART3 */
		      RCC->APB1RSTR = RCC_APB1RSTR_USART3RST;

		      /* Release reset */
		      RCC->APB1RSTR = 0;

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
		     // RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE);
		      /* Vector Table Relocation in Internal FLASH */
		      __DMB();
		      SCB->VTOR = address;
		     __DSB();
		     void (*jump_address)(void) = (void *)(*((uint32_t *)(address + 4)));

		    /* Set stack pointer */
		    __set_MSP(val);

		   /* Jump */
		    jump_address();


		    }
	     else
	     	    
	     {

     		 strTransmit(NACK);
              }
	}
	
	else
	{

                    	  strTransmit(NACK);
                    	  return;

         }
}

void wr_prot(uint8_t*p){

		current_cmd_Status = USART3_NO_cmd;
		uint32_t crc_val=0;
		uint32_t pulCmd[] = { p[0], p[1] };
		memcpy(&crc_val, p+2, sizeof(uint32_t));
		if(crc_val==CRC_CalcBlockCRC(pulCmd, 2))
		{
			FLASH_Unlock();
			FLASH_OB_Unlock();
			FLASH->OPTCR &= ~((1 << p[1]) << 16);
			 FLASH->OPTCR |= FLASH_OPTCR_OPTSTRT;
			while(FLASH_WaitForLastOperation==FLASH_BUSY);
			FLASH_OB_Lock();
			FLASH_Lock();
		    	strTransmit(ACK);
		}
		else
		{

			strTransmit(NACK);
			return;

		}

}
void Re_wr_prot(uint8_t*p){

	       current_cmd_Status = USART3_NO_cmd;
		uint32_t crc_val=0;
		uint32_t pulCmd[] = { p[0], p[1] };
		memcpy(&crc_val, p+2, sizeof(uint32_t));
		if(crc_val==CRC_CalcBlockCRC(pulCmd, 2))
		{
			FLASH_Unlock();
			FLASH_OB_Unlock();
			FLASH->OPTCR |= ((1 << p[1]) << 16);
			FLASH->OPTCR |= FLASH_OPTCR_OPTSTRT;
			FLASH_OB_Lock();
			FLASH_Lock();
		        strTransmit(ACK);
		}
		else
		{

			strTransmit(NACK);
			return;
		}

}
void update(uint8_t*p){
	uint32_t crc=0,val=0;
		current_cmd_Status = USART3_NO_cmd;
		memcpy(&crc, p + 5, sizeof(uint32_t));
		uint32_t pulData[5];
		for(int i = 0; i < 5; i++)
			pulData[i] = p[i];
		val=CRC_CalcBlockCRC(pulData, 5);
		if(crc==val){
			uint32_t address =  *(uint32_t *)(p+1);
			if(0x8000000 == (address & 0x8000000))//valid image
			    {   
				crc=0xfffff;
				strTransmit(ACK);
				while(crc--);
				//save the new App address
				FLASH_Unlock();
				FLASH_EraseSector(FLASH_Sector_2, VoltageRange_3);
				FLASH_Lock();
				FLASH_Unlock();
				FLASH_ProgramWord((uint32_t)(0x08008000),address);
				FLASH_Lock();
			    }
			else{
				strTransmit(NACK);
			     }


		}
	else	
	{
			strTransmit(NACK);
	}


}
