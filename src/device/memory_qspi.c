#include "memory_qspi.h"
#include "stm32l4xx_hal.h"
#include "stm32l475e_iot01_qspi.h"

static uint8_t isInit = 0; //!< Indicate if the flash memory has been initialized.

#define C_BOARD_QSPI_ENABLE_MEMORY_MAPPED 0 //!< Indicate that the Flash memory can be accessed
                                            //   with direct memory read.

/*
 * @brief   Initialize the QSPI memory
 * @return  BOARD_OK on success. BOARD_ERROR_FATAL on failure.
 */
BOARD_Status boardMemoryQspiInit
(
  void
)
{
  BOARD_Status status = BOARD_OK;
  QSPI_Info pQspiInfo = {0};

  for (;;)
  {
    if (isInit)
    {
      break;
    }

    if (QSPI_OK != BSP_QSPI_Init())
    {
      printf("Could not init memory QSPI");
      status = BOARD_ERROR_FATAL;
      break;
    }

    (void) BSP_QSPI_GetInfo(&pQspiInfo);
    if((C_BOARD_QSPI_MEMORY_SIZE != pQspiInfo.FlashSize) ||
       (C_BOARD_QSPI_MEMORY_SECTOR_SIZE != pQspiInfo.EraseSectorSize)  ||
       (C_BOARD_QSPI_MEMORY_SECTOR_NUMBER != pQspiInfo.EraseSectorsNumber) ||
       (C_BOARD_QSPI_MEMORY_PAGE_SIZE != pQspiInfo.ProgPageSize)  ||
       (C_BOARD_QSPI_MEMORY_PAGE_NUMBER != pQspiInfo.ProgPagesNumber))
    {
      printf("Getting the informations of the QSPI memory failed.");
      status = BOARD_ERROR_FATAL;
      break;
    }

#if (C_BOARD_QSPI_ENABLE_MEMORY_MAPPED == 1)
    BSP_QSPI_EnableMemoryMappedMode();
#endif

    isInit = 1;
    break;
  }

  return status;
} /* boardMemoryQspiInit() */

/*
 * @brief   De-initialize the QSPI memory
 * @return  BOARD_OK on success. BOARD_ERROR_FATAL on failure.
 */
BOARD_Status boardMemoryQspiDeInit
(
  void
)
{
  BOARD_Status status = BOARD_OK;
  if (QSPI_OK != BSP_QSPI_DeInit())
  {
    status = BOARD_ERROR_FATAL;
  }

  return status;
} /* boardMemoryQspiDeInit() */

/*
 * @brief               Return the status of the QSPI Memory
 * @param[out] pxIsInit Must not be NULL. Returns 1 if memory is initialized.
 *                      else 0.
 * @return              BOARD_OK means ready to use. Other errors
 */
BOARD_Status boardMemoryQspiStatus
(
  uint8_t* xpIsInit
)
{
  BOARD_Status status = BOARD_OK;

  if (NULL == xpIsInit)
  {
    *xpIsInit = isInit;
    switch (BSP_QSPI_GetStatus()) {
    case QSPI_BUSY:
      status = BOARD_BUSY;
      break;
    case QSPI_ERROR:
      status = BOARD_ERROR;
      break;
    default:
      break;
    }
  }

  return status;
} /* boardMemoryQspiStatus() */

/*
 * @brief            Write data into memory
 * @param[in] pxData Data to be written.
 * @param[in]  xAddr Address where to write the data.
 * @param[in]  xSize Size of the data to be written.
 * @return           BOARD_OK means ready to use. Other errors
 */
BOARD_Status boardMemoryQspiWrite
(
  const uint8_t* pxData,
        uint32_t  xAddr,
        uint32_t  xSize
)
{
  BOARD_Status status = BOARD_OK;
  if (QSPI_OK != BSP_QSPI_Write(pxData, xAddr, xSize))
  {
    printf("Write error.");
    status = BOARD_ERROR;
  }

  return status;
} /* boardMemoryQspiWrite() */

/*
 * @brief            Erase the memory block containing the address.
 * @param[in]  xAddr Address of the block to erase.
 * @return  BOARD_OK means ready to use. Other errors
 */
BOARD_Status boardMemoryQspiErase
(
  uint32_t xAddr
)
{
  BOARD_Status status = BOARD_OK;

#if (C_BOARD_QSPI_ENABLE_MEMORY_MAPPED == 0)
  if (QSPI_OK != BSP_QSPI_Erase_Block(xAddr))
  {
    printf("Erase error.");
    status = BOARD_ERROR;
  }
#endif

  return status;
} /* boardMemoryQspiErase() */

/*
 * @brief             Read data from memory
 * @param[out] pxData Data buffer which will contain the data read.
 * @param[in]   xAddr Address where to read the data.
 * @param[in]   xSize Size of the data to be read.
 * @return            BOARD_OK means ready to use. Other errors
 */
BOARD_Status boardMemoryQspiRead
(
  uint8_t* pxData,
  uint32_t  xAddr,
  uint32_t  xSize
)
{
  BOARD_Status status = BOARD_OK;

#if (C_BOARD_QSPI_ENABLE_MEMORY_MAPPED == 1)
  memcpy(pData, C_BOARD_QSPI_MEMORY_ADDRESS + addr, size);
#else
  if (QSPI_OK != BSP_QSPI_Read(pxData, xAddr, xSize))
  {
    printf("Read error.");
    status = BOARD_ERROR;
  }
#endif

  return status;
} /* boardMemoryQspiRead() */

/*
 * @brief            Erase the memory block containing the address.
 * @param[in]  xAddr Address of the block to erase.
 * @return  BOARD_OK means ready to use. Other errors
 */
BOARD_Status boardMemoryQspiWipe
(
  void
)
{
  BOARD_Status status = BOARD_OK;

#if (C_BOARD_QSPI_ENABLE_MEMORY_MAPPED == 0)
  if (QSPI_OK != BSP_QSPI_Erase_Chip())
  {
    printf("Erase error.");
    status = BOARD_ERROR;
  }
#endif

  return status;
} /* boardMemoryQspiErase() */
