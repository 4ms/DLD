/*
 * buttons_jacks.c - handles button presses and trigger/gate/clock input jacks
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

#include "buttons_jacks.h"

#include "globals.h"
#include "adc.h"
#include "params.h"
#include "looping_delay.h"
#include "timekeeper.h"
#include "audio_memory.h"
#include "dig_pins.h"

extern volatile uint32_t ping_tmr;

extern volatile uint32_t ping_time;
extern uint32_t locked_ping_time[NUM_CHAN];

extern uint8_t global_mode[NUM_GLOBAL_MODES];
extern uint8_t mode[NUM_CHAN][NUM_CHAN_MODES];


uint8_t flag_ping_was_changed[NUM_CHAN];

uint8_t flag_inf_change[2]={0,0};
uint8_t flag_rev_change[2]={0,0};

uint8_t flag_pot_changed_revdown[NUM_POT_ADCS]={0,0,0,0,0,0,0,0};
uint8_t flag_pot_changed_infdown[NUM_POT_ADCS]={0,0,0,0,0,0,0,0};

uint8_t ping_button_state=0;
uint8_t ping_jack_state=0;

uint8_t flag_ignore_infdown[2]={0,0};
uint8_t flag_ignore_revdown[2]={0,0};

uint32_t flag_acknowlegde_qcm=0;

void init_inputread_timer(void){
	TIM_TimeBaseInitTypeDef  tim;

	NVIC_InitTypeDef nvic;
	RCC_APB1PeriphClockCmd(INFREVBUTTONJACK_PINGBUT_TIM_RCC, ENABLE);

	nvic.NVIC_IRQChannel = INFREVBUTTONJACK_PINGBUT_TIM_IRQn;
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

	TIM_TimeBaseInit(INFREVBUTTONJACK_PINGBUT_TIM, &tim);

	TIM_ITConfig(INFREVBUTTONJACK_PINGBUT_TIM, TIM_IT_Update, ENABLE);

	TIM_Cmd(INFREVBUTTONJACK_PINGBUT_TIM, ENABLE);


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


//enum PingMethods PING_METHOD=IGNORE_PERCENT_DEVIATION;


//Ping jack read
//every 30us (33.3kHz), takes 0.3us if no ping
void TIM1_UP_TIM10_IRQHandler(void)
{
	static uint16_t State = 0; // Current debounce status
	uint16_t t;
	uint32_t t_ping_tmr;
	uint32_t diff;
	float t_f;

	static uint16_t last_ping = 0;

	static uint16_t ringbuff[8]={0,0,0,0,0,0,0,0};
	static uint16_t ringbuff_pos=0;
	static uint32_t ringbuff_filled=0;

	if (TIM_GetITStatus(TIM10, TIM_IT_Update) != RESET)
	{

		// Check Ping jack
		// ping detection time is 60us typical, 100us max from incoming clock until this line
		if (PINGJACK){
			t=0xe000;
		} else{
			t=0xe001;
		}

		State=(State<<1) | t;
		//if (State==0xfffffff0) //jack low for 27 times (0.810ms), then detected high 4 times (0.120ms)
		//if ((State & 0xff)==0xfe) //jack low for 7 times (250us), then detected high 1 time

		if (State==0xfffe) //jack low for 12 times (1ms), then detected high 1 time
		{
			ping_button_state = 0;

			ping_jack_state = 0;

			t_ping_tmr = ping_tmr;
			reset_ping_tmr();

			if (global_mode[PING_METHOD] != IGNORE_PERCENT_DEVIATION && global_mode[PING_METHOD] != IGNORE_FLAT_DEVIATION_5 && global_mode[PING_METHOD] != IGNORE_FLAT_DEVIATION_10)
			{
				CLKOUT_ON;
				reset_clkout_trigger_tmr();

				LED_PINGBUT_ON;
				reset_ping_ledbut_tmr();

				//Flag to update the divmult parameters
				if (mode[0][PING_LOCKED]==0) flag_ping_was_changed[0]=1;
				if (mode[1][PING_LOCKED]==0) flag_ping_was_changed[1]=1;
			}

			if (global_mode[PING_METHOD] != MOVING_AVERAGE_4 && global_mode[PING_METHOD] != LINEAR_AVERAGE_8 && global_mode[PING_METHOD] != LINEAR_AVERAGE_4)
			{
				ringbuff_pos=0;
				ringbuff_filled=0;
			}

			switch(global_mode[PING_METHOD])
			{
				case (IGNORE_PERCENT_DEVIATION):
					//Only update if there is a variation >1%
					t_f = (float)t_ping_tmr / (float)ping_time;
					if (t_f>1.01 || t_f<0.99)
					{
						CLKOUT_ON;
						reset_clkout_trigger_tmr();

						LED_PINGBUT_ON;
						reset_ping_ledbut_tmr();

						//Flag to update the divmult parameters
						if (mode[0][PING_LOCKED]==0) flag_ping_was_changed[0]=1;
						if (mode[1][PING_LOCKED]==0) flag_ping_was_changed[1]=1;

						ping_time = t_ping_tmr;

					}
				break;

				case (IGNORE_FLAT_DEVIATION_5):
				case (IGNORE_FLAT_DEVIATION_10):
					//Only update if there is a variation > XXXX
					if (t_ping_tmr > ping_time) diff = t_ping_tmr - ping_time;
					else diff = ping_time - t_ping_tmr;

					if (global_mode[PING_METHOD]==IGNORE_FLAT_DEVIATION_5) t=5;
					else t =10;

					if (diff>t)
					{
						CLKOUT_ON;
						reset_clkout_trigger_tmr();

						LED_PINGBUT_ON;
						reset_ping_ledbut_tmr();

						//Flag to update the divmult parameters
						if (mode[0][PING_LOCKED]==0) flag_ping_was_changed[0]=1;
						if (mode[1][PING_LOCKED]==0) flag_ping_was_changed[1]=1;

						ping_time = t_ping_tmr;

					}
				break;

				case(MOVING_AVERAGE_4):
				// Ring buffer... hmm, kinda weird when the clock slows down: all sorts of double-hits
				// But, it averages out phasing nicely

					ringbuff[ringbuff_pos] = t_ping_tmr;

					//Use the clock period the first four times we receive ping after boot
					//After that, use an average of the previous 4 clock periods
					if (ringbuff_filled)
						ping_time=(ringbuff[0] + ringbuff[1] + ringbuff[2] + ringbuff[3])/4;
					else
						ping_time=ringbuff[ringbuff_pos];

					if (ringbuff_pos++>=4) {
						ringbuff_pos=0;
						ringbuff_filled=1;
					}
				break;

				case(LINEAR_AVERAGE_8):

					ringbuff[ringbuff_pos] = t_ping_tmr;

					//Update the ping time every 8 clocks
					if (++ringbuff_pos>=8) {
						ringbuff_pos=0;
						ping_time=(ringbuff[0] + ringbuff[1] + ringbuff[2] + ringbuff[3] + ringbuff[4] + ringbuff[5] + ringbuff[6] + ringbuff[7])/8;
					}
				break;

				case(LINEAR_AVERAGE_4):

					ringbuff[ringbuff_pos] = t_ping_tmr;

					//Update the ping time every 4 clocks
					if (++ringbuff_pos>=4) {
						ringbuff_pos=0;
						ping_time=(ringbuff[0] + ringbuff[1] + ringbuff[2] + ringbuff[3])/4;
					}
				break;


				case(MOVING_AVERAGE_2):
				//Simple linear average: the catchup is weird, and the phasing is not much different
					ping_time = (t_ping_tmr + last_ping)>>1;
					last_ping = t_ping_tmr;
				break;


				case(EXPO_AVERAGE_4):
				//Exponential LPF: slowly gets to the actual tempo, so there is always drift
					ping_time=(float)t_ping_tmr*0.25 + (float)ping_time*0.75;
					ping_time=ping_time & 0xFFFFFFF8;
				break;

				case(EXPO_AVERAGE_8):
				//Exponential LPF: slowly gets to the actual tempo, so there is always drift
					ping_time=(float)t_ping_tmr*0.125 + (float)ping_time*0.875;
					ping_time=ping_time & 0xFFFFFFF8;
				break;

				case(ONE_TO_ONE):
					//Track the clock 1:1
					ping_time=t_ping_tmr;
				break;
			}

		}


		TIM_ClearITPendingBit(TIM10, TIM_IT_Update);
	}
}


// Ping and INF/REV button and jack read
// Checks each button and digital input jack to see if it's been low for a certain number of cycles,
// and high for a certain number of cycles. We shift 0's and 1's down a 16-bit variable (State[]) to indicate high/low status.
// takes 2-3us
// runs at 27kHz
void INFREVBUTTONJACK_PINGBUT_IRQHandler(void)
{
	static uint16_t State[10] = {0,0,0,0xFFFF,0xFFFF,0,0,0xFFFF,0xFFFF,0}; // Current debounce status
	uint16_t t;
	static uint32_t ch1_clear_ctr=0,ch2_clear_ctr=0, ch1_contrev_ctr=0, ch2_contrev_ctr=0;

	// Clear TIM update interrupt
	TIM_ClearITPendingBit(INFREVBUTTONJACK_PINGBUT_TIM, TIM_IT_Update);

	// Check Ping Button

	if (PINGBUT) {
		LED_PINGBUT_ON;
		t=0xe000;
	} else
		t=0xe001;

	State[0]=(State[0]<<1) | t;
	if (State[0]==0xff00){ 	//1111 1111 0000 0000 = not pressed for 8 cycles , then pressed for 8 cycles

		if (!INF1BUT && !INF2BUT && 	REV1BUT && REV2BUT)
		{
			flag_acknowlegde_qcm = (6<<8);

			if (global_mode[QUANTIZE_MODE_CHANGES]==0)
				global_mode[QUANTIZE_MODE_CHANGES] = 1;
			else
				global_mode[QUANTIZE_MODE_CHANGES] = 0;

			flag_ignore_revdown[0] = 1;
			flag_ignore_revdown[1] = 1;
		}
		else if (REV1BUT && 	!INF1BUT && !INF2BUT && !REV2BUT)
		{
			flag_ignore_revdown[0] = 1;
		}
		else if (REV2BUT && 	!INF1BUT && !INF2BUT && !REV1BUT)
		{
			flag_ignore_revdown[1] = 1;
		}

		else if (INF1BUT && 	!INF2BUT && !REV1BUT && !REV2BUT)
		{
			if (mode[0][PING_LOCKED]==0)
			{
				locked_ping_time[0] = ping_time;
				mode[0][PING_LOCKED] = 1;
			}
			else
			{
				mode[0][PING_LOCKED] = 0;
				set_divmult_time(0);
			}

			flag_ignore_infdown[0] = 1;
		}

		else if (INF2BUT && 	!INF1BUT && !REV1BUT && !REV2BUT)
		{
			if (mode[1][PING_LOCKED]==0)
			{
				locked_ping_time[1] = ping_time;
				mode[1][PING_LOCKED] = 1;
			}
			else
			{
				mode[1][PING_LOCKED] = 0;
				set_divmult_time(1);
			}

			flag_ignore_infdown[1] = 1;
		}
		else if (INF2BUT && INF1BUT && REV1BUT && REV2BUT)
		{
			flag_ignore_revdown[0] = 1;
			flag_ignore_revdown[1] = 1;
			flag_ignore_infdown[0] = 1;
			flag_ignore_infdown[1] = 1;

		}
		else if (!INF2BUT && !INF1BUT && !REV1BUT && !REV2BUT)
		{
			//Clear the ping jack state
			ping_jack_state = 0;

			//Log how much time has elapsed since last ping
			ping_time=ping_tmr & 0xFFFFFFF8; //multiple of 8

			//Reset the timers
			CLKOUT_ON;
			reset_clkout_trigger_tmr();

			LED_PINGBUT_ON;
			reset_ping_ledbut_tmr();

			ping_tmr = 0;

			//Flag to update the divmult parameters
			if (mode[0][PING_LOCKED]==0) flag_ping_was_changed[0]=1;
			if (mode[1][PING_LOCKED]==0) flag_ping_was_changed[1]=1;

		}
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

		else if (flag_ignore_infdown[0])
			flag_ignore_infdown[0]=0;

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

		else if (flag_ignore_infdown[1])
			flag_ignore_infdown[1]=0;

		else
			flag_inf_change[1]=1;
	}

	if (INF2JACK) t=0xe000; else t=0xe001;
	State[4]=(State[4]<<1) | t;
	if (State[4]==0xfff8 || (State[4]==0xe007 && global_mode[INF_GATETRIG] == GATE_MODE))
	{
		flag_inf_change[1]=1;
	}


	if (INF1JACK) t=0xe000; else t=0xe001;
	State[3]=(State[3]<<1) | t;
	//1111 1111 1111 1000 = gate low for 13 cycles (5ms), followed by gate high for 3 cycles (1.1ms)
	if (State[3]==0xfff8 || (State[3]==0xe007 && global_mode[INF_GATETRIG] == GATE_MODE))
	{
		flag_inf_change[0]=1;
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

		else if (flag_ignore_revdown[0])
			flag_ignore_revdown[0]=0;

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

		else if (flag_ignore_revdown[1])
			flag_ignore_revdown[1]=0;

		else
			flag_rev_change[1]=1;
	}



	if (REV2JACK) t=0xe000; else t=0xe001;
	State[8]=(State[8]<<1) | t;
	if (State[8]==0xfff8 || (State[8]==0xe007 && global_mode[REV_GATETRIG] == GATE_MODE))
	{
		flag_rev_change[1]=1;
	}


	if (REV1JACK) t=0xe000; else t=0xe001;
	State[7]=(State[7]<<1) | t;
	if (State[7]==0xfff8 || (State[7]==0xe007 && global_mode[REV_GATETRIG] == GATE_MODE))
	{
		flag_rev_change[0]=1;
	}

//check continuously held down buttons:

	if (RAM_CLEAR_CH1_BUTTONS)
	{
		if (ch1_clear_ctr++>54000) {
			flag_ignore_infdown[0]=1;
			flag_ignore_revdown[0]=1;

			LED_REV1_ON;
			LED_INF1_ON;
			LED_LOOP1_ON;

			memory_clear(0);
			ch1_clear_ctr=0;

			LED_REV1_OFF;
			LED_INF1_OFF;
			LED_LOOP1_OFF;
		}
	}
	else
		ch1_clear_ctr=0;



	if (RAM_CLEAR_CH2_BUTTONS)
	{
		if (ch2_clear_ctr++>54000) {
			flag_ignore_infdown[1]=1;
			flag_ignore_revdown[1]=1;

			LED_REV2_ON;
			LED_INF2_ON;
			LED_LOOP2_ON;

			memory_clear(1);
			ch2_clear_ctr=0;

			LED_REV2_OFF;
			LED_INF2_OFF;
			LED_LOOP2_OFF;

		}
	}
	else
		ch2_clear_ctr=0;

#ifdef ALLOW_CONT_REVERSE
	if (mode[0][INF]==INF_OFF && CONTINUOUS_REV1_BUTTONS)
	{
		if (ch1_contrev_ctr++>54000) {
			flag_ignore_revdown[0]=1;

			if (mode[0][CONTINUOUS_REVERSE])
			{
				mode[0][CONTINUOUS_REVERSE] = 0;
			} else
			{
				mode[0][CONTINUOUS_REVERSE] = 1;
				//mode[0][REV] = 0;
			}

			ch1_contrev_ctr = 0;
		}
	}
	else
		ch1_contrev_ctr = 0;

	if (mode[1][INF]==INF_OFF && CONTINUOUS_REV2_BUTTONS)
	{
		if (ch2_contrev_ctr++>54000) {
			flag_ignore_revdown[1]=1;

			if (mode[1][CONTINUOUS_REVERSE])
			{
				mode[1][CONTINUOUS_REVERSE] = 0;
			} else
			{
				mode[1][CONTINUOUS_REVERSE] = 1;
				//mode[1][REV] = 0;
			}
			ch2_contrev_ctr = 0;
		}
	}
	else
		ch2_contrev_ctr = 0;
#endif

}

