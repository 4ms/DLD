/*
 * timekeeper.c - keeps master clock and updates internally used timers
 *
 * Author: Dan Green (danngreen1@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
 */


#include "globals.h"
#include "timekeeper.h"
#include "params.h"
#include "calibration.h"
#include "system_settings.h"
#include "leds.h"
#include "dig_pins.h"


volatile uint32_t ping_tmr;
volatile uint32_t ping_ledbut_tmr;
volatile uint32_t clkout_trigger_tmr;
volatile uint32_t loopled_tmr[2];
extern volatile uint32_t ping_time;
extern uint8_t mode[NUM_CHAN][NUM_CHAN_MODES];
extern uint8_t loop_led_state[NUM_CHAN];

extern uint8_t global_mode[NUM_GLOBAL_MODES];

void inc_tmrs(void)
{
	ping_tmr++;
	ping_ledbut_tmr++;
	clkout_trigger_tmr++;
	loopled_tmr[0]++;
	loopled_tmr[1]++;


	if (clkout_trigger_tmr>=ping_time)
	{
		CLKOUT_ON;
		reset_clkout_trigger_tmr();
	}
	else if (clkout_trigger_tmr >= (ping_time>>1))
	{
		CLKOUT_OFF;
	}
	else if (mode[0][MAIN_CLOCK_GATETRIG]==TRIG_MODE && clkout_trigger_tmr >= TRIG_TIME){
		CLKOUT_OFF;
	}
}

void reset_ping_ledbut_tmr(void)
{
	ping_ledbut_tmr=0;


}

void reset_ping_tmr(void)
{
	ping_tmr=0;
}

void reset_clkout_trigger_tmr(void)
{
	clkout_trigger_tmr=0;

	if (global_mode[QUANTIZE_MODE_CHANGES]!=0)
	{
		process_mode_flags(0);
		process_mode_flags(1);
	}

}

void reset_loopled_tmr(uint8_t channel)
{
	loopled_tmr[channel]=0;

//	if (!mode[channel][CONTINUOUS_REVERSE])
//	{
		if (!global_mode[CALIBRATE] && !global_mode[SYSTEM_SETTINGS])
			loop_led_state[channel]=1;

		if (channel==0) {
			CLKOUT1_ON;
		} else {
			CLKOUT2_ON;
		}
//	}
}


void init_timekeeper(void){
	NVIC_InitTypeDef nvic;
	EXTI_InitTypeDef   EXTI_InitStructure;


	ping_tmr=0;
	ping_ledbut_tmr=0;
	clkout_trigger_tmr=0;
	loopled_tmr[0]=0;
	loopled_tmr[1]=0;

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
void EXTI_Handler(void)
{
	if(EXTI_GetITStatus(EXTI_CLOCK_line) != RESET)
	{
		inc_tmrs();

		if (!global_mode[SYSTEM_SETTINGS] && !global_mode[CALIBRATE])
			update_channel_leds();

		EXTI_ClearITPendingBit(EXTI_CLOCK_line);
	}
}



void init_adc_param_update_timer(void)
{
	TIM_TimeBaseInitTypeDef  tim;

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

void adc_param_update_IRQHandler(void)
{

	//Takes 7-8us
	if (TIM_GetITStatus(TIM9, TIM_IT_Update) != RESET) {

		process_adc();

		if (global_mode[CALIBRATE])
		{
			update_calibration();
			update_calibrate_leds();
		}
		else
			update_params();

		if (global_mode[SYSTEM_SETTINGS])
		{
			update_system_settings();
			update_system_settings_leds();
		}


		check_entering_system_mode();

		update_ping_ledbut();
		update_INF_REV_ledbut(0);
		update_INF_REV_ledbut(1);


		TIM_ClearITPendingBit(TIM9, TIM_IT_Update);

	}
}

