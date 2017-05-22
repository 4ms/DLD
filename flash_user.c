/*
 * flash_user.c - loading and saving user settings to/from non-volatile Flash memory
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
#include "flash.h"
#include "flash_user.h"
#include "calibration.h"
#include "system_settings.h"
#include "params.h"
#include "adc.h"
#include "leds.h"
#include "dig_pins.h"

extern int16_t CV_CALIBRATION_OFFSET[6];
extern int16_t CODEC_DAC_CALIBRATION_DCOFFSET[4];
extern int16_t CODEC_ADC_CALIBRATION_DCOFFSET[4];


extern float param[NUM_CHAN][NUM_PARAMS];
extern uint8_t mode[NUM_CHAN][NUM_CHAN_MODES];
extern uint8_t global_mode[NUM_GLOBAL_MODES];
extern float global_param[NUM_GLOBAL_PARAMS];

extern int16_t i_smoothed_cvadc[NUM_POT_ADCS];


#define FLASH_ADDR_userparams 0x08004000
//Globals:
#define FLASH_ADDR_firmware_version			FLASH_ADDR_userparams									/* 0  ..3	*/
#define SZ_FV 4

#define FLASH_ADDR_cv_calibration_offset	(FLASH_ADDR_firmware_version			+ SZ_FV)		/* 4  ..27	*/
#define SZ_CVCAL 24

#define FLASH_ADDR_adc_calibration_dcoffset	(FLASH_ADDR_cv_calibration_offset 		+ SZ_CVCAL)		/* 28  ..43	*/
#define SZ_ADCCAL 16

#define FLASH_ADDR_dac_calibration_dcoffset	(FLASH_ADDR_adc_calibration_dcoffset 	+ SZ_ADCCAL)	/* 44  ..59	*/
#define SZ_DACCAL 16

#define FLASH_ADDR_TRACKING_COMP_0			(FLASH_ADDR_dac_calibration_dcoffset 	+ SZ_DACCAL)	/* 60  ..63	*/
#define SZ_TC0 4

#define FLASH_ADDR_TRACKING_COMP_1			(FLASH_ADDR_TRACKING_COMP_0 			+ SZ_TC0)		/* 64  ..67	*/
#define SZ_TC1 4

#define FLASH_ADDR_loop_led_brightness		(FLASH_ADDR_TRACKING_COMP_1 			+ SZ_TC1)		/* 68  ..	*/
#define SZ_LLB 1

#define FLASH_ADDR_LOOP_CLOCK_GATETRIG_0	(FLASH_ADDR_loop_led_brightness 		+ SZ_LLB)		/* 69  ..	*/
#define SZ_LC0 1

#define FLASH_ADDR_LOOP_CLOCK_GATETRIG_1	(FLASH_ADDR_LOOP_CLOCK_GATETRIG_0 		+ SZ_LC0)		/* 70  ..	*/
#define SZ_LC1 1

#define FLASH_ADDR_MAIN_CLOCK_GATETRIG		(FLASH_ADDR_LOOP_CLOCK_GATETRIG_1 		+ SZ_LC1)		/* 71  ..	*/
#define SZ_MC  1

#define FLASH_ADDR_AUTO_MUTE				(FLASH_ADDR_MAIN_CLOCK_GATETRIG 		+ SZ_MC)		/* 72  ..	*/
#define SZ_AM 1

#define FLASH_ADDR_SOFTCLIP					(FLASH_ADDR_AUTO_MUTE			 		+ SZ_AM)		/* 73  ..	*/
#define SZ_SC 1

#define FLASH_ADDR_LEVELCV_IS_MIX_0			(FLASH_ADDR_SOFTCLIP			 		+ SZ_SC)		/* 74  ..	*/
#define SZ_LM0 1

#define FLASH_ADDR_LEVELCV_IS_MIX_1			(FLASH_ADDR_LEVELCV_IS_MIX_0	 		+ SZ_LM0)		/* 75  ..	*/
#define SZ_LM1 1

