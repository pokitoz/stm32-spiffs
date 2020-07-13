/******************************************************************************
 * Copyright (c) 2019 Nagravision S.A.
 ******************************************************************************/
/** \brief    IoT - SAL - I2C - STM32L4
 *  \author   KS
 *  \date     20.11.2019
 *  \file     spi_wifi.h
 ******************************************************************************/

#ifndef DEVICE_SPI_WIFI_H_
#define DEVICE_SPI_WIFI_H_

#include <stdint.h>

#include "es_wifi_io.h"
#include "es_wifi.h"

/**
 * @brief  EXTI line detection callback.
 * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
 * @retval None
 */
void HAL_GPIO_EXTI_Callback
(
  uint16_t GPIO_Pin
);

void SPI3_IRQHandler
(
  void
);

/**
 * @brief  Initialize the SPI to the WIFI and get MAC address
 * @retval Operation status
 */
ES_WIFI_Status_t wifiInit
(
  void
);

/**
 * @brief  Connect the WIFI to an access point
 * @pre    wifiInit should be called.
 * @retval Operation status
 */
ES_WIFI_Status_t wifiConnectTo
(
  void
);

/**
 * @brief  Bind a socket to a port
 * @param  xSocketId: ID of the socket to use
 * @param  xlocalPort: Port to bind to. Should be unused.
 * @retval Operation status
 */
ES_WIFI_Status_t wifiBindToUdp
(
  int8_t   xSocketId,
  uint16_t xlocalPort
);

/**
 * @brief  Receive Data from a socket
 * @param        xSocketId: ID of the socket to use
 * @param       pxDataReceive: pointer to buffer
 * @param        xBufferSize: maximum length of the data to be received
 * @param[out]  pxDataReceived: length of the data actually received
 * @param        xTimeout: Socket read timeout (ms)
 * @param       pxRecepientAddress: 4-byte array containing the IP address of the remote host
 * @param[out]   xRecepientPort: (OUT) port number of the remote host
 * @retval Operation status
 */
ES_WIFI_Status_t wifiReceiveDataFrom
(
  uint8_t    xSocketId,
  uint8_t*  pxDataReceive,
  uint16_t   xBufferSize,
  uint16_t* pxDataReceived,
  uint32_t   xTimeout,
  uint8_t*  pxRecepientAddress,
  uint16_t*  xRecepientPort
);

/**
 * @brief       Send Data on a socket
 * @param       xSocketId: ID of the socket to use
 * @param       pxDataToSend: pointer to data to be sent
 * @param       xSizeData: length of data to be sent
 * @param[out]  pxDataSent: length actually sent
 * @param       xTimeoutSend : Socket write timeout (ms)
 * @param       pxRecepientAddress: 4-byte array containing the IP address of the remote host
 * @param       xRecepientPort: port number of the remote host
 * @retval      Operation status
 */
ES_WIFI_Status_t wifiSendDataTo
(
  uint8_t    xSocketId,
  uint8_t*  pxDataToSend,
  uint16_t   xSizeData,
  uint16_t* pxDataSent,
  uint32_t   xTimeoutSend,
  uint8_t*  pxRecepientAddress,
  uint16_t   xRecepientPort
);

/**
 * @brief  Close client connection
 * @retval Operation status
 */
ES_WIFI_Status_t wifiCloseSocket
(
  uint32_t xSocketId
);

/**
 * @brief  Disconnect from a network
 * @param  None
 * @retval Operation status
 */
ES_WIFI_Status_t wifiDisconnect
(
  void
);

/**
 * @brief  Check if the wifi is connection to an Access Point
 * @param  None
 * @retval ES_WIFI_STATUS_OK or ES_WIFI_STATUS_TIMEOUT
 */
ES_WIFI_Status_t wifiIsConnected
(
  void
);


#endif /* DEVICE_SPI_WIFI_H_ */
