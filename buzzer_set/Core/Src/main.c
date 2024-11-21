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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

typedef struct {
    GPIO_TypeDef* Port; // Pointer to GPIO port
    uint16_t Pin;       // Pin number (e.g., GPIO_PIN_0)
} GPIO_PinConfig;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_LEDS 10
#define START_BUTTON_PIN GPIO_PIN_5 // GPIOB
#define RESET_BUTTON_PIN GPIO_PIN_13 // GPIOB

#define BUZZER_PIN GPIO_PIN_8

#define LED_PORT GPIOA
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

// LED pin array
const GPIO_PinConfig led_pins[NUM_LEDS] = {
    {GPIOA, GPIO_PIN_15},
    {GPIOB, GPIO_PIN_10},
    {GPIOA, GPIO_PIN_8},
    {GPIOB, GPIO_PIN_9},
    {GPIOC, GPIO_PIN_7},
    {GPIOB, GPIO_PIN_6},
    {GPIOA, GPIO_PIN_7},
    {GPIOA, GPIO_PIN_6},
    {GPIOC, GPIO_PIN_8},
    {GPIOC, GPIO_PIN_6}
};

// Timer to keep track of countdown timing
volatile uint32_t countdown_timer = 0;
volatile int countdown_index = 0;
volatile uint32_t buzzer_tick = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// code is 0 for start
// code is 1 for timer run out
// code is 2 for reset
// code is 3 for reset + enable output

uint8_t tx_buff[] = {2};
uint8_t rx_buff[1];

static void Tone(uint32_t Frequency) {
    TIM2->ARR = (1000000UL / Frequency) - 1; // Set The PWM Frequency
    TIM2->CCR1 = (TIM2->ARR >> 1); // Set Duty Cycle 50%
}

void Start_PWM(TIM_HandleTypeDef *htim, uint32_t channel) {
    HAL_TIM_PWM_Start(htim, channel);
}

void Stop_PWM(TIM_HandleTypeDef *htim, uint32_t channel) {
    HAL_TIM_PWM_Stop(htim, channel);
}

void PlayJazzyIntro(TIM_HandleTypeDef *htim, uint32_t channel, int baseFreq) {
    // Helper macro for rests
    #define REST(duration) Stop_PWM(htim, channel); HAL_Delay(duration); Start_PWM(htim, channel);

    // Start PWM
    Start_PWM(htim, channel);

    // Smooth jazz intro (add a touch of swing)
    Tone(baseFreq);         // Root note
    HAL_Delay(200);                                      // Slightly swung rhythm
    Tone(baseFreq * 6 / 5); // Minor third
    HAL_Delay(180);
    REST(40);                                           // Short syncopated rest
    Tone(baseFreq * 5 / 4); // Major third
    HAL_Delay(220);
    Tone(baseFreq * 7 / 4); // Minor seventh
    HAL_Delay(250);
    REST(150);

    // Funky walking bassline
    Tone(baseFreq * 2);     // Octave
    HAL_Delay(200);
    Tone(baseFreq * 3 / 2); // Perfect fifth
    HAL_Delay(150);
    Tone(baseFreq * 4 / 3); // Jazzy fourth
    HAL_Delay(150);
    Tone(baseFreq * 6 / 5); // Minor third
    HAL_Delay(150);                                      // Dramatic pause

    // Swing-style improvisation
    for (int i = 0; i < 3; i++) {
        Tone(baseFreq * (i % 2 == 0 ? 6 / 5 : 5 / 4));  // Alternate minor/major thirds
        HAL_Delay(180);
        Tone(baseFreq * 3 / 2);                         // Perfect fifth
        HAL_Delay(200);
    }
    REST(200);

    // Bluesy ending with a jazzy trill
    Tone(baseFreq * 7 / 4);  // Minor seventh
    HAL_Delay(180);
    Tone(baseFreq * 9 / 8);  // Jazzy ninth
    HAL_Delay(150);
    for (int i = 0; i < 6; i++) {                       // Rapid trill
        Tone(baseFreq * 6 / 5);
        HAL_Delay(80);
        Tone(baseFreq * 7 / 6);                         // Blues note (minor second)
        HAL_Delay(80);
    }

    // Strong finish
    Tone(baseFreq);          // Root
    HAL_Delay(300);
    Tone(baseFreq * 3 / 2);  // Perfect fifth
    HAL_Delay(200);
    Tone(baseFreq * 2);      // Octave
    HAL_Delay(600);                                       // Longer sustain for final emphasis

    // Stop PWM
    Stop_PWM(htim, channel);
}

