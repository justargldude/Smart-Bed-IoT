/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

#include "drv_loadcell.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
loadcell_t sensor_front_left;
loadcell_t sensor_front_right;
loadcell_t sensor_back_left;
loadcell_t sensor_back_right;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
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
  /* USER CODE BEGIN 2 */
  loadcell_init_dwt();
  
  loadcell_init(&sensor_front_left, GPIOA, FRONT_LEFT_INPUT_Pin, GPIOA, FRONT_LEFT_OUTPUT_Pin); 
                
  loadcell_init(&sensor_front_right, GPIOA, FRONT_RIGHT_INPUT_Pin, GPIOA, FRONT_RIGHT_OUTPUT_Pin);
                
  loadcell_init(&sensor_back_left, GPIOA, BACK_LEFT_INPUT_Pin, GPIOA, BACK_LEFT_OUTPUT_Pin);
                
  loadcell_init(&sensor_back_right, GPIOA, BACK_RIGHT_INPUT_Pin, GPIOA, BACK_RIGHT_OUTPUT_Pin);
  
  HAL_Delay(1000);
	
	/* HAL_Delay(3000);
  loadcell_tare(&sensor_front_left);
	HAL_Delay(3000);
  loadcell_tare(&sensor_front_right);
	HAL_Delay(3000);
	loadcell_tare(&sensor_back_left);
	HAL_Delay(3000);
	loadcell_tare(&sensor_back_right);
	
	
  HAL_Delay(3000);
  int32_t raw_fl = loadcell_read_average(&sensor_front_left, 20);
  float scale_fl = (float)(raw_fl - sensor_front_left.offset) / 1000.0f;
	HAL_Delay(3000);
	int32_t raw_fr = loadcell_read_average(&sensor_front_right, 20);
  float scale_fr = (float)(raw_fr - sensor_front_right.offset) / 1000.0f;
  HAL_Delay(3000);
  int32_t raw_bl = loadcell_read_average(&sensor_back_left, 20);
  float scale_bl = (float)(raw_bl - sensor_back_left.offset) / 1000.0f;
	HAL_Delay(3000);
	int32_t raw_br = loadcell_read_average(&sensor_back_right, 20);
  float scale_br = (float)(raw_br - sensor_back_right.offset) / 1000.0f; */
	
	sensor_front_left.offset = 8432156;
  sensor_front_left.scale = 420.5f;
  
  sensor_front_right.offset = 8431200;
  sensor_front_right.scale = 418.3f;
  
  sensor_back_left.offset = 8433500;
  sensor_back_left.scale = 422.1f;
  
  sensor_back_right.offset = 8430800;
  sensor_back_right.scale = 419.7f;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {	
		float weight_fl = loadcell_get_weight(&sensor_front_left);
    float weight_fr = loadcell_get_weight(&sensor_front_right);
    float weight_bl = loadcell_get_weight(&sensor_back_left);
    float weight_br = loadcell_get_weight(&sensor_back_right);
		
    float total_weight = weight_fl + weight_fr + weight_bl + weight_br;
		HAL_Delay(200);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, FRONT_LEFT_OUTPUT_Pin|FRONT_RIGHT_OUTPUT_Pin|BACK_RIGHT_OUTPUT_Pin|BACK_LEFT_OUTPUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : FRONT_LEFT_OUTPUT_Pin FRONT_RIGHT_OUTPUT_Pin BACK_RIGHT_OUTPUT_Pin BACK_LEFT_OUTPUT_Pin */
  GPIO_InitStruct.Pin = FRONT_LEFT_OUTPUT_Pin|FRONT_RIGHT_OUTPUT_Pin|BACK_RIGHT_OUTPUT_Pin|BACK_LEFT_OUTPUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : FRONT_LEFT_INPUT_Pin FRONT_RIGHT_INPUT_Pin BACK_RIGHT_INPUT_Pin BACK_LEFT_INPUT_Pin */
  GPIO_InitStruct.Pin = FRONT_LEFT_INPUT_Pin|FRONT_RIGHT_INPUT_Pin|BACK_RIGHT_INPUT_Pin|BACK_LEFT_INPUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
