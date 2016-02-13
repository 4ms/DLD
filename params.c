/*
 * params.c
 *
 *  Created on: Mar 27, 2015
 *      Author: design
 */

#define QUANTIZE_TIMECV_CH1 1
#define QUANTIZE_TIMECV_CH2 0

#include "globals.h"
#include "adc.h"
#include "dig_inouts.h"
#include "params.h"
#include "equalpowpan_lut.h"
#include "exp_1voct.h"
#include "si5153a.h"
#include "timekeeper.h"
#include "looping_delay.h"

extern __IO uint16_t potadc_buffer[NUM_POT_ADCS];
extern __IO uint16_t cvadc_buffer[NUM_CV_ADCS];

uint8_t flag_time_param_changed[2]={0,0};

extern volatile uint32_t write_addr[NUM_CHAN];
extern volatile uint32_t read_addr[NUM_CHAN];

extern uint32_t loop_start[NUM_CHAN];
extern uint32_t loop_end[NUM_CHAN];
extern const uint32_t LOOP_RAM_BASE[NUM_CHAN];

extern uint8_t flag_ping_was_changed;
extern uint8_t flag_inf_change[2];
extern uint8_t flag_rev_change[2];
extern uint8_t flag_timepot_changed_revdown[2];
extern uint8_t flag_timepot_changed_infdown[2];

extern volatile uint32_t divmult_time[NUM_CHAN];
extern volatile uint32_t pingled_tmr[2];

float param[NUM_CHAN][NUM_PARAMS];
uint8_t mode[NUM_CHAN][NUM_CHAN_MODES];

const int32_t MIN_POT_ADC_CHANGE[NUM_POT_ADCS] = {60, 60, 20, 20, 20, 20, 20, 20};
const int32_t MIN_CV_ADC_CHANGE[NUM_CV_ADCS] = {60, 60, 20, 20, 20, 20};

int32_t CV_CALIBRATION_OFFSET[NUM_CV_ADCS];


float POT_LPF_COEF[NUM_POT_ADCS];
float CV_LPF_COEF[NUM_CV_ADCS];


void init_params(void)
{
	uint8_t chan=0;

	for (chan=0;chan<NUM_CHAN;chan++){
		param[chan][TIME] = 1.0;
		param[chan][LEVEL] = 0.0;
		param[chan][REGEN] = 0.0;
		param[chan][MIX_DRY] = 1.0;
		param[chan][MIX_WET] = 0.0;
	}

	//ToDo: Read TIME_CV_ZERO from FLASH parameters
	//Time
	CV_CALIBRATION_OFFSET[0]=-10;
	CV_CALIBRATION_OFFSET[1]=0;

	//5=
	CV_CALIBRATION_OFFSET[2]=-5;
	CV_CALIBRATION_OFFSET[3]=-16;

	//
	CV_CALIBRATION_OFFSET[4]=-5;
	CV_CALIBRATION_OFFSET[5]=-5;
}

void init_modes(void)
{
	uint8_t chan=0;

	for (chan=0;chan<NUM_CHAN;chan++){
		mode[chan][INF] = 0;
		mode[chan][REV] = 0;
		mode[chan][TIMEMODE_POT] = MOD_READWRITE_TIME;
		mode[chan][TIMEMODE_JACK] = MOD_READWRITE_TIME;
		mode[chan][LOOP_CLOCK_JACK] = TRIG_MODE;
	}

	mode[0][MAIN_CLOCK_JACK] = TRIG_MODE;

}


inline float LowPassSmoothingFilter(float current_value, float new_value, float coef)
{
	return (current_value * coef) + (new_value * (1.0f-coef));
}

void init_LowPassCoefs(void)
{
	float t;

	CV_LPF_COEF[TIME*2] = 0.99;
	CV_LPF_COEF[TIME*2+1] = 0.99;

	CV_LPF_COEF[LEVEL*2] = 0.99;
	CV_LPF_COEF[LEVEL*2+1] = 0.99;

	CV_LPF_COEF[REGEN*2] = 0.99;
	CV_LPF_COEF[REGEN*2+1] = 0.99;

	t=50.0;

	POT_LPF_COEF[TIME*2] = 1.0-(1.0/t);
	POT_LPF_COEF[TIME*2+1] = 1.0-(1.0/t);

	POT_LPF_COEF[LEVEL*2] = 0.9;
	POT_LPF_COEF[LEVEL*2+1] = 0.9;

	POT_LPF_COEF[REGEN*2] = 0.9;
	POT_LPF_COEF[REGEN*2+1] = 0.9;

	POT_LPF_COEF[MIXPOT*2] = 0.9;
	POT_LPF_COEF[MIXPOT*2+1] = 0.9;

}

