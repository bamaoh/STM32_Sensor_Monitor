# Verification Report: UART Serial Output

| Item       | Detail                                                  |
|------------|---------------------------------------------------------|
| Report ID  | VR-008                                                  |
| Feature    | UART Serial Output (Sensor Data + Diagnostic Fault)     |
| Author     | Seongmin Oh                                             |
| Date       | 2026-03-27                                              |
| Result     | **PASS**                                                |

## 1. Purpose
Verify that sensor data and diagnostic fault information are correctly transmitted via UART (USART2) to the PC serial terminal, including alive counter, signal quality indicator, and fault message output.

## 2. Test Environment

| Item        | Detail                                           |
|-------------|--------------------------------------------------|
| MCU Board   | STM32 Nucleo-F446RE                              |
| IDE         | STM32CubeIDE                                     |
| RTOS        | FreeRTOS (CMSIS_V2)                              |
| Sensor      | BME280 (I2C1)                                    |
| UART Config | USART2, 115200 baud, 8N1                         |
| Terminal    | PuTTY (COM3)                                     |
| Connection  | ST-Link Virtual COM Port (USB)                   |

## 3. Architecture

```
ASW (Asw_Manage)        - Determines isStale flag, calls Svc_Uart
    |
Service (Svc_Uart)      - sprintf formatting, alive counter, quality indicator
    |
EcuAbs (EcuAbs_Uart)   - HAL_UART_Transmit wrapper (Polling)
    |
MCAL (HAL)              - USART2 register control
```

## 4. Output Format

| State   | Format                                                       |
|---------|--------------------------------------------------------------|
| Normal  | `[CNT:xxx Q:VALID] T:25.14C P:100840Pa H:31.64%`           |
| Stale   | `[CNT:xxx Q:STALE] T:25.11C P:100837Pa H:31.51%`           |
| Fault   | `[FAULT] COMM:1 DATA:0 ENV:0` (appended when active)        |

- **CNT**: Alive counter (increments every cycle, indicates system liveness)
- **Q**: Signal quality (VALID = fresh measurement, STALE = last valid value retained)

## 5. Test Cases and Results

### TC-01: Normal Sensor Data Output
| Item     | Detail                                           |
|----------|--------------------------------------------------|
| Action   | Power on, observe PuTTY terminal                 |
| Expected | Periodic sensor data with Q:VALID every 1 second |
| Result   | **PASS**                                         |

Observation: Temperature, pressure, and humidity values output continuously with incrementing alive counter.

### TC-02: COMM Fault - Stale Indication
| Item     | Detail                                                    |
|----------|-----------------------------------------------------------|
| Action   | Disconnect BME280 SCL during normal operation             |
| Expected | Q changes to STALE, [FAULT] COMM:1 line appears          |
| Result   | **PASS**                                                  |

Observation: Quality changed to STALE immediately on communication failure. Sensor values retained last valid reading. `[FAULT] COMM:1 DATA:0 ENV:0` line output alongside sensor data.

### TC-03: Fault Recovery - Filter Convergence
| Item     | Detail                                                    |
|----------|-----------------------------------------------------------|
| Action   | Reconnect BME280 SCL after COMM fault                     |
| Expected | Q returns to VALID after debounce, sensor values converge |
| Result   | **PASS**                                                  |

Observation: After reconnection, quality returned to VALID after debouncing period. Sensor values showed gradual convergence due to moving average filter (expected behavior). Humidity showed larger transient due to higher variability.

### TC-04: Alive Counter Continuity
| Item     | Detail                                                    |
|----------|-----------------------------------------------------------|
| Action   | Monitor CNT value across normal and fault states          |
| Expected | CNT increments continuously regardless of fault state     |
| Result   | **PASS**                                                  |

Observation: Alive counter incremented every cycle without interruption during both normal operation and fault conditions, confirming system liveness monitoring.

## 6. Known Behavior
- After fault recovery, filtered sensor values require several cycles to converge to actual values (moving average filter settling time). This is expected and consistent with the filtering design.
- Sensor data during STALE state shows the last successfully measured values, following the "Last Valid Value" strategy.

## 7. Conclusion
UART serial output correctly transmits sensor data with alive counter and signal quality indicator. Fault detection triggers STALE quality and fault message output. Recovery transitions back to VALID state with expected filter convergence behavior. All test cases passed.
