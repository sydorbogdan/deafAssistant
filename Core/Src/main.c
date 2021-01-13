/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "i2s.h"
#include "pdm2pcm.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <string.h>
#include "cs43l22.h"
#include "kiss_fft.h"
#include<math.h>
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
uint16_t txBuf[128];
uint16_t pdmRxBuf[512*4];//64-32 bit frames = 128 half words
float ReBuffer[512];
float ImBuffer[512];
float FFTBuffer[512];
uint16_t MidBuffer[16];
uint8_t txstate = 0;
uint8_t rxstate = 0;


uint16_t fifobuf[256];
uint8_t fifo_w_ptr = 0;
uint8_t fifo_r_ptr = 0;
uint8_t fifo_read_enabled = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void FifoWrite(uint16_t data) {
	fifobuf[fifo_w_ptr] = data;
	fifo_w_ptr++;
}

uint16_t FifoRead() {
	uint16_t val = fifobuf[fifo_r_ptr];
	fifo_r_ptr++;
	return val;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int itoa2(uint16_t value,char *ptr)
     {

        int count=0,temp = value;
        if(value==0)
        {
            *ptr='0';
            return 1;
        }

        if(value<0)
        {
            value*=(-1);
            *ptr++='-';
            count++;
        }
        for(temp=value;temp>0;temp/=10,ptr++);
           *ptr=',';
           *ptr++;
           *ptr='\0';
           *ptr--;
           for(temp=value;temp>0;temp/=10)
        {
            *--ptr=temp%10+'0';
            count++;
        }
        return count+1;
     }
int min(int a,int b){
	if(a>b) return b;
	return a;
}
void start_vibro()
{
	int vibros[4] = {15,50,100,1000};
	double sums[4] = {0,0,0,0};
	int j =0;
	for(int i=1;i<256;i+=1){
		if(i > vibros[j]) j++;
		if(FFTBuffer[i]/100000 > sums[j])

			sums[j] = FFTBuffer[i]/100000;

	}
	sums[2]/=1.5;
	int diff[4] = {0,0,0,0};
	for(int i= 0;i<4;i++){
		for(int j=0;j<4;j++){
			if(sums[i] >= sums[j] - 0.5){
				diff[i]+=1;
			}
			}
		}

	TIM4->CCR1 = diff[0] * 512 - 1;
	TIM4->CCR2 = diff[1] * 512 - 1;
	TIM4->CCR3 = diff[2] * 512 - 1;
	TIM4->CCR4 = diff[3] * 512 - 1;

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2S2_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_CRC_Init();
  MX_PDM2PCM_Init();
  MX_USB_DEVICE_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	CS43_Init(hi2c1,MODE_I2S);
	CS43_SetVolume(60);//70
	CS43_Enable_RightLeft(CS43_RIGHT_LEFT);
	CS43_Start();
  //HAL_I2S_Transmit_DMA(&hi2s3, &txBuf[0], 64);// 64-32 bit frames to external i2s DAC
  HAL_I2S_Receive_DMA(&hi2s2, &pdmRxBuf[0],64);//64-32 bit frames from PDM microphone
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_4);
  TIM4->CCR1 = 0;
  TIM4->CCR2 = 0;
  TIM4->CCR3 = 0;
  TIM4->CCR4 = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  const char str[] = "Hello!\n";
  uint16_t data_in[128];
  int rxIter = 0;
  int k =0;
  while (1)
  {

	  /*
	  for (int i=0; i<16; i++) {
		  char c[500];
		  int x = itoa2(MidBuffer[i],c);
		  CDC_Transmit_FS(c,x);
		  HAL_Delay(5);
		  CDC_Transmit_FS(", ",2);
		  HAL_Delay(5);
	  }
	  CDC_Transmit_FS('\n',1);
	  CDC_Transmit_FS('\n',1);
	  CDC_Transmit_FS('\n',1);
	  CDC_Transmit_FS('\n',1);

	  HAL_Delay(500);
	*/


	  volatile HAL_StatusTypeDef result = HAL_I2S_Receive(&hi2s2, data_in,32,1000);
	  if (result == HAL_OK) {
		  for(int i=0;i<32;i+=2)
			  pdmRxBuf[rxIter++] = (int16_t) data_in[i];

	  }



		 if (rxIter == 512 * 4) {//RxHalfCpltCallba
			for(int y =0;y<64*32;y+=64){
				PDM_Filter(&pdmRxBuf[y],&MidBuffer[0], &PDM1_filter_handler);//filter first 64x16bit received PDM data
				for(int i=0;i<16;i++){
					ReBuffer[i+y/4] = MidBuffer[i];
				}
			}
			FFT(ReBuffer, ImBuffer, 512, 9, -1);
			for(int i=0;i<512;i++){
				FFTBuffer[i] = ReBuffer[i] * ReBuffer[i] + ImBuffer[i] * ImBuffer[i];
				FFTBuffer[i] = sqrt(FFTBuffer[i]);
				ImBuffer[i] = 0;
			}
	    	rxIter = 0;
	    	k+=1;
	    	start_vibro();
		 }


//	    if (txstate==1) {//TxHalfCpltCallback
//	    	if (fifo_read_enabled==1) {//first 128 (64X2 channels) PCM samples to external i2s DAC
//				for (int i=0; i<64;i=i+4) {
//					uint16_t data = FifoRead();
//					txBuf[i] = data;// 16bit!!! data for first 24 bit channel
//					txBuf[i+2] = data;// 16bit!!! data for second 24 bit channel
//				}
//	    	}
//	    	txstate=0;
//	    }
//
//	    if (txstate==2) {//TxCpltCallback
//	    	if (fifo_read_enabled==1) {//second 64 PCM samples
//				for (int i=64; i<128;i=i+4) {
//					uint16_t data = FifoRead();
//					txBuf[i] = data;// 16bit!!! data for first 24 bit channel
//					txBuf[i+2] = data;// 16bit!!! data for second 24 bit channel
//				}
//
//			}
//	    	txstate=0;
//	    }

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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 200;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 5;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_I2S_TxHalfCpltCallback (I2S_HandleTypeDef *hi2s) {
	txstate = 1;
}

void HAL_I2S_TxCpltCallback (I2S_HandleTypeDef *hi2s) {
	txstate = 2;
}

void HAL_I2S_RxHalfCpltCallback (I2S_HandleTypeDef *hi2s) {
	rxstate = 1;
}

void HAL_I2S_RxCpltCallback (I2S_HandleTypeDef *hi2s) {
	rxstate = 2;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
