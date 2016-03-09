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

#define NUM_CHAN_MODES 6

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


//Gate/Trig modes
#define LOOP_CLOCK_JACK 4
#define MAIN_CLOCK_JACK 5

enum GateTrig_Modes{
	GATE_MODE,
	TRIG_MODE
};


//Global Modes
#define NUM_GLOBAL_MODES 1

enum Global_Modes{
	AUTO_MUTE
};

#define P_1 1.0
#define P_2 1.5
#define P_3 2.0
#define P_4 3.0
#define P_5 4.0
#define P_6 5.0
#define P_7 6.0
#define P_8 7.0
#define P_9 8.0
#define P_10 9.0
#define P_11 10.0
#define P_12 11.0
#define P_13 12.0
#define P_14 13.0
#define P_15 14.0
#define P_16 15.0
#define P_17 16.0


void update_params(void);
void process_adc(void);

float get_clk_div_nominal(uint16_t adc_val);
float get_clk_div_exact(uint16_t adc_val);
uint8_t get_switch_val(uint8_t channel);
float adjust_time_by_switch(float val, uint8_t channel);
void init_LowPassCoefs(void);
void init_params(void);
void init_modes(void);

#endif /* PARAMS_H_ */
