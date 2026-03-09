/*
************************************************************************************************************************
*
* Filename        : Svc_Bme280.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/26
* Description     : Service Layer - BME280 Sensor Driver Implementation.
*                   Handles BME280 sensor initialization, calibration data
*                   management, and periodic measurement acquisition.
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
#include "Svc_Bme280.h"
#include "EcuAbs_I2c.h"
#include "EcuAbs_Gpio.h"
#include "RteApp_Bme280.h"
#include "main.h"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

#define SVC_BME280_DEV_ADDR         (0x77U)    /*!< BME280 7-bit I2C address (Sparkfun default, SDO pulled high)  */

/* Register addresses */
#define SVC_BME280_REG_CHIP_ID      (0xD0U)    /*!< Chip ID register address              */
#define SVC_BME280_REG_RESET        (0xE0U)    /*!< Soft reset register address           */
#define SVC_BME280_REG_STATUS       (0xF3U)    /*!< Status register                       */
#define SVC_BME280_REG_CTRL_HUM     (0xF2U)    /*!< Humidity control register             */
#define SVC_BME280_REG_CTRL_MEAS    (0xF4U)    /*!< Measurement control register          */
#define SVC_BME280_REG_CONFIG       (0xF5U)    /*!< Configuration register                */
#define SVC_BME280_REG_CALIB_TP     (0x88U)    /*!< Calibration data start (temp/press)   */
#define SVC_BME280_REG_CALIB_H2     (0xE1U)    /*!< Calibration data start (humidity)     */

/* Constants */
#define SVC_BME280_CHIP_ID_VAL      (0x60U)    /*!< Expected Chip ID value                */
#define SVC_BME280_RESET_CMD        (0xB6U)    /*!< Soft reset command                    */
#define SVC_BME280_CALIB_TP_LEN     (26U)      /*!< Temp/Press calibration data length    */
#define SVC_BME280_CALIB_H_LEN      (7U)       /*!< Humidity calibration data length      */
#define SVC_BME280_RESET_WAIT_MS    (10U)      /*!< Wait time after soft reset (ms)       */

/* Sensor configuration */
#define SVC_BME280_OSRS_T           (0x01U)    /*!< Temperature oversampling x1           */
#define SVC_BME280_OSRS_P           (0x01U)    /*!< Pressure oversampling x1              */
#define SVC_BME280_OSRS_H           (0x01U)    /*!< Humidity oversampling x1              */
#define SVC_BME280_MODE_NORMAL      (0x03U)    /*!< Normal mode                           */
#define SVC_BME280_FILTER_OFF       (0x00U)    /*!< IIR filter off                        */
#define SVC_BME280_STANDBY_1000MS   (0x05U)    /*!< Standby time 1000ms                   */

#define SVC_BME280_REG_DATA         (0xF7U)    /*!< Measurement start register            */
#define SVC_BME280_DATA_LEN         (8U)       /*!< Measurement data length               */

/* Status register bit mask */
#define SVC_BME280_STATUS_MEASURING (0x08U)    /*!< Bit[3]: measuring in progress         */

/* Recovery configuration */
#define SVC_BME280_RETRY_MAX        (3U)       /*!< Max retry count before escalation     */
#define SVC_BME280_SCL_TOGGLE_COUNT (9U)       /*!< SCL toggles for bus recovery          */
/*
************************************************************************************************************************
*                                                         Typedefs
************************************************************************************************************************
*/

/** @brief BME280 calibration data structure */
typedef struct
{
    /* Temperature compensation parameters */
    uint16_t digT1;
    int16_t  digT2;
    int16_t  digT3;

    /* Pressure compensation parameters */
    uint16_t digP1;
    int16_t  digP2;
    int16_t  digP3;
    int16_t  digP4;
    int16_t  digP5;
    int16_t  digP6;
    int16_t  digP7;
    int16_t  digP8;
    int16_t  digP9;

    /* Humidity compensation parameters */
    uint8_t  digH1;
    int16_t  digH2;
    uint8_t  digH3;
    int16_t  digH4;
    int16_t  digH5;
    int8_t   digH6;
} Svc_Bme280_CalibType;
/*
************************************************************************************************************************
*                                                    Private variables
************************************************************************************************************************
*/

