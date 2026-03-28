/*
************************************************************************************************************************
*
* Filename        : Asw_Manage.cpp
* Project         : STM32_Sensor_Monitor
* Description     : ASW Manage Module Implementation.
*                   Reads diagnostic results from RTE, controls status LED,
*                   manages NVM diagnostic data persistence, and outputs
*                   sensor/diagnostic data via UART serial port.
*                   NVM is updated only on fault/warning state transitions.
* Version         : 0.0.3
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                                  Include header files
************************************************************************************************************************
*/
#include "Asw_Manage.hpp"
#include "RteApp_Diag.hpp"
#include "RteApp_Sensor.hpp"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    /* Diagnostic result values (matches AswDiag output) */
    constexpr uint8_t NO_FAULT = 0U;
    constexpr uint8_t FAULT    = 1U;
    constexpr uint8_t WARNING  = 2U;
}
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Periodic manage processing.
 *          Reads diagnostic results from RTE, controls status LED,
 *          and updates NVM on state transitions.
 * @retval  None
 */
void AswManage::mainFunction(void)
{
    uint8_t commFault  = NO_FAULT;
    uint8_t dataFault  = NO_FAULT;
    uint8_t envWarning = NO_FAULT;

    /* Load NVM data on first call */
    if (nvmLoaded_ == 0U)
    {
        (void)nvm_.readDiagData(&nvmData_);
        nvmLoaded_ = 1U;
    }

    /* Read diagnostic results from RTE */
    (void)RteAppDiag::readCommFault(&commFault);
    (void)RteAppDiag::readDataFault(&dataFault);
    (void)RteAppDiag::readEnvWarning(&envWarning);

    /* LED control: FAULT > WARNING > NORMAL */
    if ((commFault == FAULT) || (dataFault == FAULT))
    {
        led_.setMode(SvcLed::Mode::Off);
    }
    else if (envWarning == WARNING)
    {
        led_.setMode(SvcLed::Mode::Blink);
    }
    else
    {
        led_.setMode(SvcLed::Mode::On);
    }

    /* Update NVM on state transitions */
    updateNvm(commFault, dataFault, envWarning);

    /* UART output: sensor data + fault data (if any) */
    outputUart(commFault, dataFault, envWarning);

    /* Track previous results */
    prevCommFault_  = commFault;
    prevDataFault_  = dataFault;
    prevEnvWarning_ = envWarning;
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/

/**
 * @brief   Update NVM on diagnostic state transition.
 *          First occurrence in cycle: store fault type with cycle marker + increment count.
 *          Repeated same fault in cycle: increment count only (fault type already stored).
 *          Writes to Flash only on NORMAL -> FAULT/WARNING transitions.
 * @param   commFault   Current CommFault result
 * @param   dataFault   Current DataFault result
 * @param   envWarning  Current EnvWarning result
 * @retval  None
 */
void AswManage::updateNvm(uint8_t commFault, uint8_t dataFault, uint8_t envWarning)
{
    uint8_t nvmChanged = 0U;

    /* CommFault: NORMAL -> FAULT transition */
    if ((commFault == FAULT) && (prevCommFault_ == NO_FAULT))
    {
        if (commSavedCycle_ == 0U)
        {
            /* First occurrence: store fault type with cycle marker */
            nvmData_.commFault = nvmData_.cycleMarker | commFault;
            commSavedCycle_ = 1U;
        }

        nvmData_.commFaultCount++;
        nvmChanged = 1U;
    }

    /* DataFault: NORMAL -> FAULT transition */
    if ((dataFault == FAULT) && (prevDataFault_ == NO_FAULT))
    {
        if (dataSavedCycle_ == 0U)
        {
            nvmData_.dataFault = nvmData_.cycleMarker | dataFault;
            dataSavedCycle_ = 1U;
        }

        nvmData_.dataFaultCount++;
        nvmChanged = 1U;
    }

    /* EnvWarning: NORMAL -> WARNING transition */
    if ((envWarning == WARNING) && (prevEnvWarning_ == NO_FAULT))
    {
        if (envSavedCycle_ == 0U)
        {
            nvmData_.envWarning = nvmData_.cycleMarker | envWarning;
            envSavedCycle_ = 1U;
        }

        nvmData_.envWarningCount++;
        nvmChanged = 1U;
    }

    /* Write to Flash only if something changed */
    if (nvmChanged == 1U)
    {
        (void)nvm_.writeDiagData(&nvmData_);
    }
}

/**
 * @brief   Output sensor and diagnostic data via UART.
 *          Always sends sensor data (temperature, pressure, humidity).
 *          Additionally sends fault data when at least one fault or warning is active.
 * @param   commFault   Current CommFault result
 * @param   dataFault   Current DataFault result
 * @param   envWarning  Current EnvWarning result
 * @retval  None
 */
void AswManage::outputUart(uint8_t commFault, uint8_t dataFault, uint8_t envWarning)
{
    int32_t  tempValue  = 0;
    uint32_t pressValue = 0U;
    uint32_t humValue   = 0U;
    uint8_t  isStale    = 0U;

    /* Read filtered sensor data from RTE */
    (void)RteAppSensor::readFilteredTemperature(&tempValue);
    (void)RteAppSensor::readFilteredPressure(&pressValue);
    (void)RteAppSensor::readFilteredHumidity(&humValue);

    /* Determine if sensor data is stale (any fault or warning active) */
    if ((commFault != NO_FAULT) ||
        (dataFault != NO_FAULT) ||
        (envWarning != NO_FAULT))
    {
        isStale = 1U;
    }

    /* Output sensor data with alive counter and quality indicator */
    (void)uart_.sendSensorData(tempValue, pressValue, humValue, isStale);

    /* Output fault data only when fault or warning is active */
    if (isStale == 1U)
    {
        (void)uart_.sendDiagData(commFault, dataFault, envWarning);
    }
}
