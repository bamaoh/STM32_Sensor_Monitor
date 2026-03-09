/*
************************************************************************************************************************
*
* Filename        : RteApp_Bme280.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/26
* Description     : RTE Application Interface - BME280 Data Exchange Implementation.
*                   Manages static data buffers for BME280 measurement data
*                   exchange between Service and ASW layers.
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
#include "RteApp_Bme280.h"
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
/*
************************************************************************************************************************
*                                                    Private variables
************************************************************************************************************************
*/

static int32_t  rteTemperature = 0;      /*!< Temperature buffer (0.01 degree C)  */
static uint32_t rtePressure    = 0U;     /*!< Pressure buffer (Pa)                */
static uint32_t rteHumidity    = 0U;     /*!< Humidity buffer (0.01 %RH)          */
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                                    Public functions
************************************************************************************************************************
*/

/**
 * @brief   Write temperature data to RTE buffer.
 * @param   value   Temperature in 0.01 degree C
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Write_Temperature(int32_t value)
{
    rteTemperature = value;

    return RTEAPP_OK;
}

/**
 * @brief   Write pressure data to RTE buffer.
 * @param   value   Pressure in Pa
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Write_Pressure(uint32_t value)
{
    rtePressure = value;

    return RTEAPP_OK;
}

/**
 * @brief   Write humidity data to RTE buffer.
 * @param   value   Humidity in 0.01 %RH
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Write_Humidity(uint32_t value)
{
    rteHumidity = value;

    return RTEAPP_OK;
}

/**
 * @brief   Read temperature data from RTE buffer.
 * @param   pValue  Pointer to store temperature in 0.01 degree C
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Read_Temperature(int32_t *pValue)
{
    RteApp_StatusType retVal = RTEAPP_OK;

    if (pValue == NULL)
    {
        retVal = RTEAPP_ERROR;
    }
    else
    {
        *pValue = rteTemperature;
    }

    return retVal;
}

/**
 * @brief   Read pressure data from RTE buffer.
 * @param   pValue  Pointer to store pressure in Pa
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Read_Pressure(uint32_t *pValue)
{
    RteApp_StatusType retVal = RTEAPP_OK;

    if (pValue == NULL)
    {
        retVal = RTEAPP_ERROR;
    }
    else
    {
        *pValue = rtePressure;
    }

    return retVal;
}

/**
 * @brief   Read humidity data from RTE buffer.
 * @param   pValue  Pointer to store humidity in 0.01 %RH
 * @retval  RteApp status
 */
RteApp_StatusType RteApp_Bme280_Read_Humidity(uint32_t *pValue)
{
    RteApp_StatusType retVal = RTEAPP_OK;

    if (pValue == NULL)
    {
        retVal = RTEAPP_ERROR;
    }
    else
    {
        *pValue = rteHumidity;
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/
