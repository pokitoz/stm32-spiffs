#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "stm32l4xx_hal.h"
#include "board.h"

static UART_HandleTypeDef hUart1; //!< UART HAL Structure */


/*
 * @brief              Initialize system clocks and GPIO clocks
 * @return             BOARD_OK on success. BOARD_ERROR_FATAL on error.
 */
static BOARD_Status initClock
(
  void
)
{
  BOARD_Status status = BOARD_OK;

  for(;;)
  {
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /* MSI is enabled after System reset, activate PLL with MSI as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLP = 7;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      /* Initialization Error */
      status = BOARD_ERROR_FATAL;
      break;
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
      /* Initialization Error */
      status = BOARD_ERROR_FATAL;
      break;
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                                |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_RNG;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_PLLSAI1;
    PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
    PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
    PeriphClkInit.PLLSAI1.PLLSAI1N = 24;
    PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
    PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
    PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
    PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK;
    if (HAL_OK != HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit))
    {
      /* Initialization Error */
      status = BOARD_ERROR_FATAL;
      break;
    }

    /* Device clocks */
    __HAL_RCC_USART1_CLK_ENABLE(); /* UART through Debug/STLink */
    __HAL_RCC_SPI3_CLK_ENABLE();   /* SPI Wifi */

    /* Enable GPIO_x clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();  /* UART1, Wifi and LED GPIOs */
    __HAL_RCC_GPIOC_CLK_ENABLE();  /* User button and Wifi*/
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();  /* Wifi */

    break;
  }

  return status;
} /* initClock() */

/*
 * @brief              Initialize GPIO pins
 * @return             none.
 */
static void initGpio
(
  void
)
{
  GPIO_InitTypeDef   gpio_init = {0};

  /* Configure GPIO pin Output Level. */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

  /* PicoSE Power PA4. */
  gpio_init.Pin = GPIO_PIN_4;
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &gpio_init);

  /* User button PC13. */
  gpio_init.Pin = GPIO_PIN_13;
  gpio_init.Mode = GPIO_MODE_IT_RISING_FALLING;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &gpio_init);

  /* User LED PB14. */
  gpio_init.Pin = GPIO_PIN_14;
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &gpio_init);

  /* UART1 PB6 PB7. */
  gpio_init.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  gpio_init.Mode = GPIO_MODE_AF_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOB, &gpio_init);

  /* WIFI */
  /* configure Wake up pin PB13*/
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
  gpio_init.Pin       = GPIO_PIN_13;
  gpio_init.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull      = GPIO_NOPULL;
  gpio_init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &gpio_init );

  /* configure Data ready pin PE1*/
  gpio_init.Pin       = GPIO_PIN_1;
  gpio_init.Mode      = GPIO_MODE_IT_RISING;
  gpio_init.Pull      = GPIO_NOPULL;
  gpio_init.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &gpio_init );

  /* configure Reset pin PE8*/
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET);
  gpio_init.Pin       = GPIO_PIN_8;
  gpio_init.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull      = GPIO_NOPULL;
  gpio_init.Speed     = GPIO_SPEED_FREQ_LOW;
  gpio_init.Alternate = 0;
  HAL_GPIO_Init(GPIOE, &gpio_init );

  /* configure SPI NSS pin PE0 */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);
  gpio_init.Pin       =  GPIO_PIN_0;
  gpio_init.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull      = GPIO_NOPULL;
  gpio_init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init( GPIOE, &gpio_init );

  /* configure SPI CLK pin PC10 */
  gpio_init.Pin       =  GPIO_PIN_10;
  gpio_init.Mode      = GPIO_MODE_AF_PP;
  gpio_init.Pull      = GPIO_NOPULL;
  gpio_init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  gpio_init.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &gpio_init );

  /* configure SPI MOSI pin PC12*/
  gpio_init.Pin       = GPIO_PIN_12;
  gpio_init.Mode      = GPIO_MODE_AF_PP;
  gpio_init.Pull      = GPIO_NOPULL;
  gpio_init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  gpio_init.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init( GPIOC, &gpio_init );

  /* configure SPI MISO pin PC11 */
  gpio_init.Pin       = GPIO_PIN_11;
  gpio_init.Mode      = GPIO_MODE_AF_PP;
  gpio_init.Pull      = GPIO_PULLUP;
  gpio_init.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  gpio_init.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init( GPIOC,&gpio_init );
} /* initGPIO() */

