/*
************************************************************************************************************************
*
* Filename        : Svc_Nvm.cpp
* Project         : STM32_Sensor_Monitor
* Description     : NVM Service Module Implementation.
*                   Manages non-volatile diagnostic data using EcuAbsFlash.
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
#include "Svc_Nvm.hpp"
#include <cstring>
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    /* Data validity marker */
    constexpr uint32_t VALID_MARKER    = 0xA5A5A5A5U;   /*!< Written before data block */
    constexpr uint32_t MARKER_SIZE     = 4U;             /*!< Size of validity marker   */

    /* Power cycle marker */
    constexpr uint8_t CYCLE_MASK       = 0x80U;          /*!< Bit 7 for cycle marker    */

    /* Valid diagnostic result values (lower 7 bits) */
    constexpr uint8_t FAULT_VALUE_MASK = 0x7FU;          /*!< Mask to extract result    */
    constexpr uint8_t VALID_NO_FAULT   = 0U;
    constexpr uint8_t VALID_FAULT      = 1U;
    constexpr uint8_t VALID_WARNING    = 2U;
}
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Initialize NVM service.
 *          Reads stored data from Flash and toggles cycle marker.
 * @retval  SvcNvm::Status
 */
SvcNvm::Status SvcNvm::init(void)
{
    Status retVal;

    /* Try to read from main sector */
    retVal = readBlock(EcuAbsFlash::NVM_MAIN, &nvmMirror_);

    if (retVal == Status::Empty)
    {
        /* No stored data, initialize with defaults */
        (void)memset(&nvmMirror_, 0, sizeof(nvmMirror_));
    }
    else if (retVal == Status::Ok)
    {
        /* Validate stored data integrity */
        if (validateData(&nvmMirror_) != Status::Ok)
        {
            /* Corrupted data: clear NVM and reset to defaults */
            (void)flash_.eraseSector(EcuAbsFlash::NVM_MAIN);
            (void)memset(&nvmMirror_, 0, sizeof(nvmMirror_));
        }
        else
        {
            /* Toggle cycle marker for new power cycle */
            nvmMirror_.cycleMarker ^= CYCLE_MASK;
        }
    }
    else
    {
        /* Read error - initialize with defaults */
        (void)memset(&nvmMirror_, 0, sizeof(nvmMirror_));
    }

    nvmInitialized_ = 1U;

    return retVal;
}

/**
 * @brief   Read diagnostic data from NVM RAM mirror.
 * @param   pData   Pointer to store diagnostic data
 * @retval  SvcNvm::Status
 */
SvcNvm::Status SvcNvm::readDiagData(DiagData *pData)
{
    Status retVal = Status::Ok;

    if ((pData == nullptr) || (nvmInitialized_ == 0U))
    {
        retVal = Status::Error;
    }
    else
    {
        (void)memcpy(pData, &nvmMirror_, sizeof(DiagData));
    }

    return retVal;
}

/**
 * @brief   Write diagnostic data to NVM.
 *          Updates RAM mirror and programs to Flash main sector.
 * @param   pData   Pointer to diagnostic data to store
 * @retval  SvcNvm::Status
 */
SvcNvm::Status SvcNvm::writeDiagData(const DiagData *pData)
{
    Status retVal = Status::Ok;

    if ((pData == nullptr) || (nvmInitialized_ == 0U))
    {
        retVal = Status::Error;
    }
    else
    {
        /* Update RAM mirror */
        (void)memcpy(&nvmMirror_, pData, sizeof(DiagData));

        /* Erase main sector and write new data */
        retVal = writeBlock(EcuAbsFlash::NVM_MAIN, pData);
    }

    return retVal;
}

/**
 * @brief   Clear all NVM diagnostic data.
 *          Erases Flash sector and resets RAM mirror to zero.
 * @retval  SvcNvm::Status
 */
