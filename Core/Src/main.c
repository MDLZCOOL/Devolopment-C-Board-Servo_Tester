/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "retarget.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NO 0
#define YES 1
#define PWM_MAX 2500
#define PWM_MIN 500
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int angle = 0;
uint8_t uint8_angle[128] = {0};
uint8_t angle_lower[128] = {0};
uint8_t angle_upper[128] = {0};
uint8_t number_channels[1] = {0};
int Compare = 0;
uint8_t optNumber[50] = {0};
volatile double voltage_vrefint_proportion = 8.0586080586080586080586080586081e-4f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
int trans2Compare(int a);

void plain_servo_test(int if_multi);

void interval_servo_test(int if_multi);

int rtnTIMCHANNEL(int num);

void quick_servo_test(int if_multi);

void ShowList();

void use_multi_channels();

void use_direct_PWM();

int deterifHELP(const uint8_t *String);

void deterPWMChannel(uint8_t number[], int compare);

double get_battery_voltage(void);

int init_vrefint_reciprocal(void);

static uint16_t adcx_get_chx_value(ADC_HandleTypeDef *ADCx, uint32_t ch);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint16_t adcx_get_chx_value(ADC_HandleTypeDef *ADCx, uint32_t ch) {
    static ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ch;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES; //ADC_SAMPLETIME_3CYCLES;
    if (HAL_ADC_ConfigChannel(ADCx, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    HAL_ADC_Start(ADCx);
    HAL_ADC_PollForConversion(ADCx, 10);
    return (uint16_t) HAL_ADC_GetValue(ADCx);
}

int init_vrefint_reciprocal(void) {
    uint8_t i = 0;
    uint32_t total_adc = 0;
    for (i = 0; i < 200; i++) {
        total_adc += adcx_get_chx_value(&hadc1, ADC_CHANNEL_VREFINT);
    }
    voltage_vrefint_proportion = 200 * 1.2f / total_adc;
    return 0;
}

double get_battery_voltage(void) {
    double voltage;
    uint16_t adcx = 0;
    adcx = adcx_get_chx_value(&hadc3, ADC_CHANNEL_8);
    //(22K ¦¸ + 200K ¦¸)  / 22K ¦¸ = 10.090909090909090909090909090909
    voltage = (double) adcx * voltage_vrefint_proportion * 10.090909090909090909090909090909f;
    return voltage;
}

int trans2Compare(int a) {
    return (a * 1.0f / 180) * (PWM_MAX - PWM_MIN) + PWM_MIN;
}

void use_direct_PWM() {
    uint8_t uint8_PWM_value[1] = {0};
    int int_PWM_value;
    HAL_Delay(100);
    printf("\r\nPlease input direct PWM value\r\n");
    scanf("%s", uint8_PWM_value);
    HAL_Delay(100);
    int_PWM_value = atoi(uint8_PWM_value);
    printf("Setting...");
    HAL_Delay(100);
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, int_PWM_value);
    HAL_Delay(100);
    printf("Set OK");
}

void ShowList() {
    HAL_Delay(100);
    printf("\r\n########## Welcome to Embeded Toolbox ##########\r\n");
    HAL_Delay(100);
    printf("\r\nPlease select from below: \r\n");
    printf("\r\n1. Plain Servo Test\r\n"
        "\r\n2. Interval Servo Test\r\n"
        "\r\n3. Quick Servo Test\r\n"
        "\r\n4. Use Multi Channels\r\n"
        "\r\n5. Direct Servo Test (PWM500-2500)\r\n");
    HAL_Delay(100);
    printf("\r\nYou can input \"help\" or \"HELP\" to get help\r\n");
    printf("\r\nBattery voltage is %.2f\r\n", get_battery_voltage());
}

void plain_servo_test(int if_multi) {
    HAL_Delay(100);
    printf("\r\nPlease input your intended angle: \r\n");
    scanf("%s", uint8_angle);
    while (1) {
        angle = atoi(uint8_angle);
        if (angle > 180 || angle < 0) {
            printf("\r\nAngle is invalid, please check your input!\r\n");
            HAL_Delay(1000);
            if (NO == if_multi) {
                plain_servo_test(NO);
            } else {
                plain_servo_test(YES);
            }
        }
        HAL_Delay(100);
        printf("\r\nAngle is %d\r\n", angle);
        HAL_Delay(100);
        printf("\r\nSetting...\r\n");
        HAL_Delay(100);
        if (NO == if_multi) {
            __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, trans2Compare(angle));
        } else {
            deterPWMChannel(number_channels, trans2Compare(angle));
        }
        printf("\r\nSet OK\r\n");
        HAL_Delay(100);
        printf("\r\nYou can input \"quit\" or \"QUIT\" to quit servo test or input angle to continue\r\n");
        scanf("%s", uint8_angle);
        if (NO == strcmp(uint8_angle, "quit") || NO == strcmp(uint8_angle, "QUIT")) {
            return;
        }
    }
}

void use_multi_channels() {
    HAL_Delay(100);
    printf("\r\nPlease input the number of enabled channels\r\n");
    scanf("%s", number_channels);
    HAL_Delay(100);
    printf("\r\nPlease select from below: \r\n");
    printf("\r\n1. Plain Servo Test\r\n"
        "\r\n2. Interval Servo Test\r\n"
        "\r\n3. Quick Servo Test\r\n"
        "\r\n4. Direct Servo Test (PWM500-2500)\r\n");
    scanf("%s", optNumber);
    HAL_Delay(100);
    if (atoi(optNumber) == 1) {
        plain_servo_test(YES);
    }
    if (atoi(optNumber) == 2) {
        interval_servo_test(YES);
    }
    if (atoi(optNumber) == 3) {
        quick_servo_test(YES);
    }
    if (atoi(optNumber) == 4) {
        use_direct_PWM();
    }
}

