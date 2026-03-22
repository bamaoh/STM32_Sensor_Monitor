/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Uart.h
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : ECU Abstraction Layer - UART Interface.
*                   Provides hardware-independent UART communication interface.
*                   This layer wraps the HAL UART driver using polling mode.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef ECUABS_UART_H
#define ECUABS_UART_H
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

/** @brief EcuAbs UART operation status definition */
typedef enum
{
    ECUABS_UART_OK       = 0x00U,   /*!< No error                          */
    ECUABS_UART_ERROR    = 0x01U,   /*!< Communication error               */
    ECUABS_UART_BUSY     = 0x02U,   /*!< Peripheral is busy                */
    ECUABS_UART_TIMEOUT  = 0x03U,   /*!< Timeout error                     */
    ECUABS_UART_PARAM    = 0x04U    /*!< Invalid parameter                 */
} EcuAbs_Uart_StatusType;
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
 * @brief   Initializes the UART abstraction layer and assigns
 *          the HAL UART handle for subsequent operations.
 * @retval  EcuAbs UART status
 */
EcuAbs_Uart_StatusType EcuAbs_Uart_Init(void);

/**
 * @brief   Transmit data in blocking mode (polling).
 * @param   pData   Pointer to data buffer
 * @param   length  Number of bytes to transmit
 * @retval  EcuAbs UART status
 */
EcuAbs_Uart_StatusType EcuAbs_Uart_Transmit(const uint8_t *pData, uint16_t length);

#endif /* ECUABS_UART_H */
