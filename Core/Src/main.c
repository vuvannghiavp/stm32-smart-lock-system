/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
#include "RC522.h"
#include "mg90s.h"
#include "LCD.h"
#include "keypad.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define MAX_ANGLE 180.0f
#define SLEEP_TIMEOUT_MS 15000
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
/* USER CODE BEGIN PV */
uint8_t Backlight = 0;
char buffer[16];
char time_buffer[20];
uint8_t input_mode = 0;
uint8_t input_key = 0;
char input_buffer[10] = {0};
char password[] = "888888";
SystemState_t CurrentState = STATE_LOCK_INPUT;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  lcd_init();
  Servo_Init();
  HAL_Delay(50);
  Lcd_xoa_manhinh();
  MFRC522_Init(&MFRC522, &hspi2, SDA_GPIO_Port, SDA_Pin, rst_GPIO_Port, rst_Pin);
  Lcd_Ghi_Lenh(0x80);
  Lcd_Ghi_Chuoi("SmartDoorSystem");
  HAL_Delay(100);
  for (int countdown = 6; countdown >= 0; countdown--)
  {
    Lcd_Ghi_Lenh(0xC0);
    sprintf(time_buffer,"HOAT DONG SAU %dS", countdown);
    Lcd_Ghi_Chuoi(time_buffer);
    HAL_Delay(1000);
  }
  Lcd_xoa_manhinh();
  last_activity_time = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    char key = KeyPad_GetKey(); // xử lý nhập keypad
    if (input_mode == 0 && CurrentState == STATE_LOCK_INPUT)
    {
      Lcd_Ghi_Lenh(0xC0);
      for (int i = 0; i < input_key; i++)
      {
        Lcd_Ghi_Chuoi("*");
      }
      if (key != 0)
      {
        HAL_Delay(150);
        last_activity_time = HAL_GetTick();
        if (key >= '0' && key <= '9' && input_key < 6)
        {
          input_buffer[input_key++] = key; // nhập số
          Buzzer_Beep(50);
        }
        if (key == '#' && input_key == 0)
        {
          Lcd_xoa_manhinh();
          HAL_Delay(30);
          Lcd_Ghi_Lenh(0x83);
          Lcd_Ghi_Chuoi(" BAN CHUA");
          Lcd_Ghi_Lenh(0xC2);
          Lcd_Ghi_Chuoi("NHAP MAT KHAU!");
          HAL_Delay(2200);
          Lcd_xoa_manhinh();
          last_activity_time = HAL_GetTick();
        }
        else if (key == '#')
        {
          input_buffer[input_key] = '\0'; // xử lý xác nhận
          if (strcmp(input_buffer, password) == 0)
          {
            Lcd_xoa_manhinh();
            Lcd_Ghi_Lenh(0x80);
            Lcd_Ghi_Chuoi(" MAT KHAU DUNG!");
            Servo_SetAngle(150);
            Buzzer_Beep(50);
            COUNTDOWN();
            Servo_SetAngle(0);
            last_activity_time = HAL_GetTick();
          }
          else
          {
            Lcd_xoa_manhinh();
            Lcd_Ghi_Lenh(0x80);
            Lcd_Ghi_Chuoi(" SAI MAT KHAU");
            Flash_Buzzer_And_Led(3, 50, 50);
            countdown();
          }
          input_key = 0;
          Lcd_xoa_manhinh();
          last_activity_time = HAL_GetTick();
        }
        else if (key == '*')
        {
          // xử lý xóa
          input_key = 0;
          memset(input_buffer, 0, sizeof(input_buffer));
          Lcd_xoa_manhinh();
          last_activity_time = HAL_GetTick();
        }
      }
    }
    if (CurrentState == STATE_SLEEP)
    {
      if (Backlight == 0)
      {
        Lcd_Tat_Den();
      }
    }
    if (CurrentState != STATE_SLEEP && (HAL_GetTick() - last_activity_time > SLEEP_TIMEOUT_MS))
    {
      CurrentState = STATE_SLEEP;
      input_key = 0;
      memset(input_buffer, 0, sizeof(input_buffer));
      Lcd_xoa_manhinh();
      Lcd_Ghi_Lenh(0x80);
      Lcd_Ghi_Chuoi(" NHAP MAT KHAU:");
      Lcd_Tat_Den();
    }
    switch (CurrentState)
    {
    case STATE_LOCK_INPUT:
      Lcd_xoa_manhinh();
      Lcd_Ghi_Lenh(0x80);
      Lcd_Ghi_Chuoi(" NHAP MAT KHAU:");
      if (key == 'A')
      {
        CurrentState = STATE_RFID_ADD_MENU;
        Lcd_xoa_manhinh();
        HAL_Delay(50);
        Lcd_Ghi_Lenh(0x80);
        Lcd_Ghi_Chuoi("OPTION->B:MASTER");
        HAL_Delay(70);
        Lcd_Ghi_Lenh(0xC0);
        Lcd_Ghi_Chuoi("C:MEMBER,D:CLEAR");
        last_activity_time = HAL_GetTick();
        break;
      }
      status = MFRC522_Check(&MFRC522, CardID);
      if (status == MI_OK)
      {
        if ((memcmp(CardID, MasterCardID, 4) == 0 && Master_Card_Exists == 1) || Member(CardID, &MFRC522))
        {
          Access_GrantedCard(" QUET THE SUCCESS");
          break;
        }
        else
        {                                 
          Lcd_xoa_manhinh();
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi("   INVALID_CARD");
          CurrentState = STATE_LOCK_INPUT;
          Flash_Buzzer_And_Led(3, 50, 50);
          countdown();
          Lcd_xoa_manhinh();
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi("NHAP MAT KHAU:");
          last_activity_time = HAL_GetTick();
          break;
        }
      }
      break;
    case STATE_RFID_ADD_MENU:
      if (key == 'B')
      {
        last_activity_time = HAL_GetTick();
        CurrentState = STATE_ADD_MASTER;
        Lcd_xoa_manhinh();
        HAL_Delay(60);
        Lcd_Ghi_Lenh(0x80);
        Lcd_Ghi_Chuoi(" CHE DO THEM THE:");
        HAL_Delay(70);
        Lcd_Ghi_Lenh(0xC0);
        Lcd_Ghi_Chuoi("MASTER QUET THE");
        break;
      }
      else if (key == 'C')
      {
        if (Master_Card_Exists == 0) // kiểm tra nếu chưa có Master
        {
          Lcd_xoa_manhinh();
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi(" ERROR: THEM");
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi(" THE MASTER TRUOC");
          Flash_Buzzer_And_Led(3, 50, 50);
          HAL_Delay(2500);
          CurrentState = STATE_RFID_ADD_MENU;
          Lcd_xoa_manhinh();
          HAL_Delay(30);
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi("SELECT->B:MASTER");
          HAL_Delay(70);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi("C:MEMBER,D:CLEAR");
          last_activity_time = HAL_GetTick();
          break;
        }
        else
        {
          Lcd_xoa_manhinh();
          CurrentState = STATE_ADD_MEMBER;
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi("CHE DO THEM THE");
          HAL_Delay(70);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi("MEMBER:QUET THE");
          last_activity_time = HAL_GetTick();
          break;
        }
      }
      else if (key == 'D')
      {
        if (Master_Card_Exists == 1)
        {
          last_activity_time = HAL_GetTick();
          Lcd_xoa_manhinh();
          CurrentState = STATE_DELETE_MEMBER;
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi(" QUET THE MASTER");
          HAL_Delay(70);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi(" DE XAC NHAN!");
          break;
        }
        else
        {
          Lcd_xoa_manhinh();
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi(" ERROR:THE MASTER");
          HAL_Delay(60);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi(" CHUA DUOC THEM");
          Flash_Buzzer_And_Led(3, 50, 50);
          HAL_Delay(2200);
          Lcd_xoa_manhinh();
          CurrentState = STATE_LOCK_INPUT;
          last_activity_time = HAL_GetTick();
          break;
        }
      }
      else if (key == '*')
      {
        Lcd_xoa_manhinh();
        CurrentState = STATE_LOCK_INPUT;
        last_activity_time = HAL_GetTick();
        break;
      }
      break;
    case STATE_ADD_MASTER:
      status = MFRC522_Check(&MFRC522, CardID);
      if (status == MI_OK)
      {
        if (memcmp(CardID, MasterCardID, 4) == 0)
        {
          Lcd_xoa_manhinh();
          HAL_Delay(20);
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi(" SUCCESS MASTER");
          HAL_Delay(60);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi("DA THEM ID THE");
          Flash_Buzzer_And_Led(1, 50, 1);
          Master_Card_Exists = 1;
        }
        else if (Master_Card_Exists == 1)
        {
          Lcd_xoa_manhinh();
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi(" MASTER:ERROR");
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi("THE DA TON TAI");
          Flash_Buzzer_And_Led(3, 50, 50);
        }
        else
        {
          Lcd_xoa_manhinh();
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi(" ERROR: THE NAY");
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi("KHONG HOP LE");
          Flash_Buzzer_And_Led(3, 50, 50);
        }
        HAL_Delay(2000);
        CurrentState = STATE_LOCK_INPUT;
        last_activity_time = HAL_GetTick();
        Lcd_xoa_manhinh();
        Lcd_Ghi_Lenh(0x80);
        Lcd_Ghi_Chuoi("NHAP MAT KHAU:");
      }
      break;
    case STATE_ADD_MEMBER:
      status = MFRC522_Check(&MFRC522, CardID);
      if (status == MI_OK)
      {
        uint8_t add_result = AddNewMemberOrMasterCard(CardID);
        Lcd_xoa_manhinh();
        if (add_result == 1)
        {
          HAL_Delay(20);
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi("SUCCESS MEMBER");
          HAL_Delay(60);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi("DA THEM ID THE");
          Flash_Buzzer_And_Led(1, 50, 1);
          HAL_Delay(2000);
        }
        else if (add_result == 2 || add_result == 3)
        {
          HAL_Delay(20);
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi("MEMBER:ERROR");
          HAL_Delay(60);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi("THE DA TON TAI");
          Flash_Buzzer_And_Led(3, 50, 50);
          HAL_Delay(2000);
        }
        Lcd_xoa_manhinh();
        CurrentState = STATE_LOCK_INPUT;
        last_activity_time = HAL_GetTick();
        Lcd_Ghi_Lenh(0x80);
        Lcd_Ghi_Chuoi("NHAP MAT KHAU:");
      }
      break;
    case STATE_DELETE_WAIT:
      if (key == '*')
      {
        CurrentState = STATE_LOCK_INPUT;
        last_activity_time = HAL_GetTick();
        Lcd_xoa_manhinh();
        break;
      }
      status = MFRC522_Check(&MFRC522, CardID);
      if (status == MI_OK)
      {
        uint8_t delete_status = DeleteMemberCard(CardID);
        if (memcmp(CardID, MasterCardID, 4) == 0)
        {
          Lcd_xoa_manhinh();
          HAL_Delay(20);
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi(" KHONG XOA DUOC");
          HAL_Delay(50);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi( " THE MASTER");
          Flash_Buzzer_And_Led(3, 50, 50);
          HAL_Delay(2000);
          Lcd_xoa_manhinh();
          CurrentState = STATE_LOCK_INPUT;
          last_activity_time = HAL_GetTick();
          break;
        }
        if (delete_status == 1)
        {
          Lcd_xoa_manhinh();
          HAL_Delay(30);
          char count_buffer[15];
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi("XOA ID MEMBER:OK");
          HAL_Delay(50);
          Lcd_Ghi_Lenh(0xC2);
          sprintf(count_buffer,"CON LAI %d THE", MemberCardCount);
          Lcd_Ghi_Chuoi(count_buffer);
          HAL_Delay(2200);
          last_activity_time = HAL_GetTick();
        }
        else
        {
          Lcd_xoa_manhinh();
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi(" DELETE ERROR");
          HAL_Delay(50);
          Flash_Buzzer_And_Led(3, 50, 50);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi("ID KHONG TON TAI");
          HAL_Delay(2000);
          last_activity_time = HAL_GetTick();
        }
        Lcd_xoa_manhinh();
        CurrentState = STATE_LOCK_INPUT;
        last_activity_time = HAL_GetTick();
      }
      break;
    case STATE_DELETE_MEMBER:
    {
      status = MFRC522_Check(&MFRC522, CardID);
      if (status == MI_OK)
      {
        if ((memcmp(CardID, MasterCardID, 4) == 0 && Master_Card_Exists == 1))
        {
          CurrentState = STATE_DELETE_WAIT;
          last_activity_time = HAL_GetTick();
          Lcd_xoa_manhinh();
          Lcd_Ghi_Lenh(0x84);
          Lcd_Ghi_Chuoi(" XAC NHAN OK!");
          HAL_Delay(50);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi(" QUET THE DE XOA");
          Flash_Buzzer_And_Led(1, 50, 1);
          HAL_Delay(1200);
        }
        else
        {
          Lcd_xoa_manhinh();
          Lcd_Ghi_Lenh(0x83);
          Lcd_Ghi_Chuoi(" ERROR: KHONG");
          HAL_Delay(50);
          Lcd_Ghi_Lenh(0xC0);
          Lcd_Ghi_Chuoi(" PHAI THE MASTER");
          Flash_Buzzer_And_Led(3, 50, 50);
          HAL_Delay(2000);
          Lcd_xoa_manhinh();
          CurrentState = STATE_LOCK_INPUT;
          last_activity_time = HAL_GetTick();
        }
      }
      break;
    }
    break;
    case STATE_SLEEP:
      if (key != 0)
      {
        Lcd_xoa_manhinh();
        HAL_Delay(50);
        CurrentState = STATE_LOCK_INPUT;
        Lcd_Bat_Den();
        Lcd_xoa_manhinh();
        Lcd_Ghi_Lenh(0x84);
        Lcd_Ghi_Chuoi("NHAP MAT KHAU:");
        last_activity_time = HAL_GetTick();
        break;
      }
      status = MFRC522_Check(&MFRC522, CardID);
      if (status == MI_OK)
      {
        // kiểm tra thẻ chạy cả trong chế độ ngủ
        if ((memcmp(CardID, MasterCardID, 4) == 0 && Master_Card_Exists == 1) || Member(CardID, &MFRC522))
        {
          Lcd_xoa_manhinh();
          HAL_Delay(50);
          CurrentState = STATE_LOCK_INPUT;
          Lcd_Bat_Den();
          Access_GrantedCard(" QUET THE SUCCESS");
          last_activity_time = HAL_GetTick();
        }
        else
        {
          Lcd_xoa_manhinh();
          HAL_Delay(50);
          CurrentState = STATE_LOCK_INPUT;
          Lcd_Bat_Den();
          Lcd_Ghi_Lenh(0x80);
          Lcd_Ghi_Chuoi("INVALID_CASH");
          Flash_Buzzer_And_Led(3, 50, 50);
          countdown();
          last_activity_time = HAL_GetTick();
          Lcd_xoa_manhinh();
          CurrentState = STATE_LOCK_INPUT;
        }
        break;
      }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7;       //63
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, C1_Pin|C2_Pin|C3_Pin|C4_Pin
                          |Buzzes_Pin|rst_Pin|LED_UnlockDoor_Pin|LED_LOCKdoor_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : R1_Pin R3_Pin R4_Pin R2_Pin */
  GPIO_InitStruct.Pin = R1_Pin|R3_Pin|R4_Pin|R2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : C1_Pin C2_Pin C3_Pin C4_Pin
                           Buzzes_Pin rst_Pin LED_UnlockDoor_Pin LED_LOCKdoor_Pin */
  GPIO_InitStruct.Pin = C1_Pin|C2_Pin|C3_Pin|C4_Pin
                          |Buzzes_Pin|rst_Pin|LED_UnlockDoor_Pin|LED_LOCKdoor_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SDA_Pin */
  GPIO_InitStruct.Pin = SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SDA_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
