#include "keypad.h"
// bảng ánh xạ keypad
static char key_map[4][4] =
    {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };
char KeyPad_GetKey() // hàm lấy dữ liệu keypad
{
  GPIO_TypeDef *Col_Port[] = {C1_GPIO_Port, C2_GPIO_Port, C3_GPIO_Port, C4_GPIO_Port};
  uint16_t Col_Pin[] = {C1_Pin, C2_Pin, C3_Pin, C4_Pin};
  GPIO_TypeDef *Row_Port[] = {R1_GPIO_Port , R2_GPIO_Port, R3_GPIO_Port, R4_GPIO_Port};
  uint16_t Row_Pin[] = {R1_Pin, R2_Pin, R3_Pin, R4_Pin};
  // Quét từng cột (đặt cột đó low, các cột còn lại high)
  for (int col = 0; col < 4; col++)
  {
    // đặt tất cả các cột high
    for (int i = 0; i < 4; i++)
    {
      HAL_GPIO_WritePin(Col_Port[i], Col_Pin[i], GPIO_PIN_SET);
    }
    // đặt cột hiện tại low
    HAL_GPIO_WritePin(Col_Port[col], Col_Pin[col], GPIO_PIN_RESET);
    if (HAL_GPIO_ReadPin(Row_Port[0], Row_Pin[0]) == GPIO_PIN_RESET)
      return key_map[0][col];
    if (HAL_GPIO_ReadPin(Row_Port[1], Row_Pin[1]) == GPIO_PIN_RESET)
      return key_map[1][col];
    if (HAL_GPIO_ReadPin(Row_Port[2], Row_Pin[2]) == GPIO_PIN_RESET)
      return key_map[2][col];
    if (HAL_GPIO_ReadPin(Row_Port[3], Row_Pin[3]) == GPIO_PIN_RESET)
      return key_map[3][col];
  }
  return 0;
}