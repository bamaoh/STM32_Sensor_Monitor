/*
************************************************************************************************************************
*
* Filename        : EcuAbs_I2c.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/25
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
#include "EcuAbs_I2c.h"
#include "main.h"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

#define ECUABS_I2C_TIMEOUT_MS (100U)    /*!< Timeout duration in milliseconds          */
#define ECUABS_I2C_READY_TRIALS (3U)    /*!< Number of trials for device ready check    */
#define ECUABS_I2C_ADDR_SHIFT (1U)      /*!< Bit shift for 7-bit to HAL address format  */
/*
************************************************************************************************************************
*                                                         Typedefs
************************************************************************************************************************
*/
/*
************************************************************************************************************************
*                                                    Private variables
************************************************************************************************************************
*/

static I2C_HandleTypeDef *pI2cHandle = NULL;    /*!< Pointer to I2C handle  */
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/

static EcuAbs_I2c_StatusType EcuAbs_I2c_ConvertStatus(HAL_StatusTypeDef halStatus);
/*
************************************************************************************************************************
*                                                    Public functions
************************************************************************************************************************
*/

/**
 * @brief   Initializes the I2C abstraction layer and assigns
 *          the HAL I2C handle for subsequent operations.
 * @retval  EcuAbs I2C status
 */
EcuAbs_I2c_StatusType EcuAbs_I2c_Init(void)
{
    EcuAbs_I2c_StatusType retVal = ECUABS_I2C_OK;

    pI2cHandle = &hi2c1;

    /* Verify that HAL I2C has been initialized by CubeMX */
    if (pI2cHandle->State == HAL_I2C_STATE_RESET)
    {
        retVal = ECUABS_I2C_ERROR;
    }

    return retVal;
}

/**
 * @brief   Checks if target device is ready for communication.
 * @param   devAddr Target device 7-bit address (not shifted)
 * @retval  EcuAbs I2C status
 */
EcuAbs_I2c_StatusType EcuAbs_I2c_IsDeviceReady(uint8_t devAddr)
{
    EcuAbs_I2c_StatusType retVal = ECUABS_I2C_ERROR;
    HAL_StatusTypeDef halStatus;
    uint16_t shiftedAddr;

    if (pI2cHandle != NULL)
    {
        shiftedAddr = (uint16_t)((uint16_t)devAddr << ECUABS_I2C_ADDR_SHIFT);

        halStatus = HAL_I2C_IsDeviceReady(pI2cHandle,
                                          shiftedAddr,
                                          ECUABS_I2C_READY_TRIALS,
                                          ECUABS_I2C_TIMEOUT_MS);

        retVal = EcuAbs_I2c_ConvertStatus(halStatus);
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
EcuAbs_I2c_StatusType EcuAbs_I2c_ReadReg(uint8_t devAddr,
                                         uint8_t regAddr,
                                         uint8_t *pData,
                                         uint16_t length)
{
    EcuAbs_I2c_StatusType retVal = ECUABS_I2C_PARAM;
    HAL_StatusTypeDef halStatus;
    uint16_t shiftedAddr;

    if ((pData != NULL) && (length > 0U) && (pI2cHandle != NULL))
    {
        shiftedAddr = (uint16_t)((uint16_t)devAddr << ECUABS_I2C_ADDR_SHIFT);

        halStatus = HAL_I2C_Mem_Read(pI2cHandle,
                                     shiftedAddr,
                                     (uint16_t)regAddr,
                                     I2C_MEMADD_SIZE_8BIT,
                                     pData,
                                     length,
                                     ECUABS_I2C_TIMEOUT_MS);

        retVal = EcuAbs_I2c_ConvertStatus(halStatus);
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
EcuAbs_I2c_StatusType EcuAbs_I2c_WriteReg(uint8_t devAddr,
                                          uint8_t regAddr,
                                          const uint8_t *pData,
                                          uint16_t length)
{
    EcuAbs_I2c_StatusType retVal = ECUABS_I2C_PARAM;
    HAL_StatusTypeDef halStatus;
    uint16_t shiftedAddr;

    if ((pData != NULL) && (length > 0U) && (pI2cHandle != NULL))
    {
        shiftedAddr = (uint16_t)((uint16_t)devAddr << ECUABS_I2C_ADDR_SHIFT);

        /* MISRA deviation: HAL API requires non-const pointer */
        halStatus = HAL_I2C_Mem_Write(pI2cHandle,
                                      shiftedAddr,
                                      (uint16_t)regAddr,
                                      I2C_MEMADD_SIZE_8BIT,
                                      (uint8_t *)pData,
                                      length,
                                      ECUABS_I2C_TIMEOUT_MS);

        retVal = EcuAbs_I2c_ConvertStatus(halStatus);
    }

    return retVal;
}
/**
 * @brief   Deinitialize the I2C peripheral.
 * @retval  EcuAbs I2C status
 */
EcuAbs_I2c_StatusType EcuAbs_I2c_DeInit(void)
{
    EcuAbs_I2c_StatusType retVal = ECUABS_I2C_OK;

    if (pI2cHandle != NULL)
    {
        if (HAL_I2C_DeInit(pI2cHandle) != HAL_OK)
        {
            retVal = ECUABS_I2C_ERROR;
        }
    }
    else
    {
        retVal = ECUABS_I2C_ERROR;
    }

    return retVal;
}

/**
 * @brief   Reinitialize the I2C peripheral.
 * @retval  EcuAbs I2C status
 */
EcuAbs_I2c_StatusType EcuAbs_I2c_ReInit(void)
{
    EcuAbs_I2c_StatusType retVal = ECUABS_I2C_OK;

    if (pI2cHandle != NULL)
    {
        if (HAL_I2C_Init(pI2cHandle) != HAL_OK)
        {
            retVal = ECUABS_I2C_ERROR;
        }
    }
    else
    {
        retVal = ECUABS_I2C_ERROR;
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/

/**
 * @brief   Converts HAL_StatusTypeDef to EcuAbs_I2c_StatusType.
 * @param   halStatus HAL return status
 * @retval  EcuAbs I2C status
 */
static EcuAbs_I2c_StatusType EcuAbs_I2c_ConvertStatus(HAL_StatusTypeDef halStatus)
{
    EcuAbs_I2c_StatusType retVal;

    switch (halStatus)
    {
    case HAL_OK:
        retVal = ECUABS_I2C_OK;
        break;

    case HAL_BUSY:
        retVal = ECUABS_I2C_BUSY;
        break;

    case HAL_TIMEOUT:
        retVal = ECUABS_I2C_TIMEOUT;
        break;

    case HAL_ERROR:
        retVal = ECUABS_I2C_ERROR;
        break;

    default:
        retVal = ECUABS_I2C_ERROR;
        break;
    }

    return retVal;
}