#define FLASH_ADDR_FAST_FADE_SAMPLES		(FLASH_ADDR_LEVELCV_IS_MIX_1	 		+ SZ_LM1)		/* 76  ..79	*/
#define SZ_FFS 4

#define FLASH_ADDR_SLOW_FADE_SAMPLES		(FLASH_ADDR_FAST_FADE_SAMPLES	 		+ SZ_FFS)		/* 80  ..83	*/
#define SZ_SFS 4

#define FLASH_ADDR_REV_GATETRIG				(FLASH_ADDR_SLOW_FADE_SAMPLES			+ SZ_SFS)		/* 84 ..	*/
#define SZ_RGT 1

#define FLASH_ADDR_INF_GATETRIG				(FLASH_ADDR_REV_GATETRIG				+ SZ_RGT)		/* 85 ..	*/
#define SZ_IGT 1

#define FLASH_ADDR_PING_METHOD				(FLASH_ADDR_INF_GATETRIG				+ SZ_IGT)		/* 86 ..	*/
#define SZ_PM 1

#define FLASH_ADDR_LOG_DELAY_FEED				(FLASH_ADDR_PING_METHOD				+ SZ_PM)		/* 87 ..	*/
#define SZ_LDF 1

#define FLASH_ADDR_RUNAWAYDC_BLOCK				(FLASH_ADDR_LOG_DELAY_FEED			+ SZ_LDF)		/* 88 ..	*/
#define SZ_RDCB 1

#define FLASH_ADDR_QUANTIZE_MODE_CHANGES				(FLASH_ADDR_RUNAWAYDC_BLOCK		+ SZ_RDCB)		/* 89 ..	*/
#define SZ_QCM 1

#define FLASH_ADDR_SEND_RETURN_BEFORE_LOOP_0				(FLASH_ADDR_QUANTIZE_MODE_CHANGES		+ SZ_QCM)		/* 90 ..	*/
#define SZ_SRBL0 1

#define FLASH_ADDR_SEND_RETURN_BEFORE_LOOP_1				(FLASH_ADDR_SEND_RETURN_BEFORE_LOOP_0		+ SZ_SRBL0)		/* 91 ..	*/
#define SZ_SRBL1 1


#define FLASH_SYMBOL_bankfilled 0x01
#define FLASH_SYMBOL_startupoffset 0xAA
#define FLASH_SYMBOL_firmwareoffset 0xAA550000

uint32_t flash_firmware_version=0;

int32_t flash_CV_CALIBRATION_OFFSET[6];
int32_t flash_CODEC_DAC_CALIBRATION_DCOFFSET[4];
int32_t flash_CODEC_ADC_CALIBRATION_DCOFFSET[4];

float flash_param_TRACKING_COMP_0;
float flash_param_TRACKING_COMP_1;

uint32_t flash_global_param_FAST_FADE_SAMPLES;
uint32_t flash_global_param_SLOW_FADE_SAMPLES;

uint8_t flash_loop_led_brightness;

uint8_t flash_mode_LOOP_CLOCK_GATETRIG_0;
uint8_t flash_mode_LOOP_CLOCK_GATETRIG_1;
uint8_t flash_mode_MAIN_CLOCK_GATETRIG;

uint8_t flash_global_mode_AUTO_MUTE;
uint8_t flash_global_mode_SOFTCLIP;

uint8_t flash_global_mode_REV_GATETRIG;
uint8_t flash_global_mode_INF_GATETRIG;

uint8_t flash_mode_LEVELCV_IS_MIX_0;
uint8_t flash_mode_LEVELCV_IS_MIX_1;

uint8_t flash_global_mode_PING_METHOD;
uint8_t flash_global_mode_LOG_DELAY_FEED;
uint8_t flash_global_mode_RUNAWAYDC_BLOCK;
uint8_t flash_global_mode_QUANTIZE_MODE_CHANGES;

uint8_t flash_mode_SEND_RETURN_BEFORE_LOOP_0;
uint8_t flash_mode_SEND_RETURN_BEFORE_LOOP_1;


void set_firmware_version(void)
{
	flash_firmware_version = FW_VERSION;
}


