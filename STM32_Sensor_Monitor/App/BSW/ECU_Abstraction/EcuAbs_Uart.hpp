/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Uart.hpp
* Project         : STM32_Sensor_Monitor
* Description     : ECU Abstraction Layer - UART Interface.
*                   Provides hardware-independent UART communication interface.
*                   This layer wraps the HAL UART driver using polling mode.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#pragma once
/*
************************************************************************************************************************
*                                                  Include header files
************************************************************************************************************************
*/
extern "C"
{
    #include "stm32f4xx_hal.h"
}
#include <cstdint>
/*
************************************************************************************************************************
*                                                    Class definition
************************************************************************************************************************
*/

/** @brief ECU Abstraction Layer - UART communication class */
class EcuAbsUart
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

    /** @brief EcuAbs UART operation status definition */
    enum class Status : uint8_t
    {
        Ok      = 0x00U,   /*!< No error                          */
        Error   = 0x01U,   /*!< Communication error               */
        Busy    = 0x02U,   /*!< Peripheral is busy                */
        Timeout = 0x03U,   /*!< Timeout error                     */
        Param   = 0x04U    /*!< Invalid parameter                 */
    };

/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/

    /**
     * @brief   Initializes the UART abstraction layer and assigns
     *          the HAL UART handle for subsequent operations.
     * @retval  EcuAbsUart::Status
     */
    Status init(void);

    /**
     * @brief   Transmit data in blocking mode (polling).
     * @param   pData   Pointer to data buffer
     * @param   length  Number of bytes to transmit
     * @retval  EcuAbsUart::Status
     */
    Status transmit(const uint8_t *pData, uint16_t length);

private:
/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

    UART_HandleTypeDef *pHandle_ = nullptr;    /*!< Pointer to UART handle    */

/*
********************************************************************************************************************
*                                                  Private methods
********************************************************************************************************************
*/

    /**
     * @brief   Converts HAL_StatusTypeDef to EcuAbsUart::Status.
     * @param   halStatus HAL return status
     * @retval  EcuAbsUart::Status
     */
    Status convertStatus(HAL_StatusTypeDef halStatus);
};
