# Verification Report: BME280 Service Layer

| Item       | Detail                                      |
|------------|---------------------------------------------|
| Report ID  | VR-003                                      |
| Feature    | BME280 Service Layer (EcuAbs + Service)     |
| Author     | Seongmin Oh                                 |
| Date       | 2026-03-26                                  |
| Result     | **PASS**                                    |

## 1. Purpose
Verify that the BME280 service layer correctly initializes the sensor, reads calibration data, and acquires compensated measurement data through the ECU Abstraction I2C interface.

## 2. Test Environment

| Item        | Detail                                           |
|-------------|--------------------------------------------------|
| MCU Board   | STM32 Nucleo-F446RE                              |
| IDE         | STM32CubeIDE                                     |
| RTOS        | FreeRTOS (CMSIS_V2)                              |
| Sensor      | Sparkfun BME280 (I2C address: 0x77, SDO=VDD)    |
| I2C         | I2C1, Standard Mode (100 kHz)                    |
| SCL Pin     | PB8 (CN10 Pin 3)                                 |
| SDA Pin     | PB9 (CN10 Pin 5)                                 |
| Debugger    | ST-Link V2 (On-board)                            |

## 3. Software Architecture

| Layer              | Module         | Role                                        |
|--------------------|----------------|---------------------------------------------|
| ECU Abstraction    | EcuAbs_I2c     | HAL I2C wrapping, 7-bit address conversion  |
| ECU Abstraction    | EcuAbs_Gpio    | SCL pin toggle for I2C bus recovery         |
| Service            | Svc_Bme280     | Sensor init, calibration, measurement, recovery |

## 4. Test Procedure

| Step | Action                                              | Expected Result                              |
|------|-----------------------------------------------------|----------------------------------------------|
| 1    | Call EcuAbs_I2c_Init()                              | ECUABS_I2C_OK                                |
| 2    | Call Svc_Bme280_Init()                              | SVC_BME280_OK                                |
| 3    | Verify calibration data in debugger                 | Non-zero, sensor-specific values             |
| 4    | Call Svc_Bme280_ReadMeasurement() in 1s loop        | SVC_BME280_OK with valid physical values     |
| 5    | Verify temperature value                            | Realistic indoor range (20~30 degree C)      |
| 6    | Verify pressure value                               | Realistic range (1000~1020 hPa)              |
| 7    | Verify humidity value                               | Realistic indoor range (20~60 %RH)           |

## 5. Test Code
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
        osDelay(1000);
    }
}
```

## 6. Result

| Step | Expected                        | Actual                          | Pass/Fail |
|------|---------------------------------|---------------------------------|-----------|
| 1    | ECUABS_I2C_OK                   | ECUABS_I2C_OK                   | PASS      |
| 2    | SVC_BME280_OK                   | SVC_BME280_OK                   | PASS      |
| 3    | Non-zero calibration values     | digT1=28373, digT2=26551, etc.  | PASS      |
| 4    | SVC_BME280_OK                   | SVC_BME280_OK                   | PASS      |
| 5    | 20~30 degree C                  | 26.67 degree C (2667)           | PASS      |
| 6    | 1000~1020 hPa                   | 1009.69 hPa (100969 Pa)         | PASS      |
| 7    | 20~60 %RH                       | 26.84 %RH (2684)               | PASS      |

## 7. Issues Encountered

| Issue | Symptom                    | Root Cause                           | Resolution                          |
|-------|----------------------------|--------------------------------------|-------------------------------------|
| 1     | HAL_BUSY on all I2C calls  | Unstable hardware connection         | Power cycle of board and sensor     |
| 2     | Include path not found     | EcuAbs/Service paths not in project  | Added paths in IDE Include settings |
| 3     | Undefined reference error  | Source files not in build            | Added Source Location in project    |

## 8. Oscilloscope Captures

### 8.1 Normal Operation - I2C SCL Signal
![SCL Normal Operation](images/VR-003_SCL_Normal_Operation.jpg)
- I2C SCL signal during periodic 1-second measurement cycle
- Timebase: 5.00 ms/div
- Each burst corresponds to one ReadMeasurement I2C transaction

### 8.2 Bus Recovery - SCL Toggle
![SCL Bus Recovery](images/VR-003_SCL_Bus_Recovery.jpg)
- SCL 9-clock toggle sequence during I2C bus recovery
- Timebase: 500 us/div
- Recovery triggered after I2C bus hang detection

## 9. Implemented Features


| Feature                    | Description                                                   |
|----------------------------|---------------------------------------------------------------|
| Sensor Init                | Device ready, Chip ID verify, soft reset, calibration, config |
| Measurement Read           | 8-byte burst read, 20-bit/16-bit parsing, compensation       |
| Compensation               | Datasheet Section 4.2.3 formulas (temp, press, humidity)     |
| Status Check               | Status register (0xF3) measuring bit check before data read  |
| I2C Recovery               | 3-level escalation: retry, ReInit, bus recovery (SCL toggle) |

## 10. Verdict
BME280 service layer is fully operational. Sensor initialization, calibration data readout, and compensated measurement acquisition are verified. I2C error recovery logic and measurement status guard are implemented. Proceeding to ASW layer implementation.
