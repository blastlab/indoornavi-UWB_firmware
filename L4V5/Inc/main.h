/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_crc.h"
#include "stm32l4xx_ll_lptim.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_tim.h"
#include "stm32l4xx_ll_wwdg.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_dma.h"

#include "stm32l4xx_ll_exti.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define SW1_Pin GPIO_PIN_13
#define SW1_GPIO_Port GPIOC
#define HW_PULL_Pin GPIO_PIN_1
#define HW_PULL_GPIO_Port GPIOH
#define DW_IRQ_Pin GPIO_PIN_0
#define DW_IRQ_GPIO_Port GPIOA
#define DW_IRQ_EXTI_IRQn EXTI0_IRQn
#define HW_ADC_Pin GPIO_PIN_1
#define HW_ADC_GPIO_Port GPIOA
#define VIBR_INT_Pin GPIO_PIN_2
#define VIBR_INT_GPIO_Port GPIOA
#define VBAT_Pin GPIO_PIN_3
#define VBAT_GPIO_Port GPIOA
#define DW_CS_Pin GPIO_PIN_4
#define DW_CS_GPIO_Port GPIOA
#define DW_SCK_Pin GPIO_PIN_5
#define DW_SCK_GPIO_Port GPIOA
#define DW_MISO_Pin GPIO_PIN_6
#define DW_MISO_GPIO_Port GPIOA
#define DW_MOSI_Pin GPIO_PIN_7
#define DW_MOSI_GPIO_Port GPIOA
#define DW_WUP_Pin GPIO_PIN_0
#define DW_WUP_GPIO_Port GPIOB
#define DW_RST_Pin GPIO_PIN_1
#define DW_RST_GPIO_Port GPIOB
#define LED_R1_Pin GPIO_PIN_2
#define LED_R1_GPIO_Port GPIOB
#define LED_G1_Pin GPIO_PIN_10
#define LED_G1_GPIO_Port GPIOB
#define VBAT_MOS_Pin GPIO_PIN_11
#define VBAT_MOS_GPIO_Port GPIOB
#define SD_CS_Pin GPIO_PIN_12
#define SD_CS_GPIO_Port GPIOB
#define SD_SCK_Pin GPIO_PIN_13
#define SD_SCK_GPIO_Port GPIOB
#define SD_MISO_Pin GPIO_PIN_14
#define SD_MISO_GPIO_Port GPIOB
#define SD_MOSI_Pin GPIO_PIN_15
#define SD_MOSI_GPIO_Port GPIOB
#define SD_LOCK_Pin GPIO_PIN_8
#define SD_LOCK_GPIO_Port GPIOA
#define IMU_SCK_Pin GPIO_PIN_3
#define IMU_SCK_GPIO_Port GPIOB
#define IMU_MISO_Pin GPIO_PIN_4
#define IMU_MISO_GPIO_Port GPIOB
#define IMU_MOSI_Pin GPIO_PIN_5
#define IMU_MOSI_GPIO_Port GPIOB
#define IMU_CS_Pin GPIO_PIN_6
#define IMU_CS_GPIO_Port GPIOB
#define IMU_INT_Pin GPIO_PIN_7
#define IMU_INT_GPIO_Port GPIOB
#define BOOT0_Pin GPIO_PIN_3
#define BOOT0_GPIO_Port GPIOH

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
