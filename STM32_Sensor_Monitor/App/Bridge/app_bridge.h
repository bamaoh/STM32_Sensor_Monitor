/*
************************************************************************************************************************
*
* Filename        : app_bridge.h
* Project         : STM32_Sensor_Monitor
* Description     : C/C++ Bridge Interface.
*                   Provides C-callable functions for main.c to access
*                   C++ application layer modules. This header is included
*                   by main.c (C file) and implemented in app_bridge.cpp (C++).
* Version         : 0.0.1
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
 * @brief   Initialize all application layer modules.
 *          Initializes BME280 sensor service and managed services (NVM, UART).
 *          Must be called once after HAL peripheral initialization.
 * @retval  None
 */
void App_Init(void);

/**
 * @brief   Execute one cycle of application processing.
 *          Reads sensor data, processes ASW modules (Sensor, Diag, Manage),
 *          and outputs results via LED and UART.
 *          Must be called periodically (e.g., every 1000ms in RTOS task).
 * @retval  None
 */
void App_MainFunction(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BRIDGE_H */
