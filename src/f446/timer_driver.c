#include "dig_pins.h"
#include "panic.h"
#include "stm32f4xx.h"
#include "timers.h"

extern volatile uint32_t ping_tmr;
extern volatile uint32_t ping_ledbut_tmr;
extern volatile uint32_t clkout_trigger_tmr;
extern volatile uint32_t loopled_tmr[2];

#define LRCLK_EXTI_LINE EXTI_LINE_4

void init_timekeeper(void) {
	ping_tmr = 0;
	ping_ledbut_tmr = 0;
	clkout_trigger_tmr = 0;
	loopled_tmr[0] = 0;
	loopled_tmr[1] = 0;

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);

	__HAL_RCC_SYSCFG_CLK_ENABLE();

	// Rising edge of PE4
	EXTI_ConfigTypeDef exticonf = {
		.Line = LRCLK_EXTI_LINE,
		.Mode = EXTI_MODE_INTERRUPT,
		.Trigger = EXTI_TRIGGER_RISING,
		.GPIOSel = EXTI_GPIOE,
	};

	EXTI_HandleTypeDef lrclk_exti;
	HAL_EXTI_SetConfigLine(&lrclk_exti, &exticonf);

	HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
}

// Sample Clock EXTI line (I2S2 LRCLK)
void EXTI4_IRQHandler(void) {
	// Compute line mask
	const uint32_t maskline = (1uL << (LRCLK_EXTI_LINE & EXTI_PIN_MASK));

	if ((EXTI->PR & maskline) != 0x00u) {
		// Clear pending bit
		EXTI->PR = maskline;
		update_on_sampleclock();
	}
}

static TIM_HandleTypeDef htim9;

void init_adc_param_update_timer(void) {
	__HAL_RCC_TIM9_CLK_ENABLE();

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};

	//168MHz / prescale=3 ---> 42MHz / 30000 ---> 1.4kHz
	//180MHz / prescale=3 ---> 45MHz / 32142 ---> 1.4kHz

	htim9.Instance = TIM9;
	htim9.Init.Prescaler = 3;
	htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim9.Init.Period = 32142;
	htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim9) != HAL_OK) {
		panic();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK) {
		panic();
	}

	HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 3, 2);
	HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);

	HAL_TIM_Base_Start_IT(&htim9);
}

void TIM1_BRK_TIM9_IRQHandler(void) {
	//Takes 7-8us
	if (__HAL_TIM_GET_FLAG(&htim9, TIM_FLAG_UPDATE) != RESET) {
		if (__HAL_TIM_GET_IT_SOURCE(&htim9, TIM_IT_UPDATE) != RESET) {
			update_adc_params();
			__HAL_TIM_CLEAR_IT(&htim9, TIM_IT_UPDATE);
		}
	}
}
