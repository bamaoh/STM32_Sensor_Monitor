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

/* Valid diagnostic result values (lower 7 bits) */
#define SVC_NVM_FAULT_VALUE_MASK    (0x7FU)     /*!< Mask to extract result from cycle marker */
#define SVC_NVM_VALID_NO_FAULT      (0U)
#define SVC_NVM_VALID_FAULT         (1U)
#define SVC_NVM_VALID_WARNING       (2U)
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
 * @brief   Validate NVM diagnostic data integrity.
 *          Checks that all fields contain expected values.
 * @param   pData   Pointer to data to validate
 * @retval  SVC_NVM_OK if valid, SVC_NVM_ERROR if corrupted
 */
static Svc_Nvm_StatusType Svc_Nvm_ValidateData(const Svc_Nvm_DiagDataType *pData);

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
        /* Validate stored data integrity */
        if (Svc_Nvm_ValidateData(&nvmMirror) != SVC_NVM_OK)
        {
            /* Corrupted data: clear NVM and reset to defaults */
            (void)EcuAbs_Flash_EraseSector(ECUABS_FLASH_NVM_MAIN);
            (void)memset(&nvmMirror, 0, sizeof(nvmMirror));
        }
        else
        {
            /* Toggle cycle marker for new power cycle */
            nvmMirror.cycleMarker ^= SVC_NVM_CYCLE_MASK;
        }
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

/**
 * @brief   Clear all NVM diagnostic data.
 *          Erases Flash sector and resets RAM mirror to zero.
 * @retval  NVM status
 */
Svc_Nvm_StatusType Svc_Nvm_ClearAll(void)
{
    Svc_Nvm_StatusType retVal = SVC_NVM_OK;
    EcuAbs_Flash_StatusType flashStatus;

    flashStatus = EcuAbs_Flash_EraseSector(ECUABS_FLASH_NVM_MAIN);

    if (flashStatus != ECUABS_FLASH_OK)
    {
        retVal = SVC_NVM_ERROR;
    }
    else
    {
        (void)memset(&nvmMirror, 0, sizeof(nvmMirror));
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

/**
 * @brief   Validate NVM diagnostic data integrity.
 *          Checks cycleMarker, fault types, and count values.
 * @param   pData   Pointer to data to validate
 * @retval  SVC_NVM_OK if valid, SVC_NVM_ERROR if corrupted
 */
static Svc_Nvm_StatusType Svc_Nvm_ValidateData(const Svc_Nvm_DiagDataType *pData)
{
    Svc_Nvm_StatusType retVal = SVC_NVM_OK;
    uint8_t commVal;
    uint8_t dataVal;
    uint8_t envVal;

    /* Cycle marker must be 0x00 or 0x80 */
    if ((pData->cycleMarker != 0x00U) && (pData->cycleMarker != SVC_NVM_CYCLE_MASK))
    {
        retVal = SVC_NVM_ERROR;
    }

    if (retVal == SVC_NVM_OK)
    {
        /* Extract result values (strip cycle marker bit) */
        commVal = pData->commFault & SVC_NVM_FAULT_VALUE_MASK;
        dataVal = pData->dataFault & SVC_NVM_FAULT_VALUE_MASK;
        envVal  = pData->envWarning & SVC_NVM_FAULT_VALUE_MASK;

        /* CommFault: only NO_FAULT(0) or FAULT(1) allowed */
        if ((commVal != SVC_NVM_VALID_NO_FAULT) && (commVal != SVC_NVM_VALID_FAULT))
        {
            retVal = SVC_NVM_ERROR;
        }
    }

    if (retVal == SVC_NVM_OK)
    {
        /* DataFault: only NO_FAULT(0) or FAULT(1) allowed */
        if ((dataVal != SVC_NVM_VALID_NO_FAULT) && (dataVal != SVC_NVM_VALID_FAULT))
        {
            retVal = SVC_NVM_ERROR;
        }
    }

    if (retVal == SVC_NVM_OK)
    {
        /* EnvWarning: only NO_FAULT(0) or WARNING(2) allowed */
        if ((envVal != SVC_NVM_VALID_NO_FAULT) && (envVal != SVC_NVM_VALID_WARNING))
        {
            retVal = SVC_NVM_ERROR;
        }
    }

    return retVal;
}
