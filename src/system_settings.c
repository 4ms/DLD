/*
 * system_settings.c - interface for system mode settings (user settings)
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


#include "globals.h"
#include "system_settings.h"
#include "adc.h"
#include "params.h"
#include "flash_user.h"
#include "buttons_jacks.h"
#include "dig_pins.h"

extern int16_t i_smoothed_potadc[NUM_POT_ADCS];

extern float param[NUM_CHAN][NUM_PARAMS];
extern uint8_t mode[NUM_CHAN][NUM_CHAN_MODES];
extern uint8_t global_mode[NUM_GLOBAL_MODES];
extern float global_param[NUM_GLOBAL_PARAMS];


extern uint8_t loop_led_state[NUM_CHAN];

extern uint8_t flag_inf_change[2];
extern uint8_t flag_rev_change[2];
extern uint8_t flag_ignore_infdown[2];
extern uint8_t flag_ignore_revdown[2];


extern uint32_t flash_firmware_version;


uint8_t disable_mode_changes=0;

void set_default_system_settings(void)
{
	param[0][TRACKING_COMP]=1.0;
	param[1][TRACKING_COMP]=1.0;

	mode[0][LOOP_CLOCK_GATETRIG] = TRIG_MODE;
	mode[1][LOOP_CLOCK_GATETRIG] = TRIG_MODE;
	mode[0][MAIN_CLOCK_GATETRIG] = TRIG_MODE;

	mode[0][LEVELCV_IS_MIX] = 0;
	mode[1][LEVELCV_IS_MIX] = 0;

	global_mode[AUTO_MUTE] = 1;
	global_mode[SOFTCLIP] = 1;
	global_mode[INF_GATETRIG] = TRIG_MODE;
	global_mode[REV_GATETRIG] = TRIG_MODE;

	global_mode[RUNAWAYDC_BLOCK] = 1;
	global_mode[QUANTIZE_MODE_CHANGES] = 0;

	mode[0][LOOP_CLOCK_GATETRIG] = TRIG_MODE;
	mode[1][LOOP_CLOCK_GATETRIG] = TRIG_MODE;
	mode[0][MAIN_CLOCK_GATETRIG] = TRIG_MODE;

	mode[0][SEND_RETURN_BEFORE_LOOP] = 0;
	mode[1][SEND_RETURN_BEFORE_LOOP] = 0;


	global_mode[AUTO_UNQ] = 0;
	global_mode[EXITINF_MODE]=PLAYTHROUGH;

	global_mode[PING_METHOD] = IGNORE_FLAT_DEVIATION_10;

	global_mode[LOG_DELAY_FEED] = 0;

	global_param[FAST_FADE_SAMPLES] = 196;
	global_param[SLOW_FADE_SAMPLES]  = 196; //about 8ms
	global_param[FAST_FADE_INCREMENT] = set_fade_increment(global_param[FAST_FADE_SAMPLES]);
	global_param[SLOW_FADE_INCREMENT] = set_fade_increment(global_param[SLOW_FADE_SAMPLES]);
	global_param[LOOP_LED_BRIGHTNESS] = 4;

}


void check_entering_system_mode(void)
{

	static int32_t ctr=0;

	if (ENTER_SYSMODE_BUTTONS)
	{
		if (ctr==-1) //waiting for button release
		{
			LED_PINGBUT_ON;
			LED_REV1_ON;
			LED_REV2_ON;
			LED_LOOP1_ON;
			LED_LOOP2_ON;
			LED_INF1_ON;
			LED_INF2_ON;
		}
		else ctr+=40;

		if (ctr>100000)
		{
			if (global_mode[SYSTEM_SETTINGS] == 0)
			{
				global_mode[SYSTEM_SETTINGS] = 1;
				global_mode[CALIBRATE] = 0;
				ctr=-1;

				flag_rev_change[0] = 0;
				flag_rev_change[1] = 0;
				flag_inf_change[0] = 0;
				flag_inf_change[1] = 0;

				flag_ignore_infdown[0]=1;
				flag_ignore_infdown[1]=1;
				flag_ignore_revdown[0]=1;
				flag_ignore_revdown[1]=1;

			}
			else
			{
				save_flash_params();
				global_mode[SYSTEM_SETTINGS] = 0;
				ctr=-1;
				disable_mode_changes=0;

				flag_ignore_infdown[0]=1;
				flag_ignore_infdown[1]=1;
				flag_ignore_revdown[0]=1;
				flag_ignore_revdown[1]=1;

			}
		}
	}
	else
	{
		if ((ctr>1000) && (ctr<=100000) && (global_mode[SYSTEM_SETTINGS] == 1)) //released buttons too early ==> cancel (exit without save)
		{
			global_mode[SYSTEM_SETTINGS] = 0;
			disable_mode_changes=0;
			flag_ignore_infdown[0]=1;
			flag_ignore_infdown[1]=1;
			flag_ignore_revdown[0]=1;
			flag_ignore_revdown[1]=1;

		}
		ctr=0;
	}

}


void update_system_settings(void)
{
	uint8_t switch1, switch2;

	switch1=TIMESW_CH1;
	switch2=TIMESW_CH2;

	//
	// Switches: Up | Up
	// Entry point and Save settings mode
	//

	if (switch1==SWITCH_UP && switch2==SWITCH_UP)
	{
		disable_mode_changes=1;
	}

	//
	// Switches: Center | Center
	// Set Tracking Compensation (1V/oct) using LEVEL pots ("Delay Feed") while holding down INF buttons
	// Set Cross-fade time by holding down REV A button and turning Time A
	//
	if (switch1==SWITCH_CENTER && switch2==SWITCH_CENTER)
	{
		disable_mode_changes=1;

		if (INF1BUT)
			param[0][TRACKING_COMP]=((i_smoothed_potadc[LEVEL_POT*2]/8192.0f) + 0.75);

		if (INF2BUT)
			param[1][TRACKING_COMP]=((i_smoothed_potadc[LEVEL_POT*2+1]/8192.0f) + 0.75);

		mode[0][TIMEMODE_POT] = MOD_READWRITE_TIME_NOQ;
		mode[0][TIMEMODE_JACK] = MOD_READWRITE_TIME_NOQ;
		mode[1][TIMEMODE_POT] = MOD_READWRITE_TIME_NOQ;
		mode[1][TIMEMODE_JACK] = MOD_READWRITE_TIME_NOQ;

		param[0][LEVEL] = 1.0;
		param[1][LEVEL] = 1.0;

		if (REV1BUT)
		{
			//*_FADE_SAMPLES must equal (blocks - 1) * 4, where blocks is an integer
			//


			if (i_smoothed_potadc[TIME_POT*2] < 500) //1 - 2
			{
				global_param[FAST_FADE_SAMPLES] = 0;
				global_param[SLOW_FADE_SAMPLES]  = 0; //instant cut
			}
			else if (i_smoothed_potadc[TIME_POT*2] < 1664) //3 - 6
			{
				global_param[FAST_FADE_SAMPLES] = 196;
				global_param[SLOW_FADE_SAMPLES]  = 196; //about 8ms
			}
			else if (i_smoothed_potadc[TIME_POT*2] < 2719) //7 - 10
			{
				global_param[FAST_FADE_SAMPLES] = 196;
				global_param[SLOW_FADE_SAMPLES]  = 1596; //about 66ms
			}
			else if (i_smoothed_potadc[TIME_POT*2] < 3586) //11 - 13
			{
				global_param[FAST_FADE_SAMPLES] = 196;
				global_param[SLOW_FADE_SAMPLES]  = 4796; //about 200ms
			}
			else //14 - 16
			{
				global_param[FAST_FADE_SAMPLES] = 196;
				global_param[SLOW_FADE_SAMPLES]  = 23996; //about 0.5s
			}

			global_param[FAST_FADE_INCREMENT] = set_fade_increment(global_param[FAST_FADE_SAMPLES]);
			global_param[SLOW_FADE_INCREMENT] = set_fade_increment(global_param[SLOW_FADE_SAMPLES]);

		}

		if (REV2BUT)
		{

			if (i_smoothed_potadc[TIME_POT*2+1] < 500) //1 - 2
			{
				global_mode[PING_METHOD] = IGNORE_FLAT_DEVIATION_10;
			}
			else if (i_smoothed_potadc[TIME_POT*2+1] < 1664) //3 - 6
			{
				global_mode[PING_METHOD] = IGNORE_PERCENT_DEVIATION;
			}
			else if (i_smoothed_potadc[TIME_POT*2+1] < 2719) //7 - 10
			{
				global_mode[PING_METHOD] = ONE_TO_ONE;
			}
			else if (i_smoothed_potadc[TIME_POT*2+1] < 3586) //11 - 13
			{
				global_mode[PING_METHOD] = MOVING_AVERAGE_2;
			}
			else //14 - 16
			{
				global_mode[PING_METHOD] = LINEAR_AVERAGE_4;
			}


		}


	}

	//
	// Switches: Up | Down
	// Select Trig/Gate using INF A, REV A, and REV B buttons
	// Set LED brightness for loop LEDs using REGEN A pot while holding down INF B
	//
	if (switch1==SWITCH_UP && switch2==SWITCH_DOWN)
	{
		disable_mode_changes=1;

		if (INF2BUT)
			global_param[LOOP_LED_BRIGHTNESS] = ((i_smoothed_potadc[REGEN_POT*2]/137) + 1);

		if (flag_rev_change[0])
		{
			if (mode[0][LOOP_CLOCK_GATETRIG] == GATE_MODE)
				mode[0][LOOP_CLOCK_GATETRIG] = TRIG_MODE;
			else
				mode[0][LOOP_CLOCK_GATETRIG] = GATE_MODE;

			flag_rev_change[0]=0;
		}

		if (flag_rev_change[1])
		{
			if (mode[1][LOOP_CLOCK_GATETRIG] == GATE_MODE)
				mode[1][LOOP_CLOCK_GATETRIG] = TRIG_MODE;
			else
				mode[1][LOOP_CLOCK_GATETRIG] = GATE_MODE;

			flag_rev_change[1]=0;
		}

		if (flag_inf_change[0])
		{
			if (mode[0][MAIN_CLOCK_GATETRIG] == GATE_MODE)
				mode[0][MAIN_CLOCK_GATETRIG] = TRIG_MODE;
			else
				mode[0][MAIN_CLOCK_GATETRIG] = GATE_MODE;

			flag_inf_change[0]=0;
		}
	}

	//
	// Switches: Center | Down
	// Select Inf and Reverse toggled by a gate toggling, or a trigger
	//
	if (switch1==SWITCH_CENTER && switch2==SWITCH_DOWN)
	{
		disable_mode_changes=1;

		if (flag_rev_change[0])
		{
			if (global_mode[REV_GATETRIG] == TRIG_MODE)
				global_mode[REV_GATETRIG] = GATE_MODE;
			else
				global_mode[REV_GATETRIG] = TRIG_MODE;

			flag_rev_change[0]=0;
		}

		if (flag_inf_change[0])
		{
			if (global_mode[INF_GATETRIG] == TRIG_MODE)
				global_mode[INF_GATETRIG] = GATE_MODE;
			else
				global_mode[INF_GATETRIG] = TRIG_MODE;

			flag_inf_change[0]=0;
		}

		if (flag_inf_change[1])
		{
			if (global_mode[LOG_DELAY_FEED] == 1)
				global_mode[LOG_DELAY_FEED] = 0;
			else
				global_mode[LOG_DELAY_FEED] = 1;

			flag_inf_change[1]=0;
		}

		if (flag_rev_change[1])
		{
			if (global_mode[RUNAWAYDC_BLOCK] == 1)
				global_mode[RUNAWAYDC_BLOCK] = 0;
			else
				global_mode[RUNAWAYDC_BLOCK] = 1;

			flag_rev_change[1]=0;

		}


	}
	//
	// Switches: Up | Center
	// Set Auto-Mute, Soft Clipping, and Level CV jack to Mix
	//

	if (switch1==SWITCH_UP && switch2==SWITCH_CENTER)
	{
		disable_mode_changes=1;

		if (flag_rev_change[0])
		{
			if (global_mode[AUTO_MUTE])
				global_mode[AUTO_MUTE] = 0;
			else
				global_mode[AUTO_MUTE] = 1;

			flag_rev_change[0]=0;
		}

		if (flag_rev_change[1])
		{
			if (global_mode[SOFTCLIP])
				global_mode[SOFTCLIP] = 0;
			else
				global_mode[SOFTCLIP] = 1;

			flag_rev_change[1]=0;
		}

		if (flag_inf_change[0])
		{
			if (mode[0][LEVELCV_IS_MIX])
				mode[0][LEVELCV_IS_MIX] = 0;
			else
				mode[0][LEVELCV_IS_MIX] = 1;

			flag_inf_change[0]=0;
		}

		if (flag_inf_change[1])
		{
			if (mode[1][LEVELCV_IS_MIX])
				mode[1][LEVELCV_IS_MIX] = 0;
			else
				mode[1][LEVELCV_IS_MIX] = 1;

			flag_inf_change[1]=0;
		}
	}

	//
	// Switches: Center | Up
	// Set Send/Return Before Loop
	//

	if (switch1==SWITCH_CENTER && switch2==SWITCH_UP)
	{
		disable_mode_changes=1;

		if (flag_rev_change[0])
		{
			if (mode[0][SEND_RETURN_BEFORE_LOOP])
				mode[0][SEND_RETURN_BEFORE_LOOP] = 0;
			else
				mode[0][SEND_RETURN_BEFORE_LOOP] = 1;

			flag_rev_change[0]=0;
		}

		if (flag_rev_change[1])
		{
			if (mode[1][SEND_RETURN_BEFORE_LOOP])
				mode[1][SEND_RETURN_BEFORE_LOOP] = 0;
			else
				mode[1][SEND_RETURN_BEFORE_LOOP] = 1;

			flag_rev_change[1]=0;
		}
	}
}

void update_system_settings_button_leds(void)
{
	uint8_t switch1, switch2;
	static uint32_t led_flasher=0;

	switch1=TIMESW_CH1;
	switch2=TIMESW_CH2;

	if (global_mode[SYSTEM_SETTINGS] && switch1==SWITCH_UP && switch2==SWITCH_UP)
	{
		//Display firmware version

			if (flash_firmware_version & 0b0001)
				LED_REV1_ON;
			else
				LED_REV1_OFF;

			if (flash_firmware_version & 0b0010)
				LED_INF1_ON;
			else
				LED_INF1_OFF;

			if (flash_firmware_version & 0b0100)
				LED_INF2_ON;
			else
				LED_INF2_OFF;

			if (flash_firmware_version & 0b1000)
				LED_REV2_ON;
			else
				LED_REV2_OFF;
	}

	else if (global_mode[SYSTEM_SETTINGS] && switch1==SWITCH_UP && switch2==SWITCH_DOWN)
	{
		//Display Trig/Gate settings

			if (mode[0][LOOP_CLOCK_GATETRIG] == GATE_MODE)
				LED_REV1_ON;
			else
				LED_REV1_OFF;

			if (mode[0][MAIN_CLOCK_GATETRIG] == GATE_MODE)
				LED_INF1_ON;
			else
				LED_INF1_OFF;

			if (mode[1][LOOP_CLOCK_GATETRIG] == GATE_MODE)
				LED_REV2_ON;
			else
				LED_REV2_OFF;

			LED_INF2_OFF;
	}

	else if (global_mode[SYSTEM_SETTINGS] && switch1==SWITCH_UP && switch2==SWITCH_CENTER)
	{
		// Display Auto-Mute and Soft Clip settings

		if (global_mode[AUTO_MUTE])
			LED_REV1_ON;
		else
			LED_REV1_OFF;

		if (global_mode[SOFTCLIP])
			LED_REV2_ON;
		else
			LED_REV2_OFF;

		if (mode[0][LEVELCV_IS_MIX])
			LED_INF1_ON;
		else
			LED_INF1_OFF;

		if (mode[1][LEVELCV_IS_MIX])
			LED_INF2_ON;
		else
			LED_INF2_OFF;

	}
	else if (global_mode[SYSTEM_SETTINGS] && switch1==SWITCH_CENTER && switch2==SWITCH_CENTER)
	{
		// Blink INF buttons to indicate Tracking adjust mode

		led_flasher+=1280000;
		if (led_flasher<UINT32_MAX/20)
		{
			LED_INF1_ON;
			LED_INF2_ON;
			LED_REV1_ON;
			LED_REV2_ON;


		} else
		{
			LED_INF1_OFF;
			LED_INF2_OFF;
			LED_REV1_OFF;
			LED_REV2_OFF;

		}

	}
	else if (global_mode[SYSTEM_SETTINGS] && switch1==SWITCH_CENTER && switch2==SWITCH_DOWN)
	{
		if (global_mode[REV_GATETRIG] == TRIG_MODE)
			LED_REV1_OFF;
		else
			LED_REV1_ON;


		if (global_mode[INF_GATETRIG] == TRIG_MODE)
			LED_INF1_OFF;
		else
			LED_INF1_ON;

		if (global_mode[LOG_DELAY_FEED] == 1)
			LED_INF2_ON;
		else
			LED_INF2_OFF;

		if (global_mode[RUNAWAYDC_BLOCK] == 1)
			LED_REV2_ON;
		else
			LED_REV2_OFF;

	}
	else if (global_mode[SYSTEM_SETTINGS] && switch1==SWITCH_CENTER && switch2==SWITCH_UP)
	{
		// Display Send/Return Before Loop settings

		if (mode[0][SEND_RETURN_BEFORE_LOOP])
			LED_REV1_ON;
		else
			LED_REV1_OFF;

		if (mode[1][SEND_RETURN_BEFORE_LOOP])
			LED_REV2_ON;
		else
			LED_REV2_OFF;

		// INF LEDs are not used in this menu
		LED_INF1_OFF;
		LED_INF2_OFF;
	}
	else if (global_mode[SYSTEM_SETTINGS])
	{
		LED_REV1_OFF;
		LED_REV2_OFF;
		LED_INF1_OFF;
		LED_INF2_OFF;

	}

}

//100 * 0.2ms = 20ms
#define TRIG_CNTS 200
//1665 * 0.2ms = 333ms
#define GATE_CNTS 1665

void update_system_settings_leds(void)
{
	static uint32_t led_flasher=0;
	uint8_t switch1, switch2;
	static float old_slow_fade;
	static uint8_t old_ping_method;
	static uint32_t pulse_ctr[2]={100,100}, flashes_ctr[2];

	switch1=TIMESW_CH1;
	switch2=TIMESW_CH2;

	if (switch1==SWITCH_UP && switch2==SWITCH_DOWN)
	{
		led_flasher+=8;

		if (led_flasher < TRIG_CNTS)
		{
			loop_led_state[0]=1;
			LED_PINGBUT_OFF;
			loop_led_state[1]=0;
		}
		else if (led_flasher < GATE_CNTS)
		{
			if (mode[0][LOOP_CLOCK_GATETRIG]==TRIG_MODE)
				loop_led_state[0]=0;
		}
		else if (led_flasher < (GATE_CNTS + TRIG_CNTS))
		{
			loop_led_state[0]=0;
			LED_PINGBUT_ON;
			loop_led_state[1]=0;
		}
		else if (led_flasher < (GATE_CNTS*2))
		{
			if (mode[0][MAIN_CLOCK_GATETRIG]==TRIG_MODE)
				LED_PINGBUT_OFF;
		}
		else if (led_flasher < ((GATE_CNTS*2) + TRIG_CNTS))
		{
			loop_led_state[0]=0;
			LED_PINGBUT_OFF;
			loop_led_state[1]=1;
		}
		else if (led_flasher < (GATE_CNTS*3))
		{
			if (mode[1][LOOP_CLOCK_GATETRIG]==TRIG_MODE)
				loop_led_state[1]=0;
		}
		else
			led_flasher=0;
	}
	else if (switch1==SWITCH_CENTER && switch2==SWITCH_CENTER)
	{

		if (old_slow_fade != global_param[SLOW_FADE_SAMPLES])
		{
			old_slow_fade = global_param[SLOW_FADE_SAMPLES];
			//force reset
			loop_led_state[0] = 1;
			pulse_ctr[0] = 0;
			flashes_ctr[0] = 0;
		}

		if (pulse_ctr[0])
			pulse_ctr[0]--;
		else
		{
			loop_led_state[0]=1-loop_led_state[0];

			if (flashes_ctr[0])
			{
				pulse_ctr[0] = 250;
				flashes_ctr[0]--;
			}
			else
			{
				loop_led_state[0] = 0;
				pulse_ctr[0] = 1000;
				if (global_param[SLOW_FADE_SAMPLES]==0) flashes_ctr[0] = 1;
				else if (global_param[SLOW_FADE_SAMPLES]==196) flashes_ctr[0] = 3;
				else if (global_param[SLOW_FADE_SAMPLES]==1596) flashes_ctr[0] = 5;
				else if (global_param[SLOW_FADE_SAMPLES]==4796) flashes_ctr[0] = 7;
				else if (global_param[SLOW_FADE_SAMPLES]==23996) flashes_ctr[0] = 9;
			}
		}


		if (old_ping_method != global_mode[PING_METHOD])
			{
				old_ping_method = global_mode[PING_METHOD];
				//force reset
				loop_led_state[1] = 1;
				pulse_ctr[1] = 0;
				flashes_ctr[1] = 0;
			}

			if (pulse_ctr[1])
				pulse_ctr[1]--;
			else
			{
				loop_led_state[1]=1-loop_led_state[1];

				if (flashes_ctr[1])
				{
					pulse_ctr[1] = 250;
					flashes_ctr[1]--;
				}
				else
				{
					pulse_ctr[1] = 1000;
					flashes_ctr[1] = global_mode[PING_METHOD]*2 + 1;
				}
			}


		LED_PINGBUT_OFF;

	}
	else
	{
		loop_led_state[0]=0;
		loop_led_state[1]=0;
		LED_PINGBUT_OFF;

	}

}
