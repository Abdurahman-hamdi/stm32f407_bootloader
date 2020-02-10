#ifndef __uart_enum_H_
#define __uart_enum_H_
typedef enum
{
  USART3_NO_cmd,
  USART3_CMD_RECEIVED,

} USART3_cmd_StatusType;
#define MAX_BUFFER_LENGTH                     ((uint32_t) 200u)
void USART3_Init(void);
void USART3_Enable(void);
void NVIC_Int(void);
void USART3_IRQ_Callback(void);
//void USART3_Process(void);
void strTransmit(const char data);
#endif
