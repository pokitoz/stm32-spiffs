#include "memory_qspi.h"
#include "spiffs.h"
#include "spiffs_config.h"
#include "spiffs_fs.h"

#define C_FILESYSTEM_ENABLE_LOG 0             //!< Enable or disable the log for this module
#define C_FILESYSTEM_ENABLE_NAME "FileSystem" //!< Name of the module

/*
 * @brief Display a text if log is enabled.
 */
#if (C_FILESYSTEM_ENABLE_LOG == 1)
  #define M_FILESYSTEM_SAL_LOG printf
#else
  #define M_FILESYSTEM_SAL_LOG //
#endif

#define C_FILESYSTEM_FILE_DESCRIPTOR_SIZE 44 //!< Size in byte of a file descriptor. Given by
                                             //   function SPIFFS_buffer_bytes_for_filedescs.
                                             //   Computed depending on configuration.

#define C_FILESYSTEM_MAX_CONCURRENT_FILE_DESCRIPTOR 4 //!< Number of file descriptor that can be
                                                      //   opened concurrently.

#define C_FILESYSTEM_CACHE_PAGE_NUMBER 4 //!< Number of cache pages for the cache buffer.

spiffs gSpiffsFs; //!< Declaration of the file system

static u8_t spiffsWorkBuffer[SPIFFS_CFG_LOG_PAGE_SZ() * 2];
//!< Working buffer for file system data. Must be at least 2 times the page size.
//   Refer to https://github.com/pellepl/spiffs/wiki/Integrate-spiffs for more information

static u8_t spiffsFileDescriptorBuffer[C_FILESYSTEM_FILE_DESCRIPTOR_SIZE *
                                       C_FILESYSTEM_MAX_CONCURRENT_FILE_DESCRIPTOR];
//!< Buffer used to store the file descriptor opened.
//   Refer to https://github.com/pellepl/spiffs/wiki/Integrate-spiffs for more information

static u8_t spiffsCacheBuffer[(SPIFFS_CFG_LOG_PAGE_SZ() + C_FILESYSTEM_FILE_DESCRIPTOR_SIZE) *
                              C_FILESYSTEM_CACHE_PAGE_NUMBER];
//!< Cache buffer used for speeding up calls. Can be computed using SPIFFS_buffer_bytes_for_cache
//   Refer to https://github.com/pellepl/spiffs/wiki/Integrate-spiffs for more information

/*
 * @brief  Wrapper for the SPIFFS library to read a flash block
 * @return none
 */
static void pSpiffsWrapperRead
(
  int   xAddr,
  int   xSize,
  char* xpData
)
{
  boardMemoryQspiRead(xpData, xAddr, xSize);
} /* pSpiffsWrapperRead() */

/*
 * @brief  Wrapper for the SPIFFS library to write a flash block
 * @return none
 */
static void pSpiffsWrapperWrite
(
  int   xAddr,
  int   xSize,
  char* xpData
)
{
  boardMemoryQspiWrite(xpData, xAddr, xSize);
} /* pSpiffsWrapperWrite() */

/*
 * @brief  Wrapper for the SPIFFS library to erase a flash block
 * @return none
 */
static void pSpiffsWrapperErase
(
  int xAddr,
  int xSize
)
{
  boardMemoryQspiErase(xAddr);
} /* pSpiffsWrapperErase() */

/*
 * @brief   Mount the file system.
 * @return  BOARD_OK on success. BOARD_FATAL_ERROR on error.
 */
