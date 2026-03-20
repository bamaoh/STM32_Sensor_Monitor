/*
************************************************************************************************************************
*
* Filename        : Svc_Nvm.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : NVM Service Module Implementation.
*                   Manages non-volatile diagnostic data using EcuAbs_Flash.
*                   Sector 6 (main) stores data, Sector 7 (backup) for compaction.
*                   Power cycle marker toggles each initialization to distinguish
*                   fault data across ignition cycles.
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
#include "Svc_Nvm.h"
#include "EcuAbs_Flash.h"
#include <string.h>
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

/* Data validity marker */
#define SVC_NVM_VALID_MARKER    (0xA5A5A5A5U)   /*!< Written before data block */
#define SVC_NVM_MARKER_SIZE     (4U)            /*!< Size of validity marker   */
#define SVC_NVM_DATA_SIZE       (sizeof(Svc_Nvm_DiagDataType))

/* Power cycle marker */
#define SVC_NVM_CYCLE_MASK      (0x80U)         /*!< Bit 7 for cycle marker    */
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

static Svc_Nvm_DiagDataType nvmMirror;     /*!< RAM mirror of NVM data    */
static uint8_t nvmInitialized = 0U;        /*!< Init completed flag       */
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/

/**
 * @brief   Read and validate data block from NVM sector.
 * @param   sectorId    Logical sector ID
 * @param   pData       Pointer to store data
 * @retval  NVM status (OK if valid marker found, EMPTY otherwise)
 */
static Svc_Nvm_StatusType Svc_Nvm_ReadBlock(uint8_t sectorId, Svc_Nvm_DiagDataType *pData);

/**
 * @brief   Write data block with validity marker to NVM sector.
 * @param   sectorId    Logical sector ID
 * @param   pData       Pointer to data to store
 * @retval  NVM status
 */
static Svc_Nvm_StatusType Svc_Nvm_WriteBlock(uint8_t sectorId, const Svc_Nvm_DiagDataType *pData);
/*
************************************************************************************************************************
*                                                    Public functions
************************************************************************************************************************
*/

/**
 * @brief   Initialize NVM service.
 *          Reads stored data from Flash and toggles cycle marker.
 * @retval  NVM status
 */
Svc_Nvm_StatusType Svc_Nvm_Init(void)
{
    Svc_Nvm_StatusType retVal;

    /* Try to read from main sector */
    retVal = Svc_Nvm_ReadBlock(ECUABS_FLASH_NVM_MAIN, &nvmMirror);

    if (retVal == SVC_NVM_EMPTY)
    {
        /* No stored data, initialize with defaults */
        (void)memset(&nvmMirror, 0, sizeof(nvmMirror));
    }
    else if (retVal == SVC_NVM_OK)
    {
        /* Toggle cycle marker for new power cycle */
        nvmMirror.cycleMarker ^= SVC_NVM_CYCLE_MASK;
    }
    else
    {
        /* Read error - initialize with defaults */
        (void)memset(&nvmMirror, 0, sizeof(nvmMirror));
    }

    nvmInitialized = 1U;

    return retVal;
}

/**
 * @brief   Read diagnostic data from NVM RAM mirror.
 * @param   pData   Pointer to store diagnostic data
 * @retval  NVM status
 */
Svc_Nvm_StatusType Svc_Nvm_ReadDiagData(Svc_Nvm_DiagDataType *pData)
{
    Svc_Nvm_StatusType retVal = SVC_NVM_OK;

    if ((pData == NULL) || (nvmInitialized == 0U))
    {
        retVal = SVC_NVM_ERROR;
    }
    else
    {
        (void)memcpy(pData, &nvmMirror, sizeof(Svc_Nvm_DiagDataType));
    }

    return retVal;
}

/**
 * @brief   Write diagnostic data to NVM.
 *          Updates RAM mirror and programs to Flash main sector.
 * @param   pData   Pointer to diagnostic data to store
 * @retval  NVM status
 */
