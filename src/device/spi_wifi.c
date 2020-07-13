#include "es_wifi_io.h"
#include "es_wifi.h"
#include "spi_wifi.h"

#include <stdint.h>
#include <stdio.h>

/** To enable or disable log for this module. */
#define C_SPI_WIFI_ENABLE_LOG  1
/** Module name to display. */
#define C_SPI_WIFI_MODULE_NAME "WIFI"

/** SSID to connect to network. */
#define C_SPI_WIFI_SSID     ""
/** Password of the network. */
#define C_SPI_WIFI_PASSWORD ""
/** Number of attempts to bind to a connection. */
#define C_SPI_WIFI_MAX_CONNECTION_TRIAL 10

/** Module structure containing the state of the WIFI. */
static ES_WIFIObject_t gWifiModuleStructure;

/*
 * @brief Display a text if log is enabled.
 */
#if (C_SPI_WIFI_ENABLE_LOG == 1)
  #define M_SPI_WIFI_LOG printf
#else
  #define M_SPI_WIFI_LOG //
#endif

/** SPI handle to access the WIFI chip. Defined in es_wifi_io.c API */
extern SPI_HandleTypeDef hspi;

void HAL_GPIO_EXTI_Callback
(
  uint16_t GPIO_Pin
)
{
  if (RESET != __HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1))
  {
    /* Serve interrupt. */
    SPI_WIFI_ISR();
    /* Clean interrupt. */
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
  }

  HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
}

void SPI3_IRQHandler
(
  void
)
{
  HAL_SPI_IRQHandler(&hspi);
}

ES_WIFI_Status_t wifiInit
(
  void
)
{
  uint8_t macAddress[6] = {0};
  ES_WIFI_Status_t wifiResult = ES_WIFI_STATUS_OK;

  for (;;)
  {

    wifiResult = ES_WIFI_RegisterBusIO(&gWifiModuleStructure, SPI_WIFI_Init, SPI_WIFI_DeInit,
      SPI_WIFI_Delay, SPI_WIFI_SendData, SPI_WIFI_ReceiveData);

    if(ES_WIFI_STATUS_OK != wifiResult)
    {
      break;
    }

    wifiResult = ES_WIFI_Init(&gWifiModuleStructure);
    if(ES_WIFI_STATUS_OK != wifiResult)
    {
      break;
    }

    M_SPI_WIFI_LOG("Initialized.");

    wifiResult = ES_WIFI_GetMACAddress(&gWifiModuleStructure, macAddress);
    if(ES_WIFI_STATUS_OK != wifiResult)
    {
      break;
    }

    M_SPI_WIFI_LOG("MAC Address : %X:%X:%X:%X:%X:%X",
        macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);

    break;
  }

  return wifiResult;
}

ES_WIFI_Status_t wifiConnectTo
(
  void
)
{
  uint8_t ipaddrV4[4] = {0};
  ES_WIFI_Status_t wifiResult = ES_WIFI_STATUS_ERROR;

  for (;;)
  {
    wifiResult = ES_WIFI_Connect(&gWifiModuleStructure, C_SPI_WIFI_SSID,
      C_SPI_WIFI_PASSWORD, ES_WIFI_SEC_OPEN);

    if(ES_WIFI_STATUS_OK != wifiResult)
    {
      M_SPI_WIFI_LOG("Could not connect.");
      break;
    }

    wifiResult = ES_WIFI_GetNetworkSettings(&gWifiModuleStructure);
    if(ES_WIFI_STATUS_OK != wifiResult)
    {
       M_SPI_WIFI_LOG("Could not get network settings.");
       break;
    }

    M_SPI_WIFI_LOG("Connected to hotspot.");

    wifiResult = ES_WIFI_IsConnected(&gWifiModuleStructure);
    if (1 != wifiResult)
    {
      break;
    }

    memcpy(ipaddrV4, gWifiModuleStructure.NetSettings.IP_Addr, sizeof(ipaddrV4));
    wifiResult = ES_WIFI_STATUS_OK;

    M_SPI_WIFI_LOG("Connected to %s.", C_SPI_WIFI_SSID);
    M_SPI_WIFI_LOG("Got IP Address : %d.%d.%d.%d.",
        ipaddrV4[0], ipaddrV4[1], ipaddrV4[2], ipaddrV4[3]);

    break;
  }

  return wifiResult;
}

ES_WIFI_Status_t wifiBindToUdp
(
  int8_t   xSocketId,
  uint16_t xlocalPort
)
{
  ES_WIFI_Conn_t connection;
  ES_WIFI_Status_t wifiResult = ES_WIFI_STATUS_ERROR;
  int16_t trials = C_SPI_WIFI_MAX_CONNECTION_TRIAL;

  M_SPI_WIFI_LOG("Trying to bind socket %d on port %d",
    xSocketId, xlocalPort);

  connection.Number = xSocketId;
  connection.RemotePort = xlocalPort;
  connection.LocalPort = xlocalPort;
  connection.Type = ES_WIFI_UDP_CONNECTION;

  connection.RemoteIP[0] = 0;
  connection.RemoteIP[1] = 0;
  connection.RemoteIP[2] = 0;
  connection.RemoteIP[3] = 0;

  while (trials--)
  {
    wifiResult = ES_WIFI_StartClientConnection(&gWifiModuleStructure, &connection);
    if(ES_WIFI_STATUS_OK == wifiResult)
    {
      M_SPI_WIFI_LOG("Connection opened successfully.");
      break;
    }

    HAL_Delay(100);
  }

  return wifiResult;
}

ES_WIFI_Status_t wifiReceiveDataFrom
(
  uint8_t    xSocketId,
  uint8_t*  pxDataReceive,
  uint16_t   xBufferSize,
  uint16_t* pxDataReceived,
  uint32_t   xTimeout,
  uint8_t*  pxRecepientAddress,
  uint16_t*  xRecepientPort
)
{
  return ES_WIFI_ReceiveDataFrom(&gWifiModuleStructure, xSocketId, pxDataReceive, xBufferSize,
      pxDataReceived, xTimeout, pxRecepientAddress, xRecepientPort);
}

ES_WIFI_Status_t wifiSendDataTo
(
  uint8_t    xSocketId,
  uint8_t*  pxDataToSend,
  uint16_t   xSizeData,
  uint16_t* pxDataSent,
  uint32_t   xTimeoutSend,
  uint8_t*  pxRecepientAddress,
  uint16_t   xRecepientPort
)
{
  return ES_WIFI_SendDataTo(&gWifiModuleStructure, xSocketId, pxDataToSend, xSizeData, pxDataSent,
      xTimeoutSend, pxRecepientAddress, xRecepientPort);
}

ES_WIFI_Status_t wifiCloseSocket
(
  uint32_t xSocketId
)
{
  ES_WIFI_Conn_t conn;
  conn.Number = xSocketId;
  return ES_WIFI_StopClientConnection(&gWifiModuleStructure, &conn);
}

ES_WIFI_Status_t wifiDisconnect
(
  void
)
{
  return ES_WIFI_Disconnect(&gWifiModuleStructure);
}

ES_WIFI_Status_t wifiIsConnected
(
  void
)
{
  return (ES_WIFI_IsConnected(&gWifiModuleStructure) != 0) ? ES_WIFI_STATUS_OK : ES_WIFI_STATUS_TIMEOUT;
}
