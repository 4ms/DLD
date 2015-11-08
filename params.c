/*
 * params.c
 *
 *  Created on: Mar 27, 2015
 *      Author: design
 */

#define QUANTIZE_TIMECV_CH1 0
#define QUANTIZE_TIMECV_CH2 1

#include "adc.h"
#include "dig_inouts.h"
#include "params.h"
#include "globals.h"
#include "equalpowpan_lut.h"
#include "exp_1voct.h"

extern __IO uint16_t potadc_buffer[NUM_POT_ADCS];
extern __IO uint16_t cvadc_buffer[NUM_CV_ADCS];

extern uint8_t flag_time_param_changed[2];


float param[NUM_CHAN][NUM_PARAMS] = { {1.0,0,0,0,0,0,0}, {1.0,0,0,0,0,0,0} };

const int32_t MIN_POT_ADC_CHANGE[NUM_POT_ADCS] = {60, 60, 20, 20, 20, 20, 20, 20};
const int32_t MIN_CV_ADC_CHANGE[NUM_CV_ADCS] = {60, 60, 20, 20, 20, 20};


const float POT_LPF_COEF[NUM_POT_ADCS] = {0.999, 0.999, 0.999, 0.999, 0.999, 0.999, 0.999, 0.999};
const float CV_LPF_COEF[NUM_CV_ADCS] = {0.99, 0.99, 0.99, 0.99, 0.99, 0.99};


inline float LowPassSmoothingFilter(float current_value, float new_value, float coef){
	return (current_value * coef) + (new_value * (1.0f-coef));
}


void update_adc_params(void){
	int16_t i_smoothed_potadc[NUM_POT_ADCS]={0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF};
	int16_t i_smoothed_cvadc[NUM_CV_ADCS]={0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF};

	static float smoothed_potadc[NUM_POT_ADCS]={0,0,0,0,0,0,0,0};
	static float smoothed_cvadc[NUM_CV_ADCS]={0,0,0,0,0,0};

	static int16_t old_smoothed_cvadc[2]={0,0};
	static int16_t old_smoothed_potadc[2]={0,0};

	uint8_t i, channel;
	int32_t t,t2;
	float t_f;
	uint32_t t_combined;
	float base_time;
	int16_t adc;
	uint8_t switch1_val, switch2_val;

	for (i=0;i<NUM_POT_ADCS;i++){
		smoothed_potadc[i] = LowPassSmoothingFilter(smoothed_potadc[i], (float)potadc_buffer[i], POT_LPF_COEF[i]);
		i_smoothed_potadc[i] = (int16_t)smoothed_potadc[i];
	}

	for (i=0;i<NUM_CV_ADCS;i++){
		smoothed_cvadc[i] = LowPassSmoothingFilter(smoothed_cvadc[i], (float)cvadc_buffer[i], CV_LPF_COEF[i]);
		i_smoothed_cvadc[i] = (int16_t)smoothed_cvadc[i];
	}


	for (channel=0;channel<2;channel++){

		//Add TIME pot and cv values, hard clip at 4096

		if ((channel==0 && QUANTIZE_TIMECV_CH1) || (channel==1 && QUANTIZE_TIMECV_CH2)){

			t_combined = i_smoothed_potadc[TIME*2+channel] + i_smoothed_cvadc[TIME*2+channel];
			if (t_combined>4095) t_combined = 4095;

			base_time = get_clk_div_nominal(t_combined);

		} else {

			t=i_smoothed_cvadc[TIME*2+channel] - old_smoothed_cvadc[channel];
			t2=i_smoothed_potadc[TIME*2+channel] - old_smoothed_potadc[channel];

			if ( t>50 || t<-50 || t2<-50 || t2>50 ){
				old_smoothed_cvadc[channel] = i_smoothed_cvadc[TIME*2+channel];
				old_smoothed_potadc[channel] = i_smoothed_potadc[TIME*2+channel];
			}

			base_time = get_clk_div_nominal(old_smoothed_potadc[channel]);

			t_f=exp_1voct[old_smoothed_cvadc[channel]>>1];
			if (t_f < 1.1) t_f=1.0;

			base_time = base_time * t_f;

		}

		// Adjust TIME by the time switch position

		if (channel==0){
			switch1_val = TIMESW_CH1;
		}else{
			switch1_val = TIMESW_CH2;
		}

		if (switch1_val==0b10) base_time = base_time + 16.0; //switch up: 17-32
		//else if (switch1_val==0b11) base_time = base_time; //switch in middle: 1-16
		else if (switch1_val==0b01) base_time = base_time / 8.0; //switch down: eighth notes


		if (base_time!=param[channel][TIME]){
			flag_time_param_changed[channel]=1;

			param[channel][TIME] = base_time;
		}


		// Set LEVEL and REGEN to 0 and 1 if we're in infinite repeat mode
		// Otherwise combine Pot and CV, and hard-clip at 4096
																#ifndef INF_WP_MODE
		if (param[channel][INF]==0.0){
																#endif
			t_combined = i_smoothed_potadc[LEVEL*2+channel] + i_smoothed_cvadc[LEVEL*2+channel];
			if (t_combined>4095) t_combined = 4095;

			param[channel][LEVEL]=t_combined/4095.0;

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


																#ifndef INF_WP_MODE
		} else {
			param[channel][LEVEL]=0.0;
			param[channel][REGEN]=1.0;
		}
																#endif

		// MIX uses an equal power panning lookup table
		// Each MIX pot sets two parameters: wet and dry

		param[channel][MIX_DRY]=epp_lut[i_smoothed_potadc[MIXPOT*2+channel]];

		param[channel][MIX_WET]=epp_lut[4095 - i_smoothed_potadc[MIXPOT*2+channel]];

	}

}


float get_clk_div_nominal(uint16_t adc_val){
	if (adc_val<=100) //was 150
		return(P_1);
	else if (adc_val<=310) //was 310
		return(P_2);
	else if (adc_val<=565)
		return(P_3);
	else if (adc_val<=816)
		return(P_4);
	else if (adc_val<=1062)
		return(P_5);
	else if (adc_val<=1304)
		return(P_6);
	else if (adc_val<=1529)
		return(P_7);
	else if (adc_val<=1742)
		return(P_8);
	else if (adc_val<=1950)
		return(P_9);
	else if (adc_val<=2157) // Center
		return(P_10);
	else if (adc_val<=2365)
		return(P_11);
	else if (adc_val<=2580)
		return(P_12);
	else if (adc_val<=2806)
		return(P_13);
	else if (adc_val<=3044)
		return(P_14);
	else if (adc_val<=3289)
		return(P_15);
	else if (adc_val<=3537)
		return(P_16);
	else if (adc_val<=3790)
		return(P_17);
	else if (adc_val<=4003)
		return(P_18);
	else
		return(P_19);

}


