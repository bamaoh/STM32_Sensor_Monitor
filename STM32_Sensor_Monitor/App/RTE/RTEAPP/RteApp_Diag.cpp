/*
************************************************************************************************************************
*
* Filename        : RteApp_Diag.cpp
* Project         : STM32_Sensor_Monitor
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
#include "RteApp_Diag.hpp"
/*
************************************************************************************************************************
*                                                 Static member definitions
************************************************************************************************************************
*/

uint8_t RteAppDiag::rteCommFault_  = 0U;
uint8_t RteAppDiag::rteDataFault_  = 0U;
uint8_t RteAppDiag::rteEnvWarning_ = 0U;
/*
************************************************************************************************************************
*                                                    Public methods
************************************************************************************************************************
*/

/* ---- Diagnostic result write ---- */

/**
 * @brief   Write sensor communication fault status to RTE buffer.
 * @param   fault   0: No fault, 1: Fault
 * @retval  RteAppDiag::Status
 */
RteAppDiag::Status RteAppDiag::writeCommFault(uint8_t fault)
{
    rteCommFault_ = fault;

    return Status::Ok;
}

/**
 * @brief   Write sensor data fault status to RTE buffer.
 * @param   fault   0: No fault, 1: Fault
 * @retval  RteAppDiag::Status
 */
RteAppDiag::Status RteAppDiag::writeDataFault(uint8_t fault)
{
    rteDataFault_ = fault;

    return Status::Ok;
}

/**
 * @brief   Write environment warning status to RTE buffer.
 * @param   warning   0: No warning, 1: Warning
 * @retval  RteAppDiag::Status
 */
RteAppDiag::Status RteAppDiag::writeEnvWarning(uint8_t warning)
{
    rteEnvWarning_ = warning;

    return Status::Ok;
}

/* ---- Diagnostic result read ---- */

/**
 * @brief   Read sensor communication fault status from RTE buffer.
 * @param   pFault  Pointer to store fault status
 * @retval  RteAppDiag::Status
 */
RteAppDiag::Status RteAppDiag::readCommFault(uint8_t *pFault)
{
    Status retVal = Status::Ok;

    if (pFault == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pFault = rteCommFault_;
    }

    return retVal;
}

/**
 * @brief   Read sensor data fault status from RTE buffer.
 * @param   pFault  Pointer to store fault status
 * @retval  RteAppDiag::Status
 */
RteAppDiag::Status RteAppDiag::readDataFault(uint8_t *pFault)
{
    Status retVal = Status::Ok;

    if (pFault == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pFault = rteDataFault_;
    }

    return retVal;
}

/**
 * @brief   Read environment warning status from RTE buffer.
 * @param   pWarning  Pointer to store warning status
 * @retval  RteAppDiag::Status
 */
RteAppDiag::Status RteAppDiag::readEnvWarning(uint8_t *pWarning)
{
    Status retVal = Status::Ok;

    if (pWarning == nullptr)
    {
        retVal = Status::Error;
    }
    else
    {
        *pWarning = rteEnvWarning_;
    }

    return retVal;
}
/*
************************************************************************************************************************
*                                                   Private methods
************************************************************************************************************************
*/
