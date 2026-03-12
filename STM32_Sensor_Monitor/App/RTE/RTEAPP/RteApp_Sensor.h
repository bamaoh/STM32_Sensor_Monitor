/*
************************************************************************************************************************
*
* Filename        : RteApp_Sensor.h
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/26
* Description     : RTE Application Interface - Sensor Data Exchange.
*                   Provides read/write interface for filtered sensor data
*                   and diagnostic flags between ASW Sensor and
*                   ASW Manage / UART output modules.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef RTEAPP_SENSOR_H
#define RTEAPP_SENSOR_H
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

/** @brief RteApp operation status definition */
typedef enum
{
    RTEAPP_SENSOR_OK       = 0x00U,   /*!< No error       */
    RTEAPP_SENSOR_ERROR    = 0x01U    /*!< General error   */
} RteApp_Sensor_StatusType;
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

/* Filtered data write (ASW Sensor → RTE) */

/**
 * @brief   Write filtered temperature data to RTE buffer.
 * @param   value   Filtered temperature in 0.01 degree C
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_Filtered_Temperature(int32_t value);

/**
 * @brief   Write filtered pressure data to RTE buffer.
 * @param   value   Filtered pressure in Pa
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_Filtered_Pressure(uint32_t value);

/**
 * @brief   Write filtered humidity data to RTE buffer.
 * @param   value   Filtered humidity in 0.01 %RH
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_Filtered_Humidity(uint32_t value);

/* Diagnostic write (ASW Sensor → RTE) */

/**
 * @brief   Write temperature diagnostic flag to RTE buffer.
 * @param   diag   Temperature diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_TempDiag(uint8_t diag);

/**
 * @brief   Write pressure diagnostic flag to RTE buffer.
 * @param   diag   Pressure diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_PressDiag(uint8_t diag);

/**
 * @brief   Write humidity diagnostic flag to RTE buffer.
 * @param   diag   Humidity diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_HumDiag(uint8_t diag);

/* Filtered data read (RTE → ASW Manage / UART) */

/**
 * @brief   Read filtered temperature data from RTE buffer.
 * @param   pValue  Pointer to store filtered temperature in 0.01 degree C
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_Filtered_Temperature(int32_t *pValue);

/**
 * @brief   Read filtered pressure data from RTE buffer.
 * @param   pValue  Pointer to store filtered pressure in Pa
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_Filtered_Pressure(uint32_t *pValue);

/**
 * @brief   Read filtered humidity data from RTE buffer.
 * @param   pValue  Pointer to store filtered humidity in 0.01 %RH
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_Filtered_Humidity(uint32_t *pValue);

/* Diagnostic read (RTE → ASW Manage / Diag) */

/**
 * @brief   Read temperature diagnostic flag from RTE buffer.
 * @param   pDiag  Pointer to store temperature diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_TempDiag(uint8_t *pDiag);

/**
 * @brief   Read pressure diagnostic flag from RTE buffer.
 * @param   pDiag  Pointer to store pressure diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_PressDiag(uint8_t *pDiag);

/**
 * @brief   Read humidity diagnostic flag from RTE buffer.
 * @param   pDiag  Pointer to store humidity diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_HumDiag(uint8_t *pDiag);

#endif /* RTEAPP_SENSOR_H */