void factory_reset(uint8_t loop_afterwards)
{
	uint8_t i,fail=0;

	auto_calibrate();
	set_firmware_version();
	set_default_system_settings();

	LED_PINGBUT_OFF;
	LED_REV1_ON;
	LED_REV2_ON;
	LED_LOOP1_OFF;
	LED_LOOP2_OFF;
	LED_INF1_ON;
	LED_INF2_ON;

	store_params_into_sram();
	write_all_params_to_FLASH();

	LED_PINGBUT_OFF;
	LED_REV1_OFF;
	LED_REV2_OFF;
	LED_LOOP1_OFF;
	LED_LOOP2_OFF;
	LED_INF1_OFF;
	LED_INF2_OFF;

	if (loop_afterwards)
	{
		fail=0;
		for (i=0;i<6;i++)
		{
			if (CV_CALIBRATION_OFFSET[i]>100 || CV_CALIBRATION_OFFSET[i]<-100 )
				fail=1;
		}

		if (i_smoothed_cvadc[0] > 2150 || i_smoothed_cvadc[0]<1950 || i_smoothed_cvadc[1] > 2150 || i_smoothed_cvadc[1]<1950)
			fail=1;

		while(1)
		{
			if (fail)
				blink_all_lights(200); //error: did not auto-calibrate!
			else
				chase_all_lights(10); //All good!
		}
	}
}

uint32_t load_flash_params(void)
{

	read_all_params_from_FLASH();

	if (flash_firmware_version > 0 && flash_firmware_version < 500){

		CODEC_ADC_CALIBRATION_DCOFFSET[0] = flash_CODEC_ADC_CALIBRATION_DCOFFSET[0];
		CODEC_ADC_CALIBRATION_DCOFFSET[1] = flash_CODEC_ADC_CALIBRATION_DCOFFSET[1];
		CODEC_ADC_CALIBRATION_DCOFFSET[2] = flash_CODEC_ADC_CALIBRATION_DCOFFSET[2];
		CODEC_ADC_CALIBRATION_DCOFFSET[3] = flash_CODEC_ADC_CALIBRATION_DCOFFSET[3];

		CODEC_DAC_CALIBRATION_DCOFFSET[0] = flash_CODEC_DAC_CALIBRATION_DCOFFSET[0];
		CODEC_DAC_CALIBRATION_DCOFFSET[1] = flash_CODEC_DAC_CALIBRATION_DCOFFSET[1];
		CODEC_DAC_CALIBRATION_DCOFFSET[2] = flash_CODEC_DAC_CALIBRATION_DCOFFSET[2];
		CODEC_DAC_CALIBRATION_DCOFFSET[3] = flash_CODEC_DAC_CALIBRATION_DCOFFSET[3];

		CV_CALIBRATION_OFFSET[0] = flash_CV_CALIBRATION_OFFSET[0];
		CV_CALIBRATION_OFFSET[1] = flash_CV_CALIBRATION_OFFSET[1];
		CV_CALIBRATION_OFFSET[2] = flash_CV_CALIBRATION_OFFSET[2];
		CV_CALIBRATION_OFFSET[3] = flash_CV_CALIBRATION_OFFSET[3];
		CV_CALIBRATION_OFFSET[4] = flash_CV_CALIBRATION_OFFSET[4];
		CV_CALIBRATION_OFFSET[5] = flash_CV_CALIBRATION_OFFSET[5];

		param[0][TRACKING_COMP] = flash_param_TRACKING_COMP_0;
		param[1][TRACKING_COMP] = flash_param_TRACKING_COMP_1;

		global_param[LOOP_LED_BRIGHTNESS] = flash_loop_led_brightness;

		mode[0][LOOP_CLOCK_GATETRIG] = flash_mode_LOOP_CLOCK_GATETRIG_0;
		mode[1][LOOP_CLOCK_GATETRIG] = flash_mode_LOOP_CLOCK_GATETRIG_1;
		mode[0][MAIN_CLOCK_GATETRIG] = flash_mode_MAIN_CLOCK_GATETRIG;

		global_mode[AUTO_MUTE] = flash_global_mode_AUTO_MUTE;
		global_mode[SOFTCLIP] = flash_global_mode_SOFTCLIP;

		global_mode[REV_GATETRIG] = flash_global_mode_REV_GATETRIG;
		global_mode[INF_GATETRIG] = flash_global_mode_INF_GATETRIG;

		global_mode[PING_METHOD] = flash_global_mode_PING_METHOD;
		global_mode[LOG_DELAY_FEED] = flash_global_mode_LOG_DELAY_FEED;
		global_mode[RUNAWAYDC_BLOCK] = flash_global_mode_RUNAWAYDC_BLOCK;
		global_mode[QUANTIZE_MODE_CHANGES] = flash_global_mode_QUANTIZE_MODE_CHANGES;


		mode[0][LEVELCV_IS_MIX] = (flash_mode_LEVELCV_IS_MIX_0==1) ? 1:0;
		mode[1][LEVELCV_IS_MIX] = (flash_mode_LEVELCV_IS_MIX_1==1) ? 1:0;

		global_param[FAST_FADE_SAMPLES] = (float)flash_global_param_FAST_FADE_SAMPLES;
		global_param[SLOW_FADE_SAMPLES] = (float)flash_global_param_SLOW_FADE_SAMPLES;
		global_param[FAST_FADE_INCREMENT] = set_fade_increment(flash_global_param_FAST_FADE_SAMPLES);
		global_param[SLOW_FADE_INCREMENT] = set_fade_increment(flash_global_param_SLOW_FADE_SAMPLES);

		mode[0][SEND_RETURN_BEFORE_LOOP] = (flash_mode_SEND_RETURN_BEFORE_LOOP_0==1) ? 1:0;
		mode[1][SEND_RETURN_BEFORE_LOOP] = (flash_mode_SEND_RETURN_BEFORE_LOOP_1==1) ? 1:0;

		return (flash_firmware_version);

	} else
	{
    	set_firmware_version();

		return(0); //No calibration data found
	}
}


