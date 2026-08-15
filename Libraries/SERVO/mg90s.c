#include"mg90s.h"
void Servo_SetAngle(uint8_t angle) // hàm điều khiển servo (angle: góc từ 0 đến 180 độ)
{
  // khai báo biến pulse
  uint32_t pulse;
  pulse = (uint32_t)(MIN_PULSE + ((float)angle / 180.0f) * (float)PULSE_RANGE);
  if (pulse < MIN_PULSE)
  {
    pulse = MIN_PULSE;
  }
  if (pulse > (MIN_PULSE + PULSE_RANGE))
  {
    pulse = (MIN_PULSE + PULSE_RANGE);
  }
  __HAL_TIM_SET_COMPARE(&htim2, SERVO, pulse);
  if (angle >= 90)
  {
    HAL_GPIO_WritePin(LED_UnlockDoor_GPIO_Port,LED_UnlockDoor_Pin , GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_LOCKdoor_GPIO_Port , LED_LOCKdoor_Pin, GPIO_PIN_RESET);
  }
  else if (angle < 10)
  {
    HAL_GPIO_WritePin(LED_UnlockDoor_GPIO_Port,LED_UnlockDoor_Pin , GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_LOCKdoor_GPIO_Port , LED_LOCKdoor_Pin, GPIO_PIN_SET);
  }
}
void Servo_Init()
{
  HAL_TIM_PWM_Start(&htim2, SERVO);
  Servo_SetAngle(0);
}