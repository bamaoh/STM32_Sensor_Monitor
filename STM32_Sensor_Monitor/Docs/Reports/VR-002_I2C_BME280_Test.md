# Verification Report: I2C BME280 Communication Test

| Item       | Detail                              |
|------------|-------------------------------------|
| Report ID  | VR-002                              |
| Feature    | I2C Communication with BME280       |
| Author     | Seongmin Oh                         |
| Date       | 2026-03-25                          |
| Result     | **PASS**                            |

## 1. Purpose
Verify that I2C communication between the STM32F446RE and BME280 sensor operates correctly by reading the Chip ID register, before implementing the ECU Abstraction layer.

## 2. Test Environment

| Item        | Detail                                           |
|-------------|--------------------------------------------------|
| MCU Board   | STM32 Nucleo-F446RE                              |
| IDE         | STM32CubeIDE                                     |
| RTOS        | FreeRTOS (CMSIS_V2)                              |
| Sensor      | BME280 (I2C address: 0x77)                       |
| I2C         | I2C1, Standard Mode (100 kHz)                    |
| SCL Pin     | PB8 (CN10 Pin 3)                                 |
| SDA Pin     | PB9 (CN10 Pin 5)                                 |
| VDD         | 3.3V (CN10 Pin 7)                                |
| GND         | GND (CN10 Pin 20)                                |
| Debugger    | ST-Link V2 (On-board)                            |

## 3. Test Procedure

| Step | Action                                         | Expected Result                   |
|------|-------------------------------------------------|-----------------------------------|
| 1    | Initialize I2C1 (Standard Mode, 100 kHz)       | No error on initialization        |
| 2    | Read BME280 register 0xD0 (Chip ID) via I2C    | HAL_OK returned                   |
| 3    | Verify Chip ID value                            | chip_id == 0x60 (96 decimal)      |
| 4    | Confirm GPIO toggle still operates concurrently | PC0 toggles at 1 Hz cycle         |

## 4. Test Code
```c
uint8_t chip_id = 0U;
HAL_StatusTypeDef result;

void StartDefaultTask(void *argument)
{
  for(;;)
  {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
    test_GPIO = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0);
    osDelay(500);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
    test_GPIO = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0);
    osDelay(500);

    /* BME280 Chip ID register read */
    result = HAL_I2C_Mem_Read(&hi2c1, (0x77U << 1U), 0xD0U,
                              I2C_MEMADD_SIZE_8BIT, &chip_id, 1U, 100U);
  }
}
```

## 5. Result

| Step | Expected               | Actual                 | Pass/Fail |
|------|------------------------|------------------------|-----------|
| 1    | Init success           | Init success           | PASS      |
| 2    | result == HAL_OK       | result == HAL_OK       | PASS      |
| 3    | chip_id == 0x60        | chip_id == 0x60        | PASS      |
| 4    | GPIO 1 Hz toggle       | GPIO 1 Hz toggle       | PASS      |

## 6. Issues Encountered

| Issue | Symptom | Root Cause | Resolution |
|-------|---------|------------|------------|
| 1     | HAL_I2C_Mem_Read returned HAL_BUSY | Unstable GND connection between Nucleo and BME280 | Secured GND wiring between boards |

## 7. Verdict
I2C communication with BME280 is confirmed operational. Chip ID (0x60) was successfully read. GPIO and I2C operate concurrently under FreeRTOS without conflict. Proceeding to ECU Abstraction layer implementation.
