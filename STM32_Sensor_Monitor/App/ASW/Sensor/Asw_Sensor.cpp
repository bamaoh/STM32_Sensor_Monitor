/*
************************************************************************************************************************
*
* Filename        : Asw_Sensor.cpp
* Project         : STM32_Sensor_Monitor
* Description     : ASW Sensor Module Implementation.
*                   Performs range validation and rate-of-change filtering
*                   on BME280 measurement data read from RTE.
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
#include "Asw_Sensor.hpp"
#include "RteApp_Bme280.hpp"
#include "RteApp_Sensor.hpp"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    /* Valid range: BME280 datasheet Section 1, Table 1 (Operating range) */
    constexpr int32_t  TEMP_MIN       = -4000;      /*!< -40.00 degree C in 0.01 unit     */
    constexpr int32_t  TEMP_MAX       = 8500;       /*!< 85.00 degree C in 0.01 unit      */
    constexpr uint32_t PRESS_MIN      = 30000U;     /*!< 300.00 hPa in Pa                 */
    constexpr uint32_t PRESS_MAX      = 110000U;    /*!< 1100.00 hPa in Pa                */
    constexpr uint32_t HUM_MAX        = 10000U;     /*!< 100.00 %RH in 0.01 unit          */

    /* Rate-of-change limit per cycle (1 second) */
    constexpr int32_t  TEMP_RATE_MAX  = 100;        /*!< Max +-1.00 degree C / sec         */
    constexpr uint32_t PRESS_RATE_MAX = 1000U;      /*!< Max +-10.00 hPa / sec            */
    constexpr uint32_t HUM_RATE_MAX   = 500U;       /*!< Max +-5.00 %RH / sec             */
}
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Periodic sensor data processing.
 *          Reads from RTE, validates range, applies rate-of-change filter.
 * @retval  None
 */
void AswSensor::mainFunction(void)
{
    int32_t  tempValue;
    uint32_t pressValue;
    uint32_t humValue;

    /* Read measurement data from RTE */
    if (RteAppBme280::readTemperature(&tempValue) == RteAppBme280::Status::Ok)
    {
        processTemp(tempValue);
    }
    else
    {
        diagStatus_.tempDiag = Diag::CommError;
    }

    if (RteAppBme280::readPressure(&pressValue) == RteAppBme280::Status::Ok)
    {
        processPress(pressValue);
    }
    else
    {
        diagStatus_.pressDiag = Diag::CommError;
    }

    if (RteAppBme280::readHumidity(&humValue) == RteAppBme280::Status::Ok)
    {
        processHum(humValue);
    }
    else
    {
        diagStatus_.humDiag = Diag::CommError;
    }

    /* Count down stabilization cycles */
    if (stabilizeCount_ > 0U)
    {
        stabilizeCount_--;
    }

    /* Write filtered data to RTE */
    (void)RteAppSensor::writeFilteredTemperature(sensorData_.temperature);
    (void)RteAppSensor::writeFilteredPressure(sensorData_.pressure);
    (void)RteAppSensor::writeFilteredHumidity(sensorData_.humidity);

    /* Write diagnostic flags to RTE */
    (void)RteAppSensor::writeTempDiag(static_cast<uint8_t>(diagStatus_.tempDiag));
    (void)RteAppSensor::writePressDiag(static_cast<uint8_t>(diagStatus_.pressDiag));
    (void)RteAppSensor::writeHumDiag(static_cast<uint8_t>(diagStatus_.humDiag));
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/

/**
 * @brief   Validate temperature range and apply rate-of-change filter.
 * @param   rawValue   Raw temperature from RTE (0.01 degree C)
 * @retval  None
 */
void AswSensor::processTemp(int32_t rawValue)
{
    int32_t diff;

    /* Range check */
    if ((rawValue < TEMP_MIN) || (rawValue > TEMP_MAX))
    {
        diagStatus_.tempDiag = Diag::RangeError;
    }
    else if (stabilizeCount_ > 0U)
    {
        /* First cycle: accept value without rate check */
        sensorData_.temperature = rawValue;
        diagStatus_.tempDiag = Diag::Ok;
    }
    else
    {
        /* Rate-of-change check */
        diff = rawValue - sensorData_.temperature;

        if (diff > TEMP_RATE_MAX)
        {
            sensorData_.temperature += TEMP_RATE_MAX;
            diagStatus_.tempDiag = Diag::RateError;
        }
        else if (diff < -TEMP_RATE_MAX)
        {
            sensorData_.temperature -= TEMP_RATE_MAX;
            diagStatus_.tempDiag = Diag::RateError;
        }
        else
        {
            sensorData_.temperature = rawValue;
            diagStatus_.tempDiag = Diag::Ok;
        }
    }
}

/**
 * @brief   Validate pressure range and apply rate-of-change filter.
 * @param   rawValue   Raw pressure from RTE (Pa)
 * @retval  None
 */
void AswSensor::processPress(uint32_t rawValue)
{
    int32_t diff;

    /* Range check */
    if ((rawValue < PRESS_MIN) || (rawValue > PRESS_MAX))
    {
        diagStatus_.pressDiag = Diag::RangeError;
    }
    else if (stabilizeCount_ > 0U)
    {
        /* First cycle: accept value without rate check */
        sensorData_.pressure = rawValue;
        diagStatus_.pressDiag = Diag::Ok;
    }
    else
    {
        /* Rate-of-change check */
        diff = static_cast<int32_t>(rawValue) - static_cast<int32_t>(sensorData_.pressure);

        if (diff > static_cast<int32_t>(PRESS_RATE_MAX))
        {
            sensorData_.pressure += PRESS_RATE_MAX;
            diagStatus_.pressDiag = Diag::RateError;
        }
        else if (diff < -static_cast<int32_t>(PRESS_RATE_MAX))
        {
            sensorData_.pressure -= PRESS_RATE_MAX;
            diagStatus_.pressDiag = Diag::RateError;
        }
        else
        {
            sensorData_.pressure = rawValue;
            diagStatus_.pressDiag = Diag::Ok;
        }
    }
}

/**
 * @brief   Validate humidity range and apply rate-of-change filter.
 * @param   rawValue   Raw humidity from RTE (0.01 %RH)
 * @retval  None
 */
void AswSensor::processHum(uint32_t rawValue)
{
    int32_t diff;

    /* Range check */
    if (rawValue > HUM_MAX)
    {
        diagStatus_.humDiag = Diag::RangeError;
    }
    else if (stabilizeCount_ > 0U)
    {
        /* First cycle: accept value without rate check */
        sensorData_.humidity = rawValue;
        diagStatus_.humDiag = Diag::Ok;
    }
    else
    {
        /* Rate-of-change check */
        diff = static_cast<int32_t>(rawValue) - static_cast<int32_t>(sensorData_.humidity);

        if (diff > static_cast<int32_t>(HUM_RATE_MAX))
        {
            sensorData_.humidity += HUM_RATE_MAX;
            diagStatus_.humDiag = Diag::RateError;
        }
        else if (diff < -static_cast<int32_t>(HUM_RATE_MAX))
        {
            sensorData_.humidity -= HUM_RATE_MAX;
            diagStatus_.humDiag = Diag::RateError;
        }
        else
        {
            sensorData_.humidity = rawValue;
            diagStatus_.humDiag = Diag::Ok;
        }
    }
}
