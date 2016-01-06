/*
 * params.h
 *
 *  Created on: Mar 27, 2015
 *      Author: design
 */

#ifndef PARAMS_H_
#define PARAMS_H_
#include <stm32f4xx.h>


//
// Params
// Params are float values related to pots and CV jacks
//

#define NUM_PARAMS 5

//TIME: fractional value for time multiplication, integer value for time division
#define TIME 0

//LEVEL: 0..1 representing amount of main input mixed into delay loop
#define LEVEL 1

//REGEN: 0..1 representing amount of regeneration
#define REGEN 2

//MIX: 0..1 representing mix of delayed and clean on the main output
#define MIXPOT 3
#define MIX_DRY 3
#define MIX_WET 4


// Modes
// Modes are integer values (often) related to switches or settings


#define NUM_MODES 4

//INF: 0 = disabled, 1 = infinite repeat enabled
#define INF 0

//REV: 0 = forward (default), 1 = reverse
#define REV 1

//TIMEMODE
#define TIMEMODE_POT 2
#define TIMEMODE_JACK 3

//TIMEMODE values:
enum Time_Modes {
	MOD_READWRITE_TIME,
	MOD_SAMPLE_RATE_NOQ,
	MOD_SAMPLE_RATE_Q,
	MOD_SR_INVRW
};


#define P_1 1.0
#define P_2 1.5
#define P_3 2.0
#define P_4 2.5
#define P_5 3.0
#define P_6 4.0
#define P_7 5.0
#define P_8 6.0
#define P_9 7.0
#define P_10 8.0
#define P_11 9.0
#define P_12 9.5
#define P_13 10.0
#define P_14 11.0
#define P_15 12.0
#define P_16 13.0
#define P_17 14.0
#define P_18 15.0
#define P_19 16.0


void update_adc_params(void);
float get_clk_div_nominal(uint16_t adc_val);
void init_LowPassCoefs(void);
void init_params(void);
void init_modes(void);

#endif /* PARAMS_H_ */
