/*
************************************************************************************************************************
*
* Filename        : EcuAbs_Gpio.hpp
* Project         : STM32_Sensor_Monitor
* Description     : ECU Abstraction Layer - GPIO Interface.
*                   Provides hardware-independent GPIO control interface.
*                   Pin mapping is encapsulated within this module.
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
/** @brief ECU Abstraction Layer - GPIO control class */
class EcuAbsGpio
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/
    /** @brief EcuAbs GPIO operation status definition */
    enum class Status : uint8_t
    {
        Ok      = 0x00U,   /*!< No error                          */
        Error   = 0x01U    /*!< Operation failed                  */
    };
/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/
    /**
     * @brief   Toggle I2C SCL line to recover bus hang.
     *          Internally manages SCL pin configuration (GPIO mode switch).
     * @param   toggleCount  Number of SCL clock toggles.
     * @retval  EcuAbs GPIO status
     */
    Status toggleSclPin(uint8_t toggleCount);

    /**
     * @brief   Turn on the status LED (LD2).
     * @retval  None
     */
    void setLed(void);

    /**
     * @brief   Turn off the status LED (LD2).
     * @retval  None
     */
    void clearLed(void);

    /**
     * @brief   Toggle the status LED (LD2).
     * @retval  None
     */
    void toggleLed(void);
private:
/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

/*
********************************************************************************************************************
*                                                  Private methods
********************************************************************************************************************
*/

};
