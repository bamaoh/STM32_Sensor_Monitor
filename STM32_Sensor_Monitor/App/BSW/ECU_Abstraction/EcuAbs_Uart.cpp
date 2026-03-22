/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Uart.cpp
* Project         : STM32_Sensor_Monitor
* Description     : ECU Abstraction Layer - UART Interface Implementation.
*                   Wraps STM32 HAL UART polling transmit into a hardware-independent
*                   interface. Upper layers use this module without direct HAL dependency.
*                   Uses USART2 connected to ST-Link Virtual COM Port.
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
#include "EcuAbs_Uart.hpp"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    constexpr uint32_t TIMEOUT_MS = 100U;       /*!< Timeout duration in milliseconds  */
}
/*
************************************************************************************************************************
*                                                   External variables
************************************************************************************************************************
*/
extern "C"
{
    extern UART_HandleTypeDef huart2;           /*!< Defined in Core/Src/usart.c       */
}
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Initializes the UART abstraction layer and assigns
 *          the HAL UART handle for subsequent operations.
 * @retval  EcuAbs UART status
 */
EcuAbsUart::Status EcuAbsUart::init(void)
{
    Status retVal = Status::Ok;

    pHandle_ = &huart2;

    /* Verify that HAL UART has been initialized by CubeMX */
    if (pHandle_->gState == HAL_UART_STATE_RESET)
    {
        pHandle_ = nullptr;
        retVal = Status::Error;
    }

    return retVal;
}

/**
 * @brief   Transmit data in blocking mode (polling).
 * @param   pData   Pointer to data buffer
 * @param   length  Number of bytes to transmit
 * @retval  EcuAbs UART status
 */
EcuAbsUart::Status EcuAbsUart::transmit(const uint8_t *pData, uint16_t length)
{
    Status retVal = Status::Param;
    HAL_StatusTypeDef halStatus;

    if ((pData != nullptr) && (length > 0U) && (pHandle_ != nullptr))
    {
        /* MISRA deviation: HAL API requires non-const pointer */
        halStatus = HAL_UART_Transmit(pHandle_,
                                      const_cast<uint8_t*>(pData),
                                      length,
                                      TIMEOUT_MS);

        retVal = convertStatus(halStatus);
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/

/**
 * @brief   Converts HAL_StatusTypeDef to EcuAbsUart::Status.
 * @param   halStatus HAL return status
 * @retval  EcuAbs UART status
 */
EcuAbsUart::Status EcuAbsUart::convertStatus(HAL_StatusTypeDef halStatus)
{
    Status retVal;

    switch (halStatus)
    {
    case HAL_OK:
        retVal = Status::Ok;
        break;

    case HAL_BUSY:
        retVal = Status::Busy;
        break;

    case HAL_TIMEOUT:
        retVal = Status::Timeout;
        break;

    case HAL_ERROR:
        retVal = Status::Error;
        break;

    default:
        retVal = Status::Error;
        break;
    }

    return retVal;
}
