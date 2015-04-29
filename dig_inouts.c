/*
 * dig_inouts.c
 */

#include "stm32f4xx.h"

#include "dig_inouts.h"
#include "params.h"
#include "looping_delay.h"


extern volatile uint32_t ping_tmr;
extern volatile uint32_t ping_ledbut_tmr;
extern volatile uint32_t clkout_trigger_tmr;
extern volatile uint32_t pingled_tmr[2];

extern volatile uint32_t divmult_time[2];
extern volatile uint32_t ping_time;

extern uint8_t flag_ping_was_changed;

extern float param[2][6];

uint8_t flag_inf_change[2]={0,0};
uint8_t flag_time_param_changed[2]={0,0};

uint8_t ping_button_state=0;
uint8_t ping_jack_state=0;


void init_dig_inouts(void){
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);

	//Configure outputs
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Speed = GPIO_Speed_25MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//LEDs
	RCC_AHB1PeriphClockCmd(LED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_OVLD1 | LED_OVLD2;	GPIO_Init(LED_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(PINGBUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_PINGBUT_pin;	GPIO_Init(LED_PINGBUT_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(INF1_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_INF1_pin;	GPIO_Init(LED_INF1_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(INF2_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_INF2_pin;	GPIO_Init(LED_INF2_GPIO, &gpio);


	//CLKOUT
	RCC_AHB1PeriphClockCmd(CLKOUT_RCC, ENABLE);
	gpio.GPIO_Pin = CLKOUT_pin;	GPIO_Init(CLKOUT_GPIO, &gpio);
	CLKOUT_OFF;

	//DEBUG pins
	RCC_AHB1PeriphClockCmd(DEBUG_RCC, ENABLE);

	gpio.GPIO_Pin = DEBUG0;	GPIO_Init(DEBUG0_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG1;	GPIO_Init(DEBUG1_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG2;	GPIO_Init(DEBUG2_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG3;	GPIO_Init(DEBUG3_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG4;	GPIO_Init(DEBUG4_GPIO, &gpio);

	 DEBUG0_OFF;
	 DEBUG1_OFF;
	 DEBUG2_OFF;
	 DEBUG3_OFF;
	 DEBUG4_OFF;


	//Configure inputs
	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_PuPd = GPIO_PuPd_UP;

	//Div/Mult switches
	RCC_AHB1PeriphClockCmd(DIVSW_RCC, ENABLE);
	RCC_AHB1PeriphClockCmd(TIMESW_RCC, ENABLE);

	gpio.GPIO_Pin = DIVSW_CH1_T1_pin;	GPIO_Init(DIVSW_CH1_T1_GPIO, &gpio);
	gpio.GPIO_Pin = DIVSW_CH1_T2_pin;	GPIO_Init(DIVSW_CH1_T2_GPIO, &gpio);
	gpio.GPIO_Pin = DIVSW_CH2_T1_pin;	GPIO_Init(DIVSW_CH2_T1_GPIO, &gpio);
	gpio.GPIO_Pin = DIVSW_CH2_T2_pin;	GPIO_Init(DIVSW_CH2_T2_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH1_T1_pin;	GPIO_Init(TIMESW_CH1_T1_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH1_T2_pin;	GPIO_Init(TIMESW_CH1_T2_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH2_T1_pin;	GPIO_Init(TIMESW_CH2_T1_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH2_T2_pin;	GPIO_Init(TIMESW_CH2_T2_GPIO, &gpio);


	//Ping button and jack
	RCC_AHB1PeriphClockCmd(PING_RCC, ENABLE);

	gpio.GPIO_Pin = PINGBUT_pin;	GPIO_Init(PINGBUT_GPIO, &gpio);

	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio.GPIO_Pin = PINGJACK_pin;	GPIO_Init(PINGJACK_GPIO, &gpio);
	gpio.GPIO_Pin = GPIO_Pin_10;	GPIO_Init(GPIOC, &gpio);


	// Inf Repeat buttons and jacks
	RCC_AHB1PeriphClockCmd(INF_RCC, ENABLE);

	gpio.GPIO_PuPd = GPIO_PuPd_UP;

	gpio.GPIO_Pin = INF1BUT_pin;	GPIO_Init(INF1BUT_GPIO, &gpio);
	gpio.GPIO_Pin = INF2BUT_pin;	GPIO_Init(INF2BUT_GPIO, &gpio);

	gpio.GPIO_PuPd = GPIO_PuPd_DOWN;

	gpio.GPIO_Pin = INF1JACK_pin;	GPIO_Init(INF1JACK_GPIO, &gpio);
	gpio.GPIO_Pin = INF2JACK_pin;	GPIO_Init(INF2JACK_GPIO, &gpio);



}

/**exti_ins.c**/

void init_EXTI_inputs(void){
	  EXTI_InitTypeDef   EXTI_InitStructure;
	  NVIC_InitTypeDef   NVIC_InitStructure;

	  //Set Priority Grouping mode to 2-bits for priority and 2-bits for sub-priority
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	  SYSCFG_EXTILineConfig(EXTI_PINGJACK_GPIO, EXTI_PINGJACK_pin);
	  EXTI_InitStructure.EXTI_Line = EXTI_PINGJACK_line;
	  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	  EXTI_Init(&EXTI_InitStructure);

	  NVIC_InitStructure.NVIC_IRQChannel = EXTI_PINGJACK_IRQ;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	  NVIC_Init(&NVIC_InitStructure);


}

void EXTI2_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_PINGJACK_line) != RESET)
  {
	  //This should be software de-bounced, we can't rely on the external clock having a perfectly sharp rising edge...
	  //Probably need to set the interrupt to record both edges, and ignore the up if a down happens shortly thereafter...?
	  //Or grab PEG's ping code, which grabs the ping_tmr in the interrupt uses that to flag the main loop to check again if the jack is still high
	  //..and if so, then it initiates the new timing information

	  ping_button_state = 0;

		if (ping_jack_state==1){ //second time we got a rising edge
			ping_jack_state = 0;

			//Log how much time has elapsed since last ping
			ping_time=ping_tmr;

			//Reset the timers
			ping_ledbut_tmr=0;
			clkout_trigger_tmr=0;

			//Flag to update the divmult parameters
			flag_ping_was_changed=1;
		} else {

			// This is the first rising edge, so start the ping timer
			ping_tmr = 0;
			ping_jack_state = 1;
		}


    EXTI_ClearITPendingBit(EXTI_PINGJACK_line);
  }
}




/*** read_buttons.c ****/

void init_inputread_timer(void){
	TIM_TimeBaseInitTypeDef  tim;

	NVIC_InitTypeDef nvic;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	nvic.NVIC_IRQChannel = TIM4_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 3;
	nvic.NVIC_IRQChannelSubPriority = 3;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	TIM_TimeBaseStructInit(&tim);
	tim.TIM_Period = 32767; //168MHZ / 2 / 32767 = 2.56kHz
	tim.TIM_Prescaler = 0;
	tim.TIM_ClockDivision = 0;
	tim.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM4, &tim);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM4, ENABLE);
}

uint16_t State[6] = {0,0,0,0,0,0}; // Current debounce status


// This handy routine is called every 0.39ms by the TIMER 4 interrupt.
// It checks each button and digital input jack to see if it's been low for a certain number of cycles,
// and high for a certain number of cycles. We shift 0's and 1's down a 16-bit variable (State[]) to indicate high/low status.
// 16 bits x 0.39us means we can't change status any faster than 6.2ms or 160Hz.
// This could be sped up by checking less bits, or by running TIM4 faster (shorter Period).

void TIM4_IRQHandler(void)
{
	uint16_t t;

	// Clear TIM4 update interrupt
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

	// Check Ping Button

	if (PINGBUT) {
		LED_PINGBUT_ON;
		t=0xe000;
	} else
		t=0xe001;
	State[0]=(State[0]<<1) | t;
	if (State[0]==0xff00){ 						//1111 1111 0000 0000 = not pressed for 8 cycles , then pressed for 8 cycles

		//Clear the ping jack state
		ping_jack_state = 0;

		if (ping_button_state==1){ //second time we pressed the button
			ping_button_state = 0;

			//Log how much time has elapsed since last ping
			ping_time=ping_tmr;

			//Reset the timers
			ping_ledbut_tmr=0;
			clkout_trigger_tmr=0;

			//Flag to update the divmult parameters
			flag_ping_was_changed=1;
		} else {

			// This is the first button press, so start the ping timer
			ping_tmr = 0;
			ping_button_state = 1;
		}



	}

	if (INF1BUT) t=0xe000; else t=0xe001;
	State[1]=(State[1]<<1) | t;
	if (State[1]==0xf000){
		flag_inf_change[0]=1;
		//param[0][INF]=1.0-param[0][INF];
	}

	if (INF2BUT) t=0xe000; else t=0xe001;
	State[2]=(State[2]<<1) | t;
	if (State[2]==0xf000){
		flag_inf_change[1]=1;
		//param[1][INF]=1.0-param[1][INF];
	}

	if (INF1JACK) t=0xe000; else t=0xe001;
	State[3]=(State[3]<<1) | t;
	if (State[3]==0xfff8){					//1111 1111 1111 1000 = gate low for 13 cycles (5ms), followed by gate high for 3 cycles (1.1ms)
		flag_inf_change[0]=1;
	}

	if (INF2JACK) t=0xe000; else t=0xe001;
	State[4]=(State[4]<<1) | t;
	if (State[4]==0xfff8){
		flag_inf_change[1]=1;
	}






}

/*** trigouts.c ****/
inline void update_timeinf_params(uint8_t channel){

	if (flag_inf_change[channel]){
		flag_inf_change[channel]=0;

		// Toggle INF status
		param[channel][INF] = 1.0 - param[channel][INF];
	}

	// Update divmult time
	if (flag_time_param_changed[channel]){
		flag_time_param_changed[channel]=0;

		set_divmult_time(channel);

	}


}


inline void update_clkout_jack(void){

	// Check if clkout timer has overflowed
	// If so, we can change the Time parameters and INF status

	if (clkout_trigger_tmr>=ping_time){
		CLKOUT_ON;
		reset_clkout_trigger_tmr();

		update_timeinf_params(0);
		update_timeinf_params(1);
	}

	// Handle the jack output: 50% duty cycle
	if (ping_time > CLKOUT_TRIG_TIME<<1){
		if (clkout_trigger_tmr > CLKOUT_TRIG_TIME){
			CLKOUT_OFF;
		}
	} else {
		if (clkout_trigger_tmr > ping_time>>1){
			CLKOUT_OFF;
		}
	}
}

/*** leds.c ***/

void update_ping_ledbut(void){
	// Check if clkout timer has overflowed
	if (ping_ledbut_tmr>=ping_time){
		LED_PINGBUT_ON;
		reset_ping_ledbut_tmr();

	} else if (ping_ledbut_tmr >= (ping_time>>1)){
		LED_PINGBUT_OFF;
	}
}

void update_channel_leds(uint8_t channel){
	// Check if clkout timer has overflowed
	if (pingled_tmr[channel]>=divmult_time[channel]){
		if (channel==0) LED_OVLD1_ON; else LED_OVLD2_ON;

		reset_pingled_tmr(channel);

	} else if (pingled_tmr[channel] >= (divmult_time[channel]>>1)){
		if (channel==0) LED_OVLD1_OFF; else LED_OVLD2_OFF;
	}
}

void update_inf_ledbut(uint8_t channel){
	if (param[channel][INF]==0.0){
		if (channel==0) LED_INF1_OFF; else LED_INF2_OFF;
	}else{
		if (channel==0) LED_INF1_ON; else LED_INF2_ON;
	}

}
