/*
 * dig_inouts.c
 */

#include "globals.h"
#include "adc.h"
#include "dig_inouts.h"
#include "params.h"
#include "looping_delay.h"
#include "timekeeper.h"


extern volatile uint32_t ping_tmr;

extern volatile uint32_t ping_time;

uint8_t flag_ping_was_changed[NUM_CHAN];

uint8_t flag_inf_change[2]={0,0};
uint8_t flag_rev_change[2]={0,0};

uint8_t flag_pot_changed_revdown[NUM_POT_ADCS]={0,0,0,0,0,0,0,0};
uint8_t flag_pot_changed_infdown[NUM_POT_ADCS]={0,0,0,0,0,0,0,0};

uint8_t inf_jack_high[2]={0,0};

uint8_t ping_button_state=0;
uint8_t ping_jack_state=0;


void init_dig_inouts(void){
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);


	//Configure outputs
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//LEDs
	RCC_AHB1PeriphClockCmd(LED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_LOOP1 | LED_LOOP2;	GPIO_Init(LED_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(PINGBUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_PINGBUT_pin;	GPIO_Init(LED_PINGBUT_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(INF1_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_INF1_pin;	GPIO_Init(LED_INF1_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(INF2_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_INF2_pin;	GPIO_Init(LED_INF2_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(REV1_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_REV1_pin;	GPIO_Init(LED_REV1_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(REV2_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_REV2_pin;	GPIO_Init(LED_REV2_GPIO, &gpio);

	//CLKOUT Jacks
	RCC_AHB1PeriphClockCmd(CLKOUT_RCC, ENABLE);
	gpio.GPIO_Pin = CLKOUT_pin;	GPIO_Init(CLKOUT_GPIO, &gpio);
	gpio.GPIO_Pin = CLKOUT1_pin;	GPIO_Init(CLKOUT1_GPIO, &gpio);
	gpio.GPIO_Pin = CLKOUT2_pin;	GPIO_Init(CLKOUT2_GPIO, &gpio);
	CLKOUT_OFF;
	CLKOUT1_OFF;
	CLKOUT2_OFF;

	//DEBUG pins
	RCC_AHB1PeriphClockCmd(DEBUG_RCC, ENABLE);

	gpio.GPIO_Pin = DEBUG0;	GPIO_Init(DEBUG0_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG1;	GPIO_Init(DEBUG1_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG2;	GPIO_Init(DEBUG2_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG3;	GPIO_Init(DEBUG3_GPIO, &gpio);
//	gpio.GPIO_Pin = DEBUG4;	GPIO_Init(DEBUG4_GPIO, &gpio);
	DEBUG0_OFF;
	DEBUG1_OFF;
	DEBUG2_OFF;
	DEBUG3_OFF;
//	DEBUG4_OFF;

	//Configure inputs
	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_PuPd = GPIO_PuPd_UP;

	//Div/Mult switches
	RCC_AHB1PeriphClockCmd(TIMESW_RCC, ENABLE);

	gpio.GPIO_Pin = TIMESW_CH1_T1_pin;	GPIO_Init(TIMESW_CH1_T1_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH1_T2_pin;	GPIO_Init(TIMESW_CH1_T2_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH2_T1_pin;	GPIO_Init(TIMESW_CH2_T1_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH2_T2_pin;	GPIO_Init(TIMESW_CH2_T2_GPIO, &gpio);

	//Reverse buttons
	RCC_AHB1PeriphClockCmd(REVBUT_RCC, ENABLE);

	gpio.GPIO_Pin = REV2BUT_pin;	GPIO_Init(REV2BUT_GPIO, &gpio);
	gpio.GPIO_Pin = REV1BUT_pin;	GPIO_Init(REV1BUT_GPIO, &gpio);


	//Ping button and jack
	RCC_AHB1PeriphClockCmd(PING_RCC, ENABLE);

	gpio.GPIO_Pin = PINGBUT_pin;	GPIO_Init(PINGBUT_GPIO, &gpio);

	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio.GPIO_Pin = PINGJACK_pin;	GPIO_Init(PINGJACK_GPIO, &gpio);


	// Inf Repeat buttons and jacks
	RCC_AHB1PeriphClockCmd(INF_RCC, ENABLE);

	gpio.GPIO_PuPd = GPIO_PuPd_UP;

	gpio.GPIO_Pin = INF1BUT_pin;	GPIO_Init(INF1BUT_GPIO, &gpio);
	gpio.GPIO_Pin = INF2BUT_pin;	GPIO_Init(INF2BUT_GPIO, &gpio);

	gpio.GPIO_PuPd = GPIO_PuPd_DOWN;

	gpio.GPIO_Pin = INF1JACK_pin;	GPIO_Init(INF1JACK_GPIO, &gpio);
	gpio.GPIO_Pin = INF2JACK_pin;	GPIO_Init(INF2JACK_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(REV_RCC, ENABLE);

	gpio.GPIO_Pin = REV1JACK_pin;	GPIO_Init(REV1JACK_GPIO, &gpio);
	gpio.GPIO_Pin = REV2JACK_pin;	GPIO_Init(REV2JACK_GPIO, &gpio);

	gpio.GPIO_PuPd = GPIO_PuPd_UP;
	RCC_AHB1PeriphClockCmd(JUMPER_RCC, ENABLE);

	gpio.GPIO_Pin = JUMPER_1_pin;	GPIO_Init(JUMPER_1_GPIO, &gpio);
//	gpio.GPIO_Pin = JUMPER_2_pin;	GPIO_Init(JUMPER_2_GPIO, &gpio);
//	gpio.GPIO_Pin = JUMPER_3_pin;	GPIO_Init(JUMPER_3_GPIO, &gpio);
//	gpio.GPIO_Pin = JUMPER_4_pin;	GPIO_Init(JUMPER_4_GPIO, &gpio);


}



/*** read_buttons.c ****/

void init_inputread_timer(void){
	TIM_TimeBaseInitTypeDef  tim;

	NVIC_InitTypeDef nvic;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	nvic.NVIC_IRQChannel = TIM4_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 3;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	TIM_TimeBaseStructInit(&tim);
	//3000 --> 28kHz
	tim.TIM_Period = 3000;
	tim.TIM_Prescaler = 0;
	tim.TIM_ClockDivision = 0;
	tim.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM4, &tim);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM4, ENABLE);


	//Ping jack timer
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);

	nvic.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	TIM_TimeBaseStructInit(&tim);
	tim.TIM_Period = 5000; //every 30uS
	tim.TIM_Prescaler = 0;
	tim.TIM_ClockDivision = 0;
	tim.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM10, &tim);

	TIM_ITConfig(TIM10, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM10, ENABLE);
}

//every 30us (33.3kHz)
//takes 0.3us if no ping
void TIM1_UP_TIM10_IRQHandler(void)
{
	static uint16_t ping_tracking=100;
	static uint16_t State = 0; // Current debounce status
	uint16_t t;
	uint32_t t32;
	float t_f;

	static uint16_t ringbuff[4]={0,0,0,0};
	static uint16_t a=0;
	static ring_buffer_filled=0;


	if (TIM_GetITStatus(TIM10, TIM_IT_Update) != RESET) {


		// Check Ping jack
		// ping detection time is 60us typical, 100us max from incoming clock until this line
		if (PINGJACK){
			t=0xe000;
		} else{
			t=0xe001;
		}

		State=(State<<1) | t;
		//if (State==0xfffffff0) //jack low for 27 times (0.810ms), then detected high 4 times (0.120ms)
		if (State==0xfffe) //jack low for 12 times (1ms), then detected high 1 time
		//if ((State & 0xff)==0xfe) //jack low for 7 times (250us), then detected high 1 time

		{

			ping_button_state = 0;

				ping_jack_state = 0;

				t32=ping_tmr;
				reset_ping_tmr();

				//If the ping clock changes by +/-3% then track it until it's stable for at least 4 clocks
				//if (t_f>1.03 || t_f<0.97)
				//	ping_tracking=4;

			//	if (ping_tracking){
				//Decrement ping_tracking so that we eventually stop tracking it closely
				//ping_tracking--;

					CLKOUT_ON;
					reset_clkout_trigger_tmr();

					LED_PINGBUT_ON;
					reset_ping_ledbut_tmr();

/* Ring buffer... hmm, kinda weird when the clock slows down: all sorts of double-hits
 * But, it averages out phasing nicely

					ringbuff[a] = t32;

					//Use the clock period the first four times we receive ping after boot
					//After that, use an average of the previous 4 clock periods
					if (ring_buffer_filled)
						ping_time=(ringbuff[0] + ringbuff[1] + ringbuff[2] + ringbuff[3])/4;
					else
						ping_time=ringbuff[a];

					if (a++>=4) {
						a=0;
						ring_buffer_filled=1;
					}
*/

//Simple linear average: the catchup is weird, and the phasing is not much different
					//ping_time = (t32 + a)/2;
					//a=t32;

//Exponential LPF: never really gets to the actual tempo, so there is always drift
					//ping_time=(float)t32*0.25 + (float)ping_time*0.75;
					//ping_time=ping_time & 0xFFFFFFF8;

//Only update if there is a variation >1%
					t_f = (float)t32 / (float)ping_time;
					if (t_f>1.01 || t_f<0.99)
					{
						ping_time=t32;

					}


//Track the clock 1:1
//					ping_time=t32;

					//Flag to update the divmult parameters
					flag_ping_was_changed[0]=1;
					flag_ping_was_changed[1]=1;



		}

		TIM_ClearITPendingBit(TIM10, TIM_IT_Update);
		DEBUG2_OFF;
	}


}


// Checks each button and digital input jack to see if it's been low for a certain number of cycles,
// and high for a certain number of cycles. We shift 0's and 1's down a 16-bit variable (State[]) to indicate high/low status.
// takes 2-3us
// runs at 27kHz
void TIM4_IRQHandler(void)
{
	static uint16_t State[10] = {0,0,0,0,0,0,0,0,0,0}; // Current debounce status
	uint16_t t;
	uint32_t t32;
	float t_f;

	// Clear TIM4 update interrupt
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);


	// Check Ping Button

	if (PINGBUT) {
		LED_PINGBUT_ON;
		t=0xe000;
	} else
		t=0xe001;

	State[0]=(State[0]<<1) | t;
	if (State[0]==0xff00){ 	//1111 1111 0000 0000 = not pressed for 8 cycles , then pressed for 8 cycles

		//Clear the ping jack state
		ping_jack_state = 0;

		//if (ping_button_state==1){ //second time we pressed the button
			//ping_button_state = 0;

			//Log how much time has elapsed since last ping
			ping_time=ping_tmr & 0xFFFFFFF8; //multiple of 8

			//Reset the timers
			//ping_ledbut_tmr=0;
			reset_ping_ledbut_tmr();
			reset_clkout_trigger_tmr();

			//Flag to update the divmult parameters
			flag_ping_was_changed[0]=1;
			flag_ping_was_changed[1]=1;

		//} else {

			// This is the first button press, so start the ping timer
			ping_tmr = 0;
		//	ping_button_state = 1;
		//}
	}



	//Todo: Create a global infbut_pressed[2] and revbut_pressed[2] reflecting the de-bounced state
	if (!INF1BUT) t=0xe000; else t=0xe001;
	State[1]=(State[1]<<1) | t;
	if (State[1]==0xf000)
	{
		// If we wiggled any pots while the inf button was held down, clear the flag
		if (flag_pot_changed_infdown[TIME_POT*2])
			flag_pot_changed_infdown[TIME_POT*2]=0;

		else if (flag_pot_changed_infdown[REGEN_POT*2])
			flag_pot_changed_infdown[REGEN_POT*2]=0;

		else if (flag_pot_changed_infdown[LEVEL_POT*2])
			flag_pot_changed_infdown[LEVEL_POT*2]=0;

		else if (flag_pot_changed_infdown[MIX_POT*2])
			flag_pot_changed_infdown[MIX_POT*2]=0;

		// Only flag a toggle of INF mode if we did not wiggle any of the pots
		else
			flag_inf_change[0]=1;
	}

	if (!INF2BUT) t=0xe000; else t=0xe001;
	State[2]=(State[2]<<1) | t;
	if (State[2]==0xf000)
	{
		if (flag_pot_changed_infdown[TIME_POT*2+1])
			flag_pot_changed_infdown[TIME_POT*2+1]=0;

		else if (flag_pot_changed_infdown[REGEN_POT*2+1])
			flag_pot_changed_infdown[REGEN_POT*2+1]=0;

		else if (flag_pot_changed_infdown[LEVEL_POT*2+1])
			flag_pot_changed_infdown[LEVEL_POT*2+1]=0;

		else if (flag_pot_changed_infdown[MIX_POT*2+1])
			flag_pot_changed_infdown[MIX_POT*2+1]=0;

		else
			flag_inf_change[1]=1;
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
		inf_jack_high[1]=1;
	}


	if (!REV1BUT) t=0xe000; else t=0xe001;
	State[5]=(State[5]<<1) | t;
	if (State[5]==0xf000)
	{
		if (flag_pot_changed_revdown[TIME_POT*2])
			flag_pot_changed_revdown[TIME_POT*2]=0;

		else if (flag_pot_changed_revdown[REGEN_POT*2])
			flag_pot_changed_revdown[REGEN_POT*2]=0;

		else if (flag_pot_changed_revdown[LEVEL_POT*2])
			flag_pot_changed_revdown[LEVEL_POT*2]=0;

		else if (flag_pot_changed_revdown[MIX_POT*2])
			flag_pot_changed_revdown[MIX_POT*2]=0;

		else
			flag_rev_change[0]=1;
	}

	if (!REV2BUT) t=0xe000; else t=0xe001;
	State[6]=(State[6]<<1) | t;
	if (State[6]==0xf000)
	{
		if (flag_pot_changed_revdown[TIME_POT*2+1])
			flag_pot_changed_revdown[TIME_POT*2+1]=0;

		else if (flag_pot_changed_revdown[REGEN_POT*2+1])
			flag_pot_changed_revdown[REGEN_POT*2+1]=0;

		else if (flag_pot_changed_revdown[LEVEL_POT*2+1])
			flag_pot_changed_revdown[LEVEL_POT*2+1]=0;

		else if (flag_pot_changed_revdown[MIX_POT*2+1])
			flag_pot_changed_revdown[MIX_POT*2+1]=0;

		else
			flag_rev_change[1]=1;
	}

	if (REV1JACK) t=0xe000; else t=0xe001;
	State[7]=(State[7]<<1) | t;
	if (State[7]==0xfff8){
		flag_rev_change[0]=1;
	}

	if (REV2JACK) t=0xe000; else t=0xe001;
	State[8]=(State[8]<<1) | t;
	if (State[8]==0xfff8){
		flag_rev_change[1]=1;
	}

//	DEBUG2_OFF;


}

