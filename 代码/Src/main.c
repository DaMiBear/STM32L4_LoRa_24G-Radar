
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "tim.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "fft.h"
#include "stm32l4xx_nucleo_144.h"
#include "loranode.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

/* Private function prototypes -----------------------------------------------*/
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch, 1, 0xFFFF);
	//HAL_UART_Transmit_IT(&hlpuart1,(uint8_t *)&ch, 1);
  return ch;

}
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	int i = 0;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_LPUART1_UART_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&hlpuart1,aRxBuffer1,1);		//使能一次串口接收中断
	HAL_UART_Receive_IT(&huart3,aRxBuffer2,1);		//使能一次串口接收中断
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_ConvertedValue,ADC_CONVERTED_DATA_BUFFER_SIZE);  //以DMA的方式开启ADC转换
	printf("hello!\n");
	BSP_PB_Init(BUTTON_KEY,BUTTON_MODE_GPIO);//蓝色按键
	HAL_Delay(500);	
	//HAL_UART_Transmit(&huart3, (uint8_t *)usart3SendStr, strlen(usart3SendStr), 0xFFFF);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	/*用来判断LoRa节点的初始化*/
	while(1)
	{
		/*如果串口3一帧数据接收完成*/
		if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE)!=RESET)	
		{
			__HAL_UART_CLEAR_IDLEFLAG(&huart3);
			if(saveRxBuffer2[0]!=0)
			{
				printf("Form UART3:\r\n%s",saveRxBuffer2);
				//如果是刚刚上电的初始化信息
				if(strcmp(saveRxBuffer2,"Initialization OK!\r\n")==0)
				{
					/*发送join请求，请求入网*/
					HAL_UART_Transmit(&huart3,(uint8_t *)"at+join=otaa\r\n",strlen("at+join=otaa\r\n"),0xffff);
					printf("获得初始化信息，已发送join请求\r\n");
					joinOrSend = 'j';	//表明刚刚发送的是join
					/*清空串口3的缓冲区----因为遇到break之后，break后面程序的不会在执行*/
					memset(saveRxBuffer2,0,sizeof(saveRxBuffer2));
					bufferLength2 = 0;
					break;
				}
				/*清空串口3的缓冲区*/
				memset(saveRxBuffer2,0,sizeof(saveRxBuffer2));
				bufferLength2 = 0;
			}
		}
	}
  while (1)
  {
		/*如果串口1一帧数据接收完成*/
		if(__HAL_UART_GET_FLAG(&hlpuart1,UART_FLAG_IDLE)!=RESET)	
		{
			__HAL_UART_CLEAR_IDLEFLAG(&hlpuart1);		//清除串口1空闲中断标志位
			if(saveRxBuffer1[0]!=0)
			{
				printf("From LPUART1:\r\n%s",saveRxBuffer1);		//把收到的信息通过串口1打印出来
				//HAL_UART_Transmit(&huart3,(uint8_t *)saveRxBuffer1,strlen(saveRxBuffer1),0xFFFF);	//把串口1收到的信息通过串口3发送出去
				//HAL_UART_Transmit_IT(&hlpuart1,(uint8_t *)saveRxBuffer1,strlen(saveRxBuffer1));
				//HAL_Delay(100);
			
				/*清空串口1的缓冲区*/
				memset(saveRxBuffer1,0,sizeof(saveRxBuffer1));	
				bufferLength1 = 0;
			}
		}
		/*如果串口3一帧数据接收完成*/
		 else if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE)!=RESET)	
		{
			__HAL_UART_CLEAR_IDLEFLAG(&huart3);
			if(saveRxBuffer2[0]!=0)
			{
				printf("Form UART3:\r\n%s",saveRxBuffer2);
				//根据字符串的信息进行处理
				node_UartReturn(saveRxBuffer2);
				/*清空串口3的缓冲区*/
				memset(saveRxBuffer2,0,sizeof(saveRxBuffer2));
				bufferLength2 = 0;
			}
		}
		
		/*按键按下 开始进行采样并计算----如果允许采样和计算*/
		if(/*HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)==1*/enableSample_FFT==1)		
		{	
			//开启定时器，让ADC采集数据
			HAL_TIM_Base_Start_IT(&htim3);	
			while(enableFFTState == 0)	//等待ADC采集数据结束
			{
				HAL_Delay(1);
			}		
			//采集数据完毕，在FFT计算的时候关闭定时器，防止采样序列被重新赋值
			HAL_TIM_Base_Stop_IT(&htim3);	
			/*FFT开始*/
			FFT_Operate();	
			/*FFT结束*/
					//打印相关信息
//				printf("CH1:");
//				getMaxArr(fft_outputbuf1);	
//				printf("CH2:");
//				getMaxArr(fft_outputbuf2);
				
//				for(i = 0; i<FFT_LENGTH;i++)
//				{
//					printf("%f\n",fft_outputbuf1[i]);
//				//	printf("%d		\n",ADC_ConvertedValue[1]);
//				}
				//printf("=====================================================\n");
//				for(i = 0; i<FFT_LENGTH;i++)
//				{
//					printf("%d\n",ADC_ConvertedDate2[i]);
//				}
				//处理FFT之后的数据：获取最大值、获得相位
				processFFtArr(fft_outputbuf1);
				
				enableFFTState = 0;						
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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_LPUART1
                              |RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 16;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the main internal regulator output voltage 
    */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
