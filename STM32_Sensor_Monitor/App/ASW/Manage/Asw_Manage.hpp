/*
************************************************************************************************************************
*
* Filename        : Asw_Manage.hpp
* Project         : STM32_Sensor_Monitor
* Description     : ASW Manage Module Interface.
*                   Controls status LED based on diagnostic results.
*                   NORMAL: LED ON, WARNING: LED blink, FAULT: LED OFF.
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
#include "Svc_Led.hpp"
#include "Svc_Nvm.hpp"
#include "Svc_Uart.hpp"
#include <cstdint>
/*
************************************************************************************************************************
*                                                    Class definition
************************************************************************************************************************
*/
/** @brief ASW Manage - system management and output control class */
class AswManage
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
     * @brief   Periodic manage processing.
     *          Reads diagnostic results from RTE and controls status LED.
     * @retval  None
     */
    void mainFunction(void);

private:
/*
********************************************************************************************************************
*                                                  Member variables
********************************************************************************************************************
*/

    SvcLed  led_;                            /*!< LED service instance          */
    SvcNvm  nvm_;                            /*!< NVM service instance          */
    SvcUart uart_;                           /*!< UART service instance         */

    uint8_t prevCommFault_  = 0U;            /*!< Previous CommFault result     */
    uint8_t prevDataFault_  = 0U;            /*!< Previous DataFault result     */
    uint8_t prevEnvWarning_ = 0U;            /*!< Previous EnvWarning result    */
    SvcNvm::DiagData nvmData_ = {};          /*!< NVM diagnostic data           */
    uint8_t nvmLoaded_      = 0U;            /*!< NVM data loaded flag          */
    uint8_t commSavedCycle_ = 0U;            /*!< CommFault type saved this cycle  */
    uint8_t dataSavedCycle_ = 0U;            /*!< DataFault type saved this cycle  */
    uint8_t envSavedCycle_  = 0U;            /*!< EnvWarning type saved this cycle */

/*
********************************************************************************************************************
*                                                  Private methods
********************************************************************************************************************
*/

    /**
     * @brief   Update NVM on diagnostic state transition.
     *          Stores fault type with cycle marker and increments count.
     *          Only writes to Flash when a new fault/warning transition occurs.
     * @param   commFault   Current CommFault result
     * @param   dataFault   Current DataFault result
     * @param   envWarning  Current EnvWarning result
     * @retval  None
     */
    void updateNvm(uint8_t commFault, uint8_t dataFault, uint8_t envWarning);

    /**
     * @brief   Output sensor and diagnostic data via UART.
     *          Always sends sensor data. Additionally sends fault data
     *          when at least one fault or warning is active.
     * @param   commFault   Current CommFault result
     * @param   dataFault   Current DataFault result
     * @param   envWarning  Current EnvWarning result
     * @retval  None
     */
    void outputUart(uint8_t commFault, uint8_t dataFault, uint8_t envWarning);
};
