#ifndef SPIFF_SPIFFS_FS_H_
#define SPIFF_SPIFFS_FS_H_

#include "spiffs.h"
#include "board.h"

extern spiffs gSpiffsFs; //!< File system structure for SPIFFS.

/**
 * @brief   Mount the file system.
 * @return  BOARD_OK on success. BOARD_FATAL_ERROR on error.
 */
BOARD_Status spiffsMount
(
  void
);

/**
 * @brief   Format the file system. Must be previously mounted.
 * @return  none.
 */
void spiffsFormat
(
  void
);

/**
 * @brief   Utility function to print the files stored.
 * @return  none.
 */
void spiffsListFile
(
  void
);

/**
 * @brief                  Delete all the files prefixed by the parameter.
 * @param[in] xpPrefixName Name of the prefix. If NULL, all files are deleted.
 * @return                 none.
 */
void spiffsDeleteFiles
(
  const char* xpPrefixName
);

#endif /* SPIFF_SPIFFS_FS_H_ */
