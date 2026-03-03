# Verification Report: GPIO Read/Write Test

| Item       | Detail                          |
|------------|---------------------------------|
| Report ID  | VR-001                          |
| Feature    | GPIO Digital Output / Input     |
| Author     | Seongmin Oh                     |
| Date       | 2026-03-25                      |
| Result     | **PASS**                        |

## 1. Purpose
Verify that GPIO digital output and input functions operate correctly on the target MCU before implementing the ECU Abstraction layer.

## 2. Test Environment

| Item       | Detail                                          |
|------------|--------------------------------------------------|
| MCU Board  | STM32 Nucleo-F446RE                              |
| IDE        | STM32CubeIDE                                     |
| RTOS       | FreeRTOS (CMSIS_V2)                              |
| Clock      | 180 MHz (HSE)                                    |
| Test Pin   | PC0 (GPIO_OUTPUT, Push-Pull, No Pull, Low Speed) |
| Debugger   | ST-Link V2 (On-board)                            |

## 3. Test Procedure

| Step | Action                              | Expected Result              |
|------|-------------------------------------|------------------------------|
| 1    | Write GPIO_PIN_RESET to PC0        | PC0 output LOW (0)           |
| 2    | Read PC0 pin state                 | test_GPIO == GPIO_PIN_RESET  |
| 3    | Wait 500 ms (osDelay)              | Task yields to scheduler     |
| 4    | Write GPIO_PIN_SET to PC0          | PC0 output HIGH (1)          |
| 5    | Read PC0 pin state                 | test_GPIO == GPIO_PIN_SET    |
| 6    | Wait 500 ms (osDelay)              | Task yields to scheduler     |
| 7    | Repeat steps 1-6 continuously      | Stable toggle at 1 Hz cycle  |

## 4. Test Code
```c
GPIO_PinState test_GPIO;

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
  }
}
```

## 5. Result

| Step | Expected                    | Actual                      | Pass/Fail |
|------|-----------------------------|-----------------------------|-----------|
| 1-2  | test_GPIO == GPIO_PIN_RESET | test_GPIO == GPIO_PIN_RESET | PASS      |
| 4-5  | test_GPIO == GPIO_PIN_SET   | test_GPIO == GPIO_PIN_SET   | PASS      |
| 7    | Stable 1 Hz toggle          | Stable 1 Hz toggle          | PASS      |

## 6. Verdict
All test steps passed. GPIO output and input functions are confirmed operational. Proceeding to ECU Abstraction layer implementation.
