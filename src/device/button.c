#include <stdint.h>
#include <stddef.h>

#include "stm32l4xx_hal.h"

#include "button.h"
#include "board.h"

volatile uint32_t gUserButtonEventNb = 0;   /*!< counter of event */
volatile uint8_t wifiControl = 0;

/*
 * @brief              Handler for EXTI10 to 15. Used for User button.
 * @return             None
 */
void EXTI15_10_IRQHandler
(
  void
)
{
  if (RESET != __HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13))
  {
    /* Serve interrupt. */
    boardUserButtonTickHandler();
    /* Clean interrupt. */
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
    HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
  }
}

/*
 * @brief              Gets the number of times a given key event has been observed.
 *                     After the call the given counter is set back to 0
 * @param[in] xEvent   user button event (BOARD_USER_BUTTON_SHORT)
 * @return             number of key events
 */
uint32_t boardUserButtonGetNumEvents
(
  void
)
{
  uint32_t ret = 0;
  ret = gUserButtonEventNb;
  gUserButtonEventNb = 0;
  return ret;
} /* boardUserButtonGetNumEvents() */

/*
 * @brief   This function is called each time the user button is either pressed or released
 * @return  none
 */
void boardUserButtonTickHandler
(
  void
)
{
  static BOARD_Button_Status  lastStatus    = BOARD_USER_BUTTON_NA;
  BOARD_Button_Status         curStatus     = BOARD_USER_BUTTON_NA;

  curStatus = boardUserButtonGetStatus();

  /* Button pressed and button was previously released. */
  if((BOARD_USER_BUTTON_PRESSED == curStatus) && (BOARD_USER_BUTTON_RELEASED == lastStatus))
  {

    if (0 == wifiControl)
    {
      wifiControl = 1;
    }
    else
    {
      wifiControl = 0;
    }

    /* Register the event */
    gUserButtonEventNb++;
  } /* if */

  lastStatus = curStatus;
} /* boardUserButtonTickHandler() */

/*
 * @brief            Prints a message on the console and waits until user button is pressed
 * @param[in] xpMsg  message to print \0 terminated.
 * @return           none
 */
void boardUserButtonWait
(
  char* xpMsg
)
{
  if(NULL != xpMsg)
  {
    salLogPrint(xpMsg);
  } /* if */

  while(boardUserButtonGetNumEvents() == 0)
  {
    boardDelay(1);
  } /* while */
} /* boardUserButtonWait() */

/*
 * @brief        get status of user button
 * @return       status
 */
BOARD_Button_Status boardUserButtonGetStatus
(
  void
)
{
  return HAL_GPIO_ReadPin(BOARD_BUTTON_GPIO_BANK, BOARD_BUTTON_GPIO_PIN) ?
    BOARD_USER_BUTTON_PRESSED : BOARD_USER_BUTTON_RELEASED;
} /* boardUserButtonGetStatus() */
