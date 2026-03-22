/*
************************************************************************************************************************
*
* Filename        : Svc_Uart.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : UART Service Module Implementation.
*                   Formats sensor data and diagnostic results into human-readable
*                   strings and transmits via EcuAbs_Uart (polling mode).
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
#include "Svc_Uart.h"
#include "EcuAbs_Uart.h"
#include <stdio.h>
#include <string.h>
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

#define SVC_UART_TX_BUF_SIZE    (128U)   /*!< Transmit buffer size in bytes     */

/* Unit conversion */
#define SVC_UART_TEMP_SCALE     (100)    /*!< Temperature scale (0.01 degree)   */
#define SVC_UART_HUM_SCALE      (100U)   /*!< Humidity scale (0.01 %RH)         */
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

static char txBuffer[SVC_UART_TX_BUF_SIZE];   /*!< Transmit formatting buffer */
static uint8_t svcUartInitialized = 0U;        /*!< Init completed flag        */
static uint32_t aliveCounter = 0U;             /*!< Message alive counter      */
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/

/**
 * @brief   Transmit a null-terminated string via EcuAbs_Uart.
 * @param   pStr    Pointer to null-terminated string
 * @retval  Svc_Uart status
 */
static Svc_Uart_StatusType Svc_Uart_SendString(const char *pStr);
/*
************************************************************************************************************************
*                                                    Public functions
************************************************************************************************************************
*/

/**
 * @brief   Initialize UART service.
 * @retval  Svc_Uart status
 */
Svc_Uart_StatusType Svc_Uart_Init(void)
{
    Svc_Uart_StatusType retVal = SVC_UART_OK;
    EcuAbs_Uart_StatusType ecuAbsStatus;

    ecuAbsStatus = EcuAbs_Uart_Init();

    if (ecuAbsStatus != ECUABS_UART_OK)
    {
        retVal = SVC_UART_ERROR;
    }
    else
    {
        svcUartInitialized = 1U;
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
 * @retval  Svc_Uart status
 */
Svc_Uart_StatusType Svc_Uart_SendSensorData(int32_t temp, uint32_t press, uint32_t hum, uint8_t isStale)
{
    Svc_Uart_StatusType retVal = SVC_UART_ERROR;
    int32_t tempInt;
    int32_t tempFrac;
    uint32_t humInt;
    uint32_t humFrac;
    const char *qualityStr;
    int len;

    if (svcUartInitialized == 1U)
    {
        /* Increment alive counter */
        aliveCounter++;

        /* Split temperature into integer and fractional parts */
        tempInt  = temp / SVC_UART_TEMP_SCALE;
        tempFrac = temp % SVC_UART_TEMP_SCALE;

        /* Handle negative fractional part */
        if (tempFrac < 0)
        {
            tempFrac = -tempFrac;
        }

        /* Split humidity into integer and fractional parts */
        humInt  = hum / SVC_UART_HUM_SCALE;
        humFrac = hum % SVC_UART_HUM_SCALE;

        /* Select quality string based on data freshness */
        qualityStr = (isStale == 1U) ? "STALE" : "VALID";

        len = snprintf(txBuffer, SVC_UART_TX_BUF_SIZE,
                        "[CNT:%03lu Q:%s] T:%ld.%02ldC P:%luPa H:%lu.%02lu%%\r\n",
                        (unsigned long)(aliveCounter % 1000U),
                        qualityStr,
                        (long)tempInt, (long)tempFrac,
                        (unsigned long)press,
                        (unsigned long)humInt, (unsigned long)humFrac);

        if ((len > 0) && ((uint32_t)len < SVC_UART_TX_BUF_SIZE))
        {
            retVal = Svc_Uart_SendString(txBuffer);
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
 * @retval  Svc_Uart status
 */
Svc_Uart_StatusType Svc_Uart_SendDiagData(uint8_t commFault, uint8_t dataFault, uint8_t envWarning)
{
    Svc_Uart_StatusType retVal = SVC_UART_ERROR;
    int len;

    if (svcUartInitialized == 1U)
    {
        len = snprintf(txBuffer, SVC_UART_TX_BUF_SIZE,
                        "[FAULT] COMM:%u DATA:%u ENV:%u\r\n",
                        (unsigned int)commFault,
                        (unsigned int)dataFault,
                        (unsigned int)envWarning);

        if ((len > 0) && ((uint32_t)len < SVC_UART_TX_BUF_SIZE))
        {
            retVal = Svc_Uart_SendString(txBuffer);
        }
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/

/**
 * @brief   Transmit a null-terminated string via EcuAbs_Uart.
 * @param   pStr    Pointer to null-terminated string
 * @retval  Svc_Uart status
 */
static Svc_Uart_StatusType Svc_Uart_SendString(const char *pStr)
{
    Svc_Uart_StatusType retVal = SVC_UART_ERROR;
    EcuAbs_Uart_StatusType ecuAbsStatus;
    uint16_t len;

    if (pStr != NULL)
    {
        len = (uint16_t)strlen(pStr);

        if (len > 0U)
        {
            ecuAbsStatus = EcuAbs_Uart_Transmit((const uint8_t *)pStr, len);

            if (ecuAbsStatus == ECUABS_UART_OK)
            {
                retVal = SVC_UART_OK;
            }
        }
    }

    return retVal;
}