// Note definitions
#define a3f    208
#define b3f    233
#define b3     247
#define c4     261
#define c4s    277
#define e4f    311
#define f4     349
#define a4f    415
#define b4f    466
#define b4     493
#define c5     523
#define c5s    554
#define e5f    622
#define f5     698
#define f5s    740
#define a5f    831
#define rest   -1

// Configuration
volatile int beatlength = 100; // Tempo
float beatseparationconstant = 0.3;

int song1_chorus_melody[] =
{ b4f, b4f, a4f, a4f,
  f5, f5, e5f, b4f, b4f, a4f, a4f, e5f, e5f, c5s, c5, b4f,
  c5s, c5s, c5s, c5s,
  c5s, e5f, c5, b4f, a4f, a4f, a4f, e5f, c5s,
  b4f, b4f, a4f, a4f,
  f5,  f5, e5f, b4f, b4f, a4f, a4f, a5f, c5, c5s, c5, b4f,
  c5s, c5s, c5s, c5s,
  c5s, e5f, c5, b4f, a4f, rest, a4f, e5f, c5s, rest
};

int song1_chorus_rhythmn[]  =
{ 1, 1, 1, 1,
  3, 3, 6, 1, 1, 1, 1, 3, 3, 3, 1, 2,
  1, 1, 1, 1,
  3, 3, 3, 1, 2, 2, 2, 4, 8,
  1, 1, 1, 1,
  3, 3, 6, 1, 1, 1, 1, 3, 3, 3,  1, 2,
  1, 1, 1, 1,
  3, 3, 3, 1, 2, 2, 2, 4, 8, 4
};


// Usage example in your main loop or function
void playSegment(TIM_HandleTypeDef *htim, uint32_t channel, int melody[], int rhythmn[], int length) {
#define REST(duration) Stop_PWM(htim, channel); HAL_Delay(duration); Start_PWM(htim, channel);
    for (int i = 0; i < length; i++) {
        int duration = beatlength * rhythmn[i];
        if (melody[i] == rest) {
            REST(duration);
        } else {
            Tone(melody[i]);
            HAL_Delay(duration);
        }
        REST(duration * beatseparationconstant);
    }
}

// Additional Jazz Notes
#define g3     196
#define a3     220
#define d4     293
#define d4s    311
#define g4     392
#define a4     440
#define d5     587
#define g5     784
#define a5     880
#define d6     1174
// Additional Notes for Bebop Flavor
#define g3s    208
#define d4f    277
#define a4s    466
#define g5s    830
#define a5s    932
#define d5f    622
#define g4s	   415

// Part 1: Intro (Smooth Chromatic Walks)
int jazz_intro_melody[] = {g3, a3, b3, c4, d4, e4f, f4, g4, a4f, g4, b3, c4};
int jazz_intro_rhythmn[] = {2, 2, 1, 2, 2, 1, 2, 4, 2, 2, 2, 4};

// Part 2: Verse (Swing Feel)
int jazz_theme_melody[] = {g4, b4, a4s, g4, f4, e4f, g4s, a4, b4, g4, f4, d4, d4f, g4, f4, e4f, d4};
int jazz_theme_rhythmn[] = {1, 2, 2, 1, 2, 1, 2, 2, 2, 1, 2, 1, 2, 2, 2, 1, 4};

// Part 3: Bridge (High Energy, Jumping Notes)
int jazz_bridge_melody[] = {a4, c5, d5, g5, f5s, a5f, d6, b4, c5s, a4f, f5, e5f, d5, c5, b4f};
int jazz_bridge_rhythmn[] = {4, 2, 6, 3, 2, 4, 6, 4, 3, 4, 6, 3, 2, 3, 8};

