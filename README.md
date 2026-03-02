# STM32 Sensor Monitor

## Overview
This project is an atmospheric sensor monitoring system built on FreeRTOS with a layered, AUTOSAR-like architecture. It reads sensor data, applies filtering, manages system states, and performs fault diagnostics.

## Hardware
* **MCU Board:** STM32 Nucleo-F446RE (STM32F446RET6/64pin/512k Flash/128k Ram/180MHz Freq)
* **Sensor:** BME280 Atmospheric Sensor (Temperature, Humidity, Pressure, SPI/I2C)

## Software Architecture
The software is structured into three main layers:

* **ASW (Application Software):**
  * `Asw_Sensor`: Reads sensor data via RTE, applies compensation calculation, moving average filter, and unit conversion.
  * `Asw_Manage`: Manages the system state machine (INIT → NORMAL → FAULT).
  * `Asw_Diag`: Diagnoses sensor timeouts and data validity.
* **RTE (Runtime Environment):**
  * `Rte_App`: Data exchange interface between ASW modules. Provides read/write access to shared data buffers (e.g., sensor values, diagnostic flags, system state).
  * `Rte_Hal`: Function call interface to BSW. Wraps BSW service and ECU abstraction APIs so that ASW can invoke hardware operations (e.g., trigger sensor read, send UART data) without direct BSW dependency.
* **BSW (Base Software):**
  * **MCAL:** Lowest-level drivers with direct access to MCU registers and peripherals (GPIO, I2C, UART, TIM).
  * **ECU_Abstraction:** Wraps MCAL drivers into hardware-independent interfaces. Includes BME280 device driver (I2C read/write). Provides I/O access (e.g., GPIO On/Off, sensor read) so that upper layers do not depend on specific hardware layout.
  * **Service:** Provides system-level services such as error logging (`Svc_Log`), ECU state management, and OS-related utilities. This is the topmost BSW layer that interfaces with the RTE.

## Data Flow

### Upward Path (Sensor → Application)
1. **[BME280]** → I2C → **MCAL_I2C** → **ECU_Abs_Bme280**
   (Raw data acquisition via standard I2C driver)
2. **ECU_Abs_Bme280** → **RTE_HAL** → **RTE_APP**
   (BSW writes raw data through RTE_HAL, stored in RTE_APP data buffer)
3. **RTE_APP** → **ASW_Sensor** → **ASW_Diag** → **ASW_Manage**
   (ASW reads data from RTE_APP, then performs compensation, filtering, diagnostics, state control)

### Downward Path (Application → Output)
4. **ASW** → **RTE_HAL** → **ECU_Abstraction** → **MCAL_UART** → **[UART Terminal]**
   (ASW calls UART send function through RTE_HAL)
5. **Svc_Log** → **ECU_Abstraction** → **MCAL_UART** → **[UART Terminal]**
   (Error logging output)