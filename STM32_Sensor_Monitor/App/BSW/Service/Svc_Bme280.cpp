/*
************************************************************************************************************************
*
* Filename        : Svc_Bme280.cpp
* Project         : STM32_Sensor_Monitor
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
#include "Svc_Bme280.hpp"
#include "RteApp_Bme280.hpp"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    constexpr uint8_t DEV_ADDR          = 0x77U;    /*!< BME280 7-bit I2C address              */

    /* Register addresses */
    constexpr uint8_t REG_CHIP_ID       = 0xD0U;    /*!< Chip ID register address              */
    constexpr uint8_t REG_RESET         = 0xE0U;    /*!< Soft reset register address           */
    constexpr uint8_t REG_STATUS        = 0xF3U;    /*!< Status register                       */
    constexpr uint8_t REG_CTRL_HUM      = 0xF2U;    /*!< Humidity control register             */
    constexpr uint8_t REG_CTRL_MEAS     = 0xF4U;    /*!< Measurement control register          */
    constexpr uint8_t REG_CONFIG        = 0xF5U;    /*!< Configuration register                */
    constexpr uint8_t REG_CALIB_TP      = 0x88U;    /*!< Calibration data start (temp/press)   */
    constexpr uint8_t REG_CALIB_H2      = 0xE1U;    /*!< Calibration data start (humidity)     */

    /* Constants */
    constexpr uint8_t  CHIP_ID_VAL      = 0x60U;    /*!< Expected Chip ID value                */
    constexpr uint8_t  RESET_CMD        = 0xB6U;    /*!< Soft reset command                    */
    constexpr uint8_t  CALIB_TP_LEN     = 26U;      /*!< Temp/Press calibration data length    */
    constexpr uint8_t  CALIB_H_LEN      = 7U;       /*!< Humidity calibration data length      */
    constexpr uint32_t RESET_WAIT_MS    = 10U;       /*!< Wait time after soft reset (ms)       */

    /* Sensor configuration */
    constexpr uint8_t OSRS_T            = 0x01U;    /*!< Temperature oversampling x1           */
    constexpr uint8_t OSRS_P            = 0x01U;    /*!< Pressure oversampling x1              */
    constexpr uint8_t OSRS_H            = 0x01U;    /*!< Humidity oversampling x1              */
    constexpr uint8_t MODE_NORMAL       = 0x03U;    /*!< Normal mode                           */
    constexpr uint8_t FILTER_OFF        = 0x00U;    /*!< IIR filter off                        */
    constexpr uint8_t STANDBY_1000MS    = 0x05U;    /*!< Standby time 1000ms                   */

    constexpr uint8_t  REG_DATA         = 0xF7U;    /*!< Measurement start register            */
    constexpr uint8_t  DATA_LEN         = 8U;       /*!< Measurement data length               */

    /* Status register bit mask */
    constexpr uint8_t STATUS_MEASURING  = 0x08U;    /*!< Bit[3]: measuring in progress         */

    /* Recovery configuration */
    constexpr uint8_t RETRY_MAX         = 3U;       /*!< Max retry count before escalation     */
    constexpr uint8_t SCL_TOGGLE_COUNT  = 9U;       /*!< SCL toggles for bus recovery          */
}
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Initializes BME280 sensor and verifies Chip ID.
 * @retval  SvcBme280::Status
 */
