/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Gpio.cpp
* Project         : STM32_Sensor_Monitor
* Description     : ECU Abstraction Layer - GPIO Interface Implementation.
*                   Provides hardware-independent GPIO control.
*                   Pin mapping to physical hardware is managed here.
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
#include "EcuAbs_Gpio.hpp"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/
namespace
{
    GPIO_TypeDef * const SCL_PORT = GPIOB;          /*!< I2C1 SCL port (PB8)              */
    constexpr uint16_t SCL_PIN = GPIO_PIN_8;        /*!< I2C1 SCL pin                     */
    constexpr uint32_t TOGGLE_DELAY_MS = 1U;        /*!< Half-period delay for SCL toggle  */
    GPIO_TypeDef * const LED_PORT = GPIOA;          /*!< LD2 port (PA5)                    */
    constexpr uint16_t LED_PIN = GPIO_PIN_5;        /*!< LD2 pin                           */
}
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Toggle I2C SCL line to recover bus hang.
 *          Reconfigures SCL pin as GPIO output, toggles, then restores I2C function.
 * @param   toggleCount  Number of SCL clock toggles.
 * @retval  EcuAbs GPIO status
 */
EcuAbsGpio::Status EcuAbsGpio::toggleSclPin(uint8_t toggleCount)
{
    Status retVal = Status::Ok;
    GPIO_InitTypeDef gpioInit = {0};
    uint8_t idx;

    /* Reconfigure SCL pin as GPIO open-drain output */
    gpioInit.Pin   = SCL_PIN;
    gpioInit.Mode  = GPIO_MODE_OUTPUT_OD;
    gpioInit.Pull  = GPIO_PULLUP;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SCL_PORT, &gpioInit);

    /* Toggle SCL to release slave SDA */
    for (idx = 0U; idx < toggleCount; idx++)
    {
        HAL_GPIO_WritePin(SCL_PORT, SCL_PIN, GPIO_PIN_SET);
        HAL_Delay(TOGGLE_DELAY_MS);
        HAL_GPIO_WritePin(SCL_PORT, SCL_PIN, GPIO_PIN_RESET);
        HAL_Delay(TOGGLE_DELAY_MS);
    }

    /* Set SCL high (idle state) */
    HAL_GPIO_WritePin(SCL_PORT, SCL_PIN, GPIO_PIN_SET);

    /* Restore SCL pin to I2C alternate function (AF4 for I2C1) */
    gpioInit.Pin       = SCL_PIN;
    gpioInit.Mode      = GPIO_MODE_AF_OD;
    gpioInit.Pull      = GPIO_PULLUP;
    gpioInit.Speed     = GPIO_SPEED_FREQ_LOW;
    gpioInit.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(SCL_PORT, &gpioInit);

    return retVal;
}

/**
 * @brief   Turn on the status LED (LD2).
 * @retval  None
 */
void EcuAbsGpio::setLed(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

/**
 * @brief   Turn off the status LED (LD2).
 * @retval  None
 */
void EcuAbsGpio::clearLed(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

/**
 * @brief   Toggle the status LED (LD2).
 * @retval  None
 */
void EcuAbsGpio::toggleLed(void)
{
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/
