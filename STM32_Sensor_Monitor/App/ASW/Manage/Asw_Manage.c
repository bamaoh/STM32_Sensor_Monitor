/*
************************************************************************************************************************
*
* Filename        : Asw_Manage.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : ASW Manage Module Implementation.
*                   Reads diagnostic results from RTE, controls status LED,
*                   manages NVM diagnostic data persistence, and outputs
*                   sensor/diagnostic data via UART serial port.
*                   NVM is updated only on fault/warning state transitions.
* Version         : 0.0.3
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                                  Include header files
************************************************************************************************************************
*/
#include "Asw_Manage.h"
#include "RteApp_Diag.h"
#include "RteApp_Sensor.h"
#include "Svc_Led.h"
#include "Svc_Nvm.h"
#include "Svc_Uart.h"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

/* Diagnostic result values (matches Asw_Diag output) */
#define ASW_MANAGE_NO_FAULT     (0U)
#define ASW_MANAGE_FAULT        (1U)
#define ASW_MANAGE_WARNING      (2U)
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

static uint8_t prevCommFault  = ASW_MANAGE_NO_FAULT;
static uint8_t prevDataFault  = ASW_MANAGE_NO_FAULT;
static uint8_t prevEnvWarning = ASW_MANAGE_NO_FAULT;
static Svc_Nvm_DiagDataType nvmData;
static uint8_t nvmLoaded      = 0U;
static uint8_t commSavedCycle = 0U;    /*!< CommFault type already saved this cycle  */
static uint8_t dataSavedCycle = 0U;    /*!< DataFault type already saved this cycle  */
static uint8_t envSavedCycle  = 0U;    /*!< EnvWarning type already saved this cycle */
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/

/**
 * @brief   Update NVM on diagnostic state transition.
 *          Stores fault type with cycle marker and increments count.
 *          Only writes to Flash when a new fault/warning transition occurs.
 * @param   commFault   Current CommFault result
 * @param   dataFault   Current DataFault result
 * @param   envWarning  Current EnvWarning result
 * @retval  None
 */
static void Asw_Manage_UpdateNvm(uint8_t commFault, uint8_t dataFault, uint8_t envWarning);

/**
 * @brief   Output sensor and diagnostic data via UART.
 *          Always sends sensor data. Additionally sends fault data
 *          when at least one fault or warning is active.
 * @param   commFault   Current CommFault result
 * @param   dataFault   Current DataFault result
 * @param   envWarning  Current EnvWarning result
 * @retval  None
 */
static void Asw_Manage_OutputUart(uint8_t commFault, uint8_t dataFault, uint8_t envWarning);
/*
************************************************************************************************************************
*                                                    Public functions
************************************************************************************************************************
*/

/**
 * @brief   Periodic manage processing.
 *          Reads diagnostic results from RTE, controls status LED,
 *          and updates NVM on state transitions.
 * @retval  None
 */
