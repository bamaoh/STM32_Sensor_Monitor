/*
************************************************************************************************************************
*
* Filename        : RteApp_Diag.c
* Project         : STM32_Sensor_Monitor
* Created         : 2026/03/27
* Description     : RTE Application Interface - Diagnostic Data Exchange Implementation.
*                   Manages static data buffers for diagnostic results
*                   between ASW Diag and ASW Manage modules.
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
#include "RteApp_Diag.h"
#include <stddef.h>
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
/*
************************************************************************************************************************
*                                                    Private variables
************************************************************************************************************************
*/

static uint8_t rteCommFault  = 0U;   /*!< Sensor communication fault  */
static uint8_t rteDataFault  = 0U;   /*!< Sensor data fault           */
static uint8_t rteEnvWarning = 0U;   /*!< Environment warning         */
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

/* ---- Diagnostic result write ---- */

/**
 * @brief   Write sensor communication fault status to RTE buffer.
 * @param   fault   0: No fault, 1: Fault
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Write_CommFault(uint8_t fault)
{
    rteCommFault = fault;

    return RTEAPP_DIAG_OK;
}

/**
 * @brief   Write sensor data fault status to RTE buffer.
 * @param   fault   0: No fault, 1: Fault
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Write_DataFault(uint8_t fault)
{
    rteDataFault = fault;

    return RTEAPP_DIAG_OK;
}

/**
 * @brief   Write environment warning status to RTE buffer.
 * @param   warning   0: No warning, 1: Warning
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Write_EnvWarning(uint8_t warning)
{
    rteEnvWarning = warning;

    return RTEAPP_DIAG_OK;
}

/* ---- Diagnostic result read ---- */

/**
 * @brief   Read sensor communication fault status from RTE buffer.
 * @param   pFault  Pointer to store fault status
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Read_CommFault(uint8_t *pFault)
{
    RteApp_Diag_StatusType retVal = RTEAPP_DIAG_OK;

    if (pFault == NULL)
    {
        retVal = RTEAPP_DIAG_ERROR;
    }
    else
    {
        *pFault = rteCommFault;
    }

    return retVal;
}

/**
 * @brief   Read sensor data fault status from RTE buffer.
 * @param   pFault  Pointer to store fault status
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Read_DataFault(uint8_t *pFault)
{
    RteApp_Diag_StatusType retVal = RTEAPP_DIAG_OK;

    if (pFault == NULL)
    {
        retVal = RTEAPP_DIAG_ERROR;
    }
    else
    {
        *pFault = rteDataFault;
    }

    return retVal;
}

/**
 * @brief   Read environment warning status from RTE buffer.
 * @param   pWarning  Pointer to store warning status
 * @retval  RteApp_Diag status
 */
RteApp_Diag_StatusType RteApp_Diag_Read_EnvWarning(uint8_t *pWarning)
{
    RteApp_Diag_StatusType retVal = RTEAPP_DIAG_OK;

    if (pWarning == NULL)
    {
        retVal = RTEAPP_DIAG_ERROR;
    }
    else
    {
        *pWarning = rteEnvWarning;
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private functions
************************************************************************************************************************
*/
