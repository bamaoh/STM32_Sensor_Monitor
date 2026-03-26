/*
************************************************************************************************************************
*
* Filename        : Svc_Led.cpp
* Project         : STM32_Sensor_Monitor
* Description     : LED Service Module Implementation.
*                   Controls status LED through EcuAbsGpio based on
*                   requested operating mode (ON, OFF, BLINK).
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
#include "Svc_Led.hpp"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/**
 * @brief   Set LED operating mode.
 * @param   mode   Mode::Off, Mode::On, or Mode::Blink
 * @retval  None
 */
void SvcLed::setMode(Mode mode)
{
    currentMode_ = mode;
}

/**
 * @brief   Periodic LED processing.
 *          Executes LED action based on current mode.
 * @retval  None
 */
void SvcLed::mainFunction(void)
{
    switch (currentMode_)
    {
        case Mode::On:
            gpio_.setLed();
            break;

        case Mode::Blink:
            gpio_.toggleLed();
            break;

        case Mode::Off:
            gpio_.clearLed();
            break;

        default:
            gpio_.clearLed();
            break;
    }
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/
