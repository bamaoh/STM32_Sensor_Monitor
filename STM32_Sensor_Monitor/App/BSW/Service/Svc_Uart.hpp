/*
************************************************************************************************************************
*
* Filename        : Svc_Uart.hpp
* Project         : STM32_Sensor_Monitor
* Description     : UART Service Module Interface.
*                   Provides formatted string output for sensor data and
*                   diagnostic fault information via serial port.
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
#include "EcuAbs_Uart.hpp"
#include <cstdint>
/*
************************************************************************************************************************
*                                                    Class definition
************************************************************************************************************************
*/
/** @brief UART Service - formatted serial output class */
class SvcUart
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

    /** @brief SvcUart operation status definition */
    enum class Status : uint8_t
    {
        Ok    = 0x00U,   /*!< No error        */
        Error = 0x01U    /*!< General error    */
    };

/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/

    /**
     * @brief   Initialize UART service.
     * @retval  SvcUart::Status
     */
    Status init(void);

    /**
     * @brief   Transmit sensor data as formatted string with alive counter and quality.
     *          Normal: "[CNT:001 Q:VALID] T:25.31C P:101325Pa H:45.20%\r\n"
     *          Stale:  "[CNT:002 Q:STALE] T:25.31C P:101325Pa H:45.20%\r\n"
     * @param   temp    Filtered temperature in 0.01 degree C (e.g. 2531 = 25.31C)
     * @param   press   Filtered pressure in Pa
     * @param   hum     Filtered humidity in 0.01 %RH (e.g. 4520 = 45.20%)
     * @param   isStale 1U if any fault/warning is active (last valid value), 0U otherwise
     * @retval  SvcUart::Status
     */
    Status sendSensorData(int32_t temp, uint32_t press, uint32_t hum, uint8_t isStale);

    /**
     * @brief   Transmit diagnostic fault data as formatted string.
     *          Only called when at least one fault/warning is active.
     *          Output format: "[FAULT] COMM:1 DATA:0 ENV:0\r\n"
     * @param   commFault   Communication fault status (0 or 1)
     * @param   dataFault   Data fault status (0 or 1)
     * @param   envWarning  Environment warning status (0 or 2)
     * @retval  SvcUart::Status
     */
    Status sendDiagData(uint8_t commFault, uint8_t dataFault, uint8_t envWarning);

private:
/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

    static constexpr uint32_t TX_BUF_SIZE = 128U;   /*!< Transmit buffer size in bytes */

    EcuAbsUart uart_;                                /*!< UART driver instance          */
    char txBuffer_[TX_BUF_SIZE] = {};                /*!< Transmit formatting buffer    */
    uint8_t initialized_ = 0U;                       /*!< Init completed flag           */
    uint32_t aliveCounter_ = 0U;                     /*!< Message alive counter         */

/*
********************************************************************************************************************
*                                                  Private methods
********************************************************************************************************************
*/

    /**
     * @brief   Transmit a null-terminated string via EcuAbsUart.
     * @param   pStr    Pointer to null-terminated string
     * @retval  SvcUart::Status
     */
    Status sendString(const char *pStr);
};
