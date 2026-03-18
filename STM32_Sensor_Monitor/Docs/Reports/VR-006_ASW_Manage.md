# Verification Report: ASW Manage Module

| Item       | Detail                                      |
|------------|---------------------------------------------|
| Report ID  | VR-006                                      |
| Feature    | ASW Manage (LED Status Indication via Svc_Led) |
| Author     | Seongmin Oh                                 |
| Date       | 2026-03-27                                  |
| Result     | **PASS**                                    |

## 1. Purpose
Verify that the ASW Manage module correctly reads diagnostic results from RTE and controls the status LED through Svc_Led service layer, following AUTOSAR layer separation.

## 2. Test Environment

| Item        | Detail                                           |
|-------------|--------------------------------------------------|
| MCU Board   | STM32 Nucleo-F446RE                              |
| IDE         | STM32CubeIDE                                     |
| RTOS        | FreeRTOS (CMSIS_V2)                              |
| Sensor      | Sparkfun BME280 (I2C address: 0x77, SDO=VDD)    |
| I2C         | I2C1, Standard Mode (100 kHz)                    |
| LED         | LD2 (PA5, Green)                                 |
| Debugger    | ST-Link V2 (On-board)                            |

## 3. Software Architecture

| Layer              | Module          | Role                                          |
|--------------------|-----------------|-----------------------------------------------|
| ASW                | Asw_Diag        | Diagnostic evaluation, write to RteApp_Diag   |
| RTE                | RteApp_Diag     | Diagnostic result buffer                      |
| ASW                | Asw_Manage      | Read diag results, set LED mode               |
| Service            | Svc_Led         | Mode-based LED control (ON/OFF/BLINK)         |
| ECU Abstraction    | EcuAbs_Gpio     | Hardware LED pin control (PA5)                |

## 4. Data Flow

```
Asw_Diag → RteApp_Diag → Asw_Manage → Svc_Led_SetMode() → Svc_Led_MainFunction() → EcuAbs_Gpio
              (result)      (read)        (mode set)           (execute)              (hardware)
```

## 5. LED Control Logic

| Priority | Condition                          | LED Mode | Behavior           |
|----------|------------------------------------|----------|--------------------|
| 1 (High) | CommFault or DataFault = FAULT     | OFF      | LED off (steady)   |
| 2        | EnvWarning = WARNING               | BLINK    | LED toggle per sec  |
| 3 (Low)  | No fault, no warning               | ON       | LED on (steady)    |

## 6. Test Procedure

| Step | Action                                              | Expected Result                              |
|------|-----------------------------------------------------|----------------------------------------------|
| 1    | Normal operation (no fault)                         | LD2 steady ON                                |
| 2    | Verify Svc_Led currentMode in debugger              | SVC_LED_MODE_ON (0x01)                       |
| 3    | Inject envWarning = WARNING via debugger            | LD2 blinks (1 sec interval)                  |
| 4    | Inject commFault = FAULT via debugger               | LD2 OFF                                      |
| 5    | Inject both FAULT + WARNING                         | LD2 OFF (FAULT priority over WARNING)        |
| 6    | Remove all faults                                   | LD2 steady ON (returns to NORMAL)            |

## 7. Test Code
```c
Svc_Bme280_StatusType bme280Status;
Svc_Bme280_DataType sensorData;

void StartDefaultTask(void *argument)
{
    bme280Status = Svc_Bme280_Init();

    for(;;)
    {
        bme280Status = Svc_Bme280_ReadMeasurement(&sensorData);
        Asw_Sensor_MainFunction();
        Asw_Diag_MainFunction();
        Asw_Manage_MainFunction();
        Svc_Led_MainFunction();
        osDelay(1000);
    }
}
```

## 8. Result

| Step | Expected                        | Actual                          | Pass/Fail |
|------|---------------------------------|---------------------------------|-----------|
| 1    | LD2 steady ON                   | LD2 ON confirmed                | PASS      |
| 2    | SVC_LED_MODE_ON                 | 0x01 in debugger                | PASS      |
| 3    | LD2 blinks                      | Toggle per second confirmed     | PASS      |
| 4    | LD2 OFF                         | LD2 OFF confirmed               | PASS      |
| 5    | LD2 OFF (FAULT priority)        | LD2 OFF confirmed               | PASS      |
| 6    | LD2 steady ON                   | LD2 ON confirmed                | PASS      |

## 9. Implemented Features

| Feature                    | Description                                                        |
|----------------------------|--------------------------------------------------------------------|
| LED Status Indication      | Visual feedback of system health via LD2                           |
| Priority-based Control     | FAULT > WARNING > NORMAL                                           |
| AUTOSAR Layer Separation   | ASW calls Svc_Led, not EcuAbs_Gpio directly                       |
| Mode-based Service         | Svc_Led manages blink timing, ASW only sets desired mode           |
| EcuAbs LED Abstraction     | Hardware pin info (PA5) encapsulated in EcuAbs_Gpio               |

## 10. Verdict
ASW Manage module is fully operational. Diagnostic results are correctly read from RTE and translated to LED modes through Svc_Led service. Layer separation is maintained: ASW sets mode via Service, Service executes via EcuAbs. Priority logic ensures FAULT takes precedence over WARNING. All LED states (ON, OFF, BLINK) verified on hardware.
