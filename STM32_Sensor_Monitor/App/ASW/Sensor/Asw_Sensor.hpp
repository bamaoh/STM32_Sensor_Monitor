/*
************************************************************************************************************************
*
* Filename        : Asw_Sensor.hpp
* Project         : STM32_Sensor_Monitor
* Description     : ASW Sensor Module.
*                   Reads BME280 measurement data from RTE, performs range
*                   validation and rate-of-change filtering. Provides
*                   validated sensor data and diagnostic flags.
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
/** @brief ASW Sensor - range validation and rate-of-change filter class */
class AswSensor
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/

    /**
     * @brief   Periodic sensor data processing.
     *          Reads from RTE, validates range, applies rate-of-change filter.
     *          Writes filtered data and diagnostic flags to RTE.
     * @retval  None
     */
    void mainFunction(void);

private:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

    /** @brief Sensor diagnostic flag definition */
    enum class Diag : uint8_t
    {
        Ok         = 0x00U,   /*!< No fault                              */
        RangeError = 0x01U,   /*!< Measurement out of valid range        */
        RateError  = 0x02U,   /*!< Abnormal rate of change detected      */
        CommError  = 0x03U    /*!< RTE read failed                       */
    };

    /** @brief Validated sensor data structure */
    struct SensorData
    {
        int32_t  temperature = 0;    /*!< Filtered temperature in 0.01 degree C  */
        uint32_t pressure    = 0U;   /*!< Filtered pressure in Pa                */
        uint32_t humidity    = 0U;   /*!< Filtered humidity in 0.01 %RH          */
    };

    /** @brief Sensor diagnostic status structure */
    struct DiagStatus
    {
        Diag tempDiag  = Diag::Ok;   /*!< Temperature diagnostic flag   */
        Diag pressDiag = Diag::Ok;   /*!< Pressure diagnostic flag      */
        Diag humDiag   = Diag::Ok;   /*!< Humidity diagnostic flag      */
    };

/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

    SensorData sensorData_;                  /*!< Validated sensor data           */
    DiagStatus diagStatus_;                  /*!< Diagnostic flags                */
    uint8_t stabilizeCount_ = 3U;            /*!< Remaining cycles before filter  */

/*
********************************************************************************************************************
*                                                  Private methods
********************************************************************************************************************
*/

    /**
     * @brief   Validate temperature range and apply rate-of-change filter.
     * @param   rawValue   Raw temperature from RTE (0.01 degree C)
     * @retval  None
     */
    void processTemp(int32_t rawValue);

    /**
     * @brief   Validate pressure range and apply rate-of-change filter.
     * @param   rawValue   Raw pressure from RTE (Pa)
     * @retval  None
     */
    void processPress(uint32_t rawValue);

    /**
     * @brief   Validate humidity range and apply rate-of-change filter.
     * @param   rawValue   Raw humidity from RTE (0.01 %RH)
     * @retval  None
     */
    void processHum(uint32_t rawValue);
};
