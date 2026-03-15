/*
************************************************************************************************************************
*
* Filename        : RteApp_Diag.h
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : RTE Application Interface - Diagnostic Data Exchange.
*                   Provides read/write interface for diagnostic results
*                   between ASW Diag and ASW Manage modules.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef RTEAPP_DIAG_H
#define RTEAPP_DIAG_H
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

/** @brief RteApp_Diag operation status definition */
typedef enum
{
    RTEAPP_DIAG_OK       = 0x00U,   /*!< No error       */
    RTEAPP_DIAG_ERROR    = 0x01U    /*!< General error   */
} RteApp_Diag_StatusType;
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

/* Diagnostic result write (ASW Diag → RTE) */

/**
 * @brief   Write sensor communication fault status to RTE buffer.
 * @param   fault   0: No fault, 1: Fault
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Write_CommFault(uint8_t fault);

/**
 * @brief   Write sensor data fault status to RTE buffer.
 * @param   fault   0: No fault, 1: Fault
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Write_DataFault(uint8_t fault);

/**
 * @brief   Write environment warning status to RTE buffer.
 * @param   warning   0: No warning, 1: Warning
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Write_EnvWarning(uint8_t warning);

/* Diagnostic result read (RTE → ASW Manage) */

/**
 * @brief   Read sensor communication fault status from RTE buffer.
 * @param   pFault  Pointer to store fault status
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Read_CommFault(uint8_t *pFault);

/**
 * @brief   Read sensor data fault status from RTE buffer.
 * @param   pFault  Pointer to store fault status
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Read_DataFault(uint8_t *pFault);

/**
 * @brief   Read environment warning status from RTE buffer.
 * @param   pWarning  Pointer to store warning status
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Read_EnvWarning(uint8_t *pWarning);

#endif /* RTEAPP_DIAG_H */
