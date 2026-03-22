/*
************************************************************************************************************************
*
* Filename        : Svc_Uart.h
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : UART Service Module Interface.
*                   Provides formatted string output for sensor data and
*                   diagnostic fault information via serial port.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef SVC_UART_H
#define SVC_UART_H
/*
************************************************************************************************************************
*                                                  Include header files
************************************************************************************************************************
*/
#include <stdint.h>
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

/** @brief Svc_Uart operation status definition */
typedef enum
{
    SVC_UART_OK       = 0x00U,   /*!< No error        */
    SVC_UART_ERROR    = 0x01U    /*!< General error    */
} Svc_Uart_StatusType;
/*
************************************************************************************************************************
*                                                    Exported variables
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                               Exported function prototypes
************************************************************************************************************************
*/

/**
 * @brief   Initialize UART service.
 * @retval  Svc_Uart status
 */
Svc_Uart_StatusType Svc_Uart_Init(void);

/**
 * @brief   Transmit sensor data as formatted string with alive counter and quality.
 *          Normal: "[CNT:001 Q:VALID] T:25.31C P:101325Pa H:45.20%\r\n"
 *          Stale:  "[CNT:002 Q:STALE] T:25.31C P:101325Pa H:45.20%\r\n"
 * @param   temp    Filtered temperature in 0.01 degree C (e.g. 2531 = 25.31C)
 * @param   press   Filtered pressure in Pa
 * @param   hum     Filtered humidity in 0.01 %RH (e.g. 4520 = 45.20%)
 * @param   isStale 1U if any fault/warning is active (last valid value), 0U otherwise
 * @retval  Svc_Uart status
 */
Svc_Uart_StatusType Svc_Uart_SendSensorData(int32_t temp, uint32_t press, uint32_t hum, uint8_t isStale);

/**
 * @brief   Transmit diagnostic fault data as formatted string.
 *          Only called when at least one fault/warning is active.
 *          Output format: "[FAULT] COMM:1 DATA:0 ENV:0\r\n"
 * @param   commFault   Communication fault status (0 or 1)
 * @param   dataFault   Data fault status (0 or 1)
 * @param   envWarning  Environment warning status (0 or 2)
 * @retval  Svc_Uart status
 */
Svc_Uart_StatusType Svc_Uart_SendDiagData(uint8_t commFault, uint8_t dataFault, uint8_t envWarning);

#endif /* SVC_UART_H */
