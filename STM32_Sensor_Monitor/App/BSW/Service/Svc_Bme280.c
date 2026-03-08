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
#define SVC_BME280_STANDBY_1000MS   (0x05U)    /*!< Standby time 1000ms                  */
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
        retVal = SVC_BME280_COMM_ERROR;
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
            retVal = SVC_BME280_COMM_ERROR;
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
    }

    /* Read calibration data from sensor */
    if (retVal == SVC_BME280_OK)
    {
        retVal = Svc_Bme280_ReadCalibData();
    }

    /* Configure oversampling and operating mode */
    if (retVal == SVC_BME280_OK)
    {
        retVal = Svc_Bme280_Configure();
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
