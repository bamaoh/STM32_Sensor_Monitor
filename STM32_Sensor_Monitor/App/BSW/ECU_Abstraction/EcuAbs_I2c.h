/*
************************************************************************************************************************
*
* Filename        : EcuAbs_I2c.h
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/25
* Description     : ECU Abstraction Layer - I2C Interface.
*                   Provides hardware-independent I2C communication interface.
*                   This layer wraps the HAL I2C driver and has no knowledge
*                   of specific external devices.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef ECUABS_I2C_H
#define ECUABS_I2C_H
/*
************************************************************************************************************************
*                                                  Include header files
************************************************************************************************************************
*/
#include <stdint.h>
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                                         Typedefs
************************************************************************************************************************
*/

/** @brief EcuAbs I2C operation status definition */
typedef enum
{
    ECUABS_I2C_OK       = 0x00U,   /*!< No error                          */
    ECUABS_I2C_ERROR    = 0x01U,   /*!< Communication error               */
    ECUABS_I2C_BUSY     = 0x02U,   /*!< Bus is busy                       */
    ECUABS_I2C_TIMEOUT  = 0x03U,   /*!< Timeout error                     */
    ECUABS_I2C_PARAM    = 0x04U    /*!< Invalid parameter                 */
} EcuAbs_I2c_StatusType;
/*
************************************************************************************************************************
*                                                    Exported variables
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                               Exported function prototypes
************************************************************************************************************************
*/

/**
 * @brief   Initializes the I2C abstraction layer and assigns
 *          the HAL I2C handle for subsequent operations.
 * @retval  EcuAbs I2C status
 */
EcuAbs_I2c_StatusType EcuAbs_I2c_Init(void);

/**
 * @brief   Checks if target device is ready for communication.
 * @param   devAddr Target device 7-bit address (not shifted)
 * @retval  EcuAbs I2C status
 */
EcuAbs_I2c_StatusType EcuAbs_I2c_IsDeviceReady(uint8_t devAddr);

/**
 * @brief   Read an amount of data in blocking mode from a specific memory address.
 * @param   devAddr Target device 7-bit address (not shifted)
 * @param   regAddr Internal memory address
 * @param   pData   Pointer to data buffer
 * @param   length  Amount of data to be read
 * @retval  EcuAbs I2C status
 */
EcuAbs_I2c_StatusType EcuAbs_I2c_ReadReg(uint8_t devAddr,
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
EcuAbs_I2c_StatusType EcuAbs_I2c_WriteReg(uint8_t devAddr,
                                           uint8_t regAddr,
                                           const uint8_t *pData,
                                           uint16_t length);

#endif /* ECUABS_I2C_H */
