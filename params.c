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
#include "equal_pow_pan_padded.h"
#include "exp_1voct.h"
#include "si5153a.h"
#include "timekeeper.h"
#include "looping_delay.h"
#include "log_taper_padded.h"

extern const float log_taper[4096];
extern const float exp_1voct[4096];

extern __IO uint16_t potadc_buffer[NUM_POT_ADCS];
extern __IO uint16_t cvadc_buffer[NUM_CV_ADCS];


extern volatile uint32_t write_addr[NUM_CHAN];
extern volatile uint32_t read_addr[NUM_CHAN];

extern volatile uint32_t divmult_time[NUM_CHAN];

extern uint32_t loop_start[NUM_CHAN];
extern uint32_t loop_end[NUM_CHAN];
extern const uint32_t LOOP_RAM_BASE[NUM_CHAN];

extern uint8_t flag_ping_was_changed[NUM_CHAN];
extern uint8_t flag_inf_change[NUM_CHAN];
extern uint8_t flag_rev_change[NUM_CHAN];
extern uint8_t flag_pot_changed_revdown[NUM_POT_ADCS];
extern uint8_t flag_pot_changed_infdown[NUM_POT_ADCS];

extern uint8_t disable_mode_changes;

float param[NUM_CHAN][NUM_PARAMS];
uint8_t mode[NUM_CHAN][NUM_CHAN_MODES];
uint8_t global_mode[NUM_GLOBAL_MODES];

uint8_t flag_time_param_changed[2];

int32_t MIN_POT_ADC_CHANGE[NUM_POT_ADCS];
int32_t MIN_CV_ADC_CHANGE[NUM_CV_ADCS];

extern int16_t CV_CALIBRATION_OFFSET[NUM_CV_ADCS];

float POT_LPF_COEF[NUM_POT_ADCS];
float CV_LPF_COEF[NUM_CV_ADCS];

//Low Pass filtered adc values:
float smoothed_potadc[NUM_POT_ADCS];
float smoothed_cvadc[NUM_CV_ADCS];

//Integer-ized low pass filtered adc values:
int16_t i_smoothed_potadc[NUM_POT_ADCS];
int16_t i_smoothed_cvadc[NUM_CV_ADCS];

int16_t old_i_smoothed_cvadc[NUM_CV_ADCS];
int16_t old_i_smoothed_potadc[NUM_POT_ADCS];

float smoothed_rawcvadc[NUM_CV_ADCS];
int16_t i_smoothed_rawcvadc[NUM_CV_ADCS];

//Change in pot since last process_adc
int32_t pot_delta[NUM_POT_ADCS];
int32_t cv_delta[NUM_POT_ADCS];


void init_params(void)
{
	uint8_t channel=0;

	for (channel=0;channel<NUM_CHAN;channel++){
		param[channel][TIME] = 0.0;
		param[channel][LEVEL] = 0.7;
		param[channel][REGEN] = 0.5;
		param[channel][MIX_DRY] = 0.7;
		param[channel][MIX_WET] = 0.7;
		param[channel][TRACKING_COMP] = 1.02;
	}

}

void init_modes(void)
{
	uint8_t channel=0;

	for (channel=0;channel<NUM_CHAN;channel++){
		mode[channel][INF] = 0;
		mode[channel][REV] = 0;
		mode[channel][TIMEMODE_POT] = MOD_READWRITE_TIME_Q;
		mode[channel][TIMEMODE_JACK] = MOD_READWRITE_TIME_Q;

		mode[channel][LOOP_CLOCK_GATETRIG] = TRIG_MODE;
	}

	mode[0][MAIN_CLOCK_GATETRIG] = TRIG_MODE;
	global_mode[AUTO_UNQ] = 0;
}


inline float LowPassSmoothingFilter(float current_value, float new_value, float coef)
{
	return (current_value * coef) + (new_value * (1.0f-coef));
}