// Part 4: Outro (Resolving Down to Calm)
int jazz_outro_melody[] = {c4, b3f, a3, g3, f4, e4f, d4, c4, g3, rest, g3, a3, b3, c4};
int jazz_outro_rhythmn[] = {8, 4, 4, 6, 4, 2, 2, 8, 6, 4, 4, 4, 4, 8};

// Melody and Rhythm Arrays

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

int melody[] = {
    NOTE_C4, NOTE_E4, NOTE_G4, NOTE_E4,   // C-E-G-E
    NOTE_C4, NOTE_E4, NOTE_G4, NOTE_E4,   // C-E-G-E (repeated)
    NOTE_A3, NOTE_C4, NOTE_E4, NOTE_C4,   // A-C-E-C
    NOTE_A3, NOTE_C4, NOTE_E4, NOTE_C4,   // A-C-E-C (repeated)
    NOTE_G3, NOTE_B3, NOTE_D4, NOTE_B3,   // G-B-D-B
    NOTE_G3, NOTE_B3, NOTE_D4, NOTE_B3,   // G-B-D-B (repeated)
    NOTE_G3, NOTE_G3, NOTE_G3, NOTE_G3,   // G-G-G-G
    NOTE_C4, NOTE_E4, NOTE_G4, NOTE_E4,   // C-E-G-E
    NOTE_A3, NOTE_C4, NOTE_E4, NOTE_C4,   // A-C-E-C
    NOTE_G3, NOTE_B3, NOTE_D4, NOTE_B3,   // G-B-D-B
    NOTE_G3, NOTE_G3, NOTE_G3, NOTE_G3,   // G-G-G-G (repeat)
};

int rhythmn[] = {
    4, 4, 4, 4,   // C-E-G-E
    4, 4, 4, 4,   // C-E-G-E (repeated)
    4, 4, 4, 4,   // A-C-E-C
    4, 4, 4, 4,   // A-C-E-C (repeated)
    4, 4, 4, 4,   // G-B-D-B
    4, 4, 4, 4,   // G-B-D-B (repeated)
    4, 4, 4, 4,   // G-G-G-G
    4, 4, 4, 4,   // C-E-G-E
    4, 4, 4, 4,   // A-C-E-C
    4, 4, 4, 4,   // G-B-D-B
    4, 4, 4, 4    // G-G-G-G (repeat)
};

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
//	GPIO_PinState state;
	uint8_t X = 0;
	uint32_t last_tick = HAL_GetTick();
	uint8_t started = 0;
	countdown_index = 0;
	uint8_t reset_down = 0;
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
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  Tone(500);

//  Start_PWM(&htim2, TIM_CHANNEL_2);
//  PlayBootUpSong(1000);
//  Stop_PWM(&htim2, TIM_CHANNEL_2);
//  PlayJazzyIntro(&htim2, TIM_CHANNEL_2, 500);
  Start_PWM(&htim2, TIM_CHANNEL_2);

  // Play Chorus
  playSegment(&htim2, TIM_CHANNEL_2, song1_chorus_melody, song1_chorus_rhythmn, sizeof(song1_chorus_melody) / sizeof(int));

