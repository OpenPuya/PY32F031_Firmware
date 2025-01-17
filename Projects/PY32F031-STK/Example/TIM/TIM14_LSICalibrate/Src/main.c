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

/* Private define ------------------------------------------------------------*/
#define UpLimit    1028
#define LowLimit   1020

/* Private variables ---------------------------------------------------------*/
uint32_t             Temp_LSItrim;                /* LSI clock trimming */
uint32_t             Capture_last_cnt;            /* The number of capture when the last update interrupt occurred */ 
uint32_t             Capture_cnt;                 /* The current number of capture */
uint32_t             Adjustcnt = 0;               /* Number of adjustments */
uint32_t             CaliState = 0;               /* Show calibration result */
TIM_HandleTypeDef    TimHandle = {0};
TIM_IC_InitTypeDef   sICConfig = {0};

/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define RCC_LSI_CALIBRATIONVALUE_ADJUST(__LSICALIBRATIONVALUE__) \
                  MODIFY_REG(RCC->ICSCR, RCC_ICSCR_LSI_TRIM_Msk, ((uint32_t)(__LSICALIBRATIONVALUE__) << RCC_ICSCR_LSI_TRIM_Pos))
#define RCC_GET_LSI_TRIM_VALUE()  ((uint32_t)(READ_BIT(RCC->ICSCR,RCC_ICSCR_LSI_TRIM)))
#define MAXAdjustcnt  100

/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void APP_TIM14Config(void);

/**
  * @brief   Main program
  * @retval  int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Systick. */
  HAL_Init();

  /* Initialize button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

  /* Initialize led */  
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Off(LED_GREEN);
    
  /* Configure PA08 pin as MCO1 function to output the LSI clock */
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCOSOURCE_LSI, RCC_MCODIV_1);

  /* Wait for the button to be pressed to enable LSI */
  while (BSP_PB_GetState(BUTTON_KEY) == 1)
  {
  }
  /* Configure system clock,and enable LSI */
  APP_SystemClockConfig();
  
  /* Initialize TIM14 */
  APP_TIM14Config();
  
  while (1)
  {
    if(CaliState == 1)
    {
      BSP_LED_On(LED_GREEN);
      while(1);
    }
    BSP_LED_Toggle(LED_GREEN);
    HAL_Delay(200);
  }
}

/**
  * @brief   Configure TIM14
  * @param   None
  * @retval  None
  */
static void APP_TIM14Config(void)
{
  TimHandle.Instance = TIM14;                                          /* Select TIM14 */
  TimHandle.Init.Period            = 10001 - 1;                        /* Auto reload value：10001-1 */
  TimHandle.Init.Prescaler         = 100 - 1;                          /* Prescaler:100-1 */
  TimHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;           /* Clock division: tDTS=tCK_INT */
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;               /* CounterMode:Up */
  TimHandle.Init.RepetitionCounter = 1 - 1;                            /* repetition counter value:1-1 */
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;   /* TIM14_ARR register is not buffered */
  /* Initialize TIM14 */
  if (HAL_TIM_IC_Init(&TimHandle) != HAL_OK)
  {
    APP_ErrorHandler();
  }
  
  sICConfig.ICPolarity  = TIM_ICPOLARITY_RISING;                       /* Capture triggered by rising edge */
  sICConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;                    /* CC1 channel is configured as input */
  sICConfig.ICPrescaler = TIM_ICPSC_DIV1;                              /* Capture input not divide */
  sICConfig.ICFilter    = 0;                                           /* Input not filtered */
  /* Initializes the TIM Input Capture Channel 1 */
  if (HAL_TIM_IC_ConfigChannel(&TimHandle, &sICConfig, TIM_CHANNEL_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
  
  /* TIM14 TI1 is connected to MCO */
  if (HAL_TIMEx_RemapConfig(&TimHandle,TIM_TIM14_MCO) != HAL_OK)
  {
    APP_ErrorHandler();
  }
  
  /* Starts the TIM Input Capture measurement */
  if (HAL_TIM_IC_Start_IT(&TimHandle, TIM_CHANNEL_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* Enable update interrupt */
  __HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);
  
}

/**
  * @brief  TIM captures interrupt callback
  * @param  htim：TIM handle
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{  
  Capture_cnt++;
}

/**
  * @brief  TIM update interrupt callback
  * @param  htim：TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  uint32_t Temp_delta = 0;
  /* Count the number of times a capture occurs between update interrupts */
  Temp_delta = Capture_cnt - Capture_last_cnt;
  
  /* Get current trimming of LSI */
  Temp_LSItrim = RCC_GET_LSI_TRIM_VALUE();
  
  if(Adjustcnt > MAXAdjustcnt)
  {
    /* STOP TIM14 */
    HAL_TIM_Base_Stop_IT(htim);
    HAL_TIM_IC_Stop_IT(htim,TIM_CHANNEL_1);
    return;
  }
  
  /* Adjust LSI trimming value, target frequency is 32.768KHz  */
  if (Temp_delta < LowLimit)
  {
    Temp_LSItrim = (Temp_LSItrim >> RCC_ICSCR_LSI_TRIM_Pos)+1;
    RCC_LSI_CALIBRATIONVALUE_ADJUST(Temp_LSItrim);
  }
  else if (Temp_delta > UpLimit)
  {
    
    Temp_LSItrim = (Temp_LSItrim >> RCC_ICSCR_LSI_TRIM_Pos)-1;
    RCC_LSI_CALIBRATIONVALUE_ADJUST(Temp_LSItrim);
  }
  else
  {
    /* Calibrate ok,STOP TIM14 */
    HAL_TIM_Base_Stop_IT(htim);
    HAL_TIM_IC_Stop_IT(htim,TIM_CHANNEL_1);

    CaliState = 1;
  }    
    
  Capture_last_cnt = Capture_cnt;
  Adjustcnt++;
}

/**
  * @brief  System clock configuration function
  * @param  None
  * @retval None
  */
static void APP_SystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Oscillator configuration */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE; /* Select oscillator HSE, HSI, LSI, LSE */
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                          /* Enable HSI */
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                          /* HSI 1 frequency division */
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz; /* Configure HSI clock 16MHz */
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                         /* Close HSE */
  RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;                          /* Enable LSI */
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                         /* Close LSE */
  /*RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;*/
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                      /* PLL ON */
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  /* Configure oscillator */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* Clock source configuration */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; /* Choose to configure clock HCLK, SYSCLK, PCLK1 */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; /* Select PLL as the system clock */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;        /* AHB clock 1 division */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;         /* APB clock 1 division */
  /* Configure clock source */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  Error executing function.
  * @param  None
  * @retval None
  */
void APP_ErrorHandler(void)
{
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* Users can add their own printing information as needed,
     for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