BOARD_Status spiffsMount
(
  void
)
{
  int statusFileSystem = SPIFFS_OK;
  int32_t bufferSize = 0;
  spiffs_config wrapperConfiguration = {0};
  BOARD_Status status = BOARD_ERROR_FATAL;

  wrapperConfiguration.hal_read_f  = pSpiffsWrapperRead;
  wrapperConfiguration.hal_write_f = pSpiffsWrapperWrite;
  wrapperConfiguration.hal_erase_f = pSpiffsWrapperErase;

  statusFileSystem = SPIFFS_mount(&gSpiffsFs,
    &wrapperConfiguration,
    spiffsWorkBuffer,
    spiffsFileDescriptorBuffer,
    sizeof(spiffsFileDescriptorBuffer),
    spiffsCacheBuffer,
    sizeof(spiffsCacheBuffer),
    NULL);

  M_FILESYSTEM_SAL_LOG("Mount result: %i", statusFileSystem);
  if (SPIFFS_OK != statusFileSystem)
  {
    status = BOARD_ERROR_FATAL;
  }
  else
  {
    status = BOARD_OK;
  }

  bufferSize = SPIFFS_buffer_bytes_for_filedescs(&gSpiffsFs,
                                                 C_FILESYSTEM_MAX_CONCURRENT_FILE_DESCRIPTOR);

  M_FILESYSTEM_SAL_LOG("Computed buffer size for file descriptor: %i", bufferSize);

#ifndef  SPIFFS_CACHE
  bufferSize = SPIFFS_buffer_bytes_for_cache(&gSpiffsFs, C_FILESYSTEM_CACHE_PAGE_NUMBER);
  M_FILESYSTEM_SAL_LOG("Computed buffer size for cache: %i", bufferSize);
#endif

  return status;
} /* spiffsMount() */

/*
 * @brief   Format the file system. Must be previously mounted.
 * @return  none.
 */
void spiffsFormat
(
  void
)
{
  /* Must be mounted before the unmount. */
  SPIFFS_unmount(&gSpiffsFs);
  SPIFFS_format(&gSpiffsFs);
  if (BOARD_OK != spiffsMount())
  {
    M_FILESYSTEM_SAL_LOG("Restart the board as mounting failed..");
  }
} /* spiffsFormat() */

/*
 * @brief   Utility function to print the files stored.
 * @return  none.
 */
void spiffsListFile
(
  void
)
{
  spiffs_DIR dir;
  struct spiffs_dirent dirEntry;

  for (;;)
  {
    SPIFFS_clearerr(&gSpiffsFs);

    M_FILESYSTEM_SAL_LOG("Listing files.");
    /* Opening directory */
    SPIFFS_opendir(&gSpiffsFs, "", &dir);
    if (SPIFFS_OK != gSpiffsFs.err_code)
    {
      M_FILESYSTEM_SAL_LOG("Error opening directory.");
      break;
    }

    /* Reading directory entries */
    SPIFFS_readdir(&dir, &dirEntry);
    while (SPIFFS_OK == dir.fs->err_code)
    {
      M_FILESYSTEM_SAL_LOG("- %s", dirEntry.name);
      SPIFFS_readdir(&dir, &dirEntry);
    }

    SPIFFS_closedir(&dir);
    M_FILESYSTEM_SAL_LOG("End list");
    break;
  }
} /* spiffsListFile() */

/*
 * @brief                  Delete all the files prefixed by the parameter.
 * @param[in] xpPrefixName Name of the prefix. If NULL, all files are deleted.
 * @return                 none.
 */
void spiffsDeleteFiles
(
  const char* xpPrefixName
)
{
  spiffs_DIR dir;
  int32_t fd = 0;
  struct spiffs_dirent dirEntry;

  for (;;)
  {
    SPIFFS_clearerr(&gSpiffsFs);

    SPIFFS_opendir(&gSpiffsFs, "" /* root */, &dir);
    if (SPIFFS_OK != gSpiffsFs.err_code) {
      M_FILESYSTEM_SAL_LOG("Error opening directory");
      break;
    }

    SPIFFS_readdir(&dir, &dirEntry);
    while (SPIFFS_OK == dir.fs->err_code)
    {
      if ((NULL == xpPrefixName) ||
          (0 == strncmp(dirEntry.name, xpPrefixName, strlen(xpPrefixName))))
      {
        fd = SPIFFS_open(&gSpiffsFs, dirEntry.name, SPIFFS_RDWR, 0);
        SPIFFS_fremove(&gSpiffsFs, fd);
        SPIFFS_close(&gSpiffsFs, fd);
        M_FILESYSTEM_SAL_LOG("- %s Deleted", dirEntry.name);

        SPIFFS_clearerr(&gSpiffsFs);
      }

      SPIFFS_readdir(&dir, &dirEntry);
    }

    SPIFFS_closedir(&dir);
    break;
  }
} /* spiffsDeleteFiles() */
