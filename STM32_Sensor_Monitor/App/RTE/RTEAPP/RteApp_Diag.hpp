/*
************************************************************************************************************************
*
* Filename        : RteApp_Diag.hpp
* Project         : STM32_Sensor_Monitor
* Description     : RTE Application Interface - Diagnostic Data Exchange.
*                   Provides read/write interface for diagnostic results
*                   between ASW Diag and ASW Manage modules.
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
#include <cstdint>
/*
************************************************************************************************************************
*                                                    Class definition
************************************************************************************************************************
*/
/** @brief RTE Diagnostic data exchange - static buffer class */
class RteAppDiag
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

    /** @brief RteAppDiag operation status definition */
    enum class Status : uint8_t
    {
        Ok    = 0x00U,   /*!< No error       */
        Error = 0x01U    /*!< General error   */
    };

/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/

    /* Diagnostic result write (ASW Diag -> RTE) */

    /**
     * @brief   Write sensor communication fault status to RTE buffer.
     * @param   fault   0: No fault, 1: Fault
     * @retval  RteAppDiag::Status
     */
    static Status writeCommFault(uint8_t fault);

    /**
     * @brief   Write sensor data fault status to RTE buffer.
     * @param   fault   0: No fault, 1: Fault
     * @retval  RteAppDiag::Status
     */
    static Status writeDataFault(uint8_t fault);

    /**
     * @brief   Write environment warning status to RTE buffer.
     * @param   warning   0: No warning, 1: Warning
     * @retval  RteAppDiag::Status
     */
    static Status writeEnvWarning(uint8_t warning);

    /* Diagnostic result read (RTE -> ASW Manage) */

    /**
     * @brief   Read sensor communication fault status from RTE buffer.
     * @param   pFault  Pointer to store fault status
     * @retval  RteAppDiag::Status
     */
    static Status readCommFault(uint8_t *pFault);

    /**
     * @brief   Read sensor data fault status from RTE buffer.
     * @param   pFault  Pointer to store fault status
     * @retval  RteAppDiag::Status
     */
    static Status readDataFault(uint8_t *pFault);

    /**
     * @brief   Read environment warning status from RTE buffer.
     * @param   pWarning  Pointer to store warning status
     * @retval  RteAppDiag::Status
     */
    static Status readEnvWarning(uint8_t *pWarning);

private:
/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

    static uint8_t rteCommFault_;    /*!< Sensor communication fault  */
    static uint8_t rteDataFault_;    /*!< Sensor data fault           */
    static uint8_t rteEnvWarning_;   /*!< Environment warning         */
};