//  playSegment(&htim2, TIM_CHANNEL_2, melody, rhythmn, sizeof(melody) / sizeof(int));

  Stop_PWM(&htim2, TIM_CHANNEL_2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (HAL_GetTick() - last_tick >= 1000) {
		  last_tick = HAL_GetTick();  // Update the last tick
		  printf("Hello World! %u \r\n", X);
		  ++X;
	  }

	  // Start button pressed: begin countdown, enable all LEDs, play sound
	  if (!started && countdown_index >= 0 && HAL_GPIO_ReadPin(GPIOB, START_BUTTON_PIN) == GPIO_PIN_RESET) {
		  printf("Start button is pressed. \r\n");

		  tx_buff[0] = 0;
		  HAL_UART_Transmit(&huart1, tx_buff, 1, 100);

		  started = 1;

		  for (int i = 0; i < NUM_LEDS; ++i) {
			  HAL_GPIO_WritePin(led_pins[i].Port, led_pins[i].Pin, GPIO_PIN_SET);
		  }

		  countdown_index = 0;
		  countdown_timer = HAL_GetTick(); // Initialize timer
	  }

	  // Non-blocking LED countdown logic
	  if (countdown_index < NUM_LEDS && HAL_GetTick() - countdown_timer >= 1000 && started) {
		  HAL_GPIO_WritePin(led_pins[countdown_index].Port, led_pins[countdown_index].Pin, GPIO_PIN_RESET); // Turn off LED
		  countdown_timer = HAL_GetTick(); // Reset timer for next LED
//		  printf("Increment! \r\n");
		  countdown_index++;
	  }

	  if (countdown_index >= NUM_LEDS && started) {
//		  printf("Huh? %u \r\n", countdown_index);
		  started = 0;
		  buzzer_tick = HAL_GetTick() - 400;

		  tx_buff[0] = 1;
		  HAL_UART_Transmit(&huart1, tx_buff, 1, 100);

//		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET); // Turn on buzzer
		  Tone(800);
		  Start_PWM(&htim2, TIM_CHANNEL_2);
	  }

	  if (HAL_GetTick() - buzzer_tick >= 800) {
//		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET); // Turn off buzzer
		  Stop_PWM(&htim2, TIM_CHANNEL_2);
//		  HAL_NVIC_SystemReset();
	  }

	  // Disable inputs after countdown
//	  if (countdown_index >= NUM_LEDS) {
//		  HAL_GPIO_WritePin(COMM_PORT, COMM_PIN, GPIO_PIN_RESET); // Disable further inputs
//	  }
	  if (HAL_UART_Receive(&huart1, rx_buff, 1, 10) == HAL_OK) {
		  printf("rx_buff contents: ");
		  if (rx_buff[0] == 2) {
			  buzzer_tick = HAL_GetTick();
//			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET); // Turn on buzzer
			  Tone(1200);
			  Start_PWM(&htim2, TIM_CHANNEL_2);
			  started = 0; // stop the counter
			  countdown_index = -(countdown_index + 10);
		  }

		  for (size_t i = 0; i < sizeof(rx_buff); i++) {
//			  printf("%u ", rx_buff[i]); // Print as decimal
			  printf("0x%02X ", rx_buff[i]); // Print as hexadecimal
		  }
		  printf(" \r\n");
	  }

	  // Reset button handling
	  if (!reset_down && HAL_GPIO_ReadPin(GPIOB, RESET_BUTTON_PIN) == GPIO_PIN_RESET) {
		  printf("Reset button is pressed. \r\n");
//		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET); // Turn on buzzer
		  Tone(500);
		  Start_PWM(&htim2, TIM_CHANNEL_2);
		  tx_buff[0] = 2;

		  buzzer_tick = HAL_GetTick() - 750;

		  // reset all the states

		  X = 0;
		  last_tick = HAL_GetTick();
		  if (countdown_index >= 0) {
			  countdown_index = 0;
			  countdown_timer = 0;
			  started = 0;
			  for (size_t i = 0; i < NUM_LEDS; ++i) {
				  HAL_GPIO_WritePin(led_pins[i].Port, led_pins[i].Pin, GPIO_PIN_RESET);
			  }
			  HAL_UART_Transmit(&huart1, tx_buff, 1, 100);
		  } else {
			  // previous reset had timer still going
			  started = 1;
			  countdown_index = MAX(abs(countdown_index) - 11, 0);
			  countdown_timer = 0;
			  HAL_UART_Transmit(&huart1, tx_buff, 3, 100);
		  }

		  reset_down = 1;

//		  buzzer_tick = 0;

//		  HAL_NVIC_SystemReset();
	  }

	  // wait until button is released
	  if (reset_down && HAL_GPIO_ReadPin(GPIOB, RESET_BUTTON_PIN) != GPIO_PIN_RESET) {
		  reset_down = 0;
	  }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 249;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
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
  sConfigOC.Pulse = 125;
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
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_6|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin PA6 PA7 PA8
                           PA15 */
  GPIO_InitStruct.Pin = LD2_Pin|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB6 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_6|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB13 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC6 PC7 PC8 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN) {
	printf("Test button press trigger callback \r\n");
}

/**
  * @brief  Retargets the C library printf function to the USART.
  *   None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
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
  __disable_irq();
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
