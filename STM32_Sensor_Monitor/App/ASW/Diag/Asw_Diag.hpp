/*
************************************************************************************************************************
*
* Filename        : Asw_Diag.hpp
* Project         : STM32_Sensor_Monitor
* Description     : ASW Diagnostic Module.
*                   Performs fault diagnosis on sensor communication,
*                   sensor data quality, and environment threshold monitoring
*                   with debouncing state machine.
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
/** @brief ASW Diagnostic - debouncing fault diagnosis class */
class AswDiag
{
public:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

/*
********************************************************************************************************************
*                                                   Public methods
********************************************************************************************************************
*/

    /**
     * @brief   Periodic diagnostic processing.
     *          Reads diagnostic flags and sensor data from RTE,
     *          evaluates fault conditions with debouncing, and writes results to RTE.
     * @retval  None
     */
    void mainFunction(void);

private:
/*
********************************************************************************************************************
*                                                        Types
********************************************************************************************************************
*/

    /** @brief Diagnostic state machine states */
    enum class State : uint8_t
    {
        Normal     = 0x00U,
        FaultDeb   = 0x01U,
        Fault      = 0x02U,
        NormalDeb  = 0x03U,
        WarningDeb = 0x04U,
        Warning    = 0x05U
    };

    /** @brief Diagnostic item context (state + counter) */
    struct DiagItem
    {
        State   state      = State::Normal;
        uint8_t debCounter = 0U;
    };

/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

    DiagItem commDiag_;    /*!< Communication fault diagnosis  */
    DiagItem dataDiag_;    /*!< Data fault diagnosis           */
    DiagItem envDiag_;     /*!< Environment warning diagnosis  */

/*
********************************************************************************************************************
*                                                  Private methods
********************************************************************************************************************
*/

    /**
     * @brief   Process sensor communication fault diagnosis with debouncing.
     * @param   commStatus  Communication status from Service (0: OK, 1: Error)
     * @retval  None
     */
    void processCommFault(uint8_t commStatus);

    /**
     * @brief   Process sensor data fault diagnosis with debouncing.
     * @param   tempDiag    Temperature diagnostic flag from RTE
     * @param   pressDiag   Pressure diagnostic flag from RTE
     * @param   humDiag     Humidity diagnostic flag from RTE
     * @retval  None
     */
    void processDataFault(uint8_t tempDiag, uint8_t pressDiag, uint8_t humDiag);

    /**
     * @brief   Process environment threshold warning with debouncing.
     * @param   temp    Filtered temperature (0.01 degree C)
     * @param   press   Filtered pressure (Pa)
     * @param   hum     Filtered humidity (0.01 %RH)
     * @retval  None
     */
    void processEnvWarning(int32_t temp, uint32_t press, uint32_t hum);
};
