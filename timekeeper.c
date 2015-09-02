/*
 * timekeeper.c
 *
 *  Created on: Mar 29, 2015
 *      Author: design
 */

#include "timekeeper.h"
#include "dig_inouts.h"

volatile uint32_t ping_tmr;
volatile uint32_t ping_ledbut_tmr;
volatile uint32_t clkout_trigger_tmr;
volatile uint32_t pingled_tmr[2];
volatile uint32_t sys_time=0;


inline void inc_tmrs(void){
	ping_tmr++;
	ping_ledbut_tmr++;
	clkout_trigger_tmr++;
	pingled_tmr[0]++;
	pingled_tmr[1]++;
}

inline void reset_ping_ledbut_tmr(void){
	ping_ledbut_tmr=0;
}

inline void reset_ping_tmr(void){
	ping_tmr=0;
}

inline void reset_clkout_trigger_tmr(void){
	clkout_trigger_tmr=0;
}

inline void reset_pingled_tmr(uint8_t channel){
	pingled_tmr[channel]=0;
}


void init_timekeeper(void){
	TIM_TimeBaseInitTypeDef tim;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;


	ping_tmr=0;
	ping_ledbut_tmr=0;
	clkout_trigger_tmr=0;
	pingled_tmr[0]=0;
	pingled_tmr[1]=0;

	//Set Priority Grouping mode to 2-bits for priority and 2-bits for sub-priority
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	SYSCFG_EXTILineConfig(EXTI_CLOCK_GPIO, EXTI_CLOCK_pin);
	EXTI_InitStructure.EXTI_Line = EXTI_CLOCK_line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);


	NVIC_InitStructure.NVIC_IRQChannel = EXTI_CLOCK_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	//  NVIC_SetPriority(EXTI_CLOCK_IRQ, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
	TIM_TimeBaseStructInit(&tim);
	tim.TIM_Period = 65355;
	tim.TIM_Prescaler = 0;
	tim.TIM_ClockDivision = 0;
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM9, &tim);
	TIM_Cmd(TIM9, ENABLE);
	TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);


}

// Sample Clock EXTI line (I2S2 LRCLK)
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_CLOCK_line) != RESET) {
		inc_tmrs();
		EXTI_ClearITPendingBit(EXTI_CLOCK_line);
	}
}


void TIM1_BRK_TIM9_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM9, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
		sys_time++;
	}
}


/*
RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStructure);


TIM_TimeBaseStructInit(&tim);
//tim.TIM_Period = 0xFFFF; //  60 = 2.99MHz or 335ns x 2^32 = 1440 sec or 24minutes
//tim.TIM_Period = 874; //  874 = 206kHz or 4.85ns x 2^32 = 5.8hours before ping_tmr overflow
tim.TIM_Period = 3499; //  should be between 3500 and 3499 for no drift
//tim.TIM_Period = 0xFFFF; //  65535 = 2.75kHz or 364us x 2^32 = 434hours
tim.TIM_Prescaler = 0;
tim.TIM_ClockDivision = 0;
tim.TIM_CounterMode = TIM_CounterMode_Up;
TIM_TimeBaseInit(TIM9, &tim);

TIM_Cmd(TIM9, ENABLE);

TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);
*/
/*
void TIM1_BRK_TIM9_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM9, TIM_IT_Update) != RESET) {

		TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
		inc_tmrs();

	}


}
*/
