#ifndef DEVICE_BOARD_H_
#define DEVICE_BOARD_H_

#include <stdint.h>

/**
 * @brief  Board Status
 */
typedef enum
{
  BOARD_OK             = 0x00U,
  BOARD_ERROR          = 0x01U,
  BOARD_ERROR_FATAL    = 0x02U,
  BOARD_TIMEOUT        = 0x03U,
  BOARD_BAD_PARAMETER  = 0x04U,
  BOARD_BUSY           = 0x05U
} BOARD_Status;

/**
 * @brief    initialize the IoT device
 * @return   BOARD_OK or appropriate error code
 */
BOARD_Status boardInit();

/**
 * @brief              prints one character on the console
 * @param[in]  xCh     char to print
 * @return             none
 */
void     boardPutChar
(
  char xCh
);

/**
 * @brief              gets character from console
 * @return             character key code or 0 if no key has been pressed
 */
char     boardGetChar();

/**
 * @brief              waits for a while
 * @param[in] xDelay   delay in ms
 * @return             none
 */
void     boardDelay
(
  uint32_t xDelay
);

/**
 * @brief              get system tick
 * @return             system tick in ms
 */
uint32_t boardGetTick();

/**
 * @brief              sets the color of the LED
 * @param[in] xStatus  On or Off
 * @return             none
 */
void boardLed
(
  int xStatus
);

#endif /* DEVICE_BOARD_H_ */
