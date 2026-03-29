# Verification Report: C++ Migration & FreeRTOS Multi-Task

| Item       | Detail                                                          |
|------------|-----------------------------------------------------------------|
| Report ID  | VR-009                                                          |
| Feature    | C to C++ Migration + FreeRTOS Multi-Task Architecture           |
| Author     | Seongmin Oh                                                     |
| Date       | 2026-03-30                                                      |
| Result     | **PASS**                                                        |

## 1. Purpose
Verify that the App layer C to C++ migration and FreeRTOS multi-task restructuring produce identical functional behavior to the original single-task C implementation.

## 2. Test Environment

| Item        | Detail                                           |
|-------------|--------------------------------------------------|
| MCU Board   | STM32 Nucleo-F446RE                              |
| IDE         | STM32CubeIDE (C++ Project)                       |
| RTOS        | FreeRTOS (CMSIS_V2)                              |
| Sensor      | BME280 (I2C1)                                    |
| UART Config | USART2, 115200 baud, 8N1                         |
| Terminal    | PuTTY (COM3)                                     |
| Connection  | ST-Link Virtual COM Port (USB)                   |

## 3. Migration Scope

### 3.1 C++ Converted Layers (App only)

| Layer              | Modules                                    | Class Names                                       |
|--------------------|--------------------------------------------|---------------------------------------------------|
| ECU Abstraction    | Uart, I2c, Gpio, Flash                     | EcuAbsUart, EcuAbsI2c, EcuAbsGpio, EcuAbsFlash   |
| Service            | Bme280, Led, Nvm, Uart                     | SvcBme280, SvcLed, SvcNvm, SvcUart                |
| RTE                | Bme280, Diag, Sensor                       | RteAppBme280, RteAppDiag, RteAppSensor            |
| ASW                | Sensor, Diag, Manage                       | AswSensor, AswDiag, AswManage                     |
| Bridge             | app_bridge                                 | (C-callable extern "C" functions)                 |

### 3.2 Unchanged (C)

| Layer      | Reason                                           |
|------------|--------------------------------------------------|
| Core/      | CubeIDE auto-generated code (main.c, HAL config) |
| Drivers/   | ST HAL driver library                             |

### 3.3 C++ Conversion Patterns Applied

| C (Before)                          | C++ (After)                              |
|-------------------------------------|------------------------------------------|
| `#ifndef / #define / #endif`        | `#pragma once`                           |
| `#include <stdint.h>`               | `#include <cstdint>`                     |
| `typedef enum { ENUM_VAL }`         | `enum class Status : uint8_t { Val }`    |
| `typedef struct { ... }`            | `struct Name { ... }` (inside class)     |
| `static int x;` (file scope)       | `namespace { int x; }` (anonymous)       |
| `#define CONST 100U`                | `constexpr uint32_t CONST = 100U;`       |
| `NULL`                              | `nullptr`                                |
| `(uint8_t*)ptr`                     | `const_cast<uint8_t*>(ptr)`              |
| `Module_Function()`                 | `object.method()` (class member)         |

## 4. FreeRTOS Multi-Task Architecture

### 4.1 Task Configuration

| Task Name   | Priority          | Stack Size  | Role                               |
|-------------|-------------------|-------------|-------------------------------------|
| SensorTask  | osPriorityHigh    | 256 * 4     | BME280 read + range validation      |
| DiagTask    | osPriorityAboveNormal | 128 * 4 | Fault diagnosis with debouncing     |
| ManageTask  | osPriorityNormal  | 256 * 4     | LED / NVM / UART output control     |

### 4.2 Synchronization Mechanisms

| Mechanism    | Object       | Purpose                                       |
|--------------|-------------|-----------------------------------------------|
| Mutex        | rteMutex    | Protect shared RTE buffers from race condition |
| Event Flag   | EVT_SENSOR_DONE | SensorTask → DiagTask synchronization      |
| Event Flag   | EVT_DIAG_DONE   | DiagTask → ManageTask synchronization      |

### 4.3 Execution Flow (per 1000ms cycle)

```
SensorTask ──[Mutex Lock]──> BME280 Read + Filter ──[Mutex Unlock]──> Set EVT_SENSOR_DONE
                                                                              │
DiagTask   ──[Wait EVT_SENSOR_DONE]──[Mutex Lock]──> Diagnosis ──[Mutex Unlock]──> Set EVT_DIAG_DONE
                                                                                          │
ManageTask ──[Wait EVT_DIAG_DONE]──[Mutex Lock]──> LED + NVM + UART ──[Mutex Unlock]
```

### 4.4 C/C++ Bridge Design

```
main.c (C)  ──#include──>  app_bridge.h (C header, #ifdef __cplusplus guard)
                                  │
                           app_bridge.cpp (C++ implementation)
                                  │
                    ┌──────────────┼──────────────┐
                    ▼              ▼              ▼
              SvcBme280      AswSensor/Diag    AswManage
                                            (owns Led, Nvm, Uart)
```

## 5. Issue Found During Testing

### 5.1 BME280 Communication Failure (All values 0, COMM:1 DATA:1 ENV:2)

| Item       | Detail                                                          |
|------------|-----------------------------------------------------------------|
| Symptom    | All sensor values 0.00, COMM:1 DATA:1 ENV:2, Q:STALE           |
| Root Cause | `i2c_.init()` not called in `SvcBme280::init()`                |
| Analysis   | Original C code called `EcuAbs_I2c_Init()` separately in main.c before RTOS start. After C++ migration, this call was removed (Bridge handles everything), but `SvcBme280::init()` did not call `i2c_.init()` internally. `pHandle_` remained `nullptr`, causing all I2C operations to fail. |
| Fix        | Added `i2c_.init()` at the beginning of `SvcBme280::init()`    |
| Lesson     | In C++, each class must initialize its own dependencies internally. Unlike C where main.c managed init order, C++ objects are responsible for their own members. |

## 6. Test Results

### 6.1 Normal Operation (No Fault)

| Check Item                         | Expected                          | Result   |
|------------------------------------|-----------------------------------|----------|
| Temperature reading                | Room temperature (~25°C)          | **PASS** |
| Pressure reading                   | Atmospheric (~101325 Pa)          | **PASS** |
| Humidity reading                   | Indoor humidity (~40-60%)         | **PASS** |
| Alive counter increment            | Monotonically increasing          | **PASS** |
| Signal quality                     | Q:VALID                           | **PASS** |
| LED state                          | ON (steady)                       | **PASS** |

### 6.2 Multi-Task Verification

| Check Item                         | Expected                          | Result   |
|------------------------------------|-----------------------------------|----------|
| Task execution order               | Sensor → Diag → Manage (per cycle)| **PASS** |
| UART output consistency            | No garbled or mixed output        | **PASS** |
| 1000ms cycle timing                | Stable periodic output            | **PASS** |

### 6.3 Fault Injection (I2C cable disconnect)

| Check Item                         | Expected                          | Result   |
|------------------------------------|-----------------------------------|----------|
| COMM fault detection               | COMM:1 after debounce             | **PASS** |
| Signal quality change              | Q:STALE                           | **PASS** |
| LED state                          | OFF (fault mode)                  | **PASS** |
| Recovery after reconnect           | Returns to VALID                  | **PASS** |

## 7. Conclusion

The C to C++ migration and FreeRTOS multi-task restructuring have been verified successfully. All sensor readings, diagnostic functions, and output behaviors match the original single-task C implementation. The multi-task architecture correctly utilizes Mutex for shared resource protection and Event Flags for inter-task synchronization.
