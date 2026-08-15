/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define R1_Pin GPIO_PIN_0
#define R1_GPIO_Port GPIOA
#define Servo_Pin GPIO_PIN_1
#define Servo_GPIO_Port GPIOA
#define R3_Pin GPIO_PIN_2
#define R3_GPIO_Port GPIOA
#define R4_Pin GPIO_PIN_3
#define R4_GPIO_Port GPIOA
#define C1_Pin GPIO_PIN_4
#define C1_GPIO_Port GPIOA
#define C2_Pin GPIO_PIN_5
#define C2_GPIO_Port GPIOA
#define C3_Pin GPIO_PIN_6
#define C3_GPIO_Port GPIOA
#define C4_Pin GPIO_PIN_7
#define C4_GPIO_Port GPIOA
#define SDA_Pin GPIO_PIN_12
#define SDA_GPIO_Port GPIOB
#define Buzzes_Pin GPIO_PIN_8
#define Buzzes_GPIO_Port GPIOA
#define rst_Pin GPIO_PIN_9
#define rst_GPIO_Port GPIOA
#define R2_Pin GPIO_PIN_10
#define R2_GPIO_Port GPIOA
#define LED_UnlockDoor_Pin GPIO_PIN_11
#define LED_UnlockDoor_GPIO_Port GPIOA
#define LED_LOCKdoor_Pin GPIO_PIN_12
#define LED_LOCKdoor_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
typedef enum
{
  STATE_LOCK_INPUT,
  STATE_RFID_ADD_MENU,
  STATE_ADD_MASTER,
  STATE_ADD_MEMBER,
  STATE_DELETE_WAIT,
  STATE_DELETE_MEMBER,
  STATE_SLEEP
} SystemState_t;
extern SystemState_t CurrentState;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