void init_LowPassCoefs(void)
{
	float t;
	uint8_t i;

	t=50.0;

	CV_LPF_COEF[TIME*2] = 1.0-(1.0/t);
	CV_LPF_COEF[TIME*2+1] = 1.0-(1.0/t);

	t=10.0;

	CV_LPF_COEF[LEVEL*2] = 1.0-(1.0/t);
	CV_LPF_COEF[LEVEL*2+1] = 1.0-(1.0/t);

	CV_LPF_COEF[REGEN*2] = 1.0-(1.0/t);
	CV_LPF_COEF[REGEN*2+1] = 1.0-(1.0/t);

	MIN_CV_ADC_CHANGE[TIME*2] = 30;
	MIN_CV_ADC_CHANGE[TIME*2+1] = 30;

	MIN_CV_ADC_CHANGE[LEVEL*2] = 20;
	MIN_CV_ADC_CHANGE[LEVEL*2+1] = 20;

	MIN_CV_ADC_CHANGE[REGEN*2] = 20;
	MIN_CV_ADC_CHANGE[REGEN*2+1] = 20;


	t=50.0; //50.0 = about 100ms to turn a knob fully

	POT_LPF_COEF[TIME_POT*2] = 1.0-(1.0/t);
	POT_LPF_COEF[TIME_POT*2+1] = 1.0-(1.0/t);

	POT_LPF_COEF[LEVEL_POT*2] = 1.0-(1.0/t);
	POT_LPF_COEF[LEVEL_POT*2+1] = 1.0-(1.0/t);

	POT_LPF_COEF[REGEN_POT*2] = 1.0-(1.0/t);
	POT_LPF_COEF[REGEN_POT*2+1] = 1.0-(1.0/t);

	POT_LPF_COEF[MIX_POT*2] = 1.0-(1.0/t);
	POT_LPF_COEF[MIX_POT*2+1] = 1.0-(1.0/t);

	MIN_POT_ADC_CHANGE[TIME_POT*2] = 75;
	MIN_POT_ADC_CHANGE[TIME_POT*2+1] = 75;

	MIN_POT_ADC_CHANGE[LEVEL_POT*2] = 40;
	MIN_POT_ADC_CHANGE[LEVEL_POT*2+1] = 40;

	MIN_POT_ADC_CHANGE[REGEN_POT*2] = 60;
	MIN_POT_ADC_CHANGE[REGEN_POT*2+1] = 60;

	MIN_POT_ADC_CHANGE[MIX_POT*2] = 40;
	MIN_POT_ADC_CHANGE[MIX_POT*2+1] = 40;



	for (i=0;i<NUM_POT_ADCS;i++)
	{
		smoothed_potadc[i]=0;
		old_i_smoothed_potadc[i]=0;
		i_smoothed_potadc[i]=0x7FFF;
		pot_delta[i]=0;
	}
	for (i=0;i<NUM_CV_ADCS;i++)
	{
		smoothed_cvadc[i]=0;
		smoothed_rawcvadc[i]=0;
		old_i_smoothed_cvadc[i]=0;
		i_smoothed_cvadc[i]=0x7FFF;
		i_smoothed_rawcvadc[i]=0x7FFF;
		cv_delta[i]=0;
	}
}


