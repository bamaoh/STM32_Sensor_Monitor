/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Flash.hpp
* Project         : STM32_Sensor_Monitor
* Description     : ECU Abstraction Layer - Internal Flash Interface.
*                   Provides hardware-independent Flash read/write/erase interface.
*                   Physical sector and address mapping is encapsulated within this module.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#pragma once
/*
************************************************************************************************************************
*                                                  Include header files
************************************************************************************************************************
*/
extern "C"
{
    #include "stm32f4xx_hal.h"
}
#include <cstring>
#include <cstdint>
/*
************************************************************************************************************************
*                                                    Class definition
************************************************************************************************************************
*/
/** @brief ECU Abstraction Layer - Flash control class */
class EcuAbsFlash
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/
    /* NVM sector identifiers (logical, not physical) */
    static constexpr uint8_t NVM_MAIN = 0U;    /*!< Main NVM sector (Sector 6)   */
    static constexpr uint8_t NVM_BACKUP = 1U;  /*!< Backup NVM sector (Sector 7) */

    /** @brief EcuAbs Flash operation status definition */
    enum class Status : uint8_t
    {
        Ok     = 0x00U,   /*!< No error              */
        Error  = 0x01U    /*!< Operation failed       */
    };
/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/
    /**
     * @brief   Read data from NVM Flash sector.
     * @param   sectorId    Logical sector (NVM_MAIN or NVM_BACKUP)
     * @param   offset      Byte offset within the sector
     * @param   pData       Pointer to destination buffer
     * @param   length      Number of bytes to read
     * @retval  EcuAbs Flash status
     */
    Status read(uint8_t sectorId, uint32_t offset, uint8_t *pData, uint32_t length);

    /**
     * @brief   Program data to NVM Flash sector (word-aligned).
     * @param   sectorId    Logical sector (NVM_MAIN or NVM_BACKUP)
     * @param   offset      Byte offset within the sector (must be 4-byte aligned)
     * @param   pData       Pointer to source data
     * @param   length      Number of bytes to write
     * @retval  EcuAbs Flash status
     */
    Status write(uint8_t sectorId, uint32_t offset, const uint8_t *pData, uint32_t length);

    /**
     * @brief   Erase an NVM Flash sector.
     * @param   sectorId    Logical sector (NVM_MAIN or NVM_BACKUP)
     * @retval  EcuAbs Flash status
     */
    Status eraseSector(uint8_t sectorId);
private:
/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

/*
********************************************************************************************************************
*                                                  Private methods
********************************************************************************************************************
*/
    Status getSectorAddr(uint8_t sectorId, uint32_t *pAddr);
    Status getSectorNum(uint8_t sectorId, uint32_t *pSector);
};
