/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Gpio.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/26
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
#include "EcuAbs_Gpio.h"
#include "stm32f4xx_hal.h"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

#define ECUABS_GPIO_SCL_PORT    (GPIOB)         /*!< I2C1 SCL port (PB8)       */
#define ECUABS_GPIO_SCL_PIN     (GPIO_PIN_8)    /*!< I2C1 SCL pin              */
#define ECUABS_GPIO_TOGGLE_DELAY_US (5U)        /*!< Half-period delay for SCL toggle  */
#define ECUABS_GPIO_LED_PORT    (GPIOA)         /*!< LD2 port (PA5)            */
#define ECUABS_GPIO_LED_PIN     (GPIO_PIN_5)    /*!< LD2 pin                   */
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
 * @brief   Toggle I2C SCL line to recover bus hang.
 *          Reconfigures SCL pin as GPIO output, toggles, then restores I2C function.
 * @param   toggleCount  Number of SCL clock toggles.
 * @retval  EcuAbs GPIO status
 */
EcuAbs_Gpio_StatusType EcuAbs_Gpio_ToggleSclPin(uint8_t toggleCount)
{
    EcuAbs_Gpio_StatusType retVal = ECUABS_GPIO_OK;
    GPIO_InitTypeDef gpioInit = {0};
    uint8_t idx;

    /* Reconfigure SCL pin as GPIO open-drain output */
    gpioInit.Pin   = ECUABS_GPIO_SCL_PIN;
    gpioInit.Mode  = GPIO_MODE_OUTPUT_OD;
    gpioInit.Pull  = GPIO_PULLUP;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ECUABS_GPIO_SCL_PORT, &gpioInit);

    /* Toggle SCL to release slave SDA */
    for (idx = 0U; idx < toggleCount; idx++)
    {
        HAL_GPIO_WritePin(ECUABS_GPIO_SCL_PORT, ECUABS_GPIO_SCL_PIN, GPIO_PIN_SET);
        HAL_Delay(1U);
        HAL_GPIO_WritePin(ECUABS_GPIO_SCL_PORT, ECUABS_GPIO_SCL_PIN, GPIO_PIN_RESET);
        HAL_Delay(1U);
    }

    /* Set SCL high (idle state) */
    HAL_GPIO_WritePin(ECUABS_GPIO_SCL_PORT, ECUABS_GPIO_SCL_PIN, GPIO_PIN_SET);

    /* Restore SCL pin to I2C alternate function (AF4 for I2C1) */
    gpioInit.Pin       = ECUABS_GPIO_SCL_PIN;
    gpioInit.Mode      = GPIO_MODE_AF_OD;
    gpioInit.Pull      = GPIO_PULLUP;
    gpioInit.Speed     = GPIO_SPEED_FREQ_LOW;
    gpioInit.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(ECUABS_GPIO_SCL_PORT, &gpioInit);

    return retVal;
}
/**
 * @brief   Turn on the status LED (LD2).
 * @retval  None
 */
void EcuAbs_Gpio_SetLed(void)
{
    HAL_GPIO_WritePin(ECUABS_GPIO_LED_PORT, ECUABS_GPIO_LED_PIN, GPIO_PIN_SET);
}

/**
 * @brief   Turn off the status LED (LD2).
 * @retval  None
 */
void EcuAbs_Gpio_ClearLed(void)
{
    HAL_GPIO_WritePin(ECUABS_GPIO_LED_PORT, ECUABS_GPIO_LED_PIN, GPIO_PIN_RESET);
}

/**
 * @brief   Toggle the status LED (LD2).
 * @retval  None
 */
void EcuAbs_Gpio_ToggleLed(void)
{
    HAL_GPIO_TogglePin(ECUABS_GPIO_LED_PORT, ECUABS_GPIO_LED_PIN);
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/
