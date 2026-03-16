/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Gpio.h
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/26
* Description     : ECU Abstraction Layer - GPIO Interface.
*                   Provides hardware-independent GPIO control interface.
*                   Pin mapping is encapsulated within this module.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef ECUABS_GPIO_H
#define ECUABS_GPIO_H
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

/** @brief EcuAbs GPIO operation status definition */
typedef enum
{
    ECUABS_GPIO_OK      = 0x00U,   /*!< No error                          */
    ECUABS_GPIO_ERROR   = 0x01U    /*!< Operation failed                  */
} EcuAbs_Gpio_StatusType;
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
 * @brief   Toggle I2C SCL line to recover bus hang.
 *          Internally manages SCL pin configuration (GPIO mode switch).
 * @param   toggleCount  Number of SCL clock toggles.
 * @retval  EcuAbs GPIO status
 */
EcuAbs_Gpio_StatusType EcuAbs_Gpio_ToggleSclPin(uint8_t toggleCount);

/**
 * @brief   Turn on the status LED (LD2).
 * @retval  None
 */
void EcuAbs_Gpio_SetLed(void);

/**
 * @brief   Turn off the status LED (LD2).
 * @retval  None
 */
void EcuAbs_Gpio_ClearLed(void);

/**
 * @brief   Toggle the status LED (LD2).
 * @retval  None
 */
void EcuAbs_Gpio_ToggleLed(void);

#endif /* ECUABS_GPIO_H */
