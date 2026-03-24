/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Flash.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
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
#include "EcuAbs_Flash.h"
#include "stm32f4xx_hal.h"
#include <string.h>
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

/* Physical Flash sector mapping */
#define ECUABS_FLASH_SECTOR6_ADDR       (0x08040000U)
#define ECUABS_FLASH_SECTOR7_ADDR       (0x08060000U)
#define ECUABS_FLASH_SECTOR_SIZE        (0x00020000U)   /*!< 128KB per sector  */
#define ECUABS_FLASH_SECTOR6_NUM        (FLASH_SECTOR_6)
#define ECUABS_FLASH_SECTOR7_NUM        (FLASH_SECTOR_7)
/*
************************************************************************************************************************
*                                                         Typedefs
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                                    Private variables
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/

/**
 * @brief   Convert logical sector ID to physical base address.
 * @param   sectorId    Logical sector ID
 * @param   pAddr       Pointer to store physical base address
 * @retval  EcuAbs Flash status
 */
static EcuAbs_Flash_StatusType EcuAbs_Flash_GetSectorAddr(uint8_t sectorId, uint32_t *pAddr);

/**
 * @brief   Convert logical sector ID to physical sector number.
 * @param   sectorId    Logical sector ID
 * @param   pSector     Pointer to store physical sector number
 * @retval  EcuAbs Flash status
 */
static EcuAbs_Flash_StatusType EcuAbs_Flash_GetSectorNum(uint8_t sectorId, uint32_t *pSector);
/*
************************************************************************************************************************
*                                                    Public functions
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
EcuAbs_Flash_StatusType EcuAbs_Flash_Read(uint8_t sectorId, uint32_t offset, uint8_t *pData, uint32_t length)
{
    EcuAbs_Flash_StatusType retVal = ECUABS_FLASH_OK;
    uint32_t baseAddr = 0U;

    retVal = EcuAbs_Flash_GetSectorAddr(sectorId, &baseAddr);

    if (retVal == ECUABS_FLASH_OK)
    {
        if ((pData == NULL) || ((offset + length) > ECUABS_FLASH_SECTOR_SIZE))
        {
            retVal = ECUABS_FLASH_ERROR;
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
 * @param   sectorId    Logical sector (ECUABS_FLASH_NVM_MAIN or BACKUP)
 * @param   offset      Byte offset within the sector (must be 4-byte aligned)
 * @param   pData       Pointer to source data
 * @param   length      Number of bytes to write
 * @retval  EcuAbs Flash status
 */
EcuAbs_Flash_StatusType EcuAbs_Flash_Write(uint8_t sectorId, uint32_t offset, const uint8_t *pData, uint32_t length)
{
    EcuAbs_Flash_StatusType retVal = ECUABS_FLASH_OK;
    uint32_t baseAddr = 0U;
    uint32_t writeAddr;
    uint32_t wordData;
    uint32_t idx;
    uint32_t remaining;
    HAL_StatusTypeDef halStatus = HAL_OK;

    retVal = EcuAbs_Flash_GetSectorAddr(sectorId, &baseAddr);

    if (retVal == ECUABS_FLASH_OK)
    {
        if ((pData == NULL) || ((offset % 4U) != 0U) || ((offset + length) > ECUABS_FLASH_SECTOR_SIZE))
        {
            retVal = ECUABS_FLASH_ERROR;
        }
    }

    if (retVal == ECUABS_FLASH_OK)
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
            retVal = ECUABS_FLASH_ERROR;
        }
    }

    return retVal;
}

/**
 * @brief   Erase an NVM Flash sector.
 * @param   sectorId    Logical sector (ECUABS_FLASH_NVM_MAIN or BACKUP)
 * @retval  EcuAbs Flash status
 */
EcuAbs_Flash_StatusType EcuAbs_Flash_EraseSector(uint8_t sectorId)
{
    EcuAbs_Flash_StatusType retVal = ECUABS_FLASH_OK;
    uint32_t sectorNum = 0U;
    uint32_t sectorError = 0U;
    FLASH_EraseInitTypeDef eraseInit;
    HAL_StatusTypeDef halStatus;

    retVal = EcuAbs_Flash_GetSectorNum(sectorId, &sectorNum);

    if (retVal == ECUABS_FLASH_OK)
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
            retVal = ECUABS_FLASH_ERROR;
        }
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/

/**
 * @brief   Convert logical sector ID to physical base address.
 * @param   sectorId    Logical sector ID
 * @param   pAddr       Pointer to store physical base address
 * @retval  EcuAbs Flash status
 */
static EcuAbs_Flash_StatusType EcuAbs_Flash_GetSectorAddr(uint8_t sectorId, uint32_t *pAddr)
{
    EcuAbs_Flash_StatusType retVal = ECUABS_FLASH_OK;

    if (sectorId == ECUABS_FLASH_NVM_MAIN)
    {
        *pAddr = ECUABS_FLASH_SECTOR6_ADDR;
    }
    else if (sectorId == ECUABS_FLASH_NVM_BACKUP)
    {
        *pAddr = ECUABS_FLASH_SECTOR7_ADDR;
    }
    else
    {
        retVal = ECUABS_FLASH_ERROR;
    }

    return retVal;
}

/**
 * @brief   Convert logical sector ID to physical sector number.
 * @param   sectorId    Logical sector ID
 * @param   pSector     Pointer to store physical sector number
 * @retval  EcuAbs Flash status
 */
static EcuAbs_Flash_StatusType EcuAbs_Flash_GetSectorNum(uint8_t sectorId, uint32_t *pSector)
{
    EcuAbs_Flash_StatusType retVal = ECUABS_FLASH_OK;

    if (sectorId == ECUABS_FLASH_NVM_MAIN)
    {
        *pSector = ECUABS_FLASH_SECTOR6_NUM;
    }
    else if (sectorId == ECUABS_FLASH_NVM_BACKUP)
    {
        *pSector = ECUABS_FLASH_SECTOR7_NUM;
    }
    else
    {
        retVal = ECUABS_FLASH_ERROR;
    }

    return retVal;
}
