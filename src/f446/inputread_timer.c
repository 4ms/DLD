#include "panic.h"
#include "stm32f4xx.h"

static TIM_HandleTypeDef htim10;
static TIM_HandleTypeDef htim4;

void init_inputread_timer(void) {
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};

	// Buttons and button jacks reading timer
	__HAL_RCC_TIM4_CLK_ENABLE();

	// 28kHz
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 1;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 3214;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
		panic();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
		panic();
	}

	HAL_NVIC_SetPriority(TIM4_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);

	HAL_TIM_Base_Start_IT(&htim4);

	// Ping jack timer
	__HAL_RCC_TIM10_CLK_ENABLE();

	// 33.3kHz

	htim10.Instance = TIM10;
	htim10.Init.Prescaler = 1;
	htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim10.Init.Period = 2702;
	htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim10) != HAL_OK) {
		panic();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim10, &sClockSourceConfig) != HAL_OK) {
		panic();
	}

	HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

	HAL_TIM_Base_Start_IT(&htim10);
}

// Defined in button_jacks.c
void ping_jack_read(void);
void buttons_jacks_read(void);

void TIM1_UP_TIM10_IRQHandler(void) {
	if (__HAL_TIM_GET_FLAG(&htim10, TIM_FLAG_UPDATE) != RESET) {
		if (__HAL_TIM_GET_IT_SOURCE(&htim10, TIM_IT_UPDATE) != RESET) {
			ping_jack_read();
			__HAL_TIM_CLEAR_IT(&htim10, TIM_IT_UPDATE);
		}
	}
}

void TIM4_IRQHandler(void) {
	if (__HAL_TIM_GET_FLAG(&htim4, TIM_FLAG_UPDATE) != RESET) {
		if (__HAL_TIM_GET_IT_SOURCE(&htim4, TIM_IT_UPDATE) != RESET) {
			buttons_jacks_read();
			__HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);
		}
	}
}