SvcBme280::Status SvcBme280::init(void)
{
    Status retVal = Status::Ok;
    EcuAbsI2c::Status i2cStatus;
    uint8_t chipId = 0x00U;

    /* Initialize I2C driver */
    i2cStatus = i2c_.init();

    if (i2cStatus != EcuAbsI2c::Status::Ok)
    {
        retVal = Status::Comm_Error;
    }

    /* Check device presence on I2C bus */
    if (retVal == Status::Ok)
    {
        i2cStatus = i2c_.isDeviceReady(DEV_ADDR);
    }

    if (i2cStatus != EcuAbsI2c::Status::Ok)
    {
        retVal = recoverI2c();
    }

    /* Read and verify Chip ID */
    if (retVal == Status::Ok)
    {
        i2cStatus = i2c_.readReg(DEV_ADDR,
                                  REG_CHIP_ID,
                                  &chipId,
                                  1U);

        if (i2cStatus != EcuAbsI2c::Status::Ok)
        {
            retVal = recoverI2c();
        }
        else if (chipId != CHIP_ID_VAL)
        {
            retVal = Status::Chip_Id_Err;
        }
        else
        {
            /* Chip ID verified successfully */
        }
    }

    /* Soft reset to ensure clean state */
    if (retVal == Status::Ok)
    {
        retVal = softReset();

        if (retVal == Status::Comm_Error)
        {
            retVal = recoverI2c();
        }
    }

    /* Read calibration data from sensor */
    if (retVal == Status::Ok)
    {
        retVal = readCalibData();

        if (retVal == Status::Comm_Error)
        {
            retVal = recoverI2c();
        }
    }

    /* Configure oversampling and operating mode */
    if (retVal == Status::Ok)
    {
        retVal = configure();

        if (retVal == Status::Comm_Error)
        {
            retVal = recoverI2c();
        }
    }

    return retVal;
}

/**
 * @brief   Reads measurement data from sensor registers.
 * @param   pData   Pointer to store measurement data
 * @retval  SvcBme280::Status
 */
