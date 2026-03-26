/*
************************************************************************************************************************
*
* Filename        : Svc_Bme280.hpp
* Project         : STM32_Sensor_Monitor
* Description     : Service Layer - BME280 Sensor Driver.
*                   Provides BME280 initialization, configuration, and
*                   periodic measurement interface. This layer uses
*                   EcuAbsI2c for hardware communication.
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
#include "EcuAbs_I2c.hpp"
#include "EcuAbs_Gpio.hpp"
#include <cstdint>
/*
************************************************************************************************************************
*                                                    Class definition
************************************************************************************************************************
*/
/** @brief BME280 Service - sensor measurement and configuration class */
class SvcBme280
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

    /** @brief SvcBme280 operation status definition */
    enum class Status : uint8_t
    {
        Ok           = 0x00U,   /*!< No error                          */
        Error        = 0x01U,   /*!< General error                     */
        Comm_Error   = 0x02U,   /*!< I2C communication error           */
        Chip_Id_Err  = 0x03U,   /*!< Chip ID mismatch                  */
        Busy         = 0x04U    /*!< Sensor is measuring               */
    };

    /** @brief BME280 measurement data structure */
    struct MeasData
    {
        int32_t  temperature;    /*!< Temperature in 0.01 degree C  */
        uint32_t pressure;       /*!< Pressure in Pa                */
        uint32_t humidity;       /*!< Humidity in 0.01 %RH          */
    };

/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/

    /**
     * @brief   Initializes BME280 sensor and verifies Chip ID.
     * @retval  SvcBme280::Status
     */
    Status init(void);

    /**
     * @brief   Read measurement data from BME280 sensor and store the
     *          temperature, pressure, humidity data into pData.
     * @param   pData   Pointer to store measurement data
     * @retval  SvcBme280::Status
     */
    Status readMeasurement(MeasData *pData);

private:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

    /** @brief BME280 calibration data structure */
    struct CalibData
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
    };

/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

    EcuAbsI2c i2c_;                          /*!< I2C driver instance               */
    EcuAbsGpio gpio_;                        /*!< GPIO driver instance              */
    CalibData calibData_ = {};               /*!< Stored calibration parameters     */
    int32_t tFine_ = 0;                      /*!< Fine temperature (shared across compensation) */

/*
********************************************************************************************************************
*                                                  Private methods
********************************************************************************************************************
*/

    /**
     * @brief   Performs soft reset and waits for sensor startup.
     * @retval  SvcBme280::Status
     */
    Status softReset(void);

    /**
     * @brief   Reads calibration data from sensor registers.
     * @retval  SvcBme280::Status
     */
    Status readCalibData(void);

    /**
     * @brief   Configures oversampling, filter, and operating mode.
     * @retval  SvcBme280::Status
     */
    Status configure(void);

    /**
     * @brief   Compensate raw temperature value to physical value.
     * @param   adcT  Raw temperature ADC value (20-bit).
     * @retval  Temperature in 0.01 degree C
     */
    int32_t compensateTemp(int32_t adcT);

    /**
     * @brief   Compensate raw pressure value to physical value.
     * @param   adcP  Raw pressure ADC value (20-bit).
     * @retval  Pressure in Pa
     */
    uint32_t compensatePress(int32_t adcP);

    /**
     * @brief   Compensate raw humidity value to physical value.
     * @param   adcH  Raw humidity ADC value (16-bit).
     * @retval  Humidity in 0.01 %RH
     */
    uint32_t compensateHum(int32_t adcH);

    /**
     * @brief   Attempt I2C recovery with escalating strategy.
     *          Level 1: Retry communication
     *          Level 2: I2C ReInit -> Retry
     *          Level 3: Bus Recovery (SCL toggle) -> ReInit -> Retry
     * @retval  SvcBme280::Status
     */
    Status recoverI2c(void);
};
