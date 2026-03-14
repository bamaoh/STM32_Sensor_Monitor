# Verification Report: ASW Sensor Module

| Item       | Detail                                      |
|------------|---------------------------------------------|
| Report ID  | VR-004                                      |
| Feature    | ASW Sensor (Range Validation, Rate Filter, RTE Integration) |
| Author     | Seongmin Oh                                 |
| Date       | 2026-03-27                                  |
| Result     | **PASS**                                    |

## 1. Purpose
Verify that the ASW Sensor module correctly reads measurement data from RTE, validates range against BME280 operating limits, applies rate-of-change filtering, and writes filtered data and diagnostic flags to RTE for upper modules.

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
| Service            | Svc_Bme280      | Sensor measurement, RTE write (raw physical)  |
| RTE                | RteApp_Bme280   | Raw measurement data buffer                   |
| RTE                | RteApp_Sensor   | Filtered data + diagnostic flag buffer        |
| ASW                | Asw_Sensor      | Range validation, rate filter, diagnostic     |

## 4. Data Flow

```
Svc_Bme280 → RteApp_Bme280 → Asw_Sensor → RteApp_Sensor → (ASW Manage / UART)
               (raw data)      (validate)    (filtered data)
                               (filter)      (diag flags)
```

## 5. Test Procedure

| Step | Action                                              | Expected Result                              |
|------|-----------------------------------------------------|----------------------------------------------|
| 1    | Call Svc_Bme280_Init()                              | SVC_BME280_OK                                |
| 2    | Call ReadMeasurement + Asw_Sensor_MainFunction()    | Data flows through RTE correctly             |
| 3    | Verify rteFilteredTemp in debugger                  | Realistic indoor temperature                 |
| 4    | Verify rteFilteredPress in debugger                 | Realistic atmospheric pressure               |
| 5    | Verify rteFilteredHum in debugger                   | Realistic indoor humidity                    |
| 6    | Verify all diagnostic flags                         | All 0x00 (ASW_SENSOR_DIAG_OK)               |
| 7    | Verify stabilization: first 3 cycles                | Values accepted without rate filter          |
| 8    | Verify rate filter: after stabilization             | Rate-of-change limiting applied              |

## 6. Test Code
```c
/* USER CODE BEGIN 2 */
EcuAbs_I2c_Init();
/* USER CODE END 2 */

void StartDefaultTask(void *argument)
{
    Svc_Bme280_StatusType bme280Status;
    Svc_Bme280_DataType sensorData;

    bme280Status = Svc_Bme280_Init();

    for(;;)
    {
        bme280Status = Svc_Bme280_ReadMeasurement(&sensorData);
        Asw_Sensor_MainFunction();
        osDelay(100);
    }
}
```

## 7. Result

| Step | Expected                        | Actual                          | Pass/Fail |
|------|---------------------------------|---------------------------------|-----------|
| 1    | SVC_BME280_OK                   | SVC_BME280_OK                   | PASS      |
| 2    | Data flows through RTE          | Confirmed in debugger           | PASS      |
| 3    | Realistic temperature           | Matches Svc_Bme280 output       | PASS      |
| 4    | Realistic pressure              | Matches Svc_Bme280 output       | PASS      |
| 5    | Realistic humidity              | Matches Svc_Bme280 output       | PASS      |
| 6    | All diag flags 0x00             | All 0x00                        | PASS      |
| 7    | No filter for first 3 cycles    | Values accepted immediately     | PASS      |
| 8    | Rate filter active after 3      | Rate limiting confirmed         | PASS      |

## 8. Implemented Features

| Feature                    | Description                                                        |
|----------------------------|--------------------------------------------------------------------|
| Range Validation           | BME280 operating range check (datasheet Section 1, Table 1)       |
| Rate-of-Change Filter      | Temp: +-1.00 C/sec, Press: +-10 hPa/sec, Hum: +-5.00 %RH/sec    |
| Stabilization Period       | First 3 cycles accept raw values without rate filter               |
| Diagnostic Flags           | DIAG_OK, RANGE_ERROR, RATE_ERROR, COMM_ERROR per channel          |
| RTE Integration            | Filtered data and diag flags written to RteApp_Sensor              |

## 9. Issues Encountered

| Issue | Symptom                                    | Root Cause                                | Resolution                              |
|-------|--------------------------------------------|-------------------------------------------|-----------------------------------------|
| 1     | Filtered value slow to track actual value  | First cycle accepted 0, rate filter limited follow | Added stabilization counter (3 cycles) to accept raw values before filtering |

## 10. Verdict
ASW Sensor module is fully operational. Range validation, rate-of-change filtering, and RTE data exchange are verified. Stabilization period prevents initial value tracking delay. Diagnostic flags correctly indicate sensor channel status. Proceeding to ASW Manage and UART output implementation.
