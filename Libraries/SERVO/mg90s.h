#include "main.h"
#include "stdint.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
extern TIM_HandleTypeDef htim2;
#define SERVO TIM_CHANNEL_2
#define MIN_PULSE 600
#define MAX_PULSE 2600
#define PULSE_RANGE (MAX_PULSE - MIN_PULSE)
void Servo_Init();
void Servo_SetAngle(uint8_t angle);