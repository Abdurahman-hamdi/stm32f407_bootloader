# stm32f407_bootloader
Author  Abdurahman hamdi
Email  bg201540@gmail.com
Date   10.2.2020
>>>
This a simple bootloader for stm32f407 div-board which is based on cortexM-4 arch.
Bootloader allows firmware to be updated easily without need to use in-circuit debugger/programmer(isp),using commuincation protocols like usart,isp,i2c or can.
Here i use a usart as commuication protocol with 38.4kbs to upgrade the firmware where the host generates .hex file and sends it to board with the same protocol.



