/*
************************************************************************************************************************
*
* Filename        : Asw_Sensor.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/26
* Description     : ASW Sensor Module Implementation.
*                   Performs range validation and rate-of-change filtering
*                   on BME280 measurement data read from RTE.
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
#include "Asw_Sensor.h"
#include "RteApp_Bme280.h"
#include "RteApp_Sensor.h"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

/* Valid range: BME280 datasheet Section 1, Table 1 (Operating range) */
#define ASW_SENSOR_TEMP_MIN         (-4000)     /*!< -40.00 degree C in 0.01 unit     */
#define ASW_SENSOR_TEMP_MAX         (8500)      /*!< 85.00 degree C in 0.01 unit      */
#define ASW_SENSOR_PRESS_MIN        (30000U)    /*!< 300.00 hPa in Pa                 */
#define ASW_SENSOR_PRESS_MAX        (110000U)   /*!< 1100.00 hPa in Pa                */
#define ASW_SENSOR_HUM_MIN          (0U)        /*!< 0.00 %RH in 0.01 unit            */
#define ASW_SENSOR_HUM_MAX          (10000U)    /*!< 100.00 %RH in 0.01 unit          */

/* Rate-of-change limit per cycle (1 second) */
#define ASW_SENSOR_TEMP_RATE_MAX    (100)       /*!< Max +-1.00 degree C / sec         */
#define ASW_SENSOR_PRESS_RATE_MAX   (1000U)     /*!< Max +-10.00 hPa / sec            */
#define ASW_SENSOR_HUM_RATE_MAX     (500U)      /*!< Max +-5.00 %RH / sec             */
/*
************************************************************************************************************************
*                                                         Typedefs
************************************************************************************************************************
*/

/** @brief Sensor diagnostic flag definition */
typedef enum
{
    ASW_SENSOR_DIAG_OK              = 0x00U,   /*!< No fault                              */
    ASW_SENSOR_DIAG_RANGE_ERROR     = 0x01U,   /*!< Measurement out of valid range        */
    ASW_SENSOR_DIAG_RATE_ERROR      = 0x02U,   /*!< Abnormal rate of change detected      */
    ASW_SENSOR_DIAG_COMM_ERROR      = 0x03U    /*!< RTE read failed                       */
} Asw_Sensor_DiagType;

/** @brief Validated sensor data structure */
typedef struct
{
    int32_t  temperature;    /*!< Filtered temperature in 0.01 degree C  */
    uint32_t pressure;       /*!< Filtered pressure in Pa                */
    uint32_t humidity;       /*!< Filtered humidity in 0.01 %RH          */
} Asw_Sensor_DataType;

/** @brief Sensor diagnostic status structure */
typedef struct
{
    Asw_Sensor_DiagType tempDiag;    /*!< Temperature diagnostic flag   */
    Asw_Sensor_DiagType pressDiag;   /*!< Pressure diagnostic flag      */
    Asw_Sensor_DiagType humDiag;     /*!< Humidity diagnostic flag      */
} Asw_Sensor_DiagStatusType;
/*
************************************************************************************************************************
*                                                    Private variables
************************************************************************************************************************
*/

static Asw_Sensor_DataType       sensorData;          /*!< Validated sensor data           */
static Asw_Sensor_DiagStatusType diagStatus;          /*!< Diagnostic flags                */
static uint8_t                   isFirstCycle = 1U;   /*!< First cycle flag (skip filter)  */
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/

/**
 * @brief   Validate temperature range and apply rate-of-change filter.
 * @param   rawValue   Raw temperature from RTE (0.01 degree C)
 * @retval  None
 */
static void Asw_Sensor_ProcessTemp(int32_t rawValue);

/**
 * @brief   Validate pressure range and apply rate-of-change filter.
 * @param   rawValue   Raw pressure from RTE (Pa)
 * @retval  None
 */
static void Asw_Sensor_ProcessPress(uint32_t rawValue);

/**
 * @brief   Validate humidity range and apply rate-of-change filter.
 * @param   rawValue   Raw humidity from RTE (0.01 %RH)
 * @retval  None
 */
static void Asw_Sensor_ProcessHum(uint32_t rawValue);
/*
************************************************************************************************************************
*                                                    Public functions
************************************************************************************************************************
*/

/**
 * @brief   Periodic sensor data processing.
 *          Reads from RTE, validates range, applies rate-of-change filter.
 * @retval  None
 */
