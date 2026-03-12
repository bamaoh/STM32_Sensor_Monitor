/*
************************************************************************************************************************
*
* Filename        : Asw_Diag.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : ASW Diagnostic Module Implementation.
*                   Evaluates sensor communication, data quality, and
*                   environment thresholds using debouncing state machine.
*                   Each diagnostic item transitions through:
*                   NORMAL → FAULT_DEB → FAULT → NORMAL_DEB → NORMAL
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
#include "Asw_Diag.h"
#include "RteApp_Sensor.h"
#include "RteApp_Diag.h"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

/* Debouncing threshold (3 cycles = 3 seconds at 1 sec period) */
#define ASW_DIAG_DEB_THRESHOLD      (3U)

/* Sensor diagnostic flag values (matches Asw_Sensor_DiagType) */
#define ASW_DIAG_FLAG_OK            (0x00U)
#define ASW_DIAG_FLAG_RANGE_ERROR   (0x01U)
#define ASW_DIAG_FLAG_RATE_ERROR    (0x02U)
#define ASW_DIAG_FLAG_COMM_ERROR    (0x03U)

/* Environment thresholds */
#define ASW_DIAG_TEMP_HIGH          (5000)      /*!< Over temperature: 50.00 degree C     */
#define ASW_DIAG_HUM_HIGH           (8000U)     /*!< Over humidity: 80.00 %RH             */
#define ASW_DIAG_PRESS_LOW          (95000U)    /*!< Low pressure: 950.00 hPa             */
#define ASW_DIAG_PRESS_HIGH         (106000U)   /*!< High pressure: 1060.00 hPa           */

/* Diagnostic result values */
#define ASW_DIAG_NO_FAULT           (0U)
#define ASW_DIAG_FAULT              (1U)
#define ASW_DIAG_WARNING            (2U)
/*
************************************************************************************************************************
*                                                         Typedefs
************************************************************************************************************************
*/

/** @brief Diagnostic state machine states */
typedef enum
{
    ASW_DIAG_STATE_NORMAL          = 0x00U,
    ASW_DIAG_STATE_FAULT_DEB       = 0x01U,
    ASW_DIAG_STATE_FAULT           = 0x02U,
    ASW_DIAG_STATE_NORMAL_DEB      = 0x03U,
    ASW_DIAG_STATE_WARNING_DEB     = 0x04U,
    ASW_DIAG_STATE_WARNING         = 0x05U
} Asw_Diag_StateType;

/** @brief Diagnostic item context (state + counter) */
typedef struct
{
    Asw_Diag_StateType state;
    uint8_t debCounter;
} Asw_Diag_ItemType;
/*
************************************************************************************************************************
*                                                    Private variables
************************************************************************************************************************
*/

static Asw_Diag_ItemType commDiag = {ASW_DIAG_STATE_NORMAL, 0U};
static Asw_Diag_ItemType dataDiag = {ASW_DIAG_STATE_NORMAL, 0U};
static Asw_Diag_ItemType envDiag  = {ASW_DIAG_STATE_NORMAL, 0U};
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/

/**
 * @brief   Process sensor communication fault diagnosis with debouncing.
 * @param   tempDiag    Temperature diagnostic flag from RTE
 * @param   pressDiag   Pressure diagnostic flag from RTE
 * @param   humDiag     Humidity diagnostic flag from RTE
 * @retval  None
 */
static void Asw_Diag_ProcessCommFault(uint8_t tempDiag, uint8_t pressDiag, uint8_t humDiag);

/**
 * @brief   Process sensor data fault diagnosis with debouncing.
 * @param   tempDiag    Temperature diagnostic flag from RTE
 * @param   pressDiag   Pressure diagnostic flag from RTE
 * @param   humDiag     Humidity diagnostic flag from RTE
 * @retval  None
 */
static void Asw_Diag_ProcessDataFault(uint8_t tempDiag, uint8_t pressDiag, uint8_t humDiag);

/**
 * @brief   Process environment threshold warning with debouncing.
 * @param   temp    Filtered temperature (0.01 degree C)
 * @param   press   Filtered pressure (Pa)
 * @param   hum     Filtered humidity (0.01 %RH)
 * @retval  None
 */
static void Asw_Diag_ProcessEnvWarning(int32_t temp, uint32_t press, uint32_t hum);
/*
************************************************************************************************************************
*                                                    Public functions
************************************************************************************************************************
*/

/**
 * @brief   Periodic diagnostic processing.
 * @retval  None
 */
