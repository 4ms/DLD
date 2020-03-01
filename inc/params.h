/*
 * params.c - take raw input data, smooth/debounce it, shift/scale,
 * and convert to params and modes
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

#ifndef PARAMS_H_
#define PARAMS_H_
#include <stm32f4xx.h>


//
// Params
// Params are float values related to pots and CV jacks
//

enum Params{
	TIME,			//TIME: fractional value for time multiplication, integer value for time division
	LEVEL,			//LEVEL: 0..1 representing amount of main input mixed into delay loop
	REGEN,			//REGEN: 0..1 representing amount of regeneration
	MIX_DRY,		//MIX: 0..1 representing mix of delayed and clean on the main output
	MIX_WET,
	TRACKING_COMP,	//TRACKING_COMP: -2.0 .. 2.0 representing compensation for 1V/oct tracking
	NUM_PARAMS
};

enum Pots{
	TIME_POT,
	LEVEL_POT,
	REGEN_POT,
	MIX_POT
};

//
// Channel Modes
// Modes are integer values (often) related to switches or settings
//

enum ChannelModes{
	INF,
	REV,
	TIMEMODE_POT,
	TIMEMODE_JACK,
	LOOP_CLOCK_GATETRIG,
	MAIN_CLOCK_GATETRIG,
	WINDOWMODE_POT,
	WINDOWMODE_JACK,
	LEVELCV_IS_MIX,
	PING_LOCKED,
	CONTINUOUS_REVERSE,
	SEND_RETURN_BEFORE_LOOP,
	NUM_CHAN_MODES
};

enum InfStates {
	INF_OFF,
	INF_ON,
	INF_TRANSITIONING_OFF,
	INF_TRANSITIONING_ON
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


//
//Global Modes
//Global Modes represent global states of functionality
//
enum Global_Modes{
	AUTO_MUTE,
	SOFTCLIP,
	DCINPUT,
	CALIBRATE,
	SYSTEM_SETTINGS,
	AUTO_UNQ,
	EXITINF_MODE,
	REV_GATETRIG,
	INF_GATETRIG,
	PING_METHOD,
	LOG_DELAY_FEED,
	RUNAWAYDC_BLOCK,
	QUANTIZE_MODE_CHANGES,
	NUM_GLOBAL_MODES
};

enum PingMethods{
	IGNORE_FLAT_DEVIATION_10,
	IGNORE_PERCENT_DEVIATION,
	ONE_TO_ONE,
	MOVING_AVERAGE_2,
	LINEAR_AVERAGE_4,
	EXPO_AVERAGE_8,
	IGNORE_FLAT_DEVIATION_5,
	MOVING_AVERAGE_4,
	EXPO_AVERAGE_4,
	LINEAR_AVERAGE_8,
	NUM_PING_METHODS
};

enum ExitINFModes{
	PLAYTHROUGH,
	IMMEDIATE_GLTICH,
	RESET_LOOPSTART
};

enum Global_Params{
	FAST_FADE_SAMPLES,
	SLOW_FADE_SAMPLES,
	FAST_FADE_INCREMENT,
	SLOW_FADE_INCREMENT,
	LOOP_LED_BRIGHTNESS,
	NUM_GLOBAL_PARAMS
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

void process_mode_flags(uint8_t channel);
void process_ping_changed(uint8_t channel);

float get_clk_div_nominal(uint16_t adc_val);
float get_clk_div_exact(uint16_t adc_val);
uint8_t get_switch_val(uint8_t channel);
float adjust_time_by_switch(float val, uint8_t channel);
void init_LowPassCoefs(void);
void init_params(void);
void init_modes(void);

float set_fade_increment(uint32_t samples);

#endif /* PARAMS_H_ */