/*
 * @brief              Initialize IRQ
 * @return             none.
 */
static void initIrq
(
  void
)
{
  /* Configure IRQ for the User button. */
  HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 8, 8);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  __enable_irq();

} /* initIRQ() */

/*
 * @brief              Initialize UART interface
 * @return             BOARD_OK on success. BOARD_ERROR_FATAL on error.
 */
static BOARD_Status initUart()
{
  BOARD_Status      status    = BOARD_OK;
  HAL_StatusTypeDef halStatus = HAL_OK;

  hUart1.Instance = USART1;
  hUart1.Init.BaudRate = 115200;
  hUart1.Init.WordLength = UART_WORDLENGTH_8B;
  hUart1.Init.StopBits = UART_STOPBITS_1;
  hUart1.Init.Parity = UART_PARITY_NONE;
  hUart1.Init.Mode = UART_MODE_TX_RX;
  hUart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hUart1.Init.OverSampling = UART_OVERSAMPLING_16;

  halStatus = HAL_UART_Init(&hUart1);

  if (HAL_OK != halStatus)
  {
    status = BOARD_ERROR_FATAL;
  } /* if */

  return status;
} /* initUART() */


/*
 * @brief    initialize the IoT board
 * @return   BOARD_OK. Or Appropriate error code.
 */
BOARD_Status boardInit
(
  void
)
{
  BOARD_Status      status    = BOARD_OK;
  HAL_StatusTypeDef halStatus = HAL_OK;

  for (;;)
  {
    halStatus = HAL_Init();
    if (HAL_OK != halStatus)
    {
      status = BOARD_ERROR_FATAL;
      break;
    } /* if */

    status = initClock();
    if (BOARD_OK != status)
    {
      break;
    } /* if */

    initGpio();

    /* HAL boot, switch on the GREEN LED */
    boardLed(1);

    status = initUart();
    if (BOARD_OK != status)
    {
        break;
    } /* if */

    printf("\r\n[" __DATE__"|" __TIME__ "] ");

    status = boardMemoryQspiInit();
    if (BOARD_OK != status)
    {
      break;
    } /* if */

    initIrq();

    break;
  }

  if (BOARD_ERROR_FATAL == status)
  {
    boardLed(0);
  } /* if */

  return status;
} /* boardInit() */

/*
 * @brief              sets the color of the LED
 * @param[in] xStatus  status of the GPIO.
 * @return             none.
 */
void boardLed
(
  int xStatus
)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (xStatus == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
} /* boardLed() */

/*
 * @brief              prints one character on the console
 * @param[in]  xCh     char to print
 * @return             none.
 */
void boardPutChar
(
  char xCh
)
{
  HAL_UART_Transmit(&hUart1, (uint8_t*) &xCh, sizeof(xCh), 100);
} /* boardPutChar() */

/*
 * @brief              gets character from console
 * @return             character key code or 0 if no key has been pressed
 */
char boardGetChar()
{
  char ch;
  HAL_StatusTypeDef statusUart = HAL_UART_Receive(&hUart1, (uint8_t*) &ch, sizeof(ch), HAL_MAX_DELAY);
  if (HAL_OK != statusUart)
  {
    ch = '\0';
  } /* if */

  return ch;
} /* boardGetChar() */


#ifdef C_BOARD_USE_FREE_RTOS

uint32_t HAL_GetTick(void)
{
  return boardGetTick();
}

void HAL_Delay(uint32_t Delay)
{
  boardDelay(Delay);
}

#endif

/*
 * @brief              waits for a while
 * @param[in] xDelay   delay in ms
 * @return             none
 */
void boardDelay
(
  uint32_t xDelay
)
{
#ifdef C_BOARD_USE_FREE_RTOS
  const TickType_t xDelay_value = xDelay / portTICK_PERIOD_MS;
  vTaskDelay(xDelay_value);
#else
  HAL_Delay(xDelay);
#endif
} /* boardDelay() */

/*
 * @brief              get system tick
 * @return             system tick in ms
 */
uint32_t boardGetTick(void)
{
#ifdef C_BOARD_USE_FREE_RTOS
  return xTaskGetTickCount();
#else
  return HAL_GetTick();
#endif
} /* boardGetTick() */
