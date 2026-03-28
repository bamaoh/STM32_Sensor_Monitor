/*
************************************************************************************************************************
*
* Filename        : Asw_Diag.cpp
* Project         : STM32_Sensor_Monitor
* Description     : ASW Diagnostic Module Implementation.
*                   Evaluates sensor communication, data quality, and
*                   environment thresholds using debouncing state machine.
*                   Each diagnostic item transitions through:
*                   NORMAL -> FAULT_DEB -> FAULT -> NORMAL_DEB -> NORMAL
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
#include "Asw_Diag.hpp"
#include "RteApp_Bme280.hpp"
#include "RteApp_Sensor.hpp"
#include "RteApp_Diag.hpp"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    /* Debouncing threshold (3 cycles = 3 seconds at 1 sec period) */
    constexpr uint8_t DEB_THRESHOLD      = 3U;

    /* Sensor diagnostic flag values (matches AswSensor::Diag) */
    constexpr uint8_t FLAG_OK            = 0x00U;
    constexpr uint8_t FLAG_RANGE_ERROR   = 0x01U;
    constexpr uint8_t FLAG_RATE_ERROR    = 0x02U;

    /* Environment thresholds */
    constexpr int32_t  TEMP_HIGH         = 5000;       /*!< Over temperature: 50.00 degree C     */
    constexpr uint32_t HUM_HIGH          = 8000U;      /*!< Over humidity: 80.00 %RH             */
    constexpr uint32_t PRESS_LOW         = 95000U;     /*!< Low pressure: 950.00 hPa             */
    constexpr uint32_t PRESS_HIGH        = 106000U;    /*!< High pressure: 1060.00 hPa           */

    /* Diagnostic result values */
    constexpr uint8_t NO_FAULT           = 0U;
    constexpr uint8_t FAULT              = 1U;
    constexpr uint8_t WARNING            = 2U;
}
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Periodic diagnostic processing.
 * @retval  None
 */
