/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Uart.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
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
#include "EcuAbs_Uart.h"
#include "stm32f4xx_hal.h"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

#define ECUABS_UART_TIMEOUT_MS  (100U)    /*!< Timeout duration in milliseconds  */
/*
************************************************************************************************************************
*                                                         Typedefs
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                                    Private variables
************************************************************************************************************************
*/

extern UART_HandleTypeDef huart2;         /*!< Defined in Core/Src/usart.c       */

static UART_HandleTypeDef *pUartHandle = NULL;    /*!< Pointer to UART handle    */
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/

static EcuAbs_Uart_StatusType EcuAbs_Uart_ConvertStatus(HAL_StatusTypeDef halStatus);
/*
************************************************************************************************************************
*                                                    Public functions
************************************************************************************************************************
*/

/**
 * @brief   Initializes the UART abstraction layer and assigns
 *          the HAL UART handle for subsequent operations.
 * @retval  EcuAbs UART status
 */
EcuAbs_Uart_StatusType EcuAbs_Uart_Init(void)
{
    EcuAbs_Uart_StatusType retVal = ECUABS_UART_OK;

    pUartHandle = &huart2;

    /* Verify that HAL UART has been initialized by CubeMX */
    if (pUartHandle->gState == HAL_UART_STATE_RESET)
    {
        retVal = ECUABS_UART_ERROR;
    }

    return retVal;
}

/**
 * @brief   Transmit data in blocking mode (polling).
 * @param   pData   Pointer to data buffer
 * @param   length  Number of bytes to transmit
 * @retval  EcuAbs UART status
 */
EcuAbs_Uart_StatusType EcuAbs_Uart_Transmit(const uint8_t *pData, uint16_t length)
{
    EcuAbs_Uart_StatusType retVal = ECUABS_UART_PARAM;
    HAL_StatusTypeDef halStatus;

    if ((pData != NULL) && (length > 0U) && (pUartHandle != NULL))
    {
        /* MISRA deviation: HAL API requires non-const pointer */
        halStatus = HAL_UART_Transmit(pUartHandle,
                                      (uint8_t *)pData,
                                      length,
                                      ECUABS_UART_TIMEOUT_MS);

        retVal = EcuAbs_Uart_ConvertStatus(halStatus);
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/

/**
 * @brief   Converts HAL_StatusTypeDef to EcuAbs_Uart_StatusType.
 * @param   halStatus HAL return status
 * @retval  EcuAbs UART status
 */
static EcuAbs_Uart_StatusType EcuAbs_Uart_ConvertStatus(HAL_StatusTypeDef halStatus)
{
    EcuAbs_Uart_StatusType retVal;

    switch (halStatus)
    {
    case HAL_OK:
        retVal = ECUABS_UART_OK;
        break;

    case HAL_BUSY:
        retVal = ECUABS_UART_BUSY;
        break;

    case HAL_TIMEOUT:
        retVal = ECUABS_UART_TIMEOUT;
        break;

    case HAL_ERROR:
        retVal = ECUABS_UART_ERROR;
        break;

    default:
        retVal = ECUABS_UART_ERROR;
        break;
    }

    return retVal;
}