static Svc_Bme280_CalibType calibData;    /*!< Stored calibration parameters  */
static int32_t tFine;                     /*!< Fine temperature value shared across compensation  */
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/

/**
 * @brief   Performs soft reset and waits for sensor startup.
 * @retval  Svc_Bme280 status
 */
static Svc_Bme280_StatusType Svc_Bme280_SoftReset(void);

/**
 * @brief   Reads calibration data from sensor registers.
 * @retval  Svc_Bme280 status
 */
static Svc_Bme280_StatusType Svc_Bme280_ReadCalibData(void);

/**
 * @brief   Configures oversampling, filter, and operating mode.
 * @retval  Svc_Bme280 status
 */
static Svc_Bme280_StatusType Svc_Bme280_Configure(void);

/**
 * @brief   Compensate raw temperature value to physical value.
 * @param   adcT  Raw temperature ADC value (20-bit).
 * @retval  Temperature in 0.01 degree C
 */
static int32_t Svc_Bme280_CompensateTemp(int32_t adcT);

/**
 * @brief   Compensate raw pressure value to physical value.
 * @param   adcP  Raw pressure ADC value (20-bit).
 * @retval  Pressure in Pa
 */
static uint32_t Svc_Bme280_CompensatePress(int32_t adcP);

/**
 * @brief   Compensate raw humidity value to physical value.
 * @param   adcH  Raw humidity ADC value (16-bit).
 * @retval  Humidity in 0.01 %RH
 */
static uint32_t Svc_Bme280_CompensateHum(int32_t adcH);

/**
 * @brief   Attempt I2C recovery with escalating strategy.
 *          Level 1: Retry communication
 *          Level 2: I2C ReInit → Retry
 *          Level 3: Bus Recovery (SCL toggle) → ReInit → Retry
 * @retval  Svc_Bme280 status
 */
static Svc_Bme280_StatusType Svc_Bme280_RecoverI2c(void);
/*
************************************************************************************************************************
*                                                    Public functions
************************************************************************************************************************
*/

/**
 * @brief   Initializes BME280 sensor and verifies Chip ID.
 * @retval  Svc_Bme280 status
 */
Svc_Bme280_StatusType Svc_Bme280_Init(void)
{
    Svc_Bme280_StatusType retVal = SVC_BME280_OK;
    EcuAbs_I2c_StatusType i2cStatus;
    uint8_t chipId = 0x00U;

    /* Check device presence on I2C bus */
    i2cStatus = EcuAbs_I2c_IsDeviceReady(SVC_BME280_DEV_ADDR);

    if (i2cStatus != ECUABS_I2C_OK)
    {
        retVal = Svc_Bme280_RecoverI2c();
    }

    /* Read and verify Chip ID */
    if (retVal == SVC_BME280_OK)
    {
        i2cStatus = EcuAbs_I2c_ReadReg(SVC_BME280_DEV_ADDR,
                                        SVC_BME280_REG_CHIP_ID,
                                        &chipId,
                                        1U);

        if (i2cStatus != ECUABS_I2C_OK)
        {
            retVal = Svc_Bme280_RecoverI2c();
        }
        else if (chipId != SVC_BME280_CHIP_ID_VAL)
        {
            retVal = SVC_BME280_CHIP_ID_ERR;
        }
        else
        {
            /* Chip ID verified successfully */
        }
    }

    /* Soft reset to ensure clean state */
    if (retVal == SVC_BME280_OK)
    {
        retVal = Svc_Bme280_SoftReset();

        if (retVal == SVC_BME280_COMM_ERROR)
        {
            retVal = Svc_Bme280_RecoverI2c();
        }
    }

    /* Read calibration data from sensor */
    if (retVal == SVC_BME280_OK)
    {
        retVal = Svc_Bme280_ReadCalibData();

        if (retVal == SVC_BME280_COMM_ERROR)
        {
            retVal = Svc_Bme280_RecoverI2c();
        }
    }

    /* Configure oversampling and operating mode */
    if (retVal == SVC_BME280_OK)
    {
        retVal = Svc_Bme280_Configure();

        if (retVal == SVC_BME280_COMM_ERROR)
        {
            retVal = Svc_Bme280_RecoverI2c();
        }
    }

    return retVal;
}

