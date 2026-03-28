/*
************************************************************************************************************************
*
* Filename        : RteApp_Sensor.cpp
* Project         : STM32_Sensor_Monitor
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
#include "RteApp_Sensor.hpp"
/*
************************************************************************************************************************
*                                                 Static member definitions
************************************************************************************************************************
*/

/* Filtered sensor data buffers */
int32_t  RteAppSensor::rteFilteredTemp_  = 0;
uint32_t RteAppSensor::rteFilteredPress_ = 0U;
uint32_t RteAppSensor::rteFilteredHum_   = 0U;

/* Diagnostic flag buffers */
uint8_t RteAppSensor::rteTempDiag_  = 0U;
uint8_t RteAppSensor::rtePressDiag_ = 0U;
uint8_t RteAppSensor::rteHumDiag_   = 0U;
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/* ---- Filtered data write ---- */

/**
 * @brief   Write filtered temperature data to RTE buffer.
 * @param   value   Filtered temperature in 0.01 degree C
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::writeFilteredTemperature(int32_t value)
{
    rteFilteredTemp_ = value;

    return Status::Ok;
}

/**
 * @brief   Write filtered pressure data to RTE buffer.
 * @param   value   Filtered pressure in Pa
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::writeFilteredPressure(uint32_t value)
{
    rteFilteredPress_ = value;

    return Status::Ok;
}

/**
 * @brief   Write filtered humidity data to RTE buffer.
 * @param   value   Filtered humidity in 0.01 %RH
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::writeFilteredHumidity(uint32_t value)
{
    rteFilteredHum_ = value;

    return Status::Ok;
}

/* ---- Diagnostic write ---- */

/**
 * @brief   Write temperature diagnostic flag to RTE buffer.
 * @param   diag   Temperature diagnostic status
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::writeTempDiag(uint8_t diag)
{
    rteTempDiag_ = diag;

    return Status::Ok;
}

/**
 * @brief   Write pressure diagnostic flag to RTE buffer.
 * @param   diag   Pressure diagnostic status
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::writePressDiag(uint8_t diag)
{
    rtePressDiag_ = diag;

    return Status::Ok;
}

/**
 * @brief   Write humidity diagnostic flag to RTE buffer.
 * @param   diag   Humidity diagnostic status
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::writeHumDiag(uint8_t diag)
{
    rteHumDiag_ = diag;

    return Status::Ok;
}

/* ---- Filtered data read ---- */

/**
 * @brief   Read filtered temperature data from RTE buffer.
 * @param   pValue  Pointer to store filtered temperature in 0.01 degree C
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::readFilteredTemperature(int32_t *pValue)
{
    Status retVal = Status::Ok;

    if (pValue == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pValue = rteFilteredTemp_;
    }

    return retVal;
}

/**
 * @brief   Read filtered pressure data from RTE buffer.
 * @param   pValue  Pointer to store filtered pressure in Pa
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::readFilteredPressure(uint32_t *pValue)
{
    Status retVal = Status::Ok;

    if (pValue == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pValue = rteFilteredPress_;
    }

    return retVal;
}

/**
 * @brief   Read filtered humidity data from RTE buffer.
 * @param   pValue  Pointer to store filtered humidity in 0.01 %RH
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::readFilteredHumidity(uint32_t *pValue)
{
    Status retVal = Status::Ok;

    if (pValue == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pValue = rteFilteredHum_;
    }

    return retVal;
}

/* ---- Diagnostic read ---- */

/**
 * @brief   Read temperature diagnostic flag from RTE buffer.
 * @param   pDiag  Pointer to store temperature diagnostic status
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::readTempDiag(uint8_t *pDiag)
{
    Status retVal = Status::Ok;

    if (pDiag == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pDiag = rteTempDiag_;
    }

    return retVal;
}

/**
 * @brief   Read pressure diagnostic flag from RTE buffer.
 * @param   pDiag  Pointer to store pressure diagnostic status
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::readPressDiag(uint8_t *pDiag)
{
    Status retVal = Status::Ok;

    if (pDiag == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pDiag = rtePressDiag_;
    }

    return retVal;
}

/**
 * @brief   Read humidity diagnostic flag from RTE buffer.
 * @param   pDiag  Pointer to store humidity diagnostic status
 * @retval  RteAppSensor::Status
 */
RteAppSensor::Status RteAppSensor::readHumDiag(uint8_t *pDiag)
{
    Status retVal = Status::Ok;

    if (pDiag == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pDiag = rteHumDiag_;
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/
