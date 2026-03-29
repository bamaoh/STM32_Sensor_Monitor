/*
************************************************************************************************************************
*
* Filename        : app_bridge.cpp
* Project         : STM32_Sensor_Monitor
* Description     : C/C++ Bridge Implementation.
*                   Creates C++ application objects and exposes C-callable
*                   functions for main.c integration. All C++ object creation,
*                   initialization, and method calls are encapsulated here.
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
#include "app_bridge.h"
#include "Svc_Bme280.hpp"
#include "Asw_Sensor.hpp"
#include "Asw_Diag.hpp"
#include "Asw_Manage.hpp"
/*
************************************************************************************************************************
*                                                   Application objects
************************************************************************************************************************
*/
namespace
{
    SvcBme280           svcBme280;                  /*!< BME280 sensor service       */
    AswSensor           aswSensor;                  /*!< Sensor data processing      */
    AswDiag             aswDiag;                    /*!< Diagnostic state machine     */
    AswManage           aswManage;                  /*!< System management (LED/NVM/UART) */
    SvcBme280::MeasData sensorData = {};            /*!< Measurement data buffer     */
}
/*
************************************************************************************************************************
*                                                  Bridge functions (C-callable)
************************************************************************************************************************
*/

/**
 * @brief   Initialize all application layer modules.
 *          - SvcBme280: I2C init, chip ID verify, sensor configuration
 *          - AswManage: NVM init (Flash read), UART init
 * @retval  None
 */
extern "C" void App_Init(void)
{
    (void)svcBme280.init();
    aswManage.init();
}

/**
 * @brief   Execute one cycle of application processing.
 *          Call order matches original main.c task loop:
 *          1. Svc_Bme280_ReadMeasurement  -> svcBme280.readMeasurement()
 *          2. Asw_Sensor_MainFunction     -> aswSensor.mainFunction()
 *          3. Asw_Diag_MainFunction       -> aswDiag.mainFunction()
 *          4. Asw_Manage_MainFunction     -> aswManage.mainFunction()
 *             (internally calls led_.mainFunction())
 * @retval  None
 */
extern "C" void App_MainFunction(void)
{
    (void)svcBme280.readMeasurement(&sensorData);
    aswSensor.mainFunction();
    aswDiag.mainFunction();
    aswManage.mainFunction();
}