/**
 * @brief   Reads measurement data from sensor registers.
 * @retval  Svc_Bme280 status
 */
Svc_Bme280_StatusType Svc_Bme280_ReadMeasurement(Svc_Bme280_DataType *pData)
{
    Svc_Bme280_StatusType retVal = SVC_BME280_OK;
    EcuAbs_I2c_StatusType i2cStatus;
    uint8_t dataBuf[SVC_BME280_DATA_LEN];
    uint8_t status;
    int32_t adcP;
    int32_t adcT;
    int32_t adcH;

    /* Check if sensor is currently measuring (datasheet Section 5.4.4, status register bit[3]) */
    i2cStatus = EcuAbs_I2c_ReadReg(SVC_BME280_DEV_ADDR,
                                    SVC_BME280_REG_STATUS,
                                    &status,
                                    1U);

    if (i2cStatus != ECUABS_I2C_OK)
    {
        retVal = Svc_Bme280_RecoverI2c();
    }
    else if ((status & SVC_BME280_STATUS_MEASURING) != 0U)
    {
        retVal = SVC_BME280_BUSY;
    }
    else
    {
        /* Sensor is idle, safe to read */
    }

    /* Read measurement data */
    if (retVal == SVC_BME280_OK)
    {
        i2cStatus = EcuAbs_I2c_ReadReg(SVC_BME280_DEV_ADDR,
                                        SVC_BME280_REG_DATA,
                                        dataBuf,
                                        SVC_BME280_DATA_LEN);

        if (i2cStatus != ECUABS_I2C_OK)
        {
            retVal = Svc_Bme280_RecoverI2c();
        }
    }

    if (retVal == SVC_BME280_OK)
    {
        /* Parse 20-bit raw pressure */
        adcP = (int32_t)((uint32_t)dataBuf[0] << 12U | (uint32_t)dataBuf[1] << 4U | (uint32_t)(dataBuf[2] >> 4U));
        /* Parse 20-bit raw temperature */
        adcT = (int32_t)((uint32_t)dataBuf[3] << 12U | (uint32_t)dataBuf[4] << 4U | (uint32_t)(dataBuf[5] >> 4U));
        /* Parse 16-bit raw humidity */
        adcH = (int32_t)((uint32_t)dataBuf[6] << 8U | (uint32_t)dataBuf[7]);

        /* Compensate: temperature must be first (tFine is needed by pressure/humidity) */
        pData->temperature = Svc_Bme280_CompensateTemp(adcT);
        /* Pressure: Q24.8 format (datasheet 4.2.3) -> right shift 8 to get Pa */
        pData->pressure    = Svc_Bme280_CompensatePress(adcP) >> 8U;
        /* Humidity: Q22.10 format (datasheet 4.2.3) -> *100 / 1024 to get 0.01 %RH */
        pData->humidity    = (Svc_Bme280_CompensateHum(adcH) * 100U) >> 10U;

        /* Write compensated data to RTE buffer */
        (void)RteApp_Bme280_Write_Temperature(pData->temperature);
        (void)RteApp_Bme280_Write_Pressure(pData->pressure);
        (void)RteApp_Bme280_Write_Humidity(pData->humidity);
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/

/**
 * @brief   Performs soft reset and waits for sensor startup.
 * @retval  Svc_Bme280 status
 */
static Svc_Bme280_StatusType Svc_Bme280_SoftReset(void)
{
    Svc_Bme280_StatusType retVal = SVC_BME280_OK;
    EcuAbs_I2c_StatusType i2cStatus;
    uint8_t resetCmd = SVC_BME280_RESET_CMD;

    i2cStatus = EcuAbs_I2c_WriteReg(SVC_BME280_DEV_ADDR,
                                     SVC_BME280_REG_RESET,
                                     &resetCmd,
                                     1U);

    if (i2cStatus != ECUABS_I2C_OK)
    {
        retVal = SVC_BME280_COMM_ERROR;
    }
    else
    {
        /* Wait for sensor startup after reset */
        HAL_Delay(SVC_BME280_RESET_WAIT_MS);
    }

    return retVal;
}

/**
 * @brief   Reads calibration data from sensor registers.
 * @retval  Svc_Bme280 status
 */
static Svc_Bme280_StatusType Svc_Bme280_ReadCalibData(void)
{
    Svc_Bme280_StatusType retVal = SVC_BME280_OK;
    EcuAbs_I2c_StatusType i2cStatus;
    uint8_t tpBuf[SVC_BME280_CALIB_TP_LEN];
    uint8_t hBuf[SVC_BME280_CALIB_H_LEN];

    /* Read temperature, pressure calibration and H1 (0x88 ~ 0xA1, 26 bytes) */
    /* tpBuf[0~23]: dig_T1~T3, dig_P1~P9 / tpBuf[24]: reserved / tpBuf[25]: dig_H1 */
    i2cStatus = EcuAbs_I2c_ReadReg(SVC_BME280_DEV_ADDR,
                                    SVC_BME280_REG_CALIB_TP,
                                    tpBuf,
                                    SVC_BME280_CALIB_TP_LEN);

    if (i2cStatus != ECUABS_I2C_OK)
    {
        retVal = SVC_BME280_COMM_ERROR;
    }

    if (retVal == SVC_BME280_OK)
    {
        /* Parse temperature calibration */
        calibData.digT1 = (uint16_t)((uint16_t)tpBuf[1] << 8U) | (uint16_t)tpBuf[0];
        calibData.digT2 = (int16_t)((uint16_t)((uint16_t)tpBuf[3] << 8U) | (uint16_t)tpBuf[2]);
        calibData.digT3 = (int16_t)((uint16_t)((uint16_t)tpBuf[5] << 8U) | (uint16_t)tpBuf[4]);

        /* Parse pressure calibration */
        calibData.digP1 = (uint16_t)((uint16_t)tpBuf[7] << 8U)  | (uint16_t)tpBuf[6];
        calibData.digP2 = (int16_t)((uint16_t)((uint16_t)tpBuf[9] << 8U)  | (uint16_t)tpBuf[8]);
        calibData.digP3 = (int16_t)((uint16_t)((uint16_t)tpBuf[11] << 8U) | (uint16_t)tpBuf[10]);
        calibData.digP4 = (int16_t)((uint16_t)((uint16_t)tpBuf[13] << 8U) | (uint16_t)tpBuf[12]);
        calibData.digP5 = (int16_t)((uint16_t)((uint16_t)tpBuf[15] << 8U) | (uint16_t)tpBuf[14]);
        calibData.digP6 = (int16_t)((uint16_t)((uint16_t)tpBuf[17] << 8U) | (uint16_t)tpBuf[16]);
        calibData.digP7 = (int16_t)((uint16_t)((uint16_t)tpBuf[19] << 8U) | (uint16_t)tpBuf[18]);
        calibData.digP8 = (int16_t)((uint16_t)((uint16_t)tpBuf[21] << 8U) | (uint16_t)tpBuf[20]);
        calibData.digP9 = (int16_t)((uint16_t)((uint16_t)tpBuf[23] << 8U) | (uint16_t)tpBuf[22]);

        /* Parse humidity calibration H1 (tpBuf[25] = 0xA1) */
        calibData.digH1 = tpBuf[25];
    }

    /* Read humidity calibration H2~H6 (0xE1 ~ 0xE7) */
    if (retVal == SVC_BME280_OK)
    {
        i2cStatus = EcuAbs_I2c_ReadReg(SVC_BME280_DEV_ADDR,
                                        SVC_BME280_REG_CALIB_H2,
                                        hBuf,
                                        SVC_BME280_CALIB_H_LEN);

        if (i2cStatus != ECUABS_I2C_OK)
        {
            retVal = SVC_BME280_COMM_ERROR;
        }
    }

    if (retVal == SVC_BME280_OK)
    {
        /* Parse humidity calibration */
        calibData.digH2 = (int16_t)((uint16_t)((uint16_t)hBuf[1] << 8U) | (uint16_t)hBuf[0]);
        calibData.digH3 = hBuf[2];
        calibData.digH4 = (int16_t)(((int16_t)hBuf[3] << 4U) | ((int16_t)hBuf[4] & 0x0F));
        calibData.digH5 = (int16_t)(((int16_t)hBuf[5] << 4U) | ((int16_t)hBuf[4] >> 4U));
        calibData.digH6 = (int8_t)hBuf[6];
    }

    return retVal;
}

/**
 * @brief   Configures oversampling, filter, and operating mode.
 * @retval  Svc_Bme280 status
 */
static Svc_Bme280_StatusType Svc_Bme280_Configure(void)
{
    Svc_Bme280_StatusType retVal = SVC_BME280_OK;
    EcuAbs_I2c_StatusType i2cStatus;
    uint8_t regVal;

    /* Set humidity oversampling (must be written before ctrl_meas) */
    regVal = SVC_BME280_OSRS_H;

    i2cStatus = EcuAbs_I2c_WriteReg(SVC_BME280_DEV_ADDR,
                                     SVC_BME280_REG_CTRL_HUM,
                                     &regVal,
                                     1U);

    if (i2cStatus != ECUABS_I2C_OK)
    {
        retVal = SVC_BME280_COMM_ERROR;
    }

    /* Set config register (standby time, IIR filter) */
    if (retVal == SVC_BME280_OK)
    {
        regVal = (uint8_t)((SVC_BME280_STANDBY_1000MS << 5U) | (SVC_BME280_FILTER_OFF << 2U));

        i2cStatus = EcuAbs_I2c_WriteReg(SVC_BME280_DEV_ADDR,
                                         SVC_BME280_REG_CONFIG,
                                         &regVal,
                                         1U);

        if (i2cStatus != ECUABS_I2C_OK)
        {
            retVal = SVC_BME280_COMM_ERROR;
        }
    }

    /* Set temp/press oversampling and normal mode (activates ctrl_hum setting) */
    if (retVal == SVC_BME280_OK)
    {
        regVal = (uint8_t)((SVC_BME280_OSRS_T << 5U) | (SVC_BME280_OSRS_P << 2U) | SVC_BME280_MODE_NORMAL);

        i2cStatus = EcuAbs_I2c_WriteReg(SVC_BME280_DEV_ADDR,
                                         SVC_BME280_REG_CTRL_MEAS,
                                         &regVal,
                                         1U);

        if (i2cStatus != ECUABS_I2C_OK)
        {
            retVal = SVC_BME280_COMM_ERROR;
        }
    }

    return retVal;
}

/**
 * @brief   Compensate raw temperature value to physical value.
 *          Formula from BME280 datasheet Section 4.2.3.
 * @param   adcT  Raw temperature ADC value (20-bit).
 * @retval  Temperature in 0.01 degree C
 */
static int32_t Svc_Bme280_CompensateTemp(int32_t adcT)
{
    int32_t var1;
    int32_t var2;
    int32_t temperature;

    var1 = ((((adcT >> 3) - ((int32_t)calibData.digT1 << 1))) * ((int32_t)calibData.digT2)) >> 11;
    var2 = (((((adcT >> 4) - ((int32_t)calibData.digT1)) * ((adcT >> 4) - ((int32_t)calibData.digT1))) >> 12)
            * ((int32_t)calibData.digT3)) >> 14;

    tFine = var1 + var2;
    temperature = (tFine * 5 + 128) >> 8;

    return temperature;
}

/**
 * @brief   Compensate raw pressure value to physical value.
 *          Formula from BME280 datasheet Section 4.2.3.
 * @param   adcP  Raw pressure ADC value (20-bit).
 * @retval  Pressure in Pa
 */
static uint32_t Svc_Bme280_CompensatePress(int32_t adcP)
{
    int64_t var1;
    int64_t var2;
    int64_t p;
    uint32_t pressure = 0U;

    var1 = ((int64_t)tFine) - 128000;
    var2 = var1 * var1 * (int64_t)calibData.digP6;
    var2 = var2 + ((var1 * (int64_t)calibData.digP5) << 17);
    var2 = var2 + (((int64_t)calibData.digP4) << 35);
    var1 = ((var1 * var1 * (int64_t)calibData.digP3) >> 8) + ((var1 * (int64_t)calibData.digP2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calibData.digP1) >> 33;

    if (var1 != 0)
    {
        p = 1048576 - (int64_t)adcP;
        p = (((p << 31) - var2) * 3125) / var1;
        var1 = (((int64_t)calibData.digP9) * (p >> 13) * (p >> 13)) >> 25;
        var2 = (((int64_t)calibData.digP8) * p) >> 19;

        p = ((p + var1 + var2) >> 8) + (((int64_t)calibData.digP7) << 4);
        pressure = (uint32_t)p;
    }

    return pressure;
}

/**
 * @brief   Compensate raw humidity value to physical value.
 *          Formula from BME280 datasheet Section 4.2.3.
 * @param   adcH  Raw humidity ADC value (16-bit).
 * @retval  Humidity in 0.01 %RH
 */
static uint32_t Svc_Bme280_CompensateHum(int32_t adcH)
{
    int32_t vX1U32r;

    vX1U32r = (tFine - ((int32_t)76800));
    vX1U32r = (((((adcH << 14) - (((int32_t)calibData.digH4) << 20) - (((int32_t)calibData.digH5) * vX1U32r))
                + ((int32_t)16384)) >> 15) * (((((((vX1U32r * ((int32_t)calibData.digH6)) >> 10)
                * (((vX1U32r * ((int32_t)calibData.digH3)) >> 11) + ((int32_t)32768))) >> 10)
                + ((int32_t)2097152)) * ((int32_t)calibData.digH2) + 8192) >> 14));
    vX1U32r = (vX1U32r - (((((vX1U32r >> 15) * (vX1U32r >> 15)) >> 7) * ((int32_t)calibData.digH1)) >> 4));
    vX1U32r = (vX1U32r < 0) ? 0 : vX1U32r;
    vX1U32r = (vX1U32r > 419430400) ? 419430400 : vX1U32r;

    return (uint32_t)(vX1U32r >> 12);
}

/**
 * @brief   Attempt I2C recovery with escalating strategy.
 *          Level 1: Retry communication
 *          Level 2: I2C ReInit → Retry
 *          Level 3: Bus Recovery (SCL toggle) → ReInit → Retry
 * @retval  Svc_Bme280 status
 */
static Svc_Bme280_StatusType Svc_Bme280_RecoverI2c(void)
{
    Svc_Bme280_StatusType retVal = SVC_BME280_COMM_ERROR;
    EcuAbs_I2c_StatusType i2cStatus;
    uint8_t retry;

    /* Level 1: Retry communication */
    for (retry = 0U; retry < SVC_BME280_RETRY_MAX; retry++)
    {
        i2cStatus = EcuAbs_I2c_IsDeviceReady(SVC_BME280_DEV_ADDR);

        if (i2cStatus == ECUABS_I2C_OK)
        {
            retVal = SVC_BME280_OK;
            break;
        }
    }

    /* Level 2: I2C ReInit → Retry */
    if (retVal != SVC_BME280_OK)
    {
        (void)EcuAbs_I2c_DeInit();
        (void)EcuAbs_I2c_ReInit();

        i2cStatus = EcuAbs_I2c_IsDeviceReady(SVC_BME280_DEV_ADDR);

        if (i2cStatus == ECUABS_I2C_OK)
        {
            retVal = SVC_BME280_OK;
        }
    }

    /* Level 3: Bus Recovery (SCL toggle) → ReInit → Retry */
    if (retVal != SVC_BME280_OK)
    {
        (void)EcuAbs_I2c_DeInit();
        (void)EcuAbs_Gpio_ToggleSclPin(SVC_BME280_SCL_TOGGLE_COUNT);
        (void)EcuAbs_I2c_ReInit();

        i2cStatus = EcuAbs_I2c_IsDeviceReady(SVC_BME280_DEV_ADDR);

        if (i2cStatus == ECUABS_I2C_OK)
        {
            retVal = SVC_BME280_OK;
        }
    }

    return retVal;
}
