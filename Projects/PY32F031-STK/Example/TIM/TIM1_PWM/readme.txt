================================================================================
                                样例使用说明
                                Sample Description
================================================================================
功能描述：
本例程输出4路PWM，通道1的占空比为20%，通道2为40%，通道3为60%，通道4为80%，本例
程周期为8000000/(50+1)/800=196Hz

Function descriptions:
This sample outputs 4 channels PWM, the duty cycle of channel 1 is 20%,channel 2
is 40%, channel 3 is 60%,channel 4 is 80%.The period is 8000000/(50+1)/800=196Hz
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
1. 编译并下载程序；
2. 用逻分或者示波器连接四个通道PA8(TIM1_CH1),PA9(TIM1_CH2),PA10(TIM1_CH3)和
PA11(TIM1_CH4)
3. 查看发现周期为196Hz，占空比分别为20%，40%，60%，80%

Example execution steps:
1. compile and download the program to MCU and run it;
2. Observe four channels PA8(TIM1_CH1),PA9(TIM1_CH2),PA10(TIM1_CH3) and 
PA11(TIM1_CH4) with logic analyzer or oscilloscope
3. Check that the discovery period is 196Hz, and the duty cycle is 20%, 40%, 
60%, and 80%, respectively
================================================================================
注意事项：

Notes:

================================================================================