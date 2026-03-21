/*
************************************************************************************************************************
*
* Filename        : Svc_Nvm.h
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : NVM Service Module Interface.
*                   Provides non-volatile storage using internal Flash emulation.
*                   Sector 6 (main) and Sector 7 (backup) are used for data storage.
*                   Power cycle marker (bit 7) distinguishes fault data across ignition cycles.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef SVC_NVM_H
#define SVC_NVM_H
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
/*
************************************************************************************************************************
*                                                         Typedefs
************************************************************************************************************************
*/

/** @brief NVM operation status definition */
typedef enum
{
    SVC_NVM_OK          = 0x00U,   /*!< No error              */
    SVC_NVM_ERROR       = 0x01U,   /*!< General error         */
    SVC_NVM_EMPTY       = 0x02U    /*!< NVM contains no data  */
} Svc_Nvm_StatusType;

/** @brief NVM diagnostic data structure */
typedef struct
{
    uint8_t  cycleMarker;       /*!< Power cycle marker (0x00 or 0x80)     */
    uint8_t  commFault;         /*!< CommFault (marker | result)           */
    uint8_t  dataFault;         /*!< DataFault (marker | result)           */
    uint8_t  envWarning;        /*!< EnvWarning (marker | result)          */
    uint32_t commFaultCount;    /*!< CommFault cumulative count            */
    uint32_t dataFaultCount;    /*!< DataFault cumulative count            */
    uint32_t envWarningCount;   /*!< EnvWarning cumulative count           */
} Svc_Nvm_DiagDataType;
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
 * @brief   Initialize NVM service.
 *          Reads stored data from Flash and toggles cycle marker.
 * @retval  NVM status
 */
Svc_Nvm_StatusType Svc_Nvm_Init(void);

/**
 * @brief   Read diagnostic data from NVM RAM mirror.
 * @param   pData   Pointer to store diagnostic data
 * @retval  NVM status
 */
Svc_Nvm_StatusType Svc_Nvm_ReadDiagData(Svc_Nvm_DiagDataType *pData);

/**
 * @brief   Write diagnostic data to NVM.
 *          Programs data to Flash main sector.
 * @param   pData   Pointer to diagnostic data to store
 * @retval  NVM status
 */
Svc_Nvm_StatusType Svc_Nvm_WriteDiagData(const Svc_Nvm_DiagDataType *pData);

/**
 * @brief   Clear all NVM diagnostic data.
 *          Erases Flash sector and resets RAM mirror to zero.
 * @retval  NVM status
 */
Svc_Nvm_StatusType Svc_Nvm_ClearAll(void);

#endif /* SVC_NVM_H */
