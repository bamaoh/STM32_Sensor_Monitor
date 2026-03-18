/*
************************************************************************************************************************
*
* Filename        : Svc_Led.h
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : LED Service Module Interface.
*                   Provides mode-based LED control (ON, OFF, BLINK).
*                   Blink timing is managed internally by MainFunction.
* Version         : 0.0.1
* Author          : Seongmin Oh
*
************************************************************************************************************************
*/
#ifndef SVC_LED_H
#define SVC_LED_H
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

/** @brief LED operating mode definition */
typedef enum
{
    SVC_LED_MODE_OFF     = 0x00U,   /*!< LED off (steady)      */
    SVC_LED_MODE_ON      = 0x01U,   /*!< LED on (steady)       */
    SVC_LED_MODE_BLINK   = 0x02U    /*!< LED blinking          */
} Svc_Led_ModeType;
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
 * @brief   Set LED operating mode.
 * @param   mode   SVC_LED_MODE_OFF, SVC_LED_MODE_ON, or SVC_LED_MODE_BLINK
 * @retval  None
 */
void Svc_Led_SetMode(Svc_Led_ModeType mode);

/**
 * @brief   Periodic LED processing.
 *          Executes LED action based on current mode.
 *          Must be called periodically for blink to work.
 * @retval  None
 */
void Svc_Led_MainFunction(void);

#endif /* SVC_LED_H */
