/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include <stdio.h>
#include "main.h"
#include "string.h"
#include "cmsis_os.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Database.h"
#include "SearchMolrarity.h"
#include "FlashMemory.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define CALLBACKTEST
//#define FLASH_MODE_WRITE
#define FLASH_MODE_READ
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ETH_TxPacketConfig TxConfig;
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

 ETH_HandleTypeDef heth;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

osThreadId defaultTaskHandle;
osThreadId LD1TaskHandle;
osThreadId LD2TaskHandle;
osThreadId LD3TaskHandle;
osThreadId LEDTaskHandle;
osTimerId PTimerHandle;
osTimerId OTimerHandle;


#ifdef FLASH_MODE_WRITE
DATABASE* _pDatabase = NULL;
DATABASE _Database[20] =
{
		{ 19.03,994.42,0.525 },
		{ 19.01,994.48,0.522 },
		{ 19.01,994.59,0.516 },
		{ 19.00,994.58,0.514 },
		{ 18.99,994.55,0.510 },
		{ 18.98,994.59,0.504 },
		{ 18.97,994.49,0.502 },
		{ 18.96,994.55,0.501 },
		{ 18.95,994.64,0.501 },
		{ 18.94,994.65,0.501 },
		{ 18.93,994.55,0.501 },
		{ 18.93,994.57,0.504 },
		{ 18.92,994.53,0.507 },
		{ 18.91,994.54,0.509 },
		{ 18.90,994.57,0.508 },
		{ 18.90,994.74,0.510 },
		{ 18.89,994.67,0.511 },
		{ 18.88,994.46,0.508 },
		{ 18.88,994.47,0.507 },
		{ 18.87,994.45,0.505 }
};

/*Variable used for Erase procedure*/
static FLASH_EraseInitTypeDef EraseInitStruct;
#endif

#ifdef FLASH_MODE_READ
DATABASE* _Database = NULL;
#endif

double _dbInput_Temp[20] =
{
		19.03,
		18.6,
		18.7,
		18.8,
		18.9,
		19.0,
		19.01,
		19.2,
		19.03,
		20,
		21,
		14,
		13,
		17.888,
		19.01,
		19.1,
		19.8,
		19.09,
		19.005,
		18.898
};

double _dbInput_Density[20] =
{
		994.42,
		994.43,
		995.42,
		995.40,
		995.41,
		994.67,
		994.57,
		994.58,
		994.59,
		994.60,
		994.61,
		994.62,
		995.63,
		995.64,
		995.65,
		994.66,
		994.67,
		994.88,
		994.89,
		994.95
};

int _iLength_Database = 20;


uint32_t FirstSector = 0, NbOfSectors = 0;
uint32_t Address = 0, SECTORError = 0;
__IO uint32_t data32 = 0 , MemoryProgramStatus = 0;



/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ETH_Init(void);
void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);
void StartTask04(void const * argument);
void StartLEDTask(void const * argument);
void PTCallback(void const * argument);
void OTCallback(void const * argument);


/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
    (void) HAL_UART_Transmit(&huart3, (uint8_t*) &ch, 1, 100);
    return ch;
}
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
  MX_USB_OTG_FS_PCD_Init();
  MX_USART3_UART_Init();
  MX_ETH_Init();


  /* USER CODE BEGIN 2 */

#ifdef  FLASH_MODE_WRITE

  printf("FlashMemory UnLock\n\r");
  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();
  /* Erase the user Flash area
     (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  printf("FlashMemory EraseInitStruct\n\r");
   /* Get the 1st sector to erase */
   FirstSector = GetSector(FLASH_USER_START_ADDR);
   /* Get the number of sector to erase from 1st sector*/
   NbOfSectors = GetSector(FLASH_USER_END_ADDR) - FirstSector + 1;
   /* Fill EraseInit structure*/
   EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
   EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
   EraseInitStruct.Sector        = FirstSector;					// 지우고자 하는 섹터 시작번호
   EraseInitStruct.NbSectors     = NbOfSectors;					// 지우고자 하는 섹터 개수


   printf("FlashMemory Erase\n\r");
   /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
      you have to make sure that these data are rewritten before they are accessed during code
      execution. If this cannot be done safely, it is recommended to flush the caches by setting the
      DCRST and ICRST bits in the FLASH_CR register. */
   if(HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
   {
     /*
       Error occurred while sector erase.
       User can add here some code to deal with this error.
       SECTORError will contain the faulty sector and then to know the code error on this sector,
       user can call function 'HAL_FLASH_GetError()'
     */
	   printf("Erase Error!		code : %d\n\r", (int)HAL_FLASH_GetError());
   }

   Address = FLASH_USER_START_ADDR;
   printf("FlashMemory Program\n\r");

   uint8_t *ptr = (uint8_t *)_Database;
   uint8_t Data = 0;
   for(int i = 0; i < _iLength_Database; i++)
   {
	   for(int j = 0; j < sizeof(DATABASE); j++)
	   {
		   Data = *((uint8_t*)ptr + (sizeof(DATABASE) * i + j));
		   //printf("Data : %d\n\r", (int)Data);
		   if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, Address, Data) == HAL_OK)
		   {
			   Address++;
		   }
		   else
		   {
			   printf("Program Error!	code : %d\n\r", (int)HAL_FLASH_GetError());
		   }
	   }
   }

   printf("FlashMemory Lock\n\r");

   /* Lock the Flash to disable the flash control register access (recommended
      to protect the FLASH memory against possible unwanted operation) *********/
   HAL_FLASH_Lock();



