#include "dig_pins.h"
#include "stm32f4xx.h"
#include "timekeeper.h"

volatile uint32_t ping_tmr;
volatile uint32_t ping_ledbut_tmr;
volatile uint32_t clkout_trigger_tmr;
volatile uint32_t loopled_tmr[2];

void init_timekeeper(void) {
	NVIC_InitTypeDef nvic;
	EXTI_InitTypeDef EXTI_InitStructure;

	ping_tmr = 0;
	ping_ledbut_tmr = 0;
	clkout_trigger_tmr = 0;
	loopled_tmr[0] = 0;
	loopled_tmr[1] = 0;

	//Set Priority Grouping mode to 2-bits for priority and 2-bits for sub-priority
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	SYSCFG_EXTILineConfig(EXTI_CLOCK_GPIO, EXTI_CLOCK_pin);
	EXTI_InitStructure.EXTI_Line = EXTI_CLOCK_line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	nvic.NVIC_IRQChannel = EXTI_CLOCK_IRQ;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvic);
}

// Sample Clock EXTI line (I2S2 LRCLK)
void EXTI_Handler(void) {
	if (EXTI_GetITStatus(EXTI_CLOCK_line) != RESET) {
		update_on_sampleclock();
		EXTI_ClearITPendingBit(EXTI_CLOCK_line);
	}
}

void init_adc_param_update_timer(void) {
	TIM_TimeBaseInitTypeDef tim;

	NVIC_InitTypeDef nvic;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

	nvic.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 3;
	nvic.NVIC_IRQChannelSubPriority = 2;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	//168MHz / prescale=3 ---> 42MHz / 30000 ---> 1.4kHz
	//20000 and 0x1 ==> works well

	TIM_TimeBaseStructInit(&tim);
	tim.TIM_Period = 30000;
	tim.TIM_Prescaler = 0x3;
	tim.TIM_ClockDivision = 0;
	tim.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM9, &tim);

	TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM9, ENABLE);
}

void adc_param_update_IRQHandler(void) {
	//Takes 7-8us
	if (TIM_GetITStatus(TIM9, TIM_IT_Update) != RESET) {
		update_adc_params();
		TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
	}
}
