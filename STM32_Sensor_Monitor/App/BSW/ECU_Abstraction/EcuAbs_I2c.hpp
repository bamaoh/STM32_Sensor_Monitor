/*
************************************************************************************************************************
*
* Filename        : EcuAbs_I2c.hpp
* Project         : STM32_Sensor_Monitor
* Description     : ECU Abstraction Layer - I2C Interface.
*                   Provides hardware-independent I2C communication interface.
*                   This layer wraps the HAL I2C driver and has no knowledge
*                   of specific external devices.
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
extern "C"
{
    #include "stm32f4xx_hal.h"
}
#include <cstdint>
/*
************************************************************************************************************************
*                                                    Class definition
************************************************************************************************************************
*/
/** @brief ECU Abstraction Layer - I2C communication class */
class EcuAbsI2c
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/
    /** @brief EcuAbs I2C operation status definition */
    enum class Status : uint8_t
    {
        Ok      = 0x00U,   /*!< No error                          */
        Error   = 0x01U,   /*!< Communication error               */
        Busy    = 0x02U,   /*!< Peripheral is busy                */
        Timeout = 0x03U,   /*!< Timeout error                     */
        Param   = 0x04U    /*!< Invalid parameter                 */
    };
/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/
    /**
     * @brief   Initializes the I2C abstraction layer and assigns
     *          the HAL I2C handle for subsequent operations.
     * @retval  EcuAbs I2C status
     */
    Status init(void);

    /**
     * @brief   Checks if target device is ready for communication.
     * @param   devAddr Target device 7-bit address (not shifted)
     * @retval  EcuAbs I2C status
     */
    Status isDeviceReady(uint8_t devAddr);

    /**
     * @brief   Read an amount of data in blocking mode from a specific memory address.
     * @param   devAddr Target device 7-bit address (not shifted)
     * @param   regAddr Internal memory address
     * @param   pData   Pointer to data buffer
     * @param   length  Amount of data to be read
     * @retval  EcuAbs I2C status
     */
    Status readReg(uint8_t devAddr,
                   uint8_t regAddr,
                   uint8_t *pData,
                   uint16_t length);

    /**
     * @brief   Write an amount of data in blocking mode to a specific memory address.
     * @param   devAddr Target device 7-bit address (not shifted)
     * @param   regAddr Internal memory address
     * @param   pData   Pointer to data buffer
     * @param   length  Amount of data to be sent
     * @retval  EcuAbs I2C status
     */
    Status writeReg(uint8_t devAddr,
                    uint8_t regAddr,
                    const uint8_t *pData,
                    uint16_t length);

    /**
     * @brief   Deinitialize the I2C peripheral.
     * @retval  EcuAbs I2C status
     */
    Status deInit(void);

    /**
     * @brief   Reinitialize the I2C peripheral.
     * @retval  EcuAbs I2C status
     */
    Status reInit(void);
private:
/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/
    I2C_HandleTypeDef *pHandle_ = nullptr;      /*!< Pointer to I2C handle    */
/*
********************************************************************************************************************
*                                                  Private methods
********************************************************************************************************************
*/
    Status convertStatus(HAL_StatusTypeDef halStatus);
};
