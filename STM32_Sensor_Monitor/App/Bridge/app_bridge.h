/*
************************************************************************************************************************
*
* Filename        : app_bridge.h
* Project         : STM32_Sensor_Monitor
* Description     : C/C++ Bridge Interface with FreeRTOS Multi-Task Support.
*                   Provides C-callable task entry functions for main.c.
*                   Three tasks with priority-based scheduling:
*                     SensorTask (High)    - BME280 read + data filtering
*                     DiagTask   (AboveN)  - Fault diagnosis with debouncing
*                     ManageTask (Normal)  - LED/NVM/UART output control
*                   Uses Mutex for RTE buffer protection and Event Flags
*                   for inter-task synchronization.
* Version         : 0.1.0
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef APP_BRIDGE_H
#define APP_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Create RTOS synchronization objects (Mutex, Event Flags).
 *          Must be called after osKernelInitialize() and before osKernelStart().
 * @retval  None
 */
void App_Init(void);

/**
 * @brief   Sensor task entry function (osPriorityHigh).
 *          Initializes BME280 sensor, then periodically reads measurement
 *          data and applies range validation / rate-of-change filter.
 *          Signals DiagTask when sensor processing is complete.
 * @param   argument   Not used
 * @retval  None (infinite loop)
 */
void App_SensorTask(void *argument);

/**
 * @brief   Diagnostic task entry function (osPriorityAboveNormal).
 *          Waits for sensor data ready signal, then evaluates fault
 *          conditions with debouncing state machine.
 *          Signals ManageTask when diagnosis is complete.
 * @param   argument   Not used
 * @retval  None (infinite loop)
 */
void App_DiagTask(void *argument);

/**
 * @brief   Manage task entry function (osPriorityNormal).
 *          Initializes NVM/UART, then waits for diagnosis complete signal.
 *          Controls status LED, updates NVM, and outputs UART data.
 * @param   argument   Not used
 * @retval  None (infinite loop)
 */
void App_ManageTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* APP_BRIDGE_H */