float time_mult[2]={1.0,1.0};
//uint32_t sample_rate_div[2]={30,30};
uint32_t sample_rate_div[2]={36,40};
uint32_t sample_rate_num[2]={265,265};
uint32_t sample_rate_denom[2]={512,512};
uint32_t old_sample_rate_div[2]={0,0};
uint32_t old_sample_rate_num[2]={0,0};
uint32_t old_sample_rate_denom[2]={0,0};

void update_adc_params(void)
{
	int16_t i_smoothed_potadc[NUM_POT_ADCS]={0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF};
	int16_t i_smoothed_cvadc[NUM_CV_ADCS]={0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF};

	static float smoothed_potadc[NUM_POT_ADCS]={0,0,0,0,0,0,0,0};
	static float smoothed_cvadc[NUM_CV_ADCS]={0,0,0,0,0,0};

	static int16_t old_i_smoothed_cvadc[NUM_CV_ADCS]={0,0,0,0,0,0};
	static int16_t old_i_smoothed_potadc[NUM_POT_ADCS]={0,0,0,0,0,0,0,0};
	uint8_t flag_timepot_changed[2]={0,0};
	uint8_t flag_timecv_changed[2]={0,0};
	uint8_t flag_multswitch_changed[2]={0,0};
	uint8_t old_switch_val[2]={0,0};

	uint8_t i, channel;
	int32_t t;
	//float t_f;
	int32_t t_combined;

	uint8_t switch1_val;

	//
	// Run a LPF on the pots and CV jacks
	//
	for (i=0;i<NUM_POT_ADCS;i++)
	{
		smoothed_potadc[i] = LowPassSmoothingFilter(smoothed_potadc[i], (float)potadc_buffer[i], POT_LPF_COEF[i]);
		i_smoothed_potadc[i] = (int16_t)smoothed_potadc[i];
	}
	for (i=0;i<NUM_CV_ADCS;i++)
	{
		smoothed_cvadc[i] = LowPassSmoothingFilter(smoothed_cvadc[i], (float)(cvadc_buffer[i]+CV_CALIBRATION_OFFSET[i]), CV_LPF_COEF[i]);
		i_smoothed_cvadc[i] = (int16_t)smoothed_cvadc[i];
	}


	//
	// Cycle through channels, assigning params
	//
	for (channel=0;channel<2;channel++)
	{

		//
		// Check if TIME pot was wiggled (with or without Reverse or Inf button down)
		//
		t=old_i_smoothed_potadc[TIME*2+channel] - i_smoothed_potadc[TIME*2+channel];
		if ((t>30) || (t<-30))
		{
			if (channel==0 && REV1BUT) flag_timepot_changed_revdown[0]=1;
			else if (channel==1 && REV2BUT) flag_timepot_changed_revdown[1]=1;
			flag_timepot_changed[channel]=1;

			if (channel==0 && INF1BUT) flag_timepot_changed_infdown[0]=1;
			else if (channel==1 && INF2BUT) flag_timepot_changed_infdown[1]=1;

			flag_timepot_changed[channel]=1;


			old_i_smoothed_potadc[TIME*2+channel] = i_smoothed_potadc[TIME*2+channel];
		}

		//
		// Check if TIME CV jack had activity
		//
		t=old_i_smoothed_cvadc[TIME*2+channel] - i_smoothed_cvadc[TIME*2+channel];
		if ((t>30) || (t<-30))
		{
			flag_timecv_changed[channel]=1;
			old_i_smoothed_cvadc[TIME*2+channel] = i_smoothed_cvadc[TIME*2+channel];
		}


		//
		// Check if time mult switch changed
		//
		t=get_switch_val(channel);
		if (old_switch_val[channel] != t)
		{
			flag_multswitch_changed[channel]=1;
			old_switch_val[channel]=t;
		}

		//
		// Update the time_mult amount based on the combined values of the pot and CV jack (hard clip at 4095)
		// --> If Inf was held down and the pot changed values, then time_mult is unquantized
		// --> Otherwise update time_mult only if the pot or cv jack changed
		//

		t_combined = i_smoothed_potadc[TIME*2+channel] + (i_smoothed_cvadc[TIME*2+channel]-2048);
		if (t_combined>4095) t_combined = 4095;
		else if (t_combined<0) t_combined = 0;

		if (flag_timepot_changed_infdown[channel])
		{
			time_mult[channel] = get_clk_div_exact(t_combined);
			time_mult[channel] = adjust_time_by_switch(time_mult[channel], channel);
		}
		else
		{
			if (flag_timecv_changed[channel] || flag_timepot_changed[channel] || flag_multswitch_changed[channel])
			{
				time_mult[channel] = get_clk_div_nominal(t_combined);
				time_mult[channel] = adjust_time_by_switch(time_mult[channel], channel);
			}
		}


#ifdef USE_VCXO
		if ( ((old_sample_rate_div[channel]>sample_rate_div[channel]) && ((old_sample_rate_div[channel]-sample_rate_div[channel])>4))
				|| ((old_sample_rate_div[channel]<sample_rate_div[channel]) && ((sample_rate_div[channel]-old_sample_rate_div[channel])>4)))
		{
			old_sample_rate_div[channel] = sample_rate_div[channel];
			old_sample_rate_num[channel] = sample_rate_num[channel];
			old_sample_rate_denom[channel] = sample_rate_denom[channel];
			setupMultisynth(channel, SI5351_PLL_A, sample_rate_div[channel], sample_rate_num[channel], sample_rate_denom[channel]);
		}
#endif





		if (time_mult[channel] != param[channel][TIME])
		{
			flag_time_param_changed[channel] = 1;

			flag_timecv_changed[channel]=0;
			flag_timepot_changed[channel]=0;
			flag_multswitch_changed[channel]=0;

			param[channel][TIME] = time_mult[channel];
		}


		// Set LEVEL and REGEN to 0 and 1 if we're in infinite repeat mode
		// Otherwise combine Pot and CV, and hard-clip at 4096

		if (mode[channel][INF] == 0){

			t_combined = i_smoothed_potadc[LEVEL*2+channel] + i_smoothed_cvadc[LEVEL*2+channel];
			if (t_combined>4095) t_combined = 4095;

			param[channel][LEVEL]=t_combined/4096.0;

			if (param[channel][LEVEL]<(22.0/4096.0))
				param[channel][LEVEL]=0.0;

			t_combined = i_smoothed_potadc[REGEN*2+channel] + i_smoothed_cvadc[REGEN*2+channel];
			if (t_combined>4095) t_combined = 4095;

			// From 0 to 80% of rotation, Regen goes from 0% to 100%
			// From 80% to 90% of rotation, Regen is set at 100%
			// From 90% to 100% of rotation, Regen goes from 100% to 110%
			if (t_combined<3300.0)
				param[channel][REGEN]=t_combined/3300.0;

			else if (t_combined<=3723.0)
				param[channel][REGEN]=1.0;

			else
				param[channel][REGEN]=t_combined/3723.0; // 4096/3723 = 110% regeneration

		} else {
			param[channel][LEVEL]=0.0;
			param[channel][REGEN]=1.0;
		}

		// MIX uses an equal power panning lookup table
		// Each MIX pot sets two parameters: wet and dry

		param[channel][MIX_DRY]=epp_lut[i_smoothed_potadc[MIXPOT*2+channel]];

		param[channel][MIX_WET]=epp_lut[4095 - i_smoothed_potadc[MIXPOT*2+channel]];

	}
}