void AswDiag::mainFunction(void)
{
    uint8_t commStatus  = 0U;
    uint8_t tempDiag    = FLAG_OK;
    uint8_t pressDiag   = FLAG_OK;
    uint8_t humDiag     = FLAG_OK;
    int32_t  tempValue  = 0;
    uint32_t pressValue = 0U;
    uint32_t humValue   = 0U;
    uint8_t commResult  = NO_FAULT;
    uint8_t dataResult  = NO_FAULT;
    uint8_t envResult   = NO_FAULT;

    /* Read communication status from Service RTE */
    (void)RteAppBme280::readCommStatus(&commStatus);

    /* Read diagnostic flags from Sensor RTE */
    (void)RteAppSensor::readTempDiag(&tempDiag);
    (void)RteAppSensor::readPressDiag(&pressDiag);
    (void)RteAppSensor::readHumDiag(&humDiag);

    /* Read filtered sensor data from RTE */
    (void)RteAppSensor::readFilteredTemperature(&tempValue);
    (void)RteAppSensor::readFilteredPressure(&pressValue);
    (void)RteAppSensor::readFilteredHumidity(&humValue);

    /* Process each diagnostic item */
    processCommFault(commStatus);
    processDataFault(tempDiag, pressDiag, humDiag);
    processEnvWarning(tempValue, pressValue, humValue);

    /* Evaluate diagnostic results */
    commResult = (commDiag_.state == State::Fault) ? FAULT : NO_FAULT;
    dataResult = (dataDiag_.state == State::Fault) ? FAULT : NO_FAULT;
    envResult  = (envDiag_.state == State::Warning) ? WARNING : NO_FAULT;

    /* Write diagnostic results to RTE */
    (void)RteAppDiag::writeCommFault(commResult);
    (void)RteAppDiag::writeDataFault(dataResult);
    (void)RteAppDiag::writeEnvWarning(envResult);
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/

/**
 * @brief   Process sensor communication fault diagnosis with debouncing.
 * @param   commStatus  Communication status from Service (0: OK, 1: Error)
 * @retval  None
 */
void AswDiag::processCommFault(uint8_t commStatus)
{
    uint8_t faultDetected;

    /* Communication fault if Service reports error */
    faultDetected = (commStatus != 0U) ? 1U : 0U;

    switch (commDiag_.state)
    {
        case State::Normal:
            if (faultDetected == 1U)
            {
                commDiag_.state = State::FaultDeb;
                commDiag_.debCounter = 1U;
            }
            break;

        case State::FaultDeb:
            if (faultDetected == 1U)
            {
                commDiag_.debCounter++;

                if (commDiag_.debCounter >= DEB_THRESHOLD)
                {
                    commDiag_.state = State::Fault;
                    commDiag_.debCounter = 0U;
                }
            }
            else
            {
                commDiag_.state = State::Normal;
                commDiag_.debCounter = 0U;
            }
            break;

        case State::Fault:
            if (faultDetected == 0U)
            {
                commDiag_.state = State::NormalDeb;
                commDiag_.debCounter = 1U;
            }
            break;

        case State::NormalDeb:
            if (faultDetected == 0U)
            {
                commDiag_.debCounter++;

                if (commDiag_.debCounter >= DEB_THRESHOLD)
                {
                    commDiag_.state = State::Normal;
                    commDiag_.debCounter = 0U;
                }
            }
            else
            {
                commDiag_.state = State::Fault;
                commDiag_.debCounter = 0U;
            }
            break;

        default:
            commDiag_.state = State::Normal;
            commDiag_.debCounter = 0U;
            break;
    }
}

/**
 * @brief   Process sensor data fault diagnosis with debouncing.
 * @param   tempDiag    Temperature diagnostic flag from RTE
 * @param   pressDiag   Pressure diagnostic flag from RTE
 * @param   humDiag     Humidity diagnostic flag from RTE
 * @retval  None
 */
void AswDiag::processDataFault(uint8_t tempDiag, uint8_t pressDiag, uint8_t humDiag)
{
    uint8_t faultDetected;

    /* Data fault if any channel has RANGE_ERROR or RATE_ERROR */
    faultDetected = ((tempDiag == FLAG_RANGE_ERROR)
                  || (tempDiag == FLAG_RATE_ERROR)
                  || (pressDiag == FLAG_RANGE_ERROR)
                  || (pressDiag == FLAG_RATE_ERROR)
                  || (humDiag == FLAG_RANGE_ERROR)
                  || (humDiag == FLAG_RATE_ERROR)) ? 1U : 0U;

    switch (dataDiag_.state)
    {
        case State::Normal:
            if (faultDetected == 1U)
            {
                dataDiag_.state = State::FaultDeb;
                dataDiag_.debCounter = 1U;
            }
            break;

        case State::FaultDeb:
            if (faultDetected == 1U)
            {
                dataDiag_.debCounter++;

                if (dataDiag_.debCounter >= DEB_THRESHOLD)
                {
                    dataDiag_.state = State::Fault;
                    dataDiag_.debCounter = 0U;
                }
            }
            else
            {
                dataDiag_.state = State::Normal;
                dataDiag_.debCounter = 0U;
            }
            break;

        case State::Fault:
            if (faultDetected == 0U)
            {
                dataDiag_.state = State::NormalDeb;
                dataDiag_.debCounter = 1U;
            }
            break;

        case State::NormalDeb:
            if (faultDetected == 0U)
            {
                dataDiag_.debCounter++;

                if (dataDiag_.debCounter >= DEB_THRESHOLD)
                {
                    dataDiag_.state = State::Normal;
                    dataDiag_.debCounter = 0U;
                }
            }
            else
            {
                dataDiag_.state = State::Fault;
                dataDiag_.debCounter = 0U;
            }
            break;

        default:
            dataDiag_.state = State::Normal;
            dataDiag_.debCounter = 0U;
            break;
    }
}

/**
 * @brief   Process environment threshold warning with debouncing.
 * @param   temp    Filtered temperature (0.01 degree C)
 * @param   press   Filtered pressure (Pa)
 * @param   hum     Filtered humidity (0.01 %RH)
 * @retval  None
 */
void AswDiag::processEnvWarning(int32_t temp, uint32_t press, uint32_t hum)
{
    uint8_t warningDetected;

    /* Environment warning if any threshold exceeded */
    warningDetected = ((temp > TEMP_HIGH)
                    || (hum > HUM_HIGH)
                    || (press < PRESS_LOW)
                    || (press > PRESS_HIGH)) ? 1U : 0U;

    switch (envDiag_.state)
    {
        case State::Normal:
            if (warningDetected == 1U)
            {
                envDiag_.state = State::WarningDeb;
                envDiag_.debCounter = 1U;
            }
            break;

        case State::WarningDeb:
            if (warningDetected == 1U)
            {
                envDiag_.debCounter++;

                if (envDiag_.debCounter >= DEB_THRESHOLD)
                {
                    envDiag_.state = State::Warning;
                    envDiag_.debCounter = 0U;
                }
            }
            else
            {
                envDiag_.state = State::Normal;
                envDiag_.debCounter = 0U;
            }
            break;

        case State::Warning:
            if (warningDetected == 0U)
            {
                envDiag_.state = State::NormalDeb;
                envDiag_.debCounter = 1U;
            }
            break;

        case State::NormalDeb:
            if (warningDetected == 0U)
            {
                envDiag_.debCounter++;

                if (envDiag_.debCounter >= DEB_THRESHOLD)
                {
                    envDiag_.state = State::Normal;
                    envDiag_.debCounter = 0U;
                }
            }
            else
            {
                envDiag_.state = State::Warning;
                envDiag_.debCounter = 0U;
            }
            break;

        default:
            envDiag_.state = State::Normal;
            envDiag_.debCounter = 0U;
            break;
    }
}
