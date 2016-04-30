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

#define NUM_PARAMS 6

/*
//TIME: fractional value for time multiplication, integer value for time division
//LEVEL: 0..1 representing amount of main input mixed into delay loop
//REGEN: 0..1 representing amount of regeneration
//MIX: 0..1 representing mix of delayed and clean on the main output
//TRACKING_COMP: -2.0 .. 2.0 representing compensation for 1V/oct tracking
*/

enum Params{
	TIME,
	LEVEL,
	REGEN,
	MIX_DRY,
	MIX_WET,
	TRACKING_COMP
};

enum Pots{
	TIME_POT,
	LEVEL_POT,
	REGEN_POT,
	MIX_POT
};

// Modes
// Modes are integer values (often) related to switches or settings

#define NUM_CHAN_MODES 8

enum ChannelModes{
	INF,
	REV,
	TIMEMODE_POT,
	TIMEMODE_JACK,
	LOOP_CLOCK_GATETRIG,
	MAIN_CLOCK_GATETRIG,
	WINDOWMODE_POT,
	WINDOWMODE_JACK
};

enum TimeModes {
	MOD_READWRITE_TIME_Q,
	MOD_READWRITE_TIME_NOQ,
	MOD_SAMPLE_RATE_NOQ,
	MOD_SAMPLE_RATE_Q,
	MOD_SR_INVRW
};

enum GateTrigModes{
	GATE_MODE,
	TRIG_MODE
};

enum WindowModes{
	NO_WINDOW,
	WINDOW
};


//Global Modes
#define NUM_GLOBAL_MODES 5

enum Global_Modes{
	AUTO_MUTE,
	SOFTCLIP,
	DCINPUT,
	CALIBRATE,
	SYSTEM_SETTINGS
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

inline void process_mode_flags(void);

float get_clk_div_nominal(uint16_t adc_val);
float get_clk_div_exact(uint16_t adc_val);
uint8_t get_switch_val(uint8_t channel);
float adjust_time_by_switch(float val, uint8_t channel);
void init_LowPassCoefs(void);
void init_params(void);
void init_modes(void);

#endif /* PARAMS_H_ */