inline void update_instant_params(uint8_t channel){
uint32_t t;

	if (flag_inf_change[channel])
	{
		flag_inf_change[channel]=0;

		if (mode[channel][INF])
		{
			mode[channel][INF] = 0;

			//need to reset the read_addr to a good place
			write_addr[channel]=loop_end[channel];
			set_divmult_time(channel);

			//set the write pointer ahead of the read addr
			//this way is weird, when we exit INF, it plays the rest of the loop and then silence for the duration of the period
			//write_addr[channel]=calculate_write_addr(channel, divmult_time[channel], mode[channel][REV]);


			loop_start[channel] = LOOP_RAM_BASE[channel];
			loop_end[channel] = LOOP_RAM_BASE[channel] + LOOP_SIZE;

		} else {
			mode[channel][INF] = 1;
			reset_pingled_tmr(channel);

			loop_start[channel]=read_addr[channel];
			loop_end[channel]=write_addr[channel];
		}

	}




	if (flag_rev_change[channel])
	{
		flag_rev_change[channel]=0;

		mode[channel][REV] = 1- mode[channel][REV];

		if (channel==0)
		{
			if (mode[channel][REV]) LED_REV1_ON;
			else LED_REV1_OFF;
		} else {
			if (mode[channel][REV]) LED_REV2_ON;
			else LED_REV2_OFF;
		}

		if (mode[channel][INF])
		{
			t=loop_start[channel];
			loop_start[channel]=loop_end[channel];
			loop_end[channel]=t;
		}
		else
			swap_read_write(channel);
	}


	if (flag_time_param_changed[channel] || flag_ping_was_changed)
	{
		flag_time_param_changed[channel]=0;
		set_divmult_time(channel);
	}
}


inline uint8_t get_switch_val(uint8_t channel)
{
	if (channel==0) return (TIMESW_CH1);
	else return (TIMESW_CH2);
}

// Adjust TIME by the time switch position
float adjust_time_by_switch(float val, uint8_t channel){
	uint8_t switch_val = get_switch_val(channel);

	if (switch_val==0b10) return(val + 16.0); //switch up: 17-32
	if (switch_val==0b01) return(val * 0.125); //switch down: eighth notes
}

