/*
************************************************************************************************************************
*
* Filename        : RteApp_Sensor.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/26
* Description     : RTE Application Interface - Sensor Data Exchange Implementation.
*                   Manages static data buffers for filtered sensor data and
*                   diagnostic flags between ASW Sensor and upper modules.
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
#include "RteApp_Sensor.h"
#include "stddef.h"
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

/* Filtered sensor data buffers */
static int32_t  rteFilteredTemp     = 0;      /*!< Filtered temperature (0.01 degree C)  */
static uint32_t rteFilteredPress    = 0U;     /*!< Filtered pressure (Pa)                */
static uint32_t rteFilteredHum      = 0U;     /*!< Filtered humidity (0.01 %RH)          */

/* Diagnostic flag buffers */
static uint8_t rteTempDiag  = 0U;   /*!< Temperature diag  */
static uint8_t rtePressDiag = 0U;   /*!< Pressure diag     */
static uint8_t rteHumDiag   = 0U;   /*!< Humidity diag     */
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

/* ---- Filtered data write ---- */

/**
 * @brief   Write filtered temperature data to RTE buffer.
 * @param   value   Filtered temperature in 0.01 degree C
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_Filtered_Temperature(int32_t value)
{
    rteFilteredTemp = value;

    return RTEAPP_SENSOR_OK;
}

/**
 * @brief   Write filtered pressure data to RTE buffer.
 * @param   value   Filtered pressure in Pa
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_Filtered_Pressure(uint32_t value)
{
    rteFilteredPress = value;

    return RTEAPP_SENSOR_OK;
}

/**
 * @brief   Write filtered humidity data to RTE buffer.
 * @param   value   Filtered humidity in 0.01 %RH
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_Filtered_Humidity(uint32_t value)
{
    rteFilteredHum = value;

    return RTEAPP_SENSOR_OK;
}

/* ---- Diagnostic write ---- */

/**
 * @brief   Write temperature diagnostic flag to RTE buffer.
 * @param   diag   Temperature diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_TempDiag(uint8_t diag)
{
    rteTempDiag = diag;

    return RTEAPP_SENSOR_OK;
}

/**
 * @brief   Write pressure diagnostic flag to RTE buffer.
 * @param   diag   Pressure diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_PressDiag(uint8_t diag)
{
    rtePressDiag = diag;

    return RTEAPP_SENSOR_OK;
}

/**
 * @brief   Write humidity diagnostic flag to RTE buffer.
 * @param   diag   Humidity diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Write_HumDiag(uint8_t diag)
{
    rteHumDiag = diag;

    return RTEAPP_SENSOR_OK;
}

/* ---- Filtered data read ---- */

/**
 * @brief   Read filtered temperature data from RTE buffer.
 * @param   pValue  Pointer to store filtered temperature in 0.01 degree C
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_Filtered_Temperature(int32_t *pValue)
{
    RteApp_Sensor_StatusType retVal = RTEAPP_SENSOR_OK;

    if (pValue == NULL)
    {
        retVal = RTEAPP_SENSOR_ERROR;
    }
    else
    {
        *pValue = rteFilteredTemp;
    }

    return retVal;
}

/**
 * @brief   Read filtered pressure data from RTE buffer.
 * @param   pValue  Pointer to store filtered pressure in Pa
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_Filtered_Pressure(uint32_t *pValue)
{
    RteApp_Sensor_StatusType retVal = RTEAPP_SENSOR_OK;

    if (pValue == NULL)
    {
        retVal = RTEAPP_SENSOR_ERROR;
    }
    else
    {
        *pValue = rteFilteredPress;
    }

    return retVal;
}

/**
 * @brief   Read filtered humidity data from RTE buffer.
 * @param   pValue  Pointer to store filtered humidity in 0.01 %RH
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_Filtered_Humidity(uint32_t *pValue)
{
    RteApp_Sensor_StatusType retVal = RTEAPP_SENSOR_OK;

    if (pValue == NULL)
    {
        retVal = RTEAPP_SENSOR_ERROR;
    }
    else
    {
        *pValue = rteFilteredHum;
    }

    return retVal;
}

/* ---- Diagnostic read ---- */

/**
 * @brief   Read temperature diagnostic flag from RTE buffer.
 * @param   pDiag  Pointer to store temperature diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_TempDiag(uint8_t *pDiag)
{
    RteApp_Sensor_StatusType retVal = RTEAPP_SENSOR_OK;

    if (pDiag == NULL)
    {
        retVal = RTEAPP_SENSOR_ERROR;
    }
    else
    {
        *pDiag = rteTempDiag;
    }

    return retVal;
}

/**
 * @brief   Read pressure diagnostic flag from RTE buffer.
 * @param   pDiag  Pointer to store pressure diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_PressDiag(uint8_t *pDiag)
{
    RteApp_Sensor_StatusType retVal = RTEAPP_SENSOR_OK;

    if (pDiag == NULL)
    {
        retVal = RTEAPP_SENSOR_ERROR;
    }
    else
    {
        *pDiag = rtePressDiag;
    }

    return retVal;
}

/**
 * @brief   Read humidity diagnostic flag from RTE buffer.
 * @param   pDiag  Pointer to store humidity diagnostic status
 * @retval  RteApp_Sensor status
 */
RteApp_Sensor_StatusType RteApp_Sensor_Read_HumDiag(uint8_t *pDiag)
{
    RteApp_Sensor_StatusType retVal = RTEAPP_SENSOR_OK;

    if (pDiag == NULL)
    {
        retVal = RTEAPP_SENSOR_ERROR;
    }
    else
    {
        *pDiag = rteHumDiag;
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/
