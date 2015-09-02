/*
 * params.c
 *
 *  Created on: Mar 27, 2015
 *      Author: design
 */

#include "adc.h"
#include "dig_inouts.h"
#include "params.h"
#include "globals.h"
#include "equalpowpan_lut.h"

extern __IO uint16_t potadc_buffer[NUM_POT_ADCS];
extern __IO uint16_t cvadc_buffer[NUM_CV_ADCS];

extern uint8_t flag_time_param_changed[2];


float param[NUM_CHAN][NUM_PARAMS] = { {1,0,0,0,0,0,0}, {1,0,0,0,0,0,0} };

const int32_t MIN_POT_ADC_CHANGE[NUM_POT_ADCS] = {60, 60, 20, 20, 20, 20, 20, 20};
const int32_t MIN_CV_ADC_CHANGE[NUM_CV_ADCS] = {60, 60, 20, 20, 20, 20};


void update_adc_params(void){
	static int16_t old_potadc_buffer[NUM_POT_ADCS]={0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF};
	static int16_t old_cvadc_buffer[NUM_CV_ADCS]={0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF};

//	uint8_t cv_or_pot_changed[NUM_POT_ADCS];

	uint8_t i;
	int32_t t;
	uint32_t t_combined;
	float base_time;
	int16_t adc;
	uint8_t switch1_val, switch2_val;

//	DEBUG3_ON; //runs every 3ms, takes 10-40us

	//Ignore small variations in the ADC
	//To-do: do true hysteris checking for discrete value parameters (TIME)

	for (i=0;i<NUM_POT_ADCS;i++){
//		cv_or_pot_changed[i]=0;

		adc = (int16_t)potadc_buffer[i];
		t = adc - old_potadc_buffer[i];

		if (t<(-1*MIN_POT_ADC_CHANGE[i]) || t>MIN_POT_ADC_CHANGE[i]){
			old_potadc_buffer[i] = potadc_buffer[i];
//			cv_or_pot_changed[i] = 0b01;
		}
	}

	for (i=0;i<NUM_CV_ADCS;i++){

		adc = cvadc_buffer[i];
		t = adc - old_cvadc_buffer[i];

		if (t<(-1*MIN_CV_ADC_CHANGE[i]) || t>MIN_CV_ADC_CHANGE[i]){
			old_cvadc_buffer[i] = cvadc_buffer[i];
//			cv_or_pot_changed[i] += 0b10;
		}
	}




	for (i=0;i<2;i++){

		//Add TIME pot and cv values, hard clip at 4096

		t_combined = old_potadc_buffer[TIME*2+i] + old_cvadc_buffer[TIME*2+i];
		if (t_combined>4095) t_combined = 4095;

		base_time = get_clk_div_nominal(t_combined);

		// Adjust TIME by the time switch position

		if (i==0){
			switch1_val = TIMESW_CH1;
		}else{
			switch1_val = TIMESW_CH2;
		}

		if (switch1_val==0b10) base_time = base_time + 16; //switch up: 17-32
		else if (switch1_val==0b11) base_time = base_time; //switch in middle: 1-16
		else if (switch1_val==0b01) base_time = base_time / 8; //switch down: eighth notes


		if (base_time!=param[i][TIME]){
			flag_time_param_changed[i]=1;
			param[i][TIME] = base_time;
		}


		// Set LEVEL and REGEN to 0 and 1 if we're in infinite repeat mode
		// Otherwise combine Pot and CV, and hard-clip at 4096
																#ifndef INF_WP_MODE
		if (param[i][INF]==0.0){
																#endif

			t_combined = old_potadc_buffer[LEVEL*2+i] + old_cvadc_buffer[LEVEL*2+i];
			if (t_combined>4095) t_combined = 4095;

			param[i][LEVEL]=t_combined/4095.0;

			t_combined = old_potadc_buffer[REGEN*2+i] + old_cvadc_buffer[REGEN*2+i];
			if (t_combined>4095) t_combined = 4095;

			// From 0 to 80% of rotation, Regen goes from 0% to 100%
			// From 80% to 90% of rotation, Regen is set at 100%
			// From 90% to 100% of rotation, Regen goes from 100% to 110%
			if (t_combined<3300.0)
				param[i][REGEN]=t_combined/3300.0;

			else if (t_combined<=3723.0)
				param[i][REGEN]=1.0;

			else
				param[i][REGEN]=t_combined/3723.0; // 4096/3723 = 110% regeneration


																#ifndef INF_WP_MODE
		} else {
			param[i][LEVEL]=0.0;
			param[i][REGEN]=1.0;
		}
																#endif

		// MIX uses an equal power panning lookup table
		// Each MIX pot sets two parameters: wet and dry

		param[i][MIX_DRY]=epp_lut[old_potadc_buffer[MIXPOT*2+i]];

		param[i][MIX_WET]=epp_lut[4095 - old_potadc_buffer[MIXPOT*2+i]];



	}
//	DEBUG3_OFF;

}


float get_clk_div_nominal(uint16_t adc_val){
	if (adc_val<=150)
		return(P_1);
	else if (adc_val<=310)
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


