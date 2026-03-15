# Verification Report: ASW Diagnostic Module

| Item       | Detail                                      |
|------------|---------------------------------------------|
| Report ID  | VR-005                                      |
| Feature    | ASW Diag (Debouncing State Machine, Fault/Warning Diagnosis) |
| Author     | Seongmin Oh                                 |
| Date       | 2026-03-27                                  |
| Result     | **PASS**                                    |

## 1. Purpose
Verify that the ASW Diagnostic module correctly evaluates sensor communication faults, data quality faults, and environment threshold warnings using a debouncing state machine, with separated diagnostic sources per fault type.

## 2. Test Environment

| Item        | Detail                                           |
|-------------|--------------------------------------------------|
| MCU Board   | STM32 Nucleo-F446RE                              |
| IDE         | STM32CubeIDE                                     |
| RTOS        | FreeRTOS (CMSIS_V2)                              |
| Sensor      | Sparkfun BME280 (I2C address: 0x77, SDO=VDD)    |
| I2C         | I2C1, Standard Mode (100 kHz)                    |
| Debugger    | ST-Link V2 (On-board)                            |

## 3. Software Architecture

| Layer              | Module          | Role                                          |
|--------------------|-----------------|-----------------------------------------------|
| Service            | Svc_Bme280      | Sensor measurement, CommStatus write to RTE   |
| RTE                | RteApp_Bme280   | Raw data + communication status buffer        |
| RTE                | RteApp_Sensor   | Filtered data + diagnostic flag buffer        |
| RTE                | RteApp_Diag     | Diagnostic result buffer (Fault/Warning)      |
| ASW                | Asw_Sensor      | Range validation, rate filter, diag flags     |
| ASW                | Asw_Diag        | Debouncing state machine, fault evaluation    |

## 4. Data Flow

```
CommFault:  Svc_Bme280 → RteApp_Bme280 (CommStatus) → Asw_Diag → RteApp_Diag
DataFault:  Asw_Sensor → RteApp_Sensor (DiagFlags)   → Asw_Diag → RteApp_Diag
EnvWarning: Asw_Sensor → RteApp_Sensor (FilteredData) → Asw_Diag → RteApp_Diag
```

## 5. Diagnostic Items

| Item       | Source              | Condition                                    | Result Value |
|------------|---------------------|----------------------------------------------|--------------|
| CommFault  | RteApp_Bme280       | CommStatus != 0 (I2C communication error)    | FAULT (1)    |
| DataFault  | RteApp_Sensor       | RANGE_ERROR or RATE_ERROR on any channel     | FAULT (1)    |
| EnvWarning | RteApp_Sensor       | Temp > 50C, Hum > 80%, Press outside 950~1060 hPa | WARNING (2) |

## 6. State Machine

```
NORMAL → FAULT_DEB → FAULT → NORMAL_DEB → NORMAL  (CommFault, DataFault)
NORMAL → WARNING_DEB → WARNING → NORMAL_DEB → NORMAL  (EnvWarning)

Debouncing threshold: 3 cycles (3 seconds at 1 sec period)
```

## 7. Test Procedure

| Step | Action                                              | Expected Result                              |
|------|-----------------------------------------------------|----------------------------------------------|
| 1    | Normal operation (no fault injected)                | All results = NO_FAULT (0)                   |
| 2    | Verify commResult in debugger                       | 0 (NO_FAULT)                                 |
| 3    | Verify dataResult in debugger                       | 0 (NO_FAULT)                                 |
| 4    | Verify envResult in debugger                        | 0 (NO_FAULT) under normal indoor conditions  |
| 5    | Inject tempDiag = RANGE_ERROR (0x01) for 3+ cycles | dataResult transitions to FAULT (1)          |
| 6    | Remove injected fault for 3+ cycles                 | dataResult transitions back to NO_FAULT (0)  |
| 7    | Inject tempValue = 6000 (60.00C) for 3+ cycles     | envResult transitions to WARNING (2)         |
| 8    | Verify state machine debouncing                     | State transitions follow DEB threshold       |

## 8. Test Code
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
        osDelay(1000);
    }
}
```

## 9. Result

| Step | Expected                        | Actual                          | Pass/Fail |
|------|---------------------------------|---------------------------------|-----------|
| 1    | All NO_FAULT                    | All 0                           | PASS      |
| 2    | commResult = 0                  | 0                               | PASS      |
| 3    | dataResult = 0                  | 0                               | PASS      |
| 4    | envResult = 0                   | 0 (indoor conditions normal)    | PASS      |
| 5    | dataResult = FAULT after 3 cyc  | FAULT confirmed in debugger     | PASS      |
| 6    | dataResult = NO_FAULT after 3   | NO_FAULT confirmed              | PASS      |
| 7    | envResult = WARNING after 3 cyc | WARNING confirmed in debugger   | PASS      |
| 8    | DEB states observed             | State transitions verified      | PASS      |

## 10. Implemented Features

| Feature                    | Description                                                        |
|----------------------------|--------------------------------------------------------------------|
| Communication Fault        | I2C comm error detection via RteApp_Bme280 CommStatus              |
| Data Quality Fault         | Range/Rate error detection via RteApp_Sensor diag flags            |
| Environment Warning        | Threshold monitoring (Temp, Press, Hum) with separate WARNING state |
| Debouncing State Machine   | 3-cycle debounce for both fault assertion and recovery             |
| Separated Diagnostic Sources | CommFault from Service, DataFault from Sensor                    |
| Testable Result Variables  | Intermediate result variables for debugger/unit test observability  |

## 11. Verdict
ASW Diagnostic module is fully operational. Three independent diagnostic items (CommFault, DataFault, EnvWarning) are evaluated with debouncing state machines. Diagnostic sources are properly separated: communication faults read from Service layer (RteApp_Bme280), data faults from Sensor layer (RteApp_Sensor). Result variables are extracted for test observability. Proceeding to ASW Manage module for LED control implementation.
