/*
************************************************************************************************************************
*
* Filename        : Svc_Nvm.hpp
* Project         : STM32_Sensor_Monitor
* Description     : NVM Service Module Interface.
*                   Provides non-volatile storage using internal Flash emulation.
*                   Sector 6 (main) and Sector 7 (backup) are used for data storage.
*                   Power cycle marker (bit 7) distinguishes fault data across ignition cycles.
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
#include "EcuAbs_Flash.hpp"
#include <cstdint>
/*
************************************************************************************************************************
*                                                    Class definition
************************************************************************************************************************
*/
/** @brief NVM Service - diagnostic data persistence class */
class SvcNvm
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

    /** @brief NVM operation status definition */
    enum class Status : uint8_t
    {
        Ok    = 0x00U,   /*!< No error              */
        Error = 0x01U,   /*!< General error         */
        Empty = 0x02U    /*!< NVM contains no data  */
    };

    /** @brief NVM diagnostic data structure */
    struct DiagData
    {
        uint8_t  cycleMarker;       /*!< Power cycle marker (0x00 or 0x80)     */
        uint8_t  commFault;         /*!< CommFault (marker | result)           */
        uint8_t  dataFault;         /*!< DataFault (marker | result)           */
        uint8_t  envWarning;        /*!< EnvWarning (marker | result)          */
        uint32_t commFaultCount;    /*!< CommFault cumulative count            */
        uint32_t dataFaultCount;    /*!< DataFault cumulative count            */
        uint32_t envWarningCount;   /*!< EnvWarning cumulative count           */
    };

/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/

    /**
     * @brief   Initialize NVM service.
     *          Reads stored data from Flash and toggles cycle marker.
     * @retval  SvcNvm::Status
     */
    Status init(void);

    /**
     * @brief   Read diagnostic data from NVM RAM mirror.
     * @param   pData   Pointer to store diagnostic data
     * @retval  SvcNvm::Status
     */
    Status readDiagData(DiagData *pData);

    /**
     * @brief   Write diagnostic data to NVM.
     *          Programs data to Flash main sector.
     * @param   pData   Pointer to diagnostic data to store
     * @retval  SvcNvm::Status
     */
    Status writeDiagData(const DiagData *pData);

    /**
     * @brief   Clear all NVM diagnostic data.
     *          Erases Flash sector and resets RAM mirror to zero.
     * @retval  SvcNvm::Status
     */
    Status clearAll(void);

private:
/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

    EcuAbsFlash flash_;                      /*!< Flash driver instance     */
    DiagData nvmMirror_ = {};                /*!< RAM mirror of NVM data    */
    uint8_t nvmInitialized_ = 0U;            /*!< Init completed flag       */

/*
********************************************************************************************************************
*                                                  Private methods
********************************************************************************************************************
*/

    /**
     * @brief   Read and validate data block from NVM sector.
     * @param   sectorId    Logical sector ID
     * @param   pData       Pointer to store data
     * @retval  SvcNvm::Status (Ok if valid marker found, Empty otherwise)
     */
    Status readBlock(uint8_t sectorId, DiagData *pData);

    /**
     * @brief   Validate NVM diagnostic data integrity.
     *          Checks that all fields contain expected values.
     * @param   pData   Pointer to data to validate
     * @retval  SvcNvm::Status::Ok if valid, SvcNvm::Status::Error if corrupted
     */
    Status validateData(const DiagData *pData);

    /**
     * @brief   Write data block with validity marker to NVM sector.
     * @param   sectorId    Logical sector ID
     * @param   pData       Pointer to data to store
     * @retval  SvcNvm::Status
     */
    Status writeBlock(uint8_t sectorId, const DiagData *pData);
};
