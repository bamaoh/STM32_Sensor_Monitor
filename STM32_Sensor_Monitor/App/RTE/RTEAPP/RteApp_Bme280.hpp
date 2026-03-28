/*
************************************************************************************************************************
*
* Filename        : RteApp_Bme280.hpp
* Project         : STM32_Sensor_Monitor
* Description     : RTE Application Interface - BME280 Data Exchange.
*                   Provides read/write interface for BME280 measurement data
*                   between Service layer and ASW layer.
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
#include <cstdint>
/*
************************************************************************************************************************
*                                                    Class definition
************************************************************************************************************************
*/
/** @brief RTE BME280 data exchange - static buffer class */
class RteAppBme280
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

    /** @brief RteAppBme280 operation status definition */
    enum class Status : uint8_t
    {
        Ok    = 0x00U,   /*!< No error       */
        Error = 0x01U    /*!< General error   */
    };

/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/

    /**
     * @brief   Write temperature data to RTE buffer.
     * @param   value   Temperature in 0.01 degree C
     * @retval  RteAppBme280::Status
     */
    static Status writeTemperature(int32_t value);

    /**
     * @brief   Write pressure data to RTE buffer.
     * @param   value   Pressure in Pa
     * @retval  RteAppBme280::Status
     */
    static Status writePressure(uint32_t value);

    /**
     * @brief   Write humidity data to RTE buffer.
     * @param   value   Humidity in 0.01 %RH
     * @retval  RteAppBme280::Status
     */
    static Status writeHumidity(uint32_t value);

    /**
     * @brief   Read temperature data from RTE buffer.
     * @param   pValue  Pointer to store temperature in 0.01 degree C
     * @retval  RteAppBme280::Status
     */
    static Status readTemperature(int32_t *pValue);

    /**
     * @brief   Read pressure data from RTE buffer.
     * @param   pValue  Pointer to store pressure in Pa
     * @retval  RteAppBme280::Status
     */
    static Status readPressure(uint32_t *pValue);

    /**
     * @brief   Read humidity data from RTE buffer.
     * @param   pValue  Pointer to store humidity in 0.01 %RH
     * @retval  RteAppBme280::Status
     */
    static Status readHumidity(uint32_t *pValue);

    /**
     * @brief   Write BME280 communication status to RTE buffer.
     * @param   status   0: OK, 1: Error
     * @retval  RteAppBme280::Status
     */
    static Status writeCommStatus(uint8_t status);

    /**
     * @brief   Read BME280 communication status from RTE buffer.
     * @param   pStatus  Pointer to store communication status
     * @retval  RteAppBme280::Status
     */
    static Status readCommStatus(uint8_t *pStatus);

private:
/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

    static int32_t  rteTemperature_;     /*!< Temperature buffer (0.01 degree C)   */
    static uint32_t rtePressure_;        /*!< Pressure buffer (Pa)                 */
    static uint32_t rteHumidity_;        /*!< Humidity buffer (0.01 %RH)           */
    static uint8_t  rteCommStatus_;      /*!< Communication status (0:OK, 1:Error) */
};
