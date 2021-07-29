# stm32f407_bootloader
Author : Abdurahman hamdi
Email : bg201540@gmail.com
Date  : 10.2.2020
>>> 
This a simple bootloader for stm32f407 div-board which is based on cortex-M4 arch.
Bootloader allows firmware to be updated easily without need to use in-circuit debugger/programmer(isp),using commuincation protocols like usart,isp,i2c or can.
Here i use a usart as commuication protocol with 38.4kbs to upgrade the firmware where the host generates .hex file and sends it to board with the same protocol.
Operation principle>>
Once flash has no images to run,the bootloader resides in start of flash at 0x8000000 to be executed directly after power-on reset, bootloader first checks a dedicated address in flash (0x08008000) which contains the starting address of existing firmware and a 32_bit buffer shared between app and the bootloader which is set during app execution if app receives and allows bootlodaer to serve the upgrade request (receives->sets buf ->jumps to bootloader).
For more safety during upgrading the firmware,2 different flash sectors are assigned to hold the firmware one sector at atime (one sector holds firmware ,once upgrade request has come the ,new firmware stored at the free sector and then erase the other sector which contains the old firmware ,and the address 0x08008000 get updated to hold the new app address(one of the two sectors start address) ) and then asks the host to provide write-protection for the flash or to remove it,and then jumps to the upgraded firmware.
once an upgrade request during app execution  the app user can accept this request or not, to allow user to accept this request ,app sets the shared buffer and preforms system reset===jumps to bootloader (depending on user decision) and then bootloader checks the above conditions which are mention in section 10.
.
.
.
.
i have modified the functionality of this python script (https://github.com/cnoviello/mastering-stm32/blob/master/nucleo-l073RZ/src/ch20/flasher.py) to meet the design and gives more flexibility which is provide a channel and commands exchanging between host(pc) and the target(div-board)...........................................................................
                                                        Thanks for reading

References and url links.
>mastering stm32\n
>stm32f407 reference manual\n
>https://www.beningo.com/wp-content/uploads/images/Papers/bootloader_design_for_microcontrollers_in_embedded_systems%20.pdf
 

