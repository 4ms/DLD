#include "leds.h"
#include "stm32f4xx.h"

void init_LED_PWM_IRQ(void) {
	TIM_TimeBaseInitTypeDef tim;
	NVIC_InitTypeDef nvic;

	RCC_APB1PeriphClockCmd(LED_TIM_RCC, ENABLE);

	nvic.NVIC_IRQChannel = LED_TIM_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 2;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	TIM_TimeBaseStructInit(&tim);
	//	tim.TIM_Period = 17500; //168MHz / 2 / 17500 = 4.8kHz (208.3us) ... / 32 =
	tim.TIM_Period = 4375; //168MHz / 2 / 4375 = 19.2kHz
	tim.TIM_Prescaler = 0;
	tim.TIM_ClockDivision = 0;
	tim.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(LED_TIM, &tim);
	TIM_ITConfig(LED_TIM, TIM_IT_Update, ENABLE);
	TIM_Cmd(LED_TIM, ENABLE);
}

//runs @ 208uS (4.8kHz), with 32 steps => 6.6ms PWM period = 150Hz
void LED_PWM_IRQHandler(void) {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		update_led_pwm_handler();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