SvcNvm::Status SvcNvm::clearAll(void)
{
    Status retVal = Status::Ok;
    EcuAbsFlash::Status flashStatus;

    flashStatus = flash_.eraseSector(EcuAbsFlash::NVM_MAIN);

    if (flashStatus != EcuAbsFlash::Status::Ok)
    {
        retVal = Status::Error;
    }
    else
    {
        (void)memset(&nvmMirror_, 0, sizeof(nvmMirror_));
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/

/**
 * @brief   Read and validate data block from NVM sector.
 * @param   sectorId    Logical sector ID
 * @param   pData       Pointer to store data
 * @retval  SvcNvm::Status (Ok if valid marker found, Empty otherwise)
 */
SvcNvm::Status SvcNvm::readBlock(uint8_t sectorId, DiagData *pData)
{
    Status retVal = Status::Ok;
    uint8_t markerBuf[MARKER_SIZE];
    uint32_t marker;
    EcuAbsFlash::Status flashStatus;

    /* Read validity marker */
    flashStatus = flash_.read(sectorId, 0U, markerBuf, MARKER_SIZE);

    if (flashStatus != EcuAbsFlash::Status::Ok)
    {
        retVal = Status::Error;
    }
    else
    {
        (void)memcpy(&marker, markerBuf, sizeof(marker));

        if (marker != VALID_MARKER)
        {
            retVal = Status::Empty;
        }
        else
        {
            /* Read data after marker */
            flashStatus = flash_.read(sectorId, MARKER_SIZE,
                                      reinterpret_cast<uint8_t *>(pData),
                                      static_cast<uint32_t>(sizeof(DiagData)));

            if (flashStatus != EcuAbsFlash::Status::Ok)
            {
                retVal = Status::Error;
            }
        }
    }

    return retVal;
}

/**
 * @brief   Write data block with validity marker to NVM sector.
 * @param   sectorId    Logical sector ID
 * @param   pData       Pointer to data to store
 * @retval  SvcNvm::Status
 */
SvcNvm::Status SvcNvm::writeBlock(uint8_t sectorId, const DiagData *pData)
{
    Status retVal = Status::Ok;
    EcuAbsFlash::Status flashStatus;
    uint8_t markerBuf[MARKER_SIZE];
    uint32_t marker = VALID_MARKER;

    /* Erase sector before programming */
    flashStatus = flash_.eraseSector(sectorId);

    if (flashStatus != EcuAbsFlash::Status::Ok)
    {
        retVal = Status::Error;
    }
    else
    {
        /* Write validity marker */
        (void)memcpy(markerBuf, &marker, sizeof(marker));
        flashStatus = flash_.write(sectorId, 0U, markerBuf, MARKER_SIZE);
    }

    if ((retVal == Status::Ok) && (flashStatus == EcuAbsFlash::Status::Ok))
    {
        /* Write data after marker */
        flashStatus = flash_.write(sectorId, MARKER_SIZE,
                                   reinterpret_cast<const uint8_t *>(pData),
                                   static_cast<uint32_t>(sizeof(DiagData)));
    }

    if (flashStatus != EcuAbsFlash::Status::Ok)
    {
        retVal = Status::Error;
    }

    return retVal;
}

/**
 * @brief   Validate NVM diagnostic data integrity.
 *          Checks cycleMarker, fault types, and count values.
 * @param   pData   Pointer to data to validate
 * @retval  SvcNvm::Status::Ok if valid, SvcNvm::Status::Error if corrupted
 */
SvcNvm::Status SvcNvm::validateData(const DiagData *pData)
{
    Status retVal = Status::Ok;
    uint8_t commVal;
    uint8_t dataVal;
    uint8_t envVal;

    /* Cycle marker must be 0x00 or 0x80 */
    if ((pData->cycleMarker != 0x00U) && (pData->cycleMarker != CYCLE_MASK))
    {
        retVal = Status::Error;
    }

    if (retVal == Status::Ok)
    {
        /* Extract result values (strip cycle marker bit) */
        commVal = pData->commFault & FAULT_VALUE_MASK;
        dataVal = pData->dataFault & FAULT_VALUE_MASK;
        envVal  = pData->envWarning & FAULT_VALUE_MASK;

        /* CommFault: only NO_FAULT(0) or FAULT(1) allowed */
        if ((commVal != VALID_NO_FAULT) && (commVal != VALID_FAULT))
        {
            retVal = Status::Error;
        }
    }

    if (retVal == Status::Ok)
    {
        /* DataFault: only NO_FAULT(0) or FAULT(1) allowed */
        if ((dataVal != VALID_NO_FAULT) && (dataVal != VALID_FAULT))
        {
            retVal = Status::Error;
        }
    }

    if (retVal == Status::Ok)
    {
        /* EnvWarning: only NO_FAULT(0) or WARNING(2) allowed */
        if ((envVal != VALID_NO_FAULT) && (envVal != VALID_WARNING))
        {
            retVal = Status::Error;
        }
    }

    return retVal;
}
