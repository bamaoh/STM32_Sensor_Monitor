/*
************************************************************************************************************************
*
* Filename        : RteApp_Bme280.h
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/26
* Description     : RTE Application Interface - BME280 Data Exchange.
*                   Provides read/write interface for BME280 measurement data
*                   between Service layer and ASW layer.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef RTEAPP_BME280_H
#define RTEAPP_BME280_H
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
    RTEAPP_OK       = 0x00U,   /*!< No error       */
    RTEAPP_ERROR    = 0x01U    /*!< General error   */
} RteApp_StatusType;
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
 * @brief   Write temperature data to RTE buffer.
 * @param   value   Temperature in 0.01 degree C
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Write_Temperature(int32_t value);

/**
 * @brief   Write pressure data to RTE buffer.
 * @param   value   Pressure in Pa
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Write_Pressure(uint32_t value);

/**
 * @brief   Write humidity data to RTE buffer.
 * @param   value   Humidity in 0.01 %RH
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Write_Humidity(uint32_t value);

/**
 * @brief   Read temperature data from RTE buffer.
 * @param   pValue  Pointer to store temperature in 0.01 degree C
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Read_Temperature(int32_t *pValue);

/**
 * @brief   Read pressure data from RTE buffer.
 * @param   pValue  Pointer to store pressure in Pa
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Read_Pressure(uint32_t *pValue);

/**
 * @brief   Read humidity data from RTE buffer.
 * @param   pValue  Pointer to store humidity in 0.01 %RH
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Read_Humidity(uint32_t *pValue);

#endif /* RTEAPP_BME280_H */
