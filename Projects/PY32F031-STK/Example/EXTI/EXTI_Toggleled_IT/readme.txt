================================================================================
                                样例使用说明
                             Sample Description
================================================================================
功能描述：
此样例演示了GPIO外部中断功能，PB1引脚上的每一个下降沿都会产生中断，中断函数中
LED灯会翻转一次。

Function descriptions:
This sample demonstrates the functionality of GPIO external interrupts. 
Whenever a falling edge is detected on pin PB1, an interrupt is triggered, 
and the interrupt handler toggles the state of the LED.
================================================================================
测试环境：
测试用板：PY32F031_STK
MDK版本： 5.28
IAR版本： 9.20
GCC 版本：GNU Arm Embedded Toolchain 10.3-2021.10

Test environment:
Test board: PY32F031_STK
MDK Version: 5.28
IAR Version: 9.20
GCC Version: GNU Arm Embedded Toolchain 10.3-2021.10
================================================================================
使用步骤：
1. 编译下载程序到MCU，并运行；
2. PB1引脚上每产生一次下降沿，LED翻转就一次；

Example execution steps:
1.Compile and download the program to the MCU and run it.
2.Toggle the LED once for every falling edge generated on the PB1 pin;
================================================================================
注意事项：

Notes:

================================================================================