void Asw_Diag_MainFunction(void)
{
    uint8_t tempDiag  = ASW_DIAG_FLAG_OK;
    uint8_t pressDiag = ASW_DIAG_FLAG_OK;
    uint8_t humDiag   = ASW_DIAG_FLAG_OK;
    int32_t  tempValue  = 0;
    uint32_t pressValue = 0U;
    uint32_t humValue   = 0U;

    /* Read diagnostic flags from RTE */
    (void)RteApp_Sensor_Read_TempDiag(&tempDiag);
    (void)RteApp_Sensor_Read_PressDiag(&pressDiag);
    (void)RteApp_Sensor_Read_HumDiag(&humDiag);

    /* Read filtered sensor data from RTE */
    (void)RteApp_Sensor_Read_Filtered_Temperature(&tempValue);
    (void)RteApp_Sensor_Read_Filtered_Pressure(&pressValue);
    (void)RteApp_Sensor_Read_Filtered_Humidity(&humValue);

    /* Process each diagnostic item */
    Asw_Diag_ProcessCommFault(tempDiag, pressDiag, humDiag);
    Asw_Diag_ProcessDataFault(tempDiag, pressDiag, humDiag);
    Asw_Diag_ProcessEnvWarning(tempValue, pressValue, humValue);

    /* Write diagnostic results to RTE */
    (void)RteApp_Diag_Write_CommFault((commDiag.state == ASW_DIAG_STATE_FAULT) ? ASW_DIAG_FAULT : ASW_DIAG_NO_FAULT);
    (void)RteApp_Diag_Write_DataFault((dataDiag.state == ASW_DIAG_STATE_FAULT) ? ASW_DIAG_FAULT : ASW_DIAG_NO_FAULT);
    (void)RteApp_Diag_Write_EnvWarning((envDiag.state == ASW_DIAG_STATE_WARNING) ? ASW_DIAG_WARNING : ASW_DIAG_NO_FAULT);
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/

/**
 * @brief   Process sensor communication fault diagnosis with debouncing.
 * @param   tempDiag    Temperature diagnostic flag from RTE
 * @param   pressDiag   Pressure diagnostic flag from RTE
 * @param   humDiag     Humidity diagnostic flag from RTE
 * @retval  None
 */
static void Asw_Diag_ProcessCommFault(uint8_t tempDiag, uint8_t pressDiag, uint8_t humDiag)
{
    uint8_t faultDetected;

    /* Communication fault if any channel has COMM_ERROR */
    faultDetected = ((tempDiag == ASW_DIAG_FLAG_COMM_ERROR)
                  || (pressDiag == ASW_DIAG_FLAG_COMM_ERROR)
                  || (humDiag == ASW_DIAG_FLAG_COMM_ERROR)) ? 1U : 0U;

    switch (commDiag.state)
    {
        case ASW_DIAG_STATE_NORMAL:
            if (faultDetected == 1U)
            {
                commDiag.state = ASW_DIAG_STATE_FAULT_DEB;
                commDiag.debCounter = 1U;
            }
            break;

        case ASW_DIAG_STATE_FAULT_DEB:
            if (faultDetected == 1U)
            {
                commDiag.debCounter++;

                if (commDiag.debCounter >= ASW_DIAG_DEB_THRESHOLD)
                {
                    commDiag.state = ASW_DIAG_STATE_FAULT;
                    commDiag.debCounter = 0U;
                }
            }
            else
            {
                commDiag.state = ASW_DIAG_STATE_NORMAL;
                commDiag.debCounter = 0U;
            }
            break;

        case ASW_DIAG_STATE_FAULT:
            if (faultDetected == 0U)
            {
                commDiag.state = ASW_DIAG_STATE_NORMAL_DEB;
                commDiag.debCounter = 1U;
            }
            break;

        case ASW_DIAG_STATE_NORMAL_DEB:
            if (faultDetected == 0U)
            {
                commDiag.debCounter++;

                if (commDiag.debCounter >= ASW_DIAG_DEB_THRESHOLD)
                {
                    commDiag.state = ASW_DIAG_STATE_NORMAL;
                    commDiag.debCounter = 0U;
                }
            }
            else
            {
                commDiag.state = ASW_DIAG_STATE_FAULT;
                commDiag.debCounter = 0U;
            }
            break;

        default:
            commDiag.state = ASW_DIAG_STATE_NORMAL;
            commDiag.debCounter = 0U;
            break;
    }
}

/**
 * @brief   Process sensor data fault diagnosis with debouncing.
 * @param   tempDiag    Temperature diagnostic flag from RTE
 * @param   pressDiag   Pressure diagnostic flag from RTE
 * @param   humDiag     Humidity diagnostic flag from RTE
 * @retval  None
 */
static void Asw_Diag_ProcessDataFault(uint8_t tempDiag, uint8_t pressDiag, uint8_t humDiag)
{
    uint8_t faultDetected;

    /* Data fault if any channel has RANGE_ERROR or RATE_ERROR */
    faultDetected = ((tempDiag == ASW_DIAG_FLAG_RANGE_ERROR)
                  || (tempDiag == ASW_DIAG_FLAG_RATE_ERROR)
                  || (pressDiag == ASW_DIAG_FLAG_RANGE_ERROR)
                  || (pressDiag == ASW_DIAG_FLAG_RATE_ERROR)
                  || (humDiag == ASW_DIAG_FLAG_RANGE_ERROR)
                  || (humDiag == ASW_DIAG_FLAG_RATE_ERROR)) ? 1U : 0U;

    switch (dataDiag.state)
    {
        case ASW_DIAG_STATE_NORMAL:
            if (faultDetected == 1U)
            {
                dataDiag.state = ASW_DIAG_STATE_FAULT_DEB;
                dataDiag.debCounter = 1U;
            }
            break;

        case ASW_DIAG_STATE_FAULT_DEB:
            if (faultDetected == 1U)
            {
                dataDiag.debCounter++;

                if (dataDiag.debCounter >= ASW_DIAG_DEB_THRESHOLD)
                {
                    dataDiag.state = ASW_DIAG_STATE_FAULT;
                    dataDiag.debCounter = 0U;
                }
            }
            else
            {
                dataDiag.state = ASW_DIAG_STATE_NORMAL;
                dataDiag.debCounter = 0U;
            }
            break;

        case ASW_DIAG_STATE_FAULT:
            if (faultDetected == 0U)
            {
                dataDiag.state = ASW_DIAG_STATE_NORMAL_DEB;
                dataDiag.debCounter = 1U;
            }
            break;

        case ASW_DIAG_STATE_NORMAL_DEB:
            if (faultDetected == 0U)
            {
                dataDiag.debCounter++;

                if (dataDiag.debCounter >= ASW_DIAG_DEB_THRESHOLD)
                {
                    dataDiag.state = ASW_DIAG_STATE_NORMAL;
                    dataDiag.debCounter = 0U;
                }
            }
            else
            {
                dataDiag.state = ASW_DIAG_STATE_FAULT;
                dataDiag.debCounter = 0U;
            }
            break;

        default:
            dataDiag.state = ASW_DIAG_STATE_NORMAL;
            dataDiag.debCounter = 0U;
            break;
    }
}

/**
 * @brief   Process environment threshold warning with debouncing.
 * @param   temp    Filtered temperature (0.01 degree C)
 * @param   press   Filtered pressure (Pa)
 * @param   hum     Filtered humidity (0.01 %RH)
 * @retval  None
 */
static void Asw_Diag_ProcessEnvWarning(int32_t temp, uint32_t press, uint32_t hum)
{
    uint8_t warningDetected;

    /* Environment warning if any threshold exceeded */
    warningDetected = ((temp > ASW_DIAG_TEMP_HIGH)
                    || (hum > ASW_DIAG_HUM_HIGH)
                    || (press < ASW_DIAG_PRESS_LOW)
                    || (press > ASW_DIAG_PRESS_HIGH)) ? 1U : 0U;

    switch (envDiag.state)
    {
        case ASW_DIAG_STATE_NORMAL:
            if (warningDetected == 1U)
            {
                envDiag.state = ASW_DIAG_STATE_WARNING_DEB;
                envDiag.debCounter = 1U;
            }
            break;

        case ASW_DIAG_STATE_WARNING_DEB:
            if (warningDetected == 1U)
            {
                envDiag.debCounter++;

                if (envDiag.debCounter >= ASW_DIAG_DEB_THRESHOLD)
                {
                    envDiag.state = ASW_DIAG_STATE_WARNING;
                    envDiag.debCounter = 0U;
                }
            }
            else
            {
                envDiag.state = ASW_DIAG_STATE_NORMAL;
                envDiag.debCounter = 0U;
            }
            break;

        case ASW_DIAG_STATE_WARNING:
            if (warningDetected == 0U)
            {
                envDiag.state = ASW_DIAG_STATE_NORMAL_DEB;
                envDiag.debCounter = 1U;
            }
            break;

        case ASW_DIAG_STATE_NORMAL_DEB:
            if (warningDetected == 0U)
            {
                envDiag.debCounter++;

                if (envDiag.debCounter >= ASW_DIAG_DEB_THRESHOLD)
                {
                    envDiag.state = ASW_DIAG_STATE_NORMAL;
                    envDiag.debCounter = 0U;
                }
            }
            else
            {
                envDiag.state = ASW_DIAG_STATE_WARNING;
                envDiag.debCounter = 0U;
            }
            break;

        default:
            envDiag.state = ASW_DIAG_STATE_NORMAL;
            envDiag.debCounter = 0U;
            break;
    }
}
