/*
 * calibration.c - Calibration mode
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

#include "dig_pins.h"
#include "buttons_jacks.h"
#include "globals.h"
#include "calibration.h"
#include "adc.h"
#include "params.h"
#include "flash_user.h"

int16_t CV_CALIBRATION_OFFSET[NUM_CV_ADCS];
int16_t CODEC_DAC_CALIBRATION_DCOFFSET[4];
int16_t CODEC_ADC_CALIBRATION_DCOFFSET[4];


extern int16_t i_smoothed_potadc[NUM_POT_ADCS];
extern int16_t i_smoothed_rawcvadc[NUM_CV_ADCS];

extern volatile int16_t ch1rx_buffer[codec_BUFF_LEN];
extern volatile int16_t ch2rx_buffer[codec_BUFF_LEN];

extern uint8_t global_mode[NUM_GLOBAL_MODES];

extern uint8_t loop_led_state[NUM_CHAN];


void set_default_calibration_values(void)
{

	CODEC_DAC_CALIBRATION_DCOFFSET[0]=-1746;
	CODEC_DAC_CALIBRATION_DCOFFSET[1]=-1746;
	CODEC_DAC_CALIBRATION_DCOFFSET[2]=-1746;
	CODEC_DAC_CALIBRATION_DCOFFSET[3]=-1746;

	CODEC_ADC_CALIBRATION_DCOFFSET[0]=0;
	CODEC_ADC_CALIBRATION_DCOFFSET[1]=0;
	CODEC_ADC_CALIBRATION_DCOFFSET[2]=0;
	CODEC_ADC_CALIBRATION_DCOFFSET[3]=0;

	CV_CALIBRATION_OFFSET[0] = 0;
	CV_CALIBRATION_OFFSET[1] = 0;
	CV_CALIBRATION_OFFSET[2] = 0;
	CV_CALIBRATION_OFFSET[3] = 0;
	CV_CALIBRATION_OFFSET[4] = 0;
	CV_CALIBRATION_OFFSET[5] = 0;
}


void auto_calibrate(void)
{
	set_default_calibration_values();

	//delay_ms(7000);
	delay();
	delay();
	delay();
	delay();

	CV_CALIBRATION_OFFSET[0] = 2048-(i_smoothed_rawcvadc[0] & 0x0FFF);
	CV_CALIBRATION_OFFSET[1] = 2048-(i_smoothed_rawcvadc[1] & 0x0FFF);
	CV_CALIBRATION_OFFSET[2] = -1*(i_smoothed_rawcvadc[2] & 0x0FFF);
	CV_CALIBRATION_OFFSET[3] = -1*(i_smoothed_rawcvadc[3] & 0x0FFF);
	CV_CALIBRATION_OFFSET[4] = -1*(i_smoothed_rawcvadc[4] & 0x0FFF);
	CV_CALIBRATION_OFFSET[5] = -1*(i_smoothed_rawcvadc[5] & 0x0FFF);

}


void update_calibration(void)
{
	uint8_t switch1, switch2;
	static uint32_t buttons_down=0;
	static uint32_t factory_reset_buttons_down=0;

	CV_CALIBRATION_OFFSET[0] = 2048-(i_smoothed_rawcvadc[0] & 0x0FFF);
	CV_CALIBRATION_OFFSET[1] = 2048-(i_smoothed_rawcvadc[1] & 0x0FFF);
	CV_CALIBRATION_OFFSET[2] = -1*(i_smoothed_rawcvadc[2] & 0x0FFF);
	CV_CALIBRATION_OFFSET[3] = -1*(i_smoothed_rawcvadc[3] & 0x0FFF);
	CV_CALIBRATION_OFFSET[4] = -1*(i_smoothed_rawcvadc[4] & 0x0FFF);
	CV_CALIBRATION_OFFSET[5] = -1*(i_smoothed_rawcvadc[5] & 0x0FFF);

	switch1=TIMESW_CH1;
	switch2=TIMESW_CH2;

	if (switch1==SWITCH_UP && switch2==SWITCH_UP)//both up: calibrate audio output offset, using the knobs to set DC level. Enable by holding down button
	{

		if (REV1BUT)
			CODEC_DAC_CALIBRATION_DCOFFSET[0]=(i_smoothed_potadc[LEVEL_POT*2]-2048-1750); //OUT A
		if (REV2BUT)
			CODEC_DAC_CALIBRATION_DCOFFSET[1]=(i_smoothed_potadc[LEVEL_POT*2+1]-2048-1750); //OUT B
		if (INF1BUT)
			CODEC_DAC_CALIBRATION_DCOFFSET[2]=(i_smoothed_potadc[MIX_POT*2]-2048-1750); //SEND A
		if (INF2BUT)
			CODEC_DAC_CALIBRATION_DCOFFSET[3]=(i_smoothed_potadc[MIX_POT*2+1]-2048-1750); //SEND B

	}

	if (switch1==SWITCH_DOWN && switch2==SWITCH_DOWN)//both down: calibrate audio input DC level
	{
		CODEC_ADC_CALIBRATION_DCOFFSET[0]= -1*ch1rx_buffer[0];
		CODEC_ADC_CALIBRATION_DCOFFSET[1]= -1*ch2rx_buffer[0];
		CODEC_ADC_CALIBRATION_DCOFFSET[2]= -1*ch1rx_buffer[2];
		CODEC_ADC_CALIBRATION_DCOFFSET[3]= -1*ch2rx_buffer[2];
	}

	if (SAVE_CALIBRATE_BUTTONS)
	{
		buttons_down++;
		if (buttons_down==3000)
		{
			save_flash_params();
			global_mode[CALIBRATE] = 0;

		}
	} else
		buttons_down=0;

	if (FACTORY_RESET_BUTTONS)
	{
		factory_reset_buttons_down++;
		if (factory_reset_buttons_down==10000)
		{
			factory_reset(1);
			global_mode[CALIBRATE] = 0;
		}
	} else
		factory_reset_buttons_down=0;

}

#ifndef UINT32_MAX
#define UINT32_MAX 4294967295U
#endif

void update_calibrate_leds(void)
{
	static uint32_t led_flasher=0;

	led_flasher+=UINT32_MAX/1665;
	loop_led_state[(led_flasher < UINT32_MAX/2)?1:0] = 0;
	loop_led_state[(led_flasher < UINT32_MAX/2)?0:1] = 1;

}

void update_calibration_button_leds(void)
{
	static uint32_t led_flasher=0;

	//Flash button LEDs to indicate we're in Calibrate mode
	led_flasher+=1280000;
	if (led_flasher<UINT32_MAX/20)
	{
		LED_INF1_OFF;
		LED_INF2_OFF;
		LED_REV1_OFF;
		LED_REV2_OFF;
	}
	else
	{
		LED_INF1_ON;
		LED_INF2_ON;
		LED_REV1_ON;
		LED_REV2_ON;
	}

}

