#ifndef DEVICE_BUTTON_H_
#define DEVICE_BUTTON_H_

#include "stm32l4xx_hal.h"

#define BOARD_BUTTON_GPIO_BANK GPIOC //!< GPIO Bank for the Button GPIO
#define BOARD_BUTTON_GPIO_PIN  GPIO_PIN_13 //!< GPIO Pin for the Button GPIO

/*
 * @brief  Board Button Status
 */
typedef enum
{
  BOARD_USER_BUTTON_RELEASED = 0x00U,
  BOARD_USER_BUTTON_PRESSED  = 0x01U,
  BOARD_USER_BUTTON_NA       = 0x02U
} BOARD_Button_Status;

/*
 * @brief            prints a message on the console and waits until user button is pressed
 * @param[in] xpMsg  message to print
 * @return           none
 */
void boardUserButtonWait
(
  char* xpMsg
);

/*
 * @brief            this function is called each time the user button is either pressed or released
 * @return           none
 */
void boardUserButtonTickHandler
(
  void
);

/*
 * @brief              Gets the number of times a given key event has been observed.
 *                     After the call the given counter is set back to 0
 * @return             number of key events
 */
uint32_t boardUserButtonGetNumEvents
(
  void
);

/*
 * @brief        get status of user button
 * @return       status
 */
BOARD_Button_Status boardUserButtonGetStatus
(
  void
);

#endif /* DEVICE_BUTTON_H_ */
