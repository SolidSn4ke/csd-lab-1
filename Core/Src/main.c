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
#include "usart.h"
#include "gpio.h"
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../Drivers/MyDriver/Inc/mydriver.h"

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define RED_MS 12000U
#define GREEN_MS (RED_MS / 4U)
#define SHORT_RED_MS (RED_MS / 4U)

#define GREEN_BLINK_MS 3000U
#define BLINK_HALF_MS 500U
#define YELLOW_MS 2000U

typedef enum
{
  STATE_RED,
  STATE_GREEN,
  STATE_GREEN_BLINK,
  STATE_YELLOW
} TrafficState;

static TrafficState traffic_state;
static uint32_t state_started;
static uint32_t red_duration;

static int green_requested = 0;
static int previous_button = 0;

static void traffic_enter(TrafficState next, uint32_t now)
{
  traffic_state = next;
  state_started = now;

  turn_off_light(RED);
  turn_off_light(GREEN);
  turn_off_light(YELLOW);

  switch (next)
  {
  case STATE_RED:
    red_duration = green_requested ? SHORT_RED_MS : RED_MS;

    green_requested = 0;
    turn_on_light(RED);
    break;

  case STATE_GREEN:
  case STATE_GREEN_BLINK:
    turn_on_light(GREEN);
    break;

  case STATE_YELLOW:
    turn_on_light(YELLOW);
    break;
  }
}

static void traffic_update(uint32_t now, int press_event)
{
  uint32_t elapsed = (uint32_t)(now - state_started);

  if (press_event)
  {
    if (traffic_state == STATE_GREEN_BLINK ||
        traffic_state == STATE_YELLOW)
    {
      green_requested = 1;
    }
    else if (traffic_state == STATE_RED)
    {
      if (elapsed < red_duration &&
          red_duration - elapsed > SHORT_RED_MS)
      {
        red_duration = elapsed + SHORT_RED_MS;
      }
    }
  }

  switch (traffic_state)
  {
  case STATE_RED:
    if (elapsed >= red_duration)
    {
      traffic_enter(STATE_GREEN, now);
    }
    break;

  case STATE_GREEN:
    if (elapsed >= GREEN_MS)
    {
      traffic_enter(STATE_GREEN_BLINK, now);
    }
    break;

  case STATE_GREEN_BLINK:
    if (elapsed >= GREEN_BLINK_MS)
    {
      traffic_enter(STATE_YELLOW, now);
    }
    else if ((elapsed / BLINK_HALF_MS) % 2U == 0U)
    {
      turn_on_light(GREEN);
    }
    else
    {
      turn_off_light(GREEN);
    }
    break;

  case STATE_YELLOW:
    if (elapsed >= YELLOW_MS)
    {
      traffic_enter(STATE_RED, now);
    }
    break;
  }
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
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  Modes uart_mode = POLL;
  if (uart_receive_init(uart_mode) != HAL_OK)
    Error_Handler();

  traffic_enter(STATE_RED, HAL_GetTick());
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  char rx_buffer[128] = {0};
  LineReceiver receiver = {0};
  while (1)
  {
    /* Retry queued TX if HAL could not start it in the callback. */
    (void)uart_transmit_init();

    LineStatus status =
        receive_str(uart_mode, &receiver, rx_buffer, sizeof(rx_buffer));

    if (status == LINE_READY)
    {
      /* Process rx_buffer here before the next receive_str call. */
    }
    else if (status == LINE_OVERFLOW)
    {
      /* The oversized line was discarded; ready for the next line. */
    }

    uint32_t now = HAL_GetTick();

    int button = check_button(now);
    int press_event = button && !previous_button;

    previous_button = button;

    traffic_update(now, press_event);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
