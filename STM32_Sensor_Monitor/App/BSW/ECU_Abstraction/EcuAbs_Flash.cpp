/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Flash.cpp
* Project         : STM32_Sensor_Monitor
* Description     : ECU Abstraction Layer - Internal Flash Implementation.
*                   Wraps STM32 HAL Flash API for NVM sectors (Sector 6, 7).
*                   Physical sector numbers and addresses are encapsulated here.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                                  Include header files
************************************************************************************************************************
*/
#include "EcuAbs_Flash.hpp"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    /* Physical Flash sector mapping */
    constexpr uint32_t SECTOR6_ADDR = 0x08040000U;
    constexpr uint32_t SECTOR7_ADDR = 0x08060000U;
    constexpr uint32_t SECTOR_SIZE  = 0x00020000U;   /*!< 128KB per sector  */
    const uint32_t SECTOR6_NUM = FLASH_SECTOR_6;
    const uint32_t SECTOR7_NUM = FLASH_SECTOR_7;
}
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Read data from NVM Flash sector.
 * @param   sectorId    Logical sector (NVM_MAIN or NVM_BACKUP)
 * @param   offset      Byte offset within the sector
 * @param   pData       Pointer to destination buffer
 * @param   length      Number of bytes to read
 * @retval  EcuAbs Flash status
 */
EcuAbsFlash::Status EcuAbsFlash::read(uint8_t sectorId, uint32_t offset, uint8_t *pData, uint32_t length)
{
    Status retVal = Status::Ok;
    uint32_t baseAddr = 0U;

    retVal = getSectorAddr(sectorId, &baseAddr);

    if (retVal == Status::Ok)
    {
        if ((pData == nullptr) || ((offset + length) > SECTOR_SIZE))
        {
            retVal = Status::Error;
        }
        else
        {
            (void)memcpy(pData, (const void *)(baseAddr + offset), length);
        }
    }

    return retVal;
}

/**
 * @brief   Program data to NVM Flash sector (word-aligned).
 * @param   sectorId    Logical sector (NVM_MAIN or NVM_BACKUP)
 * @param   offset      Byte offset within the sector (must be 4-byte aligned)
 * @param   pData       Pointer to source data
 * @param   length      Number of bytes to write
 * @retval  EcuAbs Flash status
 */
EcuAbsFlash::Status EcuAbsFlash::write(uint8_t sectorId, uint32_t offset, const uint8_t *pData, uint32_t length)
{
    Status retVal = Status::Ok;
    uint32_t baseAddr = 0U;
    uint32_t writeAddr;
    uint32_t wordData;
    uint32_t idx;
    uint32_t remaining;
    HAL_StatusTypeDef halStatus = HAL_OK;

    retVal = getSectorAddr(sectorId, &baseAddr);

    if (retVal == Status::Ok)
    {
        if ((pData == nullptr) || ((offset % 4U) != 0U) || ((offset + length) > SECTOR_SIZE))
        {
            retVal = Status::Error;
        }
    }

    if (retVal == Status::Ok)
    {
        writeAddr = baseAddr + offset;

        HAL_FLASH_Unlock();

        /* Write full 32-bit words */
        for (idx = 0U; idx < (length / 4U); idx++)
        {
            (void)memcpy(&wordData, &pData[idx * 4U], 4U);
            halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, writeAddr, wordData);

            if (halStatus != HAL_OK)
            {
                break;
            }

            writeAddr += 4U;
        }

        /* Write remaining bytes (padded with 0xFF) */
        remaining = length % 4U;

        if ((halStatus == HAL_OK) && (remaining > 0U))
        {
            wordData = 0xFFFFFFFFU;
            (void)memcpy(&wordData, &pData[(length / 4U) * 4U], remaining);
            halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, writeAddr, wordData);
        }

        HAL_FLASH_Lock();

        if (halStatus != HAL_OK)
        {
            retVal = Status::Error;
        }
    }

    return retVal;
}

/**
 * @brief   Erase an NVM Flash sector.
 * @param   sectorId    Logical sector (NVM_MAIN or NVM_BACKUP)
 * @retval  EcuAbs Flash status
 */
EcuAbsFlash::Status EcuAbsFlash::eraseSector(uint8_t sectorId)
{
    Status retVal = Status::Ok;
    uint32_t sectorNum = 0U;
    uint32_t sectorError = 0U;
    FLASH_EraseInitTypeDef eraseInit;
    HAL_StatusTypeDef halStatus;

    retVal = getSectorNum(sectorId, &sectorNum);

    if (retVal == Status::Ok)
    {
        HAL_FLASH_Unlock();

        eraseInit.TypeErase     = FLASH_TYPEERASE_SECTORS;
        eraseInit.Sector        = sectorNum;
        eraseInit.NbSectors     = 1U;
        eraseInit.VoltageRange  = FLASH_VOLTAGE_RANGE_3;

        halStatus = HAL_FLASHEx_Erase(&eraseInit, &sectorError);

        HAL_FLASH_Lock();

        if (halStatus != HAL_OK)
        {
            retVal = Status::Error;
        }
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/

/**
 * @brief   Convert logical sector ID to physical base address.
 * @param   sectorId    Logical sector ID
 * @param   pAddr       Pointer to store physical base address
 * @retval  EcuAbs Flash status
 */
EcuAbsFlash::Status EcuAbsFlash::getSectorAddr(uint8_t sectorId, uint32_t *pAddr)
{
    Status retVal = Status::Ok;

    if (sectorId == NVM_MAIN)
    {
        *pAddr = SECTOR6_ADDR;
    }
    else if (sectorId == NVM_BACKUP)
    {
        *pAddr = SECTOR7_ADDR;
    }
    else
    {
        retVal = Status::Error;
    }

    return retVal;
}

/**
 * @brief   Convert logical sector ID to physical sector number.
 * @param   sectorId    Logical sector ID
 * @param   pSector     Pointer to store physical sector number
 * @retval  EcuAbs Flash status
 */
EcuAbsFlash::Status EcuAbsFlash::getSectorNum(uint8_t sectorId, uint32_t *pSector)
{
    Status retVal = Status::Ok;

    if (sectorId == NVM_MAIN)
    {
        *pSector = SECTOR6_NUM;
    }
    else if (sectorId == NVM_BACKUP)
    {
        *pSector = SECTOR7_NUM;
    }
    else
    {
        retVal = Status::Error;
    }

    return retVal;
}
