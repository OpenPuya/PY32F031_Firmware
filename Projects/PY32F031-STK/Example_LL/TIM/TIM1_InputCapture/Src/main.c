/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by Puya under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "py32f031xx_ll_Start_Kit.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void APP_ConfigTIM1(void);

/**
  * @brief  Main program
  * @param  None
  * @retval int
  */
int main(void)
{
  /* Configure Systemclock */
  APP_SystemClockConfig(); 
  
  /* Enable SYSCFG and PWR clock */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  
  /* Enable TIM1 clock */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);

  /* Enable GPIOA clock */
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  
  /*Initialize LED*/
  BSP_LED_Init(LED3);
    
  /* Configure TIM1 */
  APP_ConfigTIM1();

  while (1)
  {
  }
}

/**
  * @brief  Configure TIM1 
  * @param  None
  * @retval None
  */
static void APP_ConfigTIM1(void)
{
  LL_TIM_InitTypeDef TIM1CountInit = {0};
  LL_GPIO_InitTypeDef TIM1ChannelInit = {0};
  
  TIM1CountInit.ClockDivision       = LL_TIM_CLOCKDIVISION_DIV1;  /* Clock no divider */
  TIM1CountInit.CounterMode         = LL_TIM_COUNTERMODE_UP;      /* Count mode:up */
  TIM1CountInit.Prescaler           = 1000-1;                     /* prescaler：1000 */
  TIM1CountInit.Autoreload          = 12800-1;                    /* Autoreload value：12800 */
  TIM1CountInit.RepetitionCounter   = 0;                          /* RepetitionCounter value：0 */
  
  /* Initialize TIM1 */
  LL_TIM_Init(TIM1,&TIM1CountInit);

  /* Set CH1 as input mode */
  LL_TIM_IC_SetActiveInput(TIM1,LL_TIM_CHANNEL_CH1,LL_TIM_ACTIVEINPUT_DIRECTTI);

  /* The circuit is sensitive to TIxFP1 rising edge, TIxFP1 is not inverted */
  LL_TIM_IC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_RISING);
  
  /* Enable CC interrupt */
  LL_TIM_EnableIT_CC1(TIM1);
  NVIC_SetPriority(TIM1_CC_IRQn, 1);
  NVIC_EnableIRQ(TIM1_CC_IRQn);
  
  /* Map CH1 to PA8 */
  TIM1ChannelInit.Pin       = LL_GPIO_PIN_8;
  TIM1ChannelInit.Pull      = LL_GPIO_PULL_DOWN;
  TIM1ChannelInit.Mode      = LL_GPIO_MODE_ALTERNATE;
  TIM1ChannelInit.Alternate = LL_GPIO_AF_2;
  TIM1ChannelInit.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
  TIM1ChannelInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA,&TIM1ChannelInit);
  
  /* Enable TIM1 CH1 */
  LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH1);
  
  /* Enable TIM1 */
  LL_TIM_EnableCounter(TIM1);


}

/**
  * @brief  TIM1 CH1 input capture callback
  * @param  None
  * @retval None
  */
void APP_CCCallback(void)
{
  /* toggle LED */
  BSP_LED_Toggle(LED3);
}

/**
  * @brief  Configure Systemclock
  * @param  None
  * @retval None
  */
static void APP_SystemClockConfig(void)
{
  /* Enable HSI */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* Set AHB divider:HCLK = SYSCLK*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* HSISYS used as SYSCLK source */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSISYS);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSISYS)
  {
  }

  /* Set APB1 divider */
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_Init1msTick(8000000);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(8000000);
}

/**
  * @brief  Error handling function
  * @param  None
  * @retval None
  */
void APP_ErrorHandler(void)
{
  /* Infinite loop */
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file：Pointer to the source file name
  * @param  line：assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add His own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