void save_flash_params(void)
{
	uint32_t i;

	LED_REV1_ON;
	LED_REV2_ON;
	LED_LOOP1_ON;
	LED_LOOP2_ON;
	LED_INF1_ON;
	LED_INF2_ON;

	//copy global variables to flash_* variables (SRAM staging area)
	store_params_into_sram();

	//copy SRAM variables to FLASH
	write_all_params_to_FLASH();

	for (i=0;i<10;i++){
		LED_PINGBUT_OFF;
		LED_REV1_OFF;
		LED_REV2_OFF;
		LED_LOOP1_OFF;
		LED_LOOP2_OFF;
		LED_INF1_OFF;
		LED_INF2_OFF;
		delay_ms(10);

		LED_PINGBUT_ON;
		LED_REV1_ON;
		LED_REV2_ON;
		LED_LOOP1_ON;
		LED_LOOP2_ON;
		LED_INF1_ON;
		LED_INF2_ON;
		delay_ms(10);
	}
}


void store_params_into_sram(void)
{
	flash_CV_CALIBRATION_OFFSET[0]=CV_CALIBRATION_OFFSET[0];
	flash_CV_CALIBRATION_OFFSET[1]=CV_CALIBRATION_OFFSET[1];
	flash_CV_CALIBRATION_OFFSET[2]=CV_CALIBRATION_OFFSET[2];
	flash_CV_CALIBRATION_OFFSET[3]=CV_CALIBRATION_OFFSET[3];
	flash_CV_CALIBRATION_OFFSET[4]=CV_CALIBRATION_OFFSET[4];
	flash_CV_CALIBRATION_OFFSET[5]=CV_CALIBRATION_OFFSET[5];

	flash_CODEC_DAC_CALIBRATION_DCOFFSET[0]=CODEC_DAC_CALIBRATION_DCOFFSET[0];
	flash_CODEC_DAC_CALIBRATION_DCOFFSET[1]=CODEC_DAC_CALIBRATION_DCOFFSET[1];
	flash_CODEC_DAC_CALIBRATION_DCOFFSET[2]=CODEC_DAC_CALIBRATION_DCOFFSET[2];
	flash_CODEC_DAC_CALIBRATION_DCOFFSET[3]=CODEC_DAC_CALIBRATION_DCOFFSET[3];


	flash_CODEC_ADC_CALIBRATION_DCOFFSET[0]=CODEC_ADC_CALIBRATION_DCOFFSET[0];
	flash_CODEC_ADC_CALIBRATION_DCOFFSET[1]=CODEC_ADC_CALIBRATION_DCOFFSET[1];
	flash_CODEC_ADC_CALIBRATION_DCOFFSET[2]=CODEC_ADC_CALIBRATION_DCOFFSET[2];
	flash_CODEC_ADC_CALIBRATION_DCOFFSET[3]=CODEC_ADC_CALIBRATION_DCOFFSET[3];

	flash_param_TRACKING_COMP_0 = param[0][TRACKING_COMP];
	flash_param_TRACKING_COMP_1 = param[1][TRACKING_COMP];

	flash_global_param_FAST_FADE_SAMPLES = global_param[FAST_FADE_SAMPLES];
	flash_global_param_SLOW_FADE_SAMPLES = global_param[SLOW_FADE_SAMPLES];

	flash_loop_led_brightness = global_param[LOOP_LED_BRIGHTNESS];

	flash_mode_LOOP_CLOCK_GATETRIG_0 = mode[0][LOOP_CLOCK_GATETRIG];
	flash_mode_LOOP_CLOCK_GATETRIG_1 = mode[1][LOOP_CLOCK_GATETRIG];
	flash_mode_MAIN_CLOCK_GATETRIG = mode[0][MAIN_CLOCK_GATETRIG];

	flash_global_mode_AUTO_MUTE = global_mode[AUTO_MUTE];
	flash_global_mode_SOFTCLIP = global_mode[SOFTCLIP];

	flash_global_mode_REV_GATETRIG = global_mode[REV_GATETRIG];
	flash_global_mode_INF_GATETRIG = global_mode[INF_GATETRIG];

	flash_global_mode_PING_METHOD = global_mode[PING_METHOD];
	flash_global_mode_LOG_DELAY_FEED = global_mode[LOG_DELAY_FEED];

	flash_global_mode_RUNAWAYDC_BLOCK = global_mode[RUNAWAYDC_BLOCK];
	flash_global_mode_QUANTIZE_MODE_CHANGES = global_mode[QUANTIZE_MODE_CHANGES];

	flash_mode_LEVELCV_IS_MIX_0 = mode[0][LEVELCV_IS_MIX];
	flash_mode_LEVELCV_IS_MIX_1 = mode[1][LEVELCV_IS_MIX];

	flash_mode_SEND_RETURN_BEFORE_LOOP_0 = mode[0][SEND_RETURN_BEFORE_LOOP];
	flash_mode_SEND_RETURN_BEFORE_LOOP_1 = mode[1][SEND_RETURN_BEFORE_LOOP];
}


