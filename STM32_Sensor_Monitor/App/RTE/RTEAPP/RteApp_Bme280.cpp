/*
************************************************************************************************************************
*
* Filename        : RteApp_Bme280.cpp
* Project         : STM32_Sensor_Monitor
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
#include "RteApp_Bme280.hpp"
/*
************************************************************************************************************************
*                                                 Static member definitions
************************************************************************************************************************
*/

int32_t  RteAppBme280::rteTemperature_ = 0;
uint32_t RteAppBme280::rtePressure_    = 0U;
uint32_t RteAppBme280::rteHumidity_    = 0U;
uint8_t  RteAppBme280::rteCommStatus_  = 0U;
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Write temperature data to RTE buffer.
 * @param   value   Temperature in 0.01 degree C
 * @retval  RteAppBme280::Status
 */
RteAppBme280::Status RteAppBme280::writeTemperature(int32_t value)
{
    rteTemperature_ = value;

    return Status::Ok;
}

/**
 * @brief   Write pressure data to RTE buffer.
 * @param   value   Pressure in Pa
 * @retval  RteAppBme280::Status
 */
RteAppBme280::Status RteAppBme280::writePressure(uint32_t value)
{
    rtePressure_ = value;

    return Status::Ok;
}

/**
 * @brief   Write humidity data to RTE buffer.
 * @param   value   Humidity in 0.01 %RH
 * @retval  RteAppBme280::Status
 */
RteAppBme280::Status RteAppBme280::writeHumidity(uint32_t value)
{
    rteHumidity_ = value;

    return Status::Ok;
}

/**
 * @brief   Read temperature data from RTE buffer.
 * @param   pValue  Pointer to store temperature in 0.01 degree C
 * @retval  RteAppBme280::Status
 */
RteAppBme280::Status RteAppBme280::readTemperature(int32_t *pValue)
{
    Status retVal = Status::Ok;

    if (pValue == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pValue = rteTemperature_;
    }

    return retVal;
}

/**
 * @brief   Read pressure data from RTE buffer.
 * @param   pValue  Pointer to store pressure in Pa
 * @retval  RteAppBme280::Status
 */
RteAppBme280::Status RteAppBme280::readPressure(uint32_t *pValue)
{
    Status retVal = Status::Ok;

    if (pValue == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pValue = rtePressure_;
    }

    return retVal;
}

/**
 * @brief   Read humidity data from RTE buffer.
 * @param   pValue  Pointer to store humidity in 0.01 %RH
 * @retval  RteAppBme280::Status
 */
RteAppBme280::Status RteAppBme280::readHumidity(uint32_t *pValue)
{
    Status retVal = Status::Ok;

    if (pValue == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pValue = rteHumidity_;
    }

    return retVal;
}

/**
 * @brief   Write BME280 communication status to RTE buffer.
 * @param   status   0: OK, 1: Error
 * @retval  RteAppBme280::Status
 */
RteAppBme280::Status RteAppBme280::writeCommStatus(uint8_t status)
{
    rteCommStatus_ = status;

    return Status::Ok;
}

/**
 * @brief   Read BME280 communication status from RTE buffer.
 * @param   pStatus  Pointer to store communication status
 * @retval  RteAppBme280::Status
 */
RteAppBme280::Status RteAppBme280::readCommStatus(uint8_t *pStatus)
{
    Status retVal = Status::Ok;

    if (pStatus == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pStatus = rteCommStatus_;
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/
