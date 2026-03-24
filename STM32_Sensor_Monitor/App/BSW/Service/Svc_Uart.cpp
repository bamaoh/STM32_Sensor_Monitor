/*
************************************************************************************************************************
*
* Filename        : Svc_Uart.cpp
* Project         : STM32_Sensor_Monitor
* Description     : UART Service Module Implementation.
*                   Formats sensor data and diagnostic results into human-readable
*                   strings and transmits via EcuAbsUart (polling mode).
*                   Output is sent to ST-Link Virtual COM Port (USART2, 115200 8N1).
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
#include "Svc_Uart.hpp"
#include <cstdio>
#include <cstring>
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    /* Unit conversion */
    constexpr int32_t  TEMP_SCALE = 100;     /*!< Temperature scale (0.01 degree) */
    constexpr uint32_t HUM_SCALE  = 100U;    /*!< Humidity scale (0.01 %RH)       */
}
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Initialize UART service.
 * @retval  SvcUart::Status
 */
SvcUart::Status SvcUart::init(void)
{
    Status retVal = Status::Ok;
    EcuAbsUart::Status ecuAbsStatus;

    ecuAbsStatus = uart_.init();

    if (ecuAbsStatus != EcuAbsUart::Status::Ok)
    {
        retVal = Status::Error;
    }
    else
    {
        initialized_ = 1U;
    }

    return retVal;
}

/**
 * @brief   Transmit sensor data as formatted string with alive counter and quality.
 *          Normal: "[CNT:001 Q:VALID] T:25.31C P:101325Pa H:45.20%\r\n"
 *          Stale:  "[CNT:002 Q:STALE] T:25.31C P:101325Pa H:45.20%\r\n"
 * @param   temp    Filtered temperature in 0.01 degree C
 * @param   press   Filtered pressure in Pa
 * @param   hum     Filtered humidity in 0.01 %RH
 * @param   isStale 1U if any fault/warning is active, 0U otherwise
 * @retval  SvcUart::Status
 */
SvcUart::Status SvcUart::sendSensorData(int32_t temp, uint32_t press, uint32_t hum, uint8_t isStale)
{
    Status retVal = Status::Error;
    int32_t tempInt;
    int32_t tempFrac;
    uint32_t humInt;
    uint32_t humFrac;
    const char *qualityStr;
    int len;

    if (initialized_ == 1U)
    {
        /* Increment alive counter */
        aliveCounter_++;

        /* Split temperature into integer and fractional parts */
        tempInt  = temp / TEMP_SCALE;
        tempFrac = temp % TEMP_SCALE;

        /* Handle negative fractional part */
        if (tempFrac < 0)
        {
            tempFrac = -tempFrac;
        }

        /* Split humidity into integer and fractional parts */
        humInt  = hum / HUM_SCALE;
        humFrac = hum % HUM_SCALE;

        /* Select quality string based on data freshness */
        qualityStr = (isStale == 1U) ? "STALE" : "VALID";

        len = snprintf(txBuffer_, TX_BUF_SIZE,
                        "[CNT:%03lu Q:%s] T:%ld.%02ldC P:%luPa H:%lu.%02lu%%\r\n",
                        (unsigned long)(aliveCounter_ % 1000U),
                        qualityStr,
                        (long)tempInt, (long)tempFrac,
                        (unsigned long)press,
                        (unsigned long)humInt, (unsigned long)humFrac);

        if ((len > 0) && ((uint32_t)len < TX_BUF_SIZE))
        {
            retVal = sendString(txBuffer_);
        }
    }

    return retVal;
}

/**
 * @brief   Transmit diagnostic fault data as formatted string.
 *          Output format: "[FAULT] COMM:1 DATA:0 ENV:0\r\n"
 * @param   commFault   Communication fault status (0 or 1)
 * @param   dataFault   Data fault status (0 or 1)
 * @param   envWarning  Environment warning status (0 or 2)
 * @retval  SvcUart::Status
 */
SvcUart::Status SvcUart::sendDiagData(uint8_t commFault, uint8_t dataFault, uint8_t envWarning)
{
    Status retVal = Status::Error;
    int len;

    if (initialized_ == 1U)
    {
        len = snprintf(txBuffer_, TX_BUF_SIZE,
                        "[FAULT] COMM:%u DATA:%u ENV:%u\r\n",
                        (unsigned int)commFault,
                        (unsigned int)dataFault,
                        (unsigned int)envWarning);

        if ((len > 0) && ((uint32_t)len < TX_BUF_SIZE))
        {
            retVal = sendString(txBuffer_);
        }
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/

/**
 * @brief   Transmit a null-terminated string via EcuAbsUart.
 * @param   pStr    Pointer to null-terminated string
 * @retval  SvcUart::Status
 */
SvcUart::Status SvcUart::sendString(const char *pStr)
{
    Status retVal = Status::Error;
    EcuAbsUart::Status ecuAbsStatus;
    uint16_t len;

    if (pStr != nullptr)
    {
        len = static_cast<uint16_t>(strlen(pStr));

        if (len > 0U)
        {
            ecuAbsStatus = uart_.transmit(reinterpret_cast<const uint8_t *>(pStr), len);

            if (ecuAbsStatus == EcuAbsUart::Status::Ok)
            {
                retVal = Status::Ok;
            }
        }
    }

    return retVal;
}