#endif


#ifdef FLASH_MODE_READ
   _Database = data_DATABASE;
#endif

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

#ifdef CALLBACKTEST
  /* Create the timer(s) */
  /* definition and creation of PTimer */
  osTimerDef(PTimer, PTCallback);
  PTimerHandle = osTimerCreate(osTimer(PTimer), osTimerPeriodic, NULL);

  /* definition and creation of OTimer */
  osTimerDef(OTimer, OTCallback);
  OTimerHandle = osTimerCreate(osTimer(OTimer), osTimerOnce, NULL);
#endif
  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

#ifdef CALLBACKTEST
  /* definition and creation of LD1Task */
  osThreadDef(LD1Task, StartTask02, osPriorityNormal, 0, 128);
  LD1TaskHandle = osThreadCreate(osThread(LD1Task), NULL);

  /* definition and creation of LD2Task */
  osThreadDef(LD2Task, StartTask03, osPriorityNormal, 0, 128);
  LD2TaskHandle = osThreadCreate(osThread(LD2Task), NULL);

  /* definition and creation of LD3Task */
  osThreadDef(LD3Task, StartTask04, osPriorityBelowNormal, 0, 128);
  LD3TaskHandle = osThreadCreate(osThread(LD3Task), NULL);

  /* definition and creation of LEDTask */
  osThreadDef(LEDTask, StartLEDTask, osPriorityNormal, 0, 128);
  LEDTaskHandle = osThreadCreate(osThread(LEDTask), NULL);
#endif
  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
static void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1524;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 4;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
#ifdef CALLBACKTEST
	printf("Sending from StartDefaultTask\n\r");
	osTimerStart(PTimerHandle, 1000);
#else if
	unsigned int iIndex = 0;
	double dbRet = 0;
	int iRet = 0;
#endif

  /* Infinite loop */
  for(;;)
  {
#ifdef CALLBACKTEST
	  printf("Sending from StartDefaultTask\n\r");
#else if

	  dbRet = SearchMolrarity
			  (
				  _Database,
				  _iLength_Database,
				  (double)1.0,
				  (double)1.0,
				  _dbInput_Temp[iIndex % _iLength_Database],
				  _dbInput_Density[iIndex % _iLength_Database]
			  );

	  iRet = (int)(dbRet * 1000);
	  printf("%d\n\r", (int)iRet);

	  iIndex++;
#endif
	  osDelay(1000);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the LD1Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {
	  //printf("String Test!!\n\r");
	  //sprintf(str_num, "%d\n\r", sizeof(str));
	  //HAL_UART_Transmit(&huart3, str, sizeof(str), 100);
	  //HAL_UART_Transmit(&huart3, str_num, sizeof(str_num), 100);
	  HAL_GPIO_TogglePin(GPIOB, MCO_Pin);
	  printf("Task02\n\r");

	  osDelay(1000);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the LD2Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_TogglePin(GPIOB, LD2_Pin);
	  printf("Task03\n\r");
	  osDelay(200);
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief Function implementing the LD3Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void const * argument)
{
  /* USER CODE BEGIN StartTask04 */
  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
	  printf("Task04\n\r");
	  osDelay(300);
  }
  /* USER CODE END StartTask04 */
}

/* USER CODE BEGIN Header_StartLEDTask */
/**
* @brief Function implementing the LEDTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLEDTask */
void StartLEDTask(void const * argument)
{
  /* USER CODE BEGIN StartLEDTask */
  /* Infinite loop */
  for(;;)
  {
	  if((HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)))
	  {
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
		  printf("Sending from StartLEDTask\n\r");
		  printf("%d\n\r", (int)xTaskGetTickCount());
		  osTimerStart(OTimerHandle, 2000);
	  }
	  	  osDelay(100);
  }
  /* USER CODE END StartLEDTask */
}

/* PTCallback function */
void PTCallback(void const * argument)
{
  /* USER CODE BEGIN PTCallback */
	printf("%d\n\r", (int)xTaskGetTickCount());
	HAL_GPIO_TogglePin(GPIOB,  GPIO_PIN_7);
	printf("Sending from PTCallback\n\r");
  /* USER CODE END PTCallback */
}

/* OTCallback function */
void OTCallback(void const * argument)
{
  /* USER CODE BEGIN OTCallback */
	printf("%d\n\r", (int)xTaskGetTickCount());
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	printf("Sending from OTCallback\n\r");
  /* USER CODE END OTCallback */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  printf("Error_Handler!!\n\r");

  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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