void interval_servo_test(int if_multi) {
    int int_angle_lower, int_angle_upper, angle_lower_compare, angle_upper_compare;
    HAL_Delay(100);
    printf("\r\nPlease input the lower, upper limits and step length of the angle: \r\n");
    scanf("%s %s", angle_lower, angle_upper);
    while (1) {
        int_angle_lower = atoi(angle_lower);
        int_angle_upper = atoi(angle_upper);
        if (!(0 <= int_angle_lower && int_angle_lower <= 180 &&
              0 <= int_angle_upper && int_angle_upper <= 180 &&
              int_angle_lower <= int_angle_upper)) {
            printf("\r\nAngle is invalid, please check your input!\r\n");
            HAL_Delay(1000);
            if (NO == if_multi) {
                interval_servo_test(NO);
            } else {
                interval_servo_test(YES);
            }
        }
        angle_lower_compare = trans2Compare(int_angle_lower);
        angle_upper_compare = trans2Compare(int_angle_upper);
        if (NO == if_multi) {
            __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, angle_lower_compare);
        } else {
            deterPWMChannel(number_channels, angle_lower_compare);
        }
        HAL_Delay(100);
        printf("\r\nLower angle is %d ¡ã\r\n", int_angle_lower);
        printf("\r\nUpper angle is %d ¡ã\r\n", int_angle_upper);
        HAL_Delay(1000);
        printf("\r\nRotating...\r\n");
        HAL_Delay(200);
        for (int i = angle_lower_compare; i < angle_upper_compare; i += 20) {
            HAL_Delay(10);
            if (NO == if_multi) {
                __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, i);
            } else {
                deterPWMChannel(number_channels, i);
            }
        }
        printf("\r\nRotate Finished\r\n");
        HAL_Delay(100);
        printf("\r\nYou can input \"quit\" or \"QUIT\" to quit servo test or input angle to continue\r\n");
        scanf("%s", angle_lower);
        if (NO == strcmp(angle_lower, "quit") || NO == strcmp(angle_lower, "QUIT")) {
            return;
        }
        scanf("%s", angle_upper);
    }
}

void quick_servo_test(int if_multi) {
    HAL_Delay(100);
    printf("\r\nBegin quick servo test\r\n");
    HAL_Delay(100);
    for (int i = 500; i < 2500; i += 10) {
        HAL_Delay(10);
        if (NO == if_multi) {
            __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, i);
        } else {
            deterPWMChannel(number_channels, i);
        }
    }
    HAL_Delay(100);
    printf("\r\nQuick servo test finish\r\n");
    HAL_Delay(100);
    printf("\r\nGo back...\r\n");
    HAL_Delay(1000);
}

int rtnTIMCHANNEL(int num) {
    if (num == 1) {
        return TIM_CHANNEL_1;
    } else if (num == 2) {
        return TIM_CHANNEL_2;
    } else if (num == 3) {
        return TIM_CHANNEL_3;
    } else return TIM_CHANNEL_4;
}

void deterPWMChannel(uint8_t number[], int compare) {
    int int_number = atoi(number);
    if (int_number <= 4) {
        for (int i = 1; i <= int_number; i++) {
            __HAL_TIM_SetCompare(&htim1, rtnTIMCHANNEL(i), compare);
        }
    } else {
        __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_ALL, compare);
        for (int i = 1; i < int_number - 3; i++) {
            __HAL_TIM_SetCompare(&htim8, rtnTIMCHANNEL(i), compare);
        }
    }
}

int deterifHELP(const uint8_t *String) {
    if (NO == strcmp(String, "HELP") || NO == strcmp(String, "help")) {
        HAL_Delay(100);
        printf("\r\nAuthor: MDLZCOOL\r\n"
            "\r\nWebsite: https://mdlzcool.github.io\r\n"
            "\r\nEmail: mdlzcool@foxmail.com\r\n"
            "\r\nGithub: https://github.com/MDLZCOOL/Devolopment-C-Board-Servo_Tester\r\n");
        HAL_Delay(1000);
        return YES;
    } else return NO;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
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
    MX_TIM1_Init();
    MX_USART1_UART_Init();
    MX_TIM8_Init();
    MX_ADC1_Init();
    MX_ADC3_Init();
    /* USER CODE BEGIN 2 */
    HAL_TIM_Base_Start(&htim1);
    HAL_TIM_Base_Start(&htim8);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
    RetargetInit(&huart1);
    init_vrefint_reciprocal();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        ShowList();
        scanf("%s", optNumber);
        deterifHELP(optNumber);

        int num = atoi(optNumber);

        switch (num) {
            case 1:
                plain_servo_test(NO);
                break;
            case 2:
                interval_servo_test(NO);
                break;
            case 3:
                quick_servo_test(NO);
                break;
            case 4:
                use_multi_channels();
                break;
            case 5:
                use_direct_PWM();
                break;
            default:
                printf("Invalid option number: %s\n", optNumber);
                break;
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
void SystemClock_Config(void) {
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
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 6;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
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