void Asw_Manage_MainFunction(void)
{
    uint8_t commFault  = ASW_MANAGE_NO_FAULT;
    uint8_t dataFault  = ASW_MANAGE_NO_FAULT;
    uint8_t envWarning = ASW_MANAGE_NO_FAULT;

    /* Load NVM data on first call */
    if (nvmLoaded == 0U)
    {
        (void)Svc_Nvm_ReadDiagData(&nvmData);
        nvmLoaded = 1U;
    }

    /* Read diagnostic results from RTE */
    (void)RteApp_Diag_Read_CommFault(&commFault);
    (void)RteApp_Diag_Read_DataFault(&dataFault);
    (void)RteApp_Diag_Read_EnvWarning(&envWarning);

    /* LED control: FAULT > WARNING > NORMAL */
    if ((commFault == ASW_MANAGE_FAULT) || (dataFault == ASW_MANAGE_FAULT))
    {
        Svc_Led_SetMode(SVC_LED_MODE_OFF);
    }
    else if (envWarning == ASW_MANAGE_WARNING)
    {
        Svc_Led_SetMode(SVC_LED_MODE_BLINK);
    }
    else
    {
        Svc_Led_SetMode(SVC_LED_MODE_ON);
    }

    /* Update NVM on state transitions */
    Asw_Manage_UpdateNvm(commFault, dataFault, envWarning);

    /* UART output: sensor data + fault data (if any) */
    Asw_Manage_OutputUart(commFault, dataFault, envWarning);

    /* Track previous results */
    prevCommFault  = commFault;
    prevDataFault  = dataFault;
    prevEnvWarning = envWarning;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/

/**
 * @brief   Update NVM on diagnostic state transition.
 *          First occurrence in cycle: store fault type with cycle marker + increment count.
 *          Repeated same fault in cycle: increment count only (fault type already stored).
 *          Writes to Flash only on NORMAL -> FAULT/WARNING transitions.
 * @param   commFault   Current CommFault result
 * @param   dataFault   Current DataFault result
 * @param   envWarning  Current EnvWarning result
 * @retval  None
 */
static void Asw_Manage_UpdateNvm(uint8_t commFault, uint8_t dataFault, uint8_t envWarning)
{
    uint8_t nvmChanged = 0U;

    /* CommFault: NORMAL -> FAULT transition */
    if ((commFault == ASW_MANAGE_FAULT) && (prevCommFault == ASW_MANAGE_NO_FAULT))
    {
        if (commSavedCycle == 0U)
        {
            /* First occurrence: store fault type with cycle marker */
            nvmData.commFault = nvmData.cycleMarker | commFault;
            commSavedCycle = 1U;
        }

        nvmData.commFaultCount++;
        nvmChanged = 1U;
    }

    /* DataFault: NORMAL -> FAULT transition */
    if ((dataFault == ASW_MANAGE_FAULT) && (prevDataFault == ASW_MANAGE_NO_FAULT))
    {
        if (dataSavedCycle == 0U)
        {
            nvmData.dataFault = nvmData.cycleMarker | dataFault;
            dataSavedCycle = 1U;
        }

        nvmData.dataFaultCount++;
        nvmChanged = 1U;
    }

    /* EnvWarning: NORMAL -> WARNING transition */
    if ((envWarning == ASW_MANAGE_WARNING) && (prevEnvWarning == ASW_MANAGE_NO_FAULT))
    {
        if (envSavedCycle == 0U)
        {
            nvmData.envWarning = nvmData.cycleMarker | envWarning;
            envSavedCycle = 1U;
        }

        nvmData.envWarningCount++;
        nvmChanged = 1U;
    }

    /* Write to Flash only if something changed */
    if (nvmChanged == 1U)
    {
        (void)Svc_Nvm_WriteDiagData(&nvmData);
    }
}

/**
 * @brief   Output sensor and diagnostic data via UART.
 *          Always sends sensor data (temperature, pressure, humidity).
 *          Additionally sends fault data when at least one fault or warning is active.
 * @param   commFault   Current CommFault result
 * @param   dataFault   Current DataFault result
 * @param   envWarning  Current EnvWarning result
 * @retval  None
 */
static void Asw_Manage_OutputUart(uint8_t commFault, uint8_t dataFault, uint8_t envWarning)
{
    int32_t  tempValue  = 0;
    uint32_t pressValue = 0U;
    uint32_t humValue   = 0U;
    uint8_t  isStale    = 0U;

    /* Read filtered sensor data from RTE */
    (void)RteApp_Sensor_Read_Filtered_Temperature(&tempValue);
    (void)RteApp_Sensor_Read_Filtered_Pressure(&pressValue);
    (void)RteApp_Sensor_Read_Filtered_Humidity(&humValue);

    /* Determine if sensor data is stale (any fault or warning active) */
    if ((commFault != ASW_MANAGE_NO_FAULT) ||
        (dataFault != ASW_MANAGE_NO_FAULT) ||
        (envWarning != ASW_MANAGE_NO_FAULT))
    {
        isStale = 1U;
    }

    /* Output sensor data with alive counter and quality indicator */
    (void)Svc_Uart_SendSensorData(tempValue, pressValue, humValue, isStale);

    /* Output fault data only when fault or warning is active */
    if (isStale == 1U)
    {
        (void)Svc_Uart_SendDiagData(commFault, dataFault, envWarning);
    }
}
