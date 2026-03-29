/*
************************************************************************************************************************
*
* Filename        : app_bridge.cpp
* Project         : STM32_Sensor_Monitor
* Description     : C/C++ Bridge Implementation with FreeRTOS Multi-Task Support.
*                   Creates C++ application objects and RTOS synchronization primitives.
*
*                   Task execution flow (every 1000ms cycle):
*                     SensorTask ──[EVT_SENSOR_DONE]──> DiagTask ──[EVT_DIAG_DONE]──> ManageTask
*
*                   RTE buffer access is protected by rteMutex to prevent
*                   race conditions between tasks during cycle boundaries.
*
* Version         : 0.1.0
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

extern "C"
{
    #include "cmsis_os2.h"
}

#include "Svc_Bme280.hpp"
#include "Asw_Sensor.hpp"
#include "Asw_Diag.hpp"
#include "Asw_Manage.hpp"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    /** @brief Event flag bit definitions for inter-task synchronization */
    constexpr uint32_t EVT_SENSOR_DONE = 0x01U;   /*!< SensorTask processing complete */
    constexpr uint32_t EVT_DIAG_DONE   = 0x02U;   /*!< DiagTask processing complete   */

    /** @brief Task cycle period in milliseconds */
    constexpr uint32_t TASK_PERIOD_MS = 1000U;
}
/*
************************************************************************************************************************
*                                                   Application objects
************************************************************************************************************************
*/
namespace
{
    SvcBme280           svcBme280;                  /*!< BME280 sensor service              */
    AswSensor           aswSensor;                  /*!< Sensor data processing             */
    AswDiag             aswDiag;                    /*!< Diagnostic state machine           */
    AswManage           aswManage;                  /*!< System management (LED/NVM/UART)   */
    SvcBme280::MeasData sensorData = {};            /*!< Measurement data buffer            */
}
/*
************************************************************************************************************************
*                                                  RTOS synchronization objects
************************************************************************************************************************
*/
namespace
{
    osMutexId_t      rteMutex = nullptr;            /*!< Mutex for RTE buffer protection    */
    osEventFlagsId_t taskEvent = nullptr;           /*!< Event flags for task synchronization */
}
/*
************************************************************************************************************************
*                                                  Bridge functions (C-callable)
************************************************************************************************************************
*/

/**
 * @brief   Create RTOS synchronization objects.
 *          - rteMutex: Protects RTE shared buffers from concurrent access
 *          - taskEvent: Coordinates Sensor -> Diag -> Manage execution order
 * @retval  None
 */
extern "C" void App_Init(void)
{
    rteMutex  = osMutexNew(nullptr);
    taskEvent = osEventFlagsNew(nullptr);
}

/**
 * @brief   Sensor task entry function (osPriorityHigh).
 *          1. Init: BME280 sensor initialization (I2C, chip ID, config)
 *          2. Loop: Read measurement -> Filter -> Signal DiagTask
 *
 *          Mutex protects RTE write during readMeasurement() and
 *          aswSensor.mainFunction() which write to RTE buffers.
 * @param   argument   Not used
 * @retval  None (infinite loop)
 */
extern "C" void App_SensorTask(void *argument)
{
    (void)argument;

    /* Service initialization */
    (void)svcBme280.init();

    /* Periodic sensor processing */
    for (;;)
    {
        osMutexAcquire(rteMutex, osWaitForever);
        (void)svcBme280.readMeasurement(&sensorData);
        aswSensor.mainFunction();
        osMutexRelease(rteMutex);

        /* Signal DiagTask: sensor data is ready */
        osEventFlagsSet(taskEvent, EVT_SENSOR_DONE);

        osDelay(TASK_PERIOD_MS);
    }
}

/**
 * @brief   Diagnostic task entry function (osPriorityAboveNormal).
 *          Waits for EVT_SENSOR_DONE before processing.
 *          Reads diagnostic flags and sensor data from RTE,
 *          evaluates fault conditions, writes results to RTE.
 * @param   argument   Not used
 * @retval  None (infinite loop)
 */
extern "C" void App_DiagTask(void *argument)
{
    (void)argument;

    /* Periodic diagnostic processing */
    for (;;)
    {
        /* Wait for sensor data ready */
        osEventFlagsWait(taskEvent, EVT_SENSOR_DONE,
                         osFlagsWaitAny, osWaitForever);

        osMutexAcquire(rteMutex, osWaitForever);
        aswDiag.mainFunction();
        osMutexRelease(rteMutex);

        /* Signal ManageTask: diagnosis is complete */
        osEventFlagsSet(taskEvent, EVT_DIAG_DONE);
    }
}

/**
 * @brief   Manage task entry function (osPriorityNormal).
 *          Waits for EVT_DIAG_DONE before processing.
 *          Controls status LED, updates NVM on transitions,
 *          and outputs sensor/diagnostic data via UART.
 * @param   argument   Not used
 * @retval  None (infinite loop)
 */
extern "C" void App_ManageTask(void *argument)
{
    (void)argument;

    /* Service initialization (NVM, UART) */
    aswManage.init();

    /* Periodic manage processing */
    for (;;)
    {
        /* Wait for diagnosis complete */
        osEventFlagsWait(taskEvent, EVT_DIAG_DONE,
                         osFlagsWaitAny, osWaitForever);

        osMutexAcquire(rteMutex, osWaitForever);
        aswManage.mainFunction();
        osMutexRelease(rteMutex);
    }
}
