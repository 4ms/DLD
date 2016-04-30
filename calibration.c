/*
 * Calibration
 *
 */

#include "globals.h"
#include "calibration.h"
#include "adc.h"
#include "params.h"
#include "dig_inouts.h"
#include "flash_user.h"

int16_t CV_CALIBRATION_OFFSET[NUM_CV_ADCS];
int16_t CODEC_DAC_CALIBRATION_DCOFFSET[4];
int16_t CODEC_ADC_CALIBRATION_DCOFFSET[4];

extern uint32_t flash_firmware_version;
extern int32_t flash_CV_CALIBRATION_OFFSET[6];
extern int32_t flash_CODEC_DAC_CALIBRATION_DCOFFSET[4];
extern int32_t flash_CODEC_ADC_CALIBRATION_DCOFFSET[4];


extern int16_t i_smoothed_potadc[NUM_POT_ADCS];
extern int16_t i_smoothed_cvadc[NUM_CV_ADCS];
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

	delay();
	delay();
	delay();
	delay();


	CV_CALIBRATION_OFFSET[0] = 2048-(i_smoothed_cvadc[0] & 0x0FFF);
	CV_CALIBRATION_OFFSET[1] = 2048-(i_smoothed_cvadc[1] & 0x0FFF);
	CV_CALIBRATION_OFFSET[2] = -1*(i_smoothed_cvadc[2] & 0x0FFF);
	CV_CALIBRATION_OFFSET[3] = -1*(i_smoothed_cvadc[3] & 0x0FFF);
	CV_CALIBRATION_OFFSET[4] = -1*(i_smoothed_cvadc[4] & 0x0FFF);
	CV_CALIBRATION_OFFSET[5] = -1*(i_smoothed_cvadc[5] & 0x0FFF);

}


void update_calibration(void)
{
	uint8_t switch1, switch2;
	static uint32_t buttons_down=0;

	CV_CALIBRATION_OFFSET[0] = 2048-(i_smoothed_rawcvadc[0] & 0x0FFF);
	CV_CALIBRATION_OFFSET[1] = 2048-(i_smoothed_rawcvadc[1] & 0x0FFF);
	CV_CALIBRATION_OFFSET[2] = -1*(i_smoothed_rawcvadc[2] & 0x0FFF);
	CV_CALIBRATION_OFFSET[3] = -1*(i_smoothed_rawcvadc[3] & 0x0FFF);
	CV_CALIBRATION_OFFSET[4] = -1*(i_smoothed_rawcvadc[4] & 0x0FFF);
	CV_CALIBRATION_OFFSET[5] = -1*(i_smoothed_rawcvadc[5] & 0x0FFF);


	switch1=TIMESW_CH1;
	switch2=TIMESW_CH2;

	if (switch1==SWITCH_UP && switch2==SWITCH_UP)//both up: calibrate audio output offset, using the knobs to set DC level
	{
		CODEC_DAC_CALIBRATION_DCOFFSET[0]=(i_smoothed_potadc[LEVEL_POT*2]-2048-1750); //OUT A
		CODEC_DAC_CALIBRATION_DCOFFSET[1]=(i_smoothed_potadc[LEVEL_POT*2+1]-2048-1750); //OUT B
		CODEC_DAC_CALIBRATION_DCOFFSET[2]=(i_smoothed_potadc[MIX_POT*2]-2048-1750); //SEND A
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
}



void update_calibrate_leds(void)
{
	static uint32_t led_flasher=0;

	led_flasher+=UINT32_MAX/1665;
	loop_led_state[(led_flasher < UINT32_MAX/2)?1:0] = 0;
	loop_led_state[(led_flasher < UINT32_MAX/2)?0:1] = 1;

}