Svc_Nvm_StatusType Svc_Nvm_WriteDiagData(const Svc_Nvm_DiagDataType *pData)
{
    Svc_Nvm_StatusType retVal = SVC_NVM_OK;

    if ((pData == NULL) || (nvmInitialized == 0U))
    {
        retVal = SVC_NVM_ERROR;
    }
    else
    {
        /* Update RAM mirror */
        (void)memcpy(&nvmMirror, pData, sizeof(Svc_Nvm_DiagDataType));

        /* Erase main sector and write new data */
        retVal = Svc_Nvm_WriteBlock(ECUABS_FLASH_NVM_MAIN, pData);
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/

/**
 * @brief   Read and validate data block from NVM sector.
 * @param   sectorId    Logical sector ID
 * @param   pData       Pointer to store data
 * @retval  NVM status (OK if valid marker found, EMPTY otherwise)
 */
static Svc_Nvm_StatusType Svc_Nvm_ReadBlock(uint8_t sectorId, Svc_Nvm_DiagDataType *pData)
{
    Svc_Nvm_StatusType retVal = SVC_NVM_OK;
    uint8_t markerBuf[SVC_NVM_MARKER_SIZE];
    uint32_t marker;
    EcuAbs_Flash_StatusType flashStatus;

    /* Read validity marker */
    flashStatus = EcuAbs_Flash_Read(sectorId, 0U, markerBuf, SVC_NVM_MARKER_SIZE);

    if (flashStatus != ECUABS_FLASH_OK)
    {
        retVal = SVC_NVM_ERROR;
    }
    else
    {
        (void)memcpy(&marker, markerBuf, sizeof(marker));

        if (marker != SVC_NVM_VALID_MARKER)
        {
            retVal = SVC_NVM_EMPTY;
        }
        else
        {
            /* Read data after marker */
            flashStatus = EcuAbs_Flash_Read(sectorId, SVC_NVM_MARKER_SIZE, (uint8_t *)pData, (uint32_t)SVC_NVM_DATA_SIZE);

            if (flashStatus != ECUABS_FLASH_OK)
            {
                retVal = SVC_NVM_ERROR;
            }
        }
    }

    return retVal;
}

/**
 * @brief   Write data block with validity marker to NVM sector.
 * @param   sectorId    Logical sector ID
 * @param   pData       Pointer to data to store
 * @retval  NVM status
 */
static Svc_Nvm_StatusType Svc_Nvm_WriteBlock(uint8_t sectorId, const Svc_Nvm_DiagDataType *pData)
{
    Svc_Nvm_StatusType retVal = SVC_NVM_OK;
    EcuAbs_Flash_StatusType flashStatus;
    uint8_t markerBuf[SVC_NVM_MARKER_SIZE];
    uint32_t marker = SVC_NVM_VALID_MARKER;

    /* Erase sector before programming */
    flashStatus = EcuAbs_Flash_EraseSector(sectorId);

    if (flashStatus != ECUABS_FLASH_OK)
    {
        retVal = SVC_NVM_ERROR;
    }
    else
    {
        /* Write validity marker */
        (void)memcpy(markerBuf, &marker, sizeof(marker));
        flashStatus = EcuAbs_Flash_Write(sectorId, 0U, markerBuf, SVC_NVM_MARKER_SIZE);
    }

    if ((retVal == SVC_NVM_OK) && (flashStatus == ECUABS_FLASH_OK))
    {
        /* Write data after marker */
        flashStatus = EcuAbs_Flash_Write(sectorId, SVC_NVM_MARKER_SIZE, (const uint8_t *)pData, (uint32_t)SVC_NVM_DATA_SIZE);
    }

    if (flashStatus != ECUABS_FLASH_OK)
    {
        retVal = SVC_NVM_ERROR;
    }

    return retVal;
}