void process_adc(void)
{
	uint8_t i;
	int32_t t;

	uint8_t flag_pot_changed[NUM_POT_ADCS];

	//
	// Run a LPF on the pots and CV jacks
	//
	for (i=0;i<NUM_POT_ADCS;i++)
	{
		flag_pot_changed[i]=0;

		smoothed_potadc[i] = LowPassSmoothingFilter(smoothed_potadc[i], (float)potadc_buffer[i], POT_LPF_COEF[i]);
		i_smoothed_potadc[i] = (int16_t)smoothed_potadc[i];

		t=i_smoothed_potadc[i] - old_i_smoothed_potadc[i];
		if ((t>MIN_POT_ADC_CHANGE[i]) || (t<-MIN_POT_ADC_CHANGE[i]))
		{
			flag_pot_changed[i]=1;

			pot_delta[i] = t;

			//Todo: We could clean up the use flag_pot_changed_XXXdown, instead change the mode right here
			if (!(i&1) && REV1BUT) flag_pot_changed_revdown[i]=1;
			else if ((i&1) && REV2BUT) flag_pot_changed_revdown[i]=1;

			if (!(i&1) && INF1BUT) flag_pot_changed_infdown[i]=1;
			else if ((i&1) && INF2BUT) flag_pot_changed_infdown[i]=1;

			old_i_smoothed_potadc[i] = i_smoothed_potadc[i];
		}
	}

	for (i=0;i<NUM_CHAN;i++)
	{
		if (flag_pot_changed_infdown[TIME_POT*2+i])
		{
			mode[i][TIMEMODE_POT]=MOD_READWRITE_TIME_NOQ;
			mode[i][TIMEMODE_JACK] = MOD_READWRITE_TIME_NOQ;
		}
		else if (flag_pot_changed[TIME_POT*2+i])
		{
			mode[i][TIMEMODE_POT]=MOD_READWRITE_TIME_Q;
			if (!global_mode[AUTO_UNQ])
				mode[i][TIMEMODE_JACK] = MOD_READWRITE_TIME_Q;

		}

		//Audio rate divmult time: auto switch to Unquantized mode
		if (global_mode[AUTO_UNQ]){
			if (divmult_time[i] < 1024) // 48000 / 1023 = 47Hz
				mode[i][TIMEMODE_JACK] = MOD_READWRITE_TIME_NOQ;
			else
				mode[i][TIMEMODE_JACK] = MOD_READWRITE_TIME_Q;
		}



		if (flag_pot_changed_infdown[REGEN_POT*2+i])
		{
			mode[i][WINDOWMODE_POT]=WINDOW;
		}
		else if (flag_pot_changed[REGEN_POT*2+i])
		{
			mode[i][WINDOWMODE_POT]=NO_WINDOW;
		}
	}



	for (i=0;i<NUM_CV_ADCS;i++)
	{
		smoothed_cvadc[i] = LowPassSmoothingFilter(smoothed_cvadc[i], (float)(cvadc_buffer[i]+CV_CALIBRATION_OFFSET[i]), CV_LPF_COEF[i]);
		i_smoothed_cvadc[i] = (int16_t)smoothed_cvadc[i];

		t=i_smoothed_cvadc[i] - old_i_smoothed_cvadc[i];
		if ((t>MIN_CV_ADC_CHANGE[i]) || (t<-MIN_CV_ADC_CHANGE[i]))
		{
			cv_delta[i] = t;
			old_i_smoothed_cvadc[i] = i_smoothed_cvadc[i];
		}
	}

	if (global_mode[CALIBRATE])
	{
		for (i=0;i<NUM_CV_ADCS;i++)
		{
			smoothed_rawcvadc[i] = LowPassSmoothingFilter(smoothed_rawcvadc[i], (float)(cvadc_buffer[i]), 0.999);
			i_smoothed_rawcvadc[i] = (int16_t)smoothed_rawcvadc[i];
		}
	}



}