float get_clk_div_nominal(uint16_t adc_val)
{
	if (adc_val<=40) //was 150
		return(P_1);
	else if (adc_val<=176) //was 310
		return(P_2);
	else if (adc_val<=471)
		return(P_3);
	else if (adc_val<=780)
		return(P_4);
	else if (adc_val<=1076)
		return(P_5);
	else if (adc_val<=1368)
		return(P_6);
	else if (adc_val<=1664)
		return(P_7);
	else if (adc_val<=1925)
		return(P_8);
	else if (adc_val<=2179) // Center
		return(P_9);
	else if (adc_val<=2448)
		return(P_10);
	else if (adc_val<=2714)
		return(P_11);
	else if (adc_val<=2991)
		return(P_12);
	else if (adc_val<=3276)
		return(P_13);
	else if (adc_val<=3586)
		return(P_14);
	else if (adc_val<=3879)
		return(P_15);
	else if (adc_val<=4046)
		return(P_16);
	else
		return(P_17);
}

float get_clk_div_exact(uint16_t adc_val)
{
	float t, b, tval, bval;

	if (adc_val<=40) //was 150
		return(P_1);
	else if (adc_val<=176)
	{t=176;b=40;tval=P_2;bval=P_1;}
	else if (adc_val<=471)
	{t=471;b=176;tval=P_3;bval=P_2;}
	else if (adc_val<=780)
	{t=780;b=471;tval=P_4;bval=P_3;}
	else if (adc_val<=1076)
	{t=1076;b=780;tval=P_5;bval=P_4;}
	else if (adc_val<=1368)
	{t=1368;b=1076;tval=P_6;bval=P_5;}
	else if (adc_val<=1664)
	{t=1664;b=1368;tval=P_7;bval=P_6;}
	else if (adc_val<=1925)
	{t=1925;b=1664;tval=P_8;bval=P_7;}
	else if (adc_val<=2179) // Center
	{t=2179;b=1925;tval=P_9;bval=P_8;}
	else if (adc_val<=2448)
	{t=2448;b=2179;tval=P_10;bval=P_9;}
	else if (adc_val<=2714)
	{t=2714;b=2448;tval=P_11;bval=P_10;}
	else if (adc_val<=2991)
	{t=2991;b=2714;tval=P_12;bval=P_11;}
	else if (adc_val<=3276)
	{t=3276;b=2991;tval=P_13;bval=P_12;}
	else if (adc_val<=3586)
	{t=3586;b=3276;tval=P_14;bval=P_13;}
	else if (adc_val<=3879)
	{t=3879;b=3586;tval=P_15;bval=P_14;}
	else if (adc_val<=4046)
	{t=4046;b=3879;tval=P_16;bval=P_15;}
	else
		return(P_17);

	return( ((t-adc_val)/(t-b))*bval + ((adc_val-b)/(t-b))*tval );
}



/*if (mode[channel][TIMEMODE_POT]==MOD_SAMPLE_RATE_Q)
{
	sample_rate_div[channel] = 11 * (get_clk_div_nominal(4095-i_smoothed_potadc[TIME*2+channel]) + get_clk_div_nominal(4095-i_smoothed_cvadc[TIME*2+channel]));

	if (sample_rate_div[channel]>182) sample_rate_div[channel]=182; //limit at 8kHz SR = 2.048MHz MCLK

	sample_rate_num[channel] = 265;
	sample_rate_denom[channel] = 512;
}

else if (mode[channel][TIMEMODE_POT]==MOD_SAMPLE_RATE_NOQ)
{
	t=i_smoothed_cvadc[TIME*2+channel] - old_smoothed_cvadc[channel];
	t2=i_smoothed_potadc[TIME*2+channel] - old_smoothed_potadc[channel];

	if ( t>50 || t<-50 || t2<-50 || t2>50 )
	{
		old_smoothed_cvadc[channel] = i_smoothed_cvadc[TIME*2+channel];
		old_smoothed_potadc[channel] = i_smoothed_potadc[TIME*2+channel];


		sample_rate_div[channel] = ((4095-i_smoothed_potadc[TIME*2+channel])/27)+30; //8 to 520 to 1032
		sample_rate_div[channel] += ((4095-i_smoothed_cvadc[TIME*2+channel])/27)+30; //8 to 520 to 1032

		if (sample_rate_div[channel]>182) sample_rate_div[channel]=182; //limit at 8kHz SR = 2.048MHz MCLK
	}

	sample_rate_num[channel] = 265;
	sample_rate_denom[channel] = 512;
}

else if (mode[channel][TIMEMODE_POT]==MOD_READWRITE_TIME)
{*/
