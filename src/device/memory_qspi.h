#ifndef DEVICE_MEMORY_QSPI_H_
#define DEVICE_MEMORY_QSPI_H_

#include "board.h"
#include "mx25r6435f.h"

#include <stdint.h>

#define C_BOARD_QSPI_MEMORY_ADDRESS  0x90000000 //!< Physical address of memory
#define C_BOARD_QSPI_MEMORY_SIZE     MX25R6435F_FLASH_SIZE //!< Size of memory

#define C_BOARD_QSPI_MEMORY_SECTOR_SIZE   MX25R6435F_SECTOR_SIZE //!< Size of a flash sector
#define C_BOARD_QSPI_MEMORY_SECTOR_NUMBER (MX25R6435F_FLASH_SIZE / MX25R6435F_SECTOR_SIZE)
//!< Number of sectors (2048 sectors of 4kBytes)

#define C_BOARD_QSPI_MEMORY_PAGE_SIZE     MX25R6435F_PAGE_SIZE //!< Size of a flash page
#define C_BOARD_QSPI_MEMORY_PAGE_NUMBER   (MX25R6435F_FLASH_SIZE / MX25R6435F_PAGE_SIZE)
//!< Number of pages (32768 pages of 256 bytes)

/**
 * @brief   Initialize the QSPI memory
 * @return  BOARD_OK on success. BOARD_ERROR_FATAL on failure.
 */
BOARD_Status boardMemoryQspiInit
(
  void
);

/**
 * @brief   De-initialize the QSPI memory
 * @return  BOARD_OK on success. BOARD_ERROR_FATAL on failure.
 */
BOARD_Status boardMemoryQspiDeInit
(
  void
);

/**
 * @brief               Return the status of the QSPI Memory
 * @param[out] pxIsInit Must not be NULL. Returns 1 if memory is initialized.
 *                      else 0.
 * @return              BOARD_OK means ready to use. Other errors
 */
BOARD_Status boardMemoryQspiStatus
(
  uint8_t* pxIsInit
);

/**
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
);

/**
 * @brief            Erase the memory block containing the address.
 * @param[in]  xAddr Address of the block to erase.
 * @return  BOARD_OK means ready to use. Other errors
 */
BOARD_Status boardMemoryQspiErase
(
  uint32_t xAddr
);

/**
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
);

#endif /* DEVICE_MEMORY_QSPI_H_ */