void update_params(void)
{
	uint8_t channel;
	int32_t t;

	int32_t t_combined;

	float abs_amt;
	uint8_t subtract;

	float time_mult[2];
	//time_mult[channel]=0.0;


	global_mode[DCINPUT]=(DCINPUT_JUMPER > 0)?1:0;

	for (channel=0;channel<2;channel++)
	{

//
// ******* TIME **********
//

		if (mode[channel][TIMEMODE_POT]==MOD_READWRITE_TIME_NOQ)
		{
			if (mode[channel][TIMEMODE_JACK]==MOD_READWRITE_TIME_NOQ) //Pot and Jack are unquantized
			{

				if (old_i_smoothed_cvadc[TIME*2+channel] <= 2048) //positive voltage on the Time CV jack
				{
					t_combined = (2048-old_i_smoothed_cvadc[TIME*2+channel]) * param[channel][TRACKING_COMP];
					time_mult[channel] = get_clk_div_exact(i_smoothed_potadc[TIME_POT*2+channel])  / exp_1voct[t_combined];
				}
				else
				{
					t_combined = i_smoothed_potadc[TIME_POT*2+channel] + (old_i_smoothed_cvadc[TIME*2+channel]-2048);
					time_mult[channel] = get_clk_div_exact(t_combined);
				}
			}
			else //Pot is unquantized, Jack is quantized
			{
				time_mult[channel] = get_clk_div_exact(i_smoothed_potadc[TIME_POT*2+channel]);
				if (i_smoothed_cvadc[TIME*2+channel] >= 2048)
					time_mult[channel] *= get_clk_div_nominal(i_smoothed_cvadc[TIME*2+channel]-2048);
				else
					time_mult[channel] = time_mult[channel] / get_clk_div_nominal(2048 - i_smoothed_cvadc[TIME*2+channel]);

			}
		}
		else
		{
			if (mode[channel][TIMEMODE_JACK]==MOD_READWRITE_TIME_Q) //Pot and Jack are quantized
			{
				t_combined = i_smoothed_potadc[TIME_POT*2+channel] + (i_smoothed_cvadc[TIME*2+channel]-2048);

				if (t_combined>4095) t_combined = 4095;
				else if (t_combined<0) t_combined = 0;

				time_mult[channel] = get_clk_div_nominal(t_combined);
			}
			else //Pot is quantized, Jack is unquantized
			{

				if (i_smoothed_cvadc[TIME*2+channel] <= 2048) //positive voltage on the Time CV jack
				{
					t_combined = (2048-old_i_smoothed_cvadc[TIME*2+channel]) * param[channel][TRACKING_COMP];
					time_mult[channel] = get_clk_div_nominal(i_smoothed_potadc[TIME_POT*2+channel])  / exp_1voct[t_combined];
				}
				else
				{
					time_mult[channel] = get_clk_div_nominal(i_smoothed_potadc[TIME_POT*2+channel]);
					time_mult[channel] *= get_clk_div_exact(old_i_smoothed_cvadc[TIME*2+channel]-2048);
				}


			}
		}
		time_mult[channel] = adjust_time_by_switch(time_mult[channel], channel);




		if (time_mult[channel] != param[channel][TIME])
		{
			flag_time_param_changed[channel] = 1;
			param[channel][TIME] = time_mult[channel];
		}


		// Set LEVEL and REGEN to 0 and 1 if we're in infinite repeat mode
		// Otherwise combine Pot and CV, and hard-clip at 4096

		if (mode[channel][INF] == 0)
		{

// ******* LEVEL **********

			t_combined = i_smoothed_potadc[LEVEL_POT*2+channel] + i_smoothed_cvadc[LEVEL*2+channel];
			if (t_combined>4095) t_combined = 4095;

			param[channel][LEVEL] = log_taper[t_combined];



// ******* REGEN **********

			t_combined = i_smoothed_potadc[REGEN_POT*2+channel] + i_smoothed_cvadc[REGEN*2+channel];
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

		}
		else
		{

// *******  INF  **********

			param[channel][LEVEL]=0.0;
			param[channel][REGEN]=1.0;


			//
			// If REGEN was wiggled while INF is held down, then scroll the loop
			//
			t = pot_delta[REGEN_POT*2+channel] + cv_delta[REGEN_POT*2+channel];

			if (mode[channel][WINDOWMODE_POT]==WINDOW && (t != 0))
			{
				if (t < 0)
				{
					abs_amt = t / -4096.0;
					//abs_amt = pot_delta[REGEN*2+channel] / -4096.0;
					subtract = 1;
				} else
				{
					abs_amt = t / 4096.0;
					//abs_amt = pot_delta[REGEN*2+channel] / 4096.0;
					subtract = 0;
				}

				scroll_loop(channel, abs_amt, subtract);

				pot_delta[REGEN_POT*2+channel]=0;
				cv_delta[REGEN*2+channel]=0;

			}

		}

// ******* MIX **********

		//
		// MIX uses an equal power panning lookup table
		// Each MIX pot sets two parameters: wet and dry
		//

		param[channel][MIX_DRY]=epp_lut[i_smoothed_potadc[MIX_POT*2+channel]];

		param[channel][MIX_WET]=epp_lut[4095 - i_smoothed_potadc[MIX_POT*2+channel]];

	}
}


