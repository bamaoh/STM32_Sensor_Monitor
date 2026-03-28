/*
************************************************************************************************************************
*
* Filename        : RteApp_Sensor.hpp
* Project         : STM32_Sensor_Monitor
* Description     : RTE Application Interface - Sensor Data Exchange.
*                   Provides read/write interface for filtered sensor data
*                   and diagnostic flags between ASW Sensor and
*                   ASW Manage / UART output modules.
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
/** @brief RTE Sensor data exchange - static buffer class */
class RteAppSensor
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

    /** @brief RteAppSensor operation status definition */
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

    /* Filtered data write (ASW Sensor -> RTE) */

    /**
     * @brief   Write filtered temperature data to RTE buffer.
     * @param   value   Filtered temperature in 0.01 degree C
     * @retval  RteAppSensor::Status
     */
    static Status writeFilteredTemperature(int32_t value);

    /**
     * @brief   Write filtered pressure data to RTE buffer.
     * @param   value   Filtered pressure in Pa
     * @retval  RteAppSensor::Status
     */
    static Status writeFilteredPressure(uint32_t value);

    /**
     * @brief   Write filtered humidity data to RTE buffer.
     * @param   value   Filtered humidity in 0.01 %RH
     * @retval  RteAppSensor::Status
     */
    static Status writeFilteredHumidity(uint32_t value);

    /* Diagnostic write (ASW Sensor -> RTE) */

    /**
     * @brief   Write temperature diagnostic flag to RTE buffer.
     * @param   diag   Temperature diagnostic status
     * @retval  RteAppSensor::Status
     */
    static Status writeTempDiag(uint8_t diag);

    /**
     * @brief   Write pressure diagnostic flag to RTE buffer.
     * @param   diag   Pressure diagnostic status
     * @retval  RteAppSensor::Status
     */
    static Status writePressDiag(uint8_t diag);

    /**
     * @brief   Write humidity diagnostic flag to RTE buffer.
     * @param   diag   Humidity diagnostic status
     * @retval  RteAppSensor::Status
     */
    static Status writeHumDiag(uint8_t diag);

    /* Filtered data read (RTE -> ASW Manage / UART) */

    /**
     * @brief   Read filtered temperature data from RTE buffer.
     * @param   pValue  Pointer to store filtered temperature in 0.01 degree C
     * @retval  RteAppSensor::Status
     */
    static Status readFilteredTemperature(int32_t *pValue);

    /**
     * @brief   Read filtered pressure data from RTE buffer.
     * @param   pValue  Pointer to store filtered pressure in Pa
     * @retval  RteAppSensor::Status
     */
    static Status readFilteredPressure(uint32_t *pValue);

    /**
     * @brief   Read filtered humidity data from RTE buffer.
     * @param   pValue  Pointer to store filtered humidity in 0.01 %RH
     * @retval  RteAppSensor::Status
     */
    static Status readFilteredHumidity(uint32_t *pValue);

    /* Diagnostic read (RTE -> ASW Manage / Diag) */

    /**
     * @brief   Read temperature diagnostic flag from RTE buffer.
     * @param   pDiag  Pointer to store temperature diagnostic status
     * @retval  RteAppSensor::Status
     */
    static Status readTempDiag(uint8_t *pDiag);

    /**
     * @brief   Read pressure diagnostic flag from RTE buffer.
     * @param   pDiag  Pointer to store pressure diagnostic status
     * @retval  RteAppSensor::Status
     */
    static Status readPressDiag(uint8_t *pDiag);

    /**
     * @brief   Read humidity diagnostic flag from RTE buffer.
     * @param   pDiag  Pointer to store humidity diagnostic status
     * @retval  RteAppSensor::Status
     */
    static Status readHumDiag(uint8_t *pDiag);

private:
/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

    /* Filtered sensor data buffers */
    static int32_t  rteFilteredTemp_;    /*!< Filtered temperature (0.01 degree C) */
    static uint32_t rteFilteredPress_;   /*!< Filtered pressure (Pa)               */
    static uint32_t rteFilteredHum_;     /*!< Filtered humidity (0.01 %RH)         */

    /* Diagnostic flag buffers */
    static uint8_t rteTempDiag_;         /*!< Temperature diag  */
    static uint8_t rtePressDiag_;        /*!< Pressure diag     */
    static uint8_t rteHumDiag_;          /*!< Humidity diag     */
};
