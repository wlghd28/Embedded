#ifndef _FLASHMEMORY_H_
#define _FLASHMEMORY_H_

#include <stdint.h>
#include "Database.h"

#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_4   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ADDR_FLASH_SECTOR_23  +  GetSectorSize(ADDR_FLASH_SECTOR_23) -1 /* End @ of user Flash area : sector start address + sector size -1 */

#define DATA_32                 ((uint32_t)0x12345678)
#define data_DATABASE			((DATABASE*)ADDR_FLASH_SECTOR_4)

uint32_t GetSector(uint32_t Address);
uint32_t GetSectorSize(uint32_t Sector);

#endif