//
// Handle all flags to change modes: INF, REV, and ping or div/mult time
//
inline void process_mode_flags(void){
	uint8_t channel;
	uint32_t t;

	if (!disable_mode_changes)
	{
		for (channel=0;channel<2;channel++){

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
					reset_loopled_tmr(channel);

					loop_start[channel]=read_addr[channel];
					loop_end[channel]=write_addr[channel];
				}

			}


			if (flag_rev_change[channel])
			{
				flag_rev_change[channel]=0;

				mode[channel][REV] = 1- mode[channel][REV];

				if (mode[channel][INF])
				{
					t=loop_start[channel];
					loop_start[channel]=loop_end[channel];
					loop_end[channel]=t;
				}
				else
					swap_read_write(channel);
			}


			if (flag_time_param_changed[channel] || flag_ping_was_changed[channel])
			{
				flag_time_param_changed[channel]=0;
				flag_ping_was_changed[channel]=0;
				set_divmult_time(channel);
			}
		}
	}
}


inline uint8_t get_switch_val(uint8_t channel)
{

	if (channel==0)
		return (TIMESW_CH1);
	else
		return (TIMESW_CH2);
}


// Adjust TIME by the time switch position
float adjust_time_by_switch(float val, uint8_t channel){
	uint8_t switch_val = get_switch_val(channel);

	if (switch_val==0b10) return(val + 16.0); //switch up: 17-32
	if (switch_val==0b01) return(val * 0.125); //switch down: eighth notes
	return val;
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


/*
uint32_t sample_rate_div[2]={36,36};
uint32_t sample_rate_num[2]={265,265};
uint32_t sample_rate_denom[2]={512,512};
uint32_t old_sample_rate_div[2]={0,0};
uint32_t old_sample_rate_num[2]={0,0};
uint32_t old_sample_rate_denom[2]={0,0};


#ifdef USE_VCXO

		if (mode[channel][TIMEMODE_POT]==MOD_SAMPLE_RATE_Q)
		{
			sample_rate_div[channel] = 11 * (get_clk_div_nominal(4095-i_smoothed_potadc[TIME*2+channel]) + get_clk_div_nominal(4095-i_smoothed_cvadc[TIME*2+channel]));

			//if (sample_rate_div[channel]>182) sample_rate_div[channel]=182; //limit at 8kHz SR = 2.048MHz MCLK
			if (sample_rate_div[channel]>360) sample_rate_div[channel]=360; //limit at 8kHz SR = 2.048MHz MCLK

			sample_rate_num[channel] = 265;
			sample_rate_denom[channel] = 512;
		}

		if ( ((old_sample_rate_div[channel]>sample_rate_div[channel]) && ((old_sample_rate_div[channel]-sample_rate_div[channel])>4))
				|| ((old_sample_rate_div[channel]<sample_rate_div[channel]) && ((sample_rate_div[channel]-old_sample_rate_div[channel])>4)))
		{
			old_sample_rate_div[channel] = sample_rate_div[channel];
			old_sample_rate_num[channel] = sample_rate_num[channel];
			old_sample_rate_denom[channel] = sample_rate_denom[channel];
			setupMultisynth(channel, SI5351_PLL_A, sample_rate_div[channel], sample_rate_num[channel], sample_rate_denom[channel]);
		}
#endif
*/

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
