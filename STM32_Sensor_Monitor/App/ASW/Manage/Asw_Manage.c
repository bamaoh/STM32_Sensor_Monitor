/*
************************************************************************************************************************
*
* Filename        : Asw_Manage.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : ASW Manage Module Implementation.
*                   Reads diagnostic results from RTE and controls status LED.
*                   Priority: FAULT (LED OFF) > WARNING (LED blink) > NORMAL (LED ON).
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
#include "Asw_Manage.h"
#include "RteApp_Diag.h"
#include "Svc_Led.h"
/*
************************************************************************************************************************
*                                                   Defines and macros
************************************************************************************************************************
*/

/* Diagnostic result values (matches Asw_Diag output) */
#define ASW_MANAGE_NO_FAULT     (0U)
#define ASW_MANAGE_FAULT        (1U)
#define ASW_MANAGE_WARNING      (2U)
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
 * @brief   Periodic manage processing.
 *          Reads diagnostic results from RTE and controls status LED.
 *          Priority: FAULT (LED OFF) > WARNING (LED blink) > NORMAL (LED ON).
 * @retval  None
 */
void Asw_Manage_MainFunction(void)
{
    uint8_t commFault  = ASW_MANAGE_NO_FAULT;
    uint8_t dataFault  = ASW_MANAGE_NO_FAULT;
    uint8_t envWarning = ASW_MANAGE_NO_FAULT;

    /* Read diagnostic results from RTE */
    (void)RteApp_Diag_Read_CommFault(&commFault);
    (void)RteApp_Diag_Read_DataFault(&dataFault);
    (void)RteApp_Diag_Read_EnvWarning(&envWarning);

    /* LED control: FAULT > WARNING > NORMAL */
    if ((commFault == ASW_MANAGE_FAULT) || (dataFault == ASW_MANAGE_FAULT))
    {
        Svc_Led_SetMode(SVC_LED_MODE_OFF);
    }
    else if (envWarning == ASW_MANAGE_WARNING)
    {
        Svc_Led_SetMode(SVC_LED_MODE_BLINK);
    }
    else
    {
        Svc_Led_SetMode(SVC_LED_MODE_ON);
    }
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/
