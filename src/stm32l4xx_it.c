/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @author  Ac6
  * @version V1.0
  * @date    02-Feb-2015
  * @brief   Default Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif
#include "stm32l4xx_it.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            	  	    Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles SysTick Handler, but only if no RTOS defines it.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
#ifdef USE_RTOS_SYSTICK
	osSystickHandler();
#endif
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/**
  * @brief  This function handles external lines 1interrupt request.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void)
{
  if (RESET != __HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1))
  {
    // Serve interrupt.
    SPI_WIFI_ISR();
    // Clean interrupt.
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
    HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
  }

}
