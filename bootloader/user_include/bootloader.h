#ifndef __BL_enum_H_
#define __BL_enum_H_
#include"uart.h"
#include "stm32f4xx.h"
#include"stm32f4xx_flash.h"
#define ACK        0x79
#define NACK       0x1F
#define CMD_GETID  0x02
#define CMD_WRITE  0x2b
#define CMD_ERASE  0x43
#define CMD_Jump   0x44
#define CMD_wr_p   0x33
#define RE_wr_pr   0x55
#define up_date     0x66

void boot_process(void);
void cmdErase(uint8_t*pucData);
void cmdWrite(uint8_t *pucData);
void cmdjump(uint8_t*p);
void wr_prot(uint8_t*p);
void Re_wr_prot(uint8_t*p);
void update(uint8_t*p);
#endif