SvcBme280::Status SvcBme280::readMeasurement(MeasData *pData)
{
    Status retVal = Status::Ok;
    EcuAbsI2c::Status i2cStatus;
    uint8_t dataBuf[DATA_LEN];
    uint8_t status;
    int32_t adcP;
    int32_t adcT;
    int32_t adcH;

    /* Check if sensor is currently measuring (datasheet Section 5.4.4, status register bit[3]) */
    i2cStatus = i2c_.readReg(DEV_ADDR,
                              REG_STATUS,
                              &status,
                              1U);

    if (i2cStatus != EcuAbsI2c::Status::Ok)
    {
        retVal = recoverI2c();
    }
    else if ((status & STATUS_MEASURING) != 0U)
    {
        retVal = Status::Busy;
    }
    else
    {
        /* Sensor is idle, safe to read */
    }

    /* Read measurement data */
    if (retVal == Status::Ok)
    {
        i2cStatus = i2c_.readReg(DEV_ADDR,
                                  REG_DATA,
                                  dataBuf,
                                  DATA_LEN);

        if (i2cStatus != EcuAbsI2c::Status::Ok)
        {
            retVal = recoverI2c();
        }
    }

    if (retVal == Status::Ok)
    {
        /* Parse 20-bit raw pressure */
        adcP = static_cast<int32_t>(static_cast<uint32_t>(dataBuf[0]) << 12U
                                  | static_cast<uint32_t>(dataBuf[1]) << 4U
                                  | static_cast<uint32_t>(dataBuf[2] >> 4U));
        /* Parse 20-bit raw temperature */
        adcT = static_cast<int32_t>(static_cast<uint32_t>(dataBuf[3]) << 12U
                                  | static_cast<uint32_t>(dataBuf[4]) << 4U
                                  | static_cast<uint32_t>(dataBuf[5] >> 4U));
        /* Parse 16-bit raw humidity */
        adcH = static_cast<int32_t>(static_cast<uint32_t>(dataBuf[6]) << 8U
                                  | static_cast<uint32_t>(dataBuf[7]));

        /* Compensate: temperature must be first (tFine_ is needed by pressure/humidity) */
        pData->temperature = compensateTemp(adcT);
        /* Pressure: Q24.8 format (datasheet 4.2.3) -> right shift 8 to get Pa */
        pData->pressure    = compensatePress(adcP) >> 8U;
        /* Humidity: Q22.10 format (datasheet 4.2.3) -> *100 / 1024 to get 0.01 %RH */
        pData->humidity    = (compensateHum(adcH) * 100U) >> 10U;

        /* Write compensated data to RTE buffer */
        (void)RteAppBme280::writeTemperature(pData->temperature);
        (void)RteAppBme280::writePressure(pData->pressure);
        (void)RteAppBme280::writeHumidity(pData->humidity);
    }

    /* Write communication status to RTE (0: OK, 1: Error) */
    (void)RteAppBme280::writeCommStatus((retVal == Status::Comm_Error) ? 1U : 0U);

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/

/**
 * @brief   Performs soft reset and waits for sensor startup.
 * @retval  SvcBme280::Status
 */
SvcBme280::Status SvcBme280::softReset(void)
{
    Status retVal = Status::Ok;
    EcuAbsI2c::Status i2cStatus;
    uint8_t resetCmd = RESET_CMD;

    i2cStatus = i2c_.writeReg(DEV_ADDR,
                               REG_RESET,
                               &resetCmd,
                               1U);

    if (i2cStatus != EcuAbsI2c::Status::Ok)
    {
        retVal = Status::Comm_Error;
    }
    else
    {
        /* Wait for sensor startup after reset */
        HAL_Delay(RESET_WAIT_MS);
    }

    return retVal;
}

/**
 * @brief   Reads calibration data from sensor registers.
 * @retval  SvcBme280::Status
 */
SvcBme280::Status SvcBme280::readCalibData(void)
{
    Status retVal = Status::Ok;
    EcuAbsI2c::Status i2cStatus;
    uint8_t tpBuf[CALIB_TP_LEN];
    uint8_t hBuf[CALIB_H_LEN];

    /* Read temperature, pressure calibration and H1 (0x88 ~ 0xA1, 26 bytes) */
    /* tpBuf[0~23]: dig_T1~T3, dig_P1~P9 / tpBuf[24]: reserved / tpBuf[25]: dig_H1 */
    i2cStatus = i2c_.readReg(DEV_ADDR,
                              REG_CALIB_TP,
                              tpBuf,
                              CALIB_TP_LEN);

    if (i2cStatus != EcuAbsI2c::Status::Ok)
    {
        retVal = Status::Comm_Error;
    }

    if (retVal == Status::Ok)
    {
        /* Parse temperature calibration */
        calibData_.digT1 = static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[1]) << 8U) | static_cast<uint16_t>(tpBuf[0]);
        calibData_.digT2 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[3]) << 8U) | static_cast<uint16_t>(tpBuf[2]));
        calibData_.digT3 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[5]) << 8U) | static_cast<uint16_t>(tpBuf[4]));

        /* Parse pressure calibration */
        calibData_.digP1 = static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[7]) << 8U)  | static_cast<uint16_t>(tpBuf[6]);
        calibData_.digP2 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[9]) << 8U)  | static_cast<uint16_t>(tpBuf[8]));
        calibData_.digP3 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[11]) << 8U) | static_cast<uint16_t>(tpBuf[10]));
        calibData_.digP4 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[13]) << 8U) | static_cast<uint16_t>(tpBuf[12]));
        calibData_.digP5 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[15]) << 8U) | static_cast<uint16_t>(tpBuf[14]));
        calibData_.digP6 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[17]) << 8U) | static_cast<uint16_t>(tpBuf[16]));
        calibData_.digP7 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[19]) << 8U) | static_cast<uint16_t>(tpBuf[18]));
        calibData_.digP8 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[21]) << 8U) | static_cast<uint16_t>(tpBuf[20]));
        calibData_.digP9 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(tpBuf[23]) << 8U) | static_cast<uint16_t>(tpBuf[22]));

        /* Parse humidity calibration H1 (tpBuf[25] = 0xA1) */
        calibData_.digH1 = tpBuf[25];
    }

    /* Read humidity calibration H2~H6 (0xE1 ~ 0xE7) */
    if (retVal == Status::Ok)
    {
        i2cStatus = i2c_.readReg(DEV_ADDR,
                                  REG_CALIB_H2,
                                  hBuf,
                                  CALIB_H_LEN);

        if (i2cStatus != EcuAbsI2c::Status::Ok)
        {
            retVal = Status::Comm_Error;
        }
    }

    if (retVal == Status::Ok)
    {
        /* Parse humidity calibration */
        calibData_.digH2 = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(hBuf[1]) << 8U) | static_cast<uint16_t>(hBuf[0]));
        calibData_.digH3 = hBuf[2];
        calibData_.digH4 = static_cast<int16_t>((static_cast<int16_t>(hBuf[3]) << 4) | (static_cast<int16_t>(hBuf[4]) & 0x0F));
        calibData_.digH5 = static_cast<int16_t>((static_cast<int16_t>(hBuf[5]) << 4) | (static_cast<int16_t>(hBuf[4]) >> 4));
        calibData_.digH6 = static_cast<int8_t>(hBuf[6]);
    }

    return retVal;
}

