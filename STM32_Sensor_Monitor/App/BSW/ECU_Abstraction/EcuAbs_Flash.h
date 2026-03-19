/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Flash.h
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : ECU Abstraction Layer - Internal Flash Interface.
*                   Provides hardware-independent Flash read/write/erase interface.
*                   Physical sector and address mapping is encapsulated within this module.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef ECUABS_FLASH_H
#define ECUABS_FLASH_H
/*
************************************************************************************************************************
*                                                  Include header files
************************************************************************************************************************
*/
#include <stdint.h>
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

/* NVM sector identifiers (logical, not physical) */
#define ECUABS_FLASH_NVM_MAIN       (0U)    /*!< Main NVM sector (Sector 6)   */
#define ECUABS_FLASH_NVM_BACKUP     (1U)    /*!< Backup NVM sector (Sector 7) */
/*
************************************************************************************************************************
*                                                         Typedefs
************************************************************************************************************************
*/

/** @brief EcuAbs Flash operation status definition */
typedef enum
{
    ECUABS_FLASH_OK     = 0x00U,   /*!< No error              */
    ECUABS_FLASH_ERROR  = 0x01U    /*!< Operation failed       */
} EcuAbs_Flash_StatusType;
/*
************************************************************************************************************************
*                                                    Exported variables
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                               Exported function prototypes
************************************************************************************************************************
*/

/**
 * @brief   Read data from NVM Flash sector.
 * @param   sectorId    Logical sector (ECUABS_FLASH_NVM_MAIN or BACKUP)
 * @param   offset      Byte offset within the sector
 * @param   pData       Pointer to destination buffer
 * @param   length      Number of bytes to read
 * @retval  EcuAbs Flash status
 */
EcuAbs_Flash_StatusType EcuAbs_Flash_Read(uint8_t sectorId, uint32_t offset, uint8_t *pData, uint32_t length);

/**
 * @brief   Program data to NVM Flash sector (word-aligned).
 * @param   sectorId    Logical sector (ECUABS_FLASH_NVM_MAIN or BACKUP)
 * @param   offset      Byte offset within the sector (must be 4-byte aligned)
 * @param   pData       Pointer to source data
 * @param   length      Number of bytes to write
 * @retval  EcuAbs Flash status
 */
EcuAbs_Flash_StatusType EcuAbs_Flash_Write(uint8_t sectorId, uint32_t offset, const uint8_t *pData, uint32_t length);

/**
 * @brief   Erase an NVM Flash sector.
 * @param   sectorId    Logical sector (ECUABS_FLASH_NVM_MAIN or BACKUP)
 * @retval  EcuAbs Flash status
 */
EcuAbs_Flash_StatusType EcuAbs_Flash_EraseSector(uint8_t sectorId);

#endif /* ECUABS_FLASH_H */
