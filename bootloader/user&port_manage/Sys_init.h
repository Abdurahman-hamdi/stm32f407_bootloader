#ifndef __SYS_INIT__
#define __SYS_INIT__

typedef enum{
	__FAILED,
	__SUUCESS

}oper_statues;

typedef enum{
	__ACK=0x79,
	__NACK=0x1F


}cmd_oper;


//Perform branching to existed firmware when system runs after power-on reset
void jump_to_exist_app(uint32_t*);

//perform branching to new_firmware when system receives an upgrade_request and enters boatloader or where there is no firmware found
oper_statues jump_to_new_app(uint32_t*,void (*)(cmd_oper ));




#endif