void write_all_params_to_FLASH(void)
{
	uint8_t i;

	flash_begin_open_program();

	flash_open_erase_sector(FLASH_ADDR_userparams);

	flash_open_program_word(flash_firmware_version + FLASH_SYMBOL_firmwareoffset, FLASH_ADDR_firmware_version);


	for (i=0;i<6;i++)
	{
		flash_open_program_word(*(uint32_t *)&(flash_CV_CALIBRATION_OFFSET[i]), FLASH_ADDR_cv_calibration_offset+(i*4));
	}
	for (i=0;i<4;i++)
	{
		flash_open_program_word(*(uint32_t *)&(flash_CODEC_DAC_CALIBRATION_DCOFFSET[i]), FLASH_ADDR_dac_calibration_dcoffset+(i*4));
		flash_open_program_word(*(uint32_t *)&(flash_CODEC_ADC_CALIBRATION_DCOFFSET[i]), FLASH_ADDR_adc_calibration_dcoffset+(i*4));
	}



	flash_open_program_word(*(uint32_t *)&flash_param_TRACKING_COMP_0, FLASH_ADDR_TRACKING_COMP_0);
	flash_open_program_word(*(uint32_t *)&flash_param_TRACKING_COMP_1, FLASH_ADDR_TRACKING_COMP_1);

	flash_open_program_byte(flash_loop_led_brightness, FLASH_ADDR_loop_led_brightness);

	flash_open_program_byte(flash_mode_LOOP_CLOCK_GATETRIG_0, FLASH_ADDR_LOOP_CLOCK_GATETRIG_0);
	flash_open_program_byte(flash_mode_LOOP_CLOCK_GATETRIG_1, FLASH_ADDR_LOOP_CLOCK_GATETRIG_1);
	flash_open_program_byte(flash_mode_MAIN_CLOCK_GATETRIG, FLASH_ADDR_MAIN_CLOCK_GATETRIG);

	flash_open_program_byte(flash_global_mode_AUTO_MUTE, FLASH_ADDR_AUTO_MUTE);
	flash_open_program_byte(flash_global_mode_SOFTCLIP, FLASH_ADDR_SOFTCLIP);

	flash_open_program_byte(flash_mode_LEVELCV_IS_MIX_0, FLASH_ADDR_LEVELCV_IS_MIX_0);
	flash_open_program_byte(flash_mode_LEVELCV_IS_MIX_1, FLASH_ADDR_LEVELCV_IS_MIX_1);

	flash_open_program_word(flash_global_param_FAST_FADE_SAMPLES, FLASH_ADDR_FAST_FADE_SAMPLES);
	flash_open_program_word(flash_global_param_SLOW_FADE_SAMPLES, FLASH_ADDR_SLOW_FADE_SAMPLES);

	flash_open_program_byte(flash_global_mode_REV_GATETRIG, FLASH_ADDR_REV_GATETRIG);
	flash_open_program_byte(flash_global_mode_INF_GATETRIG, FLASH_ADDR_INF_GATETRIG);

	flash_open_program_byte(flash_global_mode_PING_METHOD, FLASH_ADDR_PING_METHOD);
	flash_open_program_byte(flash_global_mode_LOG_DELAY_FEED, FLASH_ADDR_LOG_DELAY_FEED);
	flash_open_program_byte(flash_global_mode_RUNAWAYDC_BLOCK, FLASH_ADDR_RUNAWAYDC_BLOCK);
	flash_open_program_byte(flash_global_mode_QUANTIZE_MODE_CHANGES, FLASH_ADDR_QUANTIZE_MODE_CHANGES);

	flash_open_program_byte(flash_mode_SEND_RETURN_BEFORE_LOOP_0, FLASH_ADDR_SEND_RETURN_BEFORE_LOOP_0);
	flash_open_program_byte(flash_mode_SEND_RETURN_BEFORE_LOOP_1, FLASH_ADDR_SEND_RETURN_BEFORE_LOOP_1);

	flash_end_open_program();
}


