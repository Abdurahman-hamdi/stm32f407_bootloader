#include "bootloader.h"
#include "stm32f4xx_gpio.h"
#include "gpio.h"
#include "uart.h"

#include <string.h>

extern USART3_cmd_StatusType current_cmd_Status;//CMD received status which is defined in uart.c
extern uint8_t RxBuffer[MAX_BUFFER_LENGTH + 1];//cmd buffer which is defined in uart.c

void boot_process(){
	 led_init();
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

		 else
		 {
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
		    uint32_t *address =  *(uint32_t *)(p+1);
		    jump_to_new_app(address, strTransmit);

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
			if(FLASH_SADDR == (address & FLASH_SADDR))//valid image
			    {   
				strTransmit(ACK);
				//save the new App address
				FLASH_Unlock();
				FLASH_EraseSector(FLASH_Sector_2, VoltageRange_3);
				FLASH_Lock();
				FLASH_Unlock();
				FLASH_ProgramWord((uint32_t)(IMAGE_SADDR),address);
				FLASH_Lock();
			    }
			else
			    {
				strTransmit(NACK);
			     }


		}
	else	
	{
			strTransmit(NACK);
	}


}
