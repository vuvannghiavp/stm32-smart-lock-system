#include "LCD.h"
/**** CHUONG TRINH CHO LCD***************/
static uint8_t lcd_backlight_mask = LCD_BL_ON;
void Lcd_Ghi_Lenh(char lenh)
{
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (lenh & 0xf0);
  data_l = ((lenh << 4) & 0xf0);
  data_t[0] = data_u | 0x0C; // en=1, rs=0
  data_t[1] = data_u | 0x08; // en=0, rs=0
  data_t[2] = data_l | 0x0C; // en=1, rs=0
  data_t[3] = data_l | 0x08; // en=0, rs=0
  HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 100);
}

void Lcd_Ghi_Dulieu(char data)
{
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (data & 0xf0);
  data_l = ((data << 4) & 0xf0);
  data_t[0] = data_u | 0x0D; // en=1, rs=0
  data_t[1] = data_u | 0x09; // en=0, rs=0
  data_t[2] = data_l | 0x0D; // en=1, rs=0
  data_t[3] = data_l | 0x09; // en=0, rs=0
  HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 100);
}
void Lcd_Bat_Den(void)
{
    lcd_backlight_mask = LCD_BL_ON;
    // Gửi 1 byte rỗng qua I2C để cập nhật ngay lập tức trạng thái chân P3
    uint8_t cmd = lcd_backlight_mask;
    HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, &cmd, 1, 100);
}

void Lcd_Tat_Den(void)
{
    lcd_backlight_mask = LCD_BL_OFF;
    // Gửi 1 byte rỗng để tắt đèn ngay
    uint8_t cmd = lcd_backlight_mask;
    HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, &cmd, 1, 100);
}


void lcd_init(void)
{
  Lcd_Ghi_Lenh(0x01);
  HAL_Delay(50);
  Lcd_Ghi_Lenh(0x06);
  HAL_Delay(50);
  Lcd_Ghi_Lenh(0x0c);
  HAL_Delay(50);
  Lcd_Ghi_Lenh(0x02);
  HAL_Delay(50);
  Lcd_Ghi_Lenh(0x80);
}

void Lcd_Ghi_Chuoi(char *str)
{
  while (*str)
    Lcd_Ghi_Dulieu(*str++);
}

void Lcd_xoa_manhinh(void)
{
  Lcd_Ghi_Lenh(0x01); // clear display
}
void COUNTDOWN() // hàm đếm ngược đóng cửa sau 10s
{
  for (int countdown = 9; countdown >= 0; countdown--)
  {
    Lcd_Ghi_Lenh(0x80);
    sprintf(time_buffer, "CUA DONG SAU %dS    ", countdown);
    Lcd_Ghi_Chuoi(time_buffer);
    HAL_Delay(1000);
  }
}
void countdown() // hàm đếm ngược nếu nhập sai mật khẩu
{
  for (int countdown = 9; countdown >= 0; countdown--)
  {
    Lcd_Ghi_Lenh(0xC0);
    sprintf(time_buffer, "THU LAI SAU %dS   ", countdown);
    Lcd_Ghi_Chuoi(time_buffer);
    HAL_Delay(1000);
  }
}