/**
 * @brief   Configures oversampling, filter, and operating mode.
 * @retval  SvcBme280::Status
 */
SvcBme280::Status SvcBme280::configure(void)
{
    Status retVal = Status::Ok;
    EcuAbsI2c::Status i2cStatus;
    uint8_t regVal;

    /* Set humidity oversampling (must be written before ctrl_meas) */
    regVal = OSRS_H;

    i2cStatus = i2c_.writeReg(DEV_ADDR,
                               REG_CTRL_HUM,
                               &regVal,
                               1U);

    if (i2cStatus != EcuAbsI2c::Status::Ok)
    {
        retVal = Status::Comm_Error;
    }

    /* Set config register (standby time, IIR filter) */
    if (retVal == Status::Ok)
    {
        regVal = static_cast<uint8_t>((STANDBY_1000MS << 5U) | (FILTER_OFF << 2U));

        i2cStatus = i2c_.writeReg(DEV_ADDR,
                                   REG_CONFIG,
                                   &regVal,
                                   1U);

        if (i2cStatus != EcuAbsI2c::Status::Ok)
        {
            retVal = Status::Comm_Error;
        }
    }

    /* Set temp/press oversampling and normal mode (activates ctrl_hum setting) */
    if (retVal == Status::Ok)
    {
        regVal = static_cast<uint8_t>((OSRS_T << 5U) | (OSRS_P << 2U) | MODE_NORMAL);

        i2cStatus = i2c_.writeReg(DEV_ADDR,
                                   REG_CTRL_MEAS,
                                   &regVal,
                                   1U);

        if (i2cStatus != EcuAbsI2c::Status::Ok)
        {
            retVal = Status::Comm_Error;
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
int32_t SvcBme280::compensateTemp(int32_t adcT)
{
    int32_t var1;
    int32_t var2;
    int32_t temperature;

    var1 = ((((adcT >> 3) - (static_cast<int32_t>(calibData_.digT1) << 1))) * (static_cast<int32_t>(calibData_.digT2))) >> 11;
    var2 = (((((adcT >> 4) - (static_cast<int32_t>(calibData_.digT1))) * ((adcT >> 4) - (static_cast<int32_t>(calibData_.digT1)))) >> 12)
            * (static_cast<int32_t>(calibData_.digT3))) >> 14;

    tFine_ = var1 + var2;
    temperature = (tFine_ * 5 + 128) >> 8;

    return temperature;
}

/**
 * @brief   Compensate raw pressure value to physical value.
 *          Formula from BME280 datasheet Section 4.2.3.
 * @param   adcP  Raw pressure ADC value (20-bit).
 * @retval  Pressure in Pa
 */
uint32_t SvcBme280::compensatePress(int32_t adcP)
{
    int64_t var1;
    int64_t var2;
    int64_t p;
    uint32_t pressure = 0U;

    var1 = (static_cast<int64_t>(tFine_)) - 128000;
    var2 = var1 * var1 * static_cast<int64_t>(calibData_.digP6);
    var2 = var2 + ((var1 * static_cast<int64_t>(calibData_.digP5)) << 17);
    var2 = var2 + ((static_cast<int64_t>(calibData_.digP4)) << 35);
    var1 = ((var1 * var1 * static_cast<int64_t>(calibData_.digP3)) >> 8) + ((var1 * static_cast<int64_t>(calibData_.digP2)) << 12);
    var1 = (((static_cast<int64_t>(1) << 47) + var1)) * (static_cast<int64_t>(calibData_.digP1)) >> 33;

    if (var1 != 0)
    {
        p = 1048576 - static_cast<int64_t>(adcP);
        p = (((p << 31) - var2) * 3125) / var1;
        var1 = (static_cast<int64_t>(calibData_.digP9) * (p >> 13) * (p >> 13)) >> 25;
        var2 = (static_cast<int64_t>(calibData_.digP8) * p) >> 19;

        p = ((p + var1 + var2) >> 8) + ((static_cast<int64_t>(calibData_.digP7)) << 4);
        pressure = static_cast<uint32_t>(p);
    }

    return pressure;
}

/**
 * @brief   Compensate raw humidity value to physical value.
 *          Formula from BME280 datasheet Section 4.2.3.
 * @param   adcH  Raw humidity ADC value (16-bit).
 * @retval  Humidity in 0.01 %RH
 */
uint32_t SvcBme280::compensateHum(int32_t adcH)
{
    int32_t vX1U32r;

    vX1U32r = (tFine_ - (static_cast<int32_t>(76800)));
    vX1U32r = (((((adcH << 14) - ((static_cast<int32_t>(calibData_.digH4)) << 20) - ((static_cast<int32_t>(calibData_.digH5)) * vX1U32r))
                + (static_cast<int32_t>(16384))) >> 15) * (((((((vX1U32r * (static_cast<int32_t>(calibData_.digH6))) >> 10)
                * (((vX1U32r * (static_cast<int32_t>(calibData_.digH3))) >> 11) + (static_cast<int32_t>(32768)))) >> 10)
                + (static_cast<int32_t>(2097152))) * (static_cast<int32_t>(calibData_.digH2)) + 8192) >> 14));
    vX1U32r = (vX1U32r - (((((vX1U32r >> 15) * (vX1U32r >> 15)) >> 7) * (static_cast<int32_t>(calibData_.digH1))) >> 4));
    vX1U32r = (vX1U32r < 0) ? 0 : vX1U32r;
    vX1U32r = (vX1U32r > 419430400) ? 419430400 : vX1U32r;

    return static_cast<uint32_t>(vX1U32r >> 12);
}

/**
 * @brief   Attempt I2C recovery with escalating strategy.
 *          Level 1: Retry communication
 *          Level 2: I2C ReInit -> Retry
 *          Level 3: Bus Recovery (SCL toggle) -> ReInit -> Retry
 * @retval  SvcBme280::Status
 */
SvcBme280::Status SvcBme280::recoverI2c(void)
{
    Status retVal = Status::Comm_Error;
    EcuAbsI2c::Status i2cStatus;
    uint8_t retry;

    /* Level 1: Retry communication */
    for (retry = 0U; retry < RETRY_MAX; retry++)
    {
        i2cStatus = i2c_.isDeviceReady(DEV_ADDR);

        if (i2cStatus == EcuAbsI2c::Status::Ok)
        {
            retVal = Status::Ok;
            break;
        }
    }

    /* Level 2: I2C ReInit -> Retry */
    if (retVal != Status::Ok)
    {
        (void)i2c_.deInit();
        (void)i2c_.reInit();

        i2cStatus = i2c_.isDeviceReady(DEV_ADDR);

        if (i2cStatus == EcuAbsI2c::Status::Ok)
        {
            retVal = Status::Ok;
        }
    }

    /* Level 3: Bus Recovery (SCL toggle) -> ReInit -> Retry */
    if (retVal != Status::Ok)
    {
        (void)i2c_.deInit();
        (void)gpio_.toggleSclPin(SCL_TOGGLE_COUNT);
        (void)i2c_.reInit();

        i2cStatus = i2c_.isDeviceReady(DEV_ADDR);

        if (i2cStatus == EcuAbsI2c::Status::Ok)
        {
            retVal = Status::Ok;
        }
    }

    return retVal;
}