void Asw_Sensor_MainFunction(void)
{
    int32_t  tempValue;
    uint32_t pressValue;
    uint32_t humValue;

    /* Read measurement data from RTE */
    if (RteApp_Bme280_Read_Temperature(&tempValue) == RTEAPP_OK)
    {
        Asw_Sensor_ProcessTemp(tempValue);
    }
    else
    {
        diagStatus.tempDiag = ASW_SENSOR_DIAG_COMM_ERROR;
    }

    if (RteApp_Bme280_Read_Pressure(&pressValue) == RTEAPP_OK)
    {
        Asw_Sensor_ProcessPress(pressValue);
    }
    else
    {
        diagStatus.pressDiag = ASW_SENSOR_DIAG_COMM_ERROR;
    }

    if (RteApp_Bme280_Read_Humidity(&humValue) == RTEAPP_OK)
    {
        Asw_Sensor_ProcessHum(humValue);
    }
    else
    {
        diagStatus.humDiag = ASW_SENSOR_DIAG_COMM_ERROR;
    }

    /* First cycle complete */
    if (isFirstCycle == 1U)
    {
        isFirstCycle = 0U;
    }

    /* Write filtered data to RTE */
    (void)RteApp_Sensor_Write_Filtered_Temperature(sensorData.temperature);
    (void)RteApp_Sensor_Write_Filtered_Pressure(sensorData.pressure);
    (void)RteApp_Sensor_Write_Filtered_Humidity(sensorData.humidity);

    /* Write diagnostic flags to RTE */
    (void)RteApp_Sensor_Write_TempDiag((uint8_t)diagStatus.tempDiag);
    (void)RteApp_Sensor_Write_PressDiag((uint8_t)diagStatus.pressDiag);
    (void)RteApp_Sensor_Write_HumDiag((uint8_t)diagStatus.humDiag);
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/

/**
 * @brief   Validate temperature range and apply rate-of-change filter.
 * @param   rawValue   Raw temperature from RTE (0.01 degree C)
 * @retval  None
 */
static void Asw_Sensor_ProcessTemp(int32_t rawValue)
{
    int32_t diff;

    /* Range check */
    if ((rawValue < ASW_SENSOR_TEMP_MIN) || (rawValue > ASW_SENSOR_TEMP_MAX))
    {
        diagStatus.tempDiag = ASW_SENSOR_DIAG_RANGE_ERROR;
    }
    else if (isFirstCycle == 1U)
    {
        /* First cycle: accept value without rate check */
        sensorData.temperature = rawValue;
        diagStatus.tempDiag = ASW_SENSOR_DIAG_OK;
    }
    else
    {
        /* Rate-of-change check */
        diff = rawValue - sensorData.temperature;

        if (diff > ASW_SENSOR_TEMP_RATE_MAX)
        {
            sensorData.temperature += ASW_SENSOR_TEMP_RATE_MAX;
            diagStatus.tempDiag = ASW_SENSOR_DIAG_RATE_ERROR;
        }
        else if (diff < -ASW_SENSOR_TEMP_RATE_MAX)
        {
            sensorData.temperature -= ASW_SENSOR_TEMP_RATE_MAX;
            diagStatus.tempDiag = ASW_SENSOR_DIAG_RATE_ERROR;
        }
        else
        {
            sensorData.temperature = rawValue;
            diagStatus.tempDiag = ASW_SENSOR_DIAG_OK;
        }
    }
}

/**
 * @brief   Validate pressure range and apply rate-of-change filter.
 * @param   rawValue   Raw pressure from RTE (Pa)
 * @retval  None
 */
static void Asw_Sensor_ProcessPress(uint32_t rawValue)
{
    int32_t diff;

    /* Range check */
    if ((rawValue < ASW_SENSOR_PRESS_MIN) || (rawValue > ASW_SENSOR_PRESS_MAX))
    {
        diagStatus.pressDiag = ASW_SENSOR_DIAG_RANGE_ERROR;
    }
    else if (isFirstCycle == 1U)
    {
        /* First cycle: accept value without rate check */
        sensorData.pressure = rawValue;
        diagStatus.pressDiag = ASW_SENSOR_DIAG_OK;
    }
    else
    {
        /* Rate-of-change check */
        diff = (int32_t)rawValue - (int32_t)sensorData.pressure;

        if (diff > (int32_t)ASW_SENSOR_PRESS_RATE_MAX)
        {
            sensorData.pressure += ASW_SENSOR_PRESS_RATE_MAX;
            diagStatus.pressDiag = ASW_SENSOR_DIAG_RATE_ERROR;
        }
        else if (diff < -(int32_t)ASW_SENSOR_PRESS_RATE_MAX)
        {
            sensorData.pressure -= ASW_SENSOR_PRESS_RATE_MAX;
            diagStatus.pressDiag = ASW_SENSOR_DIAG_RATE_ERROR;
        }
        else
        {
            sensorData.pressure = rawValue;
            diagStatus.pressDiag = ASW_SENSOR_DIAG_OK;
        }
    }
}

/**
 * @brief   Validate humidity range and apply rate-of-change filter.
 * @param   rawValue   Raw humidity from RTE (0.01 %RH)
 * @retval  None
 */
static void Asw_Sensor_ProcessHum(uint32_t rawValue)
{
    int32_t diff;

    /* Range check */
    if (rawValue > ASW_SENSOR_HUM_MAX)
    {
        diagStatus.humDiag = ASW_SENSOR_DIAG_RANGE_ERROR;
    }
    else if (isFirstCycle == 1U)
    {
        /* First cycle: accept value without rate check */
        sensorData.humidity = rawValue;
        diagStatus.humDiag = ASW_SENSOR_DIAG_OK;
    }
    else
    {
        /* Rate-of-change check */
        diff = (int32_t)rawValue - (int32_t)sensorData.humidity;

        if (diff > (int32_t)ASW_SENSOR_HUM_RATE_MAX)
        {
            sensorData.humidity += ASW_SENSOR_HUM_RATE_MAX;
            diagStatus.humDiag = ASW_SENSOR_DIAG_RATE_ERROR;
        }
        else if (diff < -(int32_t)ASW_SENSOR_HUM_RATE_MAX)
        {
            sensorData.humidity -= ASW_SENSOR_HUM_RATE_MAX;
            diagStatus.humDiag = ASW_SENSOR_DIAG_RATE_ERROR;
        }
        else
        {
            sensorData.humidity = rawValue;
            diagStatus.humDiag = ASW_SENSOR_DIAG_OK;
        }
    }
}
