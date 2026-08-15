#include <stdint.h>
#include "stm32f1xx_hal.h"
#include <stdio.h>
#define SLAVE_ADDRESS_LCD 0x4E // address 8 bit
// Định nghĩa Bit 3 (P3) điều khiển Backlight
#define LCD_BL_ON   0x08  // 0000 1000 (Bit 3 = 1)
#define LCD_BL_OFF  0x00  // 0000 0000 (Bit 3 = 0)
// Biến lưu trạng thái đèn nền hiện tại (mặc định cho Bật)
extern char time_buffer[20];
extern I2C_HandleTypeDef hi2c1;



void Lcd_Ghi_Lenh(char lenh);
void Lcd_Ghi_Dulieu(char data);
void Lcd_Bat_Den(void);
void Lcd_Tat_Den(void);
void lcd_init(void);
void Lcd_Ghi_Chuoi(char *str);
void Lcd_xoa_manhinh(void);
void COUNTDOWN();
void countdown(); 