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
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

#define SVC_BME280_DEV_ADDR     (0x77U)    /*!< BME280 7-bit I2C address (Sparkfun default, SDO pulled high)  */
#define SVC_BME280_REG_CHIP_ID  (0xD0U)    /*!< Chip ID register address              */
#define SVC_BME280_CHIP_ID_VAL  (0x60U)    /*!< Expected Chip ID value                */
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
/*
************************************************************************************************************************
*                                              Private function prototypes
************************************************************************************************************************
*/
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

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/