void read_all_params_from_FLASH(void)
{
	uint8_t i;

	flash_firmware_version = flash_read_word(FLASH_ADDR_firmware_version) - FLASH_SYMBOL_firmwareoffset;

	for (i=0;i<6;i++)
	{
		flash_CV_CALIBRATION_OFFSET[i] = flash_read_word(FLASH_ADDR_cv_calibration_offset+(i*4));
	}
	for (i=0;i<4;i++)
	{
		flash_CODEC_DAC_CALIBRATION_DCOFFSET[i] = flash_read_word(FLASH_ADDR_dac_calibration_dcoffset+(i*4));
		flash_CODEC_ADC_CALIBRATION_DCOFFSET[i] = flash_read_word(FLASH_ADDR_adc_calibration_dcoffset+(i*4));
	}

	flash_param_TRACKING_COMP_0 = flash_read_word(FLASH_ADDR_TRACKING_COMP_0);
	if (flash_param_TRACKING_COMP_0 > 1.25 || flash_param_TRACKING_COMP_0 < 0.75)
		flash_param_TRACKING_COMP_0 = 1.0;

	flash_param_TRACKING_COMP_1 = flash_read_word(FLASH_ADDR_TRACKING_COMP_1);
	if (flash_param_TRACKING_COMP_1 > 1.25 || flash_param_TRACKING_COMP_1 < 0.75)
		flash_param_TRACKING_COMP_1 = 1.0;


	flash_global_param_FAST_FADE_SAMPLES = flash_read_word(FLASH_ADDR_FAST_FADE_SAMPLES);
	if (flash_global_param_FAST_FADE_SAMPLES < 1 || flash_global_param_FAST_FADE_SAMPLES > 48000)
		flash_global_param_FAST_FADE_SAMPLES = 196;

	flash_global_param_SLOW_FADE_SAMPLES = flash_read_word(FLASH_ADDR_SLOW_FADE_SAMPLES);
	if (flash_global_param_SLOW_FADE_SAMPLES < 1 || flash_global_param_SLOW_FADE_SAMPLES > 48000)
		flash_global_param_SLOW_FADE_SAMPLES = 196;

	flash_loop_led_brightness = flash_read_byte(FLASH_ADDR_loop_led_brightness);
	if (flash_loop_led_brightness > 31 || flash_loop_led_brightness < 2)
		flash_loop_led_brightness = 4;

	flash_mode_LOOP_CLOCK_GATETRIG_0 = flash_read_byte(FLASH_ADDR_LOOP_CLOCK_GATETRIG_0) ? 1 : 0; //default trig
	flash_mode_LOOP_CLOCK_GATETRIG_1 = flash_read_byte(FLASH_ADDR_LOOP_CLOCK_GATETRIG_1) ? 1 : 0; //default trig
	flash_mode_MAIN_CLOCK_GATETRIG = flash_read_byte(FLASH_ADDR_MAIN_CLOCK_GATETRIG) ? 1 : 0; //default trig

	flash_global_mode_AUTO_MUTE = flash_read_byte(FLASH_ADDR_AUTO_MUTE) ? 1 : 0;  //default ON
	flash_global_mode_SOFTCLIP = flash_read_byte(FLASH_ADDR_SOFTCLIP) ? 1 : 0; //default ON


	flash_mode_LEVELCV_IS_MIX_0 = (flash_read_byte(FLASH_ADDR_LEVELCV_IS_MIX_0)==1) ? 1 : 0; //default OFF
	flash_mode_LEVELCV_IS_MIX_1 = (flash_read_byte(FLASH_ADDR_LEVELCV_IS_MIX_1)==1) ? 1 : 0; //default OFF

	flash_global_mode_PING_METHOD = flash_read_byte(FLASH_ADDR_PING_METHOD);
	if (flash_global_mode_PING_METHOD > NUM_PING_METHODS || flash_global_mode_PING_METHOD < 0)
		flash_global_mode_PING_METHOD = IGNORE_FLAT_DEVIATION_10;

	flash_global_mode_LOG_DELAY_FEED = (flash_read_byte(FLASH_ADDR_LOG_DELAY_FEED)==1) ? 1 : 0; //default OFF

	flash_global_mode_RUNAWAYDC_BLOCK = (flash_read_byte(FLASH_ADDR_RUNAWAYDC_BLOCK)==0) ? 0 : 1; //default ON

	flash_global_mode_QUANTIZE_MODE_CHANGES = (flash_read_byte(FLASH_ADDR_QUANTIZE_MODE_CHANGES)==1) ? 1 : 0; //default OFF

	flash_global_mode_REV_GATETRIG = flash_read_byte(FLASH_ADDR_REV_GATETRIG) ? 1 : 0; //default trig
	flash_global_mode_INF_GATETRIG = flash_read_byte(FLASH_ADDR_INF_GATETRIG) ? 1 : 0; //default trig

	flash_mode_SEND_RETURN_BEFORE_LOOP_0 = (flash_read_byte(FLASH_ADDR_SEND_RETURN_BEFORE_LOOP_0)==1) ? 1 : 0; //default OFF
	flash_mode_SEND_RETURN_BEFORE_LOOP_1 = (flash_read_byte(FLASH_ADDR_SEND_RETURN_BEFORE_LOOP_1)==1) ? 1 : 0; //default OFF

}
