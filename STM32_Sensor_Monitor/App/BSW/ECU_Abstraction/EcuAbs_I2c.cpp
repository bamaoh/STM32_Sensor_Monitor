/*
************************************************************************************************************************
*
* Filename        : EcuAbs_I2c.cpp
* Project         : STM32_Sensor_Monitor
* Description     : ECU Abstraction Layer - I2C Interface Implementation.
*                   Wraps STM32 HAL I2C functions into a hardware-independent
*                   interface. Upper layers use this module without direct
*                   HAL dependency.
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
#include "EcuAbs_I2c.hpp"

extern "C"
{
    extern I2C_HandleTypeDef hi2c1;    /*!< Defined in Core/Src/i2c.c */
}
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    constexpr uint32_t TIMEOUT_MS = 100U;    /*!< Timeout duration in milliseconds          */
    constexpr uint32_t READY_TRIALS = 3U;    /*!< Number of trials for device ready check    */
    constexpr uint32_t ADDR_SHIFT = 1U;      /*!< Bit shift for 7-bit to HAL address format  */
}
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Initializes the I2C abstraction layer and assigns
 *          the HAL I2C handle for subsequent operations.
 * @retval  EcuAbs I2C status
 */
EcuAbsI2c::Status EcuAbsI2c::init(void)
{
    Status retVal = Status::Ok;

    pHandle_ = &hi2c1;

    /* Verify that HAL I2C has been initialized by CubeMX */
    if (pHandle_->State == HAL_I2C_STATE_RESET)
    {
        pHandle_ = nullptr;
        retVal = Status::Error;
    }

    return retVal;
}

/**
 * @brief   Checks if target device is ready for communication.
 * @param   devAddr Target device 7-bit address (not shifted)
 * @retval  EcuAbs I2C status
 */
EcuAbsI2c::Status EcuAbsI2c::isDeviceReady(uint8_t devAddr)
{
    Status retVal = Status::Error;
    HAL_StatusTypeDef halStatus;
    uint16_t shiftedAddr;

    if (pHandle_ != nullptr)
    {
        shiftedAddr = (uint16_t)((uint16_t)devAddr << ADDR_SHIFT);

        halStatus = HAL_I2C_IsDeviceReady(pHandle_,
                                          shiftedAddr,
                                          READY_TRIALS,
                                          TIMEOUT_MS);

        retVal = convertStatus(halStatus);
    }

    return retVal;
}

/**
 * @brief   Read an amount of data in blocking mode from a specific memory address.
 * @param   devAddr Target device 7-bit address (not shifted)
 * @param   regAddr Internal memory address
 * @param   pData   Pointer to data buffer
 * @param   length  Amount of data to be read
 * @retval  EcuAbs I2C status
 */
EcuAbsI2c::Status EcuAbsI2c::readReg(uint8_t devAddr,
                                      uint8_t regAddr,
                                      uint8_t *pData,
                                      uint16_t length)
{
    Status retVal = Status::Param;
    HAL_StatusTypeDef halStatus;
    uint16_t shiftedAddr;

    if ((pData != nullptr) && (length > 0U) && (pHandle_ != nullptr))
    {
        shiftedAddr = (uint16_t)((uint16_t)devAddr << ADDR_SHIFT);

        halStatus = HAL_I2C_Mem_Read(pHandle_,
                                     shiftedAddr,
                                     (uint16_t)regAddr,
                                     I2C_MEMADD_SIZE_8BIT,
                                     pData,
                                     length,
                                     TIMEOUT_MS);

        retVal = convertStatus(halStatus);
    }

    return retVal;
}

/**
 * @brief   Write an amount of data in blocking mode to a specific memory address.
 * @param   devAddr Target device 7-bit address (not shifted)
 * @param   regAddr Internal memory address
 * @param   pData   Pointer to data buffer
 * @param   length  Amount of data to be sent
 * @retval  EcuAbs I2C status
 */
EcuAbsI2c::Status EcuAbsI2c::writeReg(uint8_t devAddr,
                                       uint8_t regAddr,
                                       const uint8_t *pData,
                                       uint16_t length)
{
    Status retVal = Status::Param;
    HAL_StatusTypeDef halStatus;
    uint16_t shiftedAddr;

    if ((pData != nullptr) && (length > 0U) && (pHandle_ != nullptr))
    {
        shiftedAddr = (uint16_t)((uint16_t)devAddr << ADDR_SHIFT);

        /* MISRA deviation: HAL API requires non-const pointer */
        halStatus = HAL_I2C_Mem_Write(pHandle_,
                                      shiftedAddr,
                                      (uint16_t)regAddr,
                                      I2C_MEMADD_SIZE_8BIT,
                                      const_cast<uint8_t *>(pData),
                                      length,
                                      TIMEOUT_MS);

        retVal = convertStatus(halStatus);
    }

    return retVal;
}

/**
 * @brief   Deinitialize the I2C peripheral.
 * @retval  EcuAbs I2C status
 */
EcuAbsI2c::Status EcuAbsI2c::deInit(void)
{
    Status retVal = Status::Ok;

    if (pHandle_ != nullptr)
    {
        if (HAL_I2C_DeInit(pHandle_) != HAL_OK)
        {
            retVal = Status::Error;
        }
    }
    else
    {
        retVal = Status::Error;
    }

    return retVal;
}

/**
 * @brief   Reinitialize the I2C peripheral.
 * @retval  EcuAbs I2C status
 */
EcuAbsI2c::Status EcuAbsI2c::reInit(void)
{
    Status retVal = Status::Ok;

    if (pHandle_ != nullptr)
    {
        if (HAL_I2C_Init(pHandle_) != HAL_OK)
        {
            retVal = Status::Error;
        }
    }
    else
    {
        retVal = Status::Error;
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/

/**
 * @brief   Converts HAL_StatusTypeDef to EcuAbsI2c::Status.
 * @param   halStatus HAL return status
 * @retval  EcuAbs I2C status
 */
EcuAbsI2c::Status EcuAbsI2c::convertStatus(HAL_StatusTypeDef halStatus)
{
    Status retVal;

    switch (halStatus)
    {
    case HAL_OK:
        retVal = Status::Ok;
        break;

    case HAL_BUSY:
        retVal = Status::Busy;
        break;

    case HAL_TIMEOUT:
        retVal = Status::Timeout;
        break;

    case HAL_ERROR:
        retVal = Status::Error;
        break;

    default:
        retVal = Status::Error;
        break;
    }

    return retVal;
}
