#include <stdio.h>
#include "stm32l4xx.h"
#include "stm32l475e_iot01.h"
#include "stm32l4xx_it.h"
#include "FreeRTOS.h"
#include "task.h"

#include "spiffs_fs.h"
#include "spi_wifi.h"
#include "board.h"

/* Activated with the User button. */
extern volatile uint8_t wifiControl;

/*
 * @brief          prints one character on the console. Used for printf.
 * @param[in]  xCh char to print
 * @return         none.
 */
int __io_putchar(int ch) {
  boardPutChar(ch);
  return ch;
}

/*
 * @brief                   Wifi function to enable or disable wifi when user button is pressed.
 *                          Prints cannot be sent when the UART is waiting on input from user.
 * @param[in]  pParameters  Unused parameter list for the task
 * @return                  none.
 */
void wifiTask
(
  void* pParameters
)
{
  ES_WIFI_APState_t wifiState = ES_WIFI_AP_NONE;
  ES_WIFI_Status_t wifiResult = ES_WIFI_STATUS_OK;

  /* Init needs HAL_Delay to be enabled. */
  wifiResult = wifiInit();
  if (ES_WIFI_STATUS_OK != wifiResult)
  {
    printf("Could not initialize WIFI.\n");
    printf("Limited test availability.\n");
    vTaskDelete(NULL);
  }

  for(;;)
  {
    if (ES_WIFI_STATUS_OK == wifiResult)
    {
      if ((0 != wifiControl) && (ES_WIFI_AP_JOINED != wifiState))
      {
        /* Enable Wifi */
        printf("Enable Wifi\n");
        if (ES_WIFI_STATUS_OK != wifiConnectTo())
        {
          printf("Failure to enable Wifi\n");
          /* Keep old state */
        }
        else
        {
          wifiState = ES_WIFI_AP_JOINED;
        }
      }
      else if ((0 == wifiControl) && (ES_WIFI_AP_NONE != wifiState))
      {
        printf("Disable Wifi\n");
        if (ES_WIFI_STATUS_OK != wifiDisconnect())
        {
          printf("Failure to disable Wifi\n");
          /* Keep old state */
        }
        else
        {
          wifiState = ES_WIFI_AP_NONE;
        }
      }
    }
    HAL_Delay(100);
  }

  vTaskDelete(NULL);
}

/*
 * @brief                   freeRTOS task test function.
 * @param[in]  pParameters Unused parameter list for the task
 * @return                  none.
 */
void testTask
(
  void* pParameters
)
{
  char charInput;
  char itemPath[SPIFFS_OBJ_NAME_LEN] = { 'a', 'b', 'c', '\0'};
  const unsigned char data[16] = {'1', '2', '3'};
  size_t dataSize = sizeof(data);

  const unsigned char dataRead[16] = {'1', '2', '3'};
  s32_t dataReadSize = sizeof(dataRead);

  spiffs_file fd;
  int32_t resultWrite = 0;
  int32_t  statusFileSystem = 0;

  /* Wait for network prints from wifiTask */
  HAL_Delay(1000);
  for(;;)
  {
    charInput = boardGetChar();
    if (((charInput < 'a') || (charInput > 'd')) &&
        ((charInput < '0') || (charInput > '9')))
    {
      HAL_Delay(100);
      continue;
    }

    printf("Option: %c.", charInput);

    switch(charInput) {
    case 'a':
      spiffsFormat();
      break;
    case 'b':
      spiffsDeleteFiles(NULL);
      break;
    case 'c':
      SPIFFS_clearerr(&gSpiffsFs);
      printf("Set storage: %s", itemPath);

      fd = SPIFFS_open(&gSpiffsFs, itemPath, SPIFFS_CREAT | SPIFFS_RDWR, 0);
      resultWrite = SPIFFS_write(&gSpiffsFs, fd, (u8_t*) data, dataSize);
      if (dataSize != resultWrite)
      {
        printf(" Write: Expected %d, got %d", dataSize, resultWrite);
        printf(" File system error %d", gSpiffsFs.err_code);
      }

      SPIFFS_close(&gSpiffsFs, fd);
      break;

    case 'd':

      SPIFFS_clearerr(&gSpiffsFs);
      fd = SPIFFS_open(&gSpiffsFs, itemPath, SPIFFS_RDONLY, 0);
      if (SPIFFS_ERR_NOT_FOUND == gSpiffsFs.err_code) {
        printf("Could not find file.");
        break;
      }

      int32_t resultRead = SPIFFS_read(&gSpiffsFs, fd, (u8_t *)data, &dataReadSize);
      if (resultRead < 0)
      {
        printf("Could not read file.");
        SPIFFS_close(&gSpiffsFs, fd);
        break;
      }

      s32_t seekCurrent = SPIFFS_lseek(&gSpiffsFs, fd, 0, SPIFFS_SEEK_CUR);
      s32_t seekEnd = SPIFFS_lseek(&gSpiffsFs, fd, 0, SPIFFS_SEEK_END);
      if (0 < (seekEnd - seekCurrent))
      {
        /* There is data unread in the file. */
      }

      SPIFFS_close(&gSpiffsFs, fd);
      break;

    case 'e':

      SPIFFS_clearerr(&gSpiffsFs);
      statusFileSystem = SPIFFS_remove(&gSpiffsFs, data);
      if (SPIFFS_OK != statusFileSystem)
      {
        if (SPIFFS_ERR_NOT_FOUND == statusFileSystem)
        {
          printf("File not found.");
        }
        break;
      }
      break;

    default:
      break;
    }
  }

  vTaskDelete(NULL);
}

int main
(
  void
)
{
  char charInput;
  BOARD_Status status = boardInit();
  if(BOARD_ERROR_FATAL == status)
  {
	  while(1);
  } /* if */

  status = spiffsMount();
  if(BOARD_ERROR_FATAL == status)
  {
    printf("Could not init filesystem.\n");

    printf("Press a key to wipe the memory.\n");
    charInput = boardGetChar();
    boardLed(0);
    printf("Wiping...\n");
    boardMemoryQspiWipe();
    printf("Done, restart the board..\n");

    while(1);
  } /* if */

  spiffsListFile();

#ifdef C_BOARD_USE_FREE_RTOS
  xTaskCreate(wifiTask, "Wifi Control", 2048, NULL /* parameters */, 10, NULL);
  xTaskCreate(testTask, "Tests", 4096, NULL /* parameters */, 8, NULL);
  /* Start the scheduler. */
  vTaskStartScheduler();
#else
  HAL_InitTick(0);
  testTask(NULL);
#endif

  return 0;
}
