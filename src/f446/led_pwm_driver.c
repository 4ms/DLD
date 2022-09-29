#include "leds.h"
#include "panic.h"
#include "stm32f4xx.h"

static TIM_HandleTypeDef htim2;

void init_LED_PWM_IRQ(void) {
	__HAL_RCC_TIM2_CLK_ENABLE();

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};

	//180MHz / 2(APB clock) / 1(prescaler) / 4687 = 19.2kHz

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 4687;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		panic();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		panic();
	}

	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 2);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	HAL_TIM_Base_Start_IT(&htim2);
}

//runs @ 52uS (19.2kHz), with 32 steps => 1.6ms PWM period = 564Hz
void TIM2_IRQHandler(void) {
	if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET) {
		if (__HAL_TIM_GET_IT_SOURCE(&htim2, TIM_IT_UPDATE) != RESET) {
			update_led_pwm_handler();
			__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
		}
	}
}
