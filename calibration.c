/*
 * Calibration
 *
 * Loop A/B Clock Out jacks:
 * -Gate
 * -Trigger
 *
 * Level A/B CV jacks:
 * -Delay Level (default)
 * -Dry/Wet amount
 *
 * Time A/B knob:
 * -Quantized normally, unquantized when turned with Reverse held down (default)
 * -Unquantized normally, quantized when turned with Reverse held down
 *
 * Time A/B CV jack:
 * -Quantized (default)
 * -Always unquantized (...try this?)
 * -Follows mode of knob (quantized until Time knob is wiggled with Reverse held down)
 *
 *
 *
 * Noisegate on/off
 *
 *
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

extern volatile int16_t ch1rx_buffer[codec_BUFF_LEN];
extern volatile int16_t ch2rx_buffer[codec_BUFF_LEN];

extern uint8_t global_mode[NUM_GLOBAL_MODES];


void set_default_calibration_values(void)
{

	CODEC_DAC_CALIBRATION_DCOFFSET[0]=-1746;
	CODEC_DAC_CALIBRATION_DCOFFSET[1]=-1746;
	CODEC_DAC_CALIBRATION_DCOFFSET[2]=-1746;
	CODEC_DAC_CALIBRATION_DCOFFSET[3]=-1746;

/*
	CV_CALIBRATION_OFFSET[0] = 0;
	CV_CALIBRATION_OFFSET[1] = 0;
	CV_CALIBRATION_OFFSET[2] = 0;
	CV_CALIBRATION_OFFSET[3] = 0;
	CV_CALIBRATION_OFFSET[4] = 0;
	CV_CALIBRATION_OFFSET[5] = 0;
*/
}

extern float smoothed_rawcvadc[NUM_CV_ADCS];
extern int16_t i_smoothed_rawcvadc[NUM_CV_ADCS];

void auto_calibrate(void)
{
	delay();
	delay();
	delay();
	delay();

	CV_CALIBRATION_OFFSET[0] = 2048-i_smoothed_cvadc[0];
	CV_CALIBRATION_OFFSET[1] = 2048-i_smoothed_cvadc[1];
	CV_CALIBRATION_OFFSET[2] = -1*i_smoothed_cvadc[2];
	CV_CALIBRATION_OFFSET[3] = -1*i_smoothed_cvadc[3];
	CV_CALIBRATION_OFFSET[4] = -1*i_smoothed_cvadc[4];
	CV_CALIBRATION_OFFSET[5] = -1*i_smoothed_cvadc[5];

	CODEC_ADC_CALIBRATION_DCOFFSET[0]=-29;
	CODEC_ADC_CALIBRATION_DCOFFSET[1]=-29;
	CODEC_ADC_CALIBRATION_DCOFFSET[2]=-29;
	CODEC_ADC_CALIBRATION_DCOFFSET[3]=-29;

	set_default_calibration_values();
}

void update_calibration(void)
{
	uint8_t switch1, switch2;
	static uint32_t buttons_down=0;

	CV_CALIBRATION_OFFSET[0] = 2048-i_smoothed_rawcvadc[0];
	CV_CALIBRATION_OFFSET[1] = 2048-i_smoothed_rawcvadc[1];
	CV_CALIBRATION_OFFSET[2] = -1*i_smoothed_rawcvadc[2];
	CV_CALIBRATION_OFFSET[3] = -1*i_smoothed_rawcvadc[3];
	CV_CALIBRATION_OFFSET[4] = -1*i_smoothed_rawcvadc[4];
	CV_CALIBRATION_OFFSET[5] = -1*i_smoothed_rawcvadc[5];


	switch1=TIMESW_CH1;
	switch2=TIMESW_CH2;

	if (switch1==SWITCH_UP && switch2==SWITCH_UP)//both up: calibrate audio output offset, using the knobs to set DC level
	{
		CODEC_DAC_CALIBRATION_DCOFFSET[0]=(i_smoothed_potadc[LEVEL*2]-2048-1750); //OUT A
		CODEC_DAC_CALIBRATION_DCOFFSET[1]=(i_smoothed_potadc[LEVEL*2+1]-2048-1750); //OUT B
		CODEC_DAC_CALIBRATION_DCOFFSET[2]=(i_smoothed_potadc[MIXPOT*2]-2048-1750); //SEND A
		CODEC_DAC_CALIBRATION_DCOFFSET[3]=(i_smoothed_potadc[MIXPOT*2+1]-2048-1750); //SEND B
	}

	if (switch1==SWITCH_DOWN && switch2==SWITCH_DOWN)//both down: calibrate audio input DC level
	{
		CODEC_ADC_CALIBRATION_DCOFFSET[0]= -1*ch1rx_buffer[0];
		CODEC_ADC_CALIBRATION_DCOFFSET[1]= -1*ch2rx_buffer[0];
		CODEC_ADC_CALIBRATION_DCOFFSET[2]= -1*ch1rx_buffer[2];
		CODEC_ADC_CALIBRATION_DCOFFSET[3]= -1*ch2rx_buffer[2];
	}

//	if ((switch1==SWITCH_CENTER) && (switch2==SWITCH_CENTER) && REV1BUT && INF1BUT && REV2BUT && INF2BUT)//switches both center, four buttons held
	if (SAVE_CALIBRATE_BUTTONS)
	{
		buttons_down++;
		if (buttons_down==3000)
		{
			save_calibration();
			global_mode[CALIBRATE] = 0;

		}
	} else
		buttons_down=0;



}

uint32_t load_calibration(void)
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


		return (1);

	} else
		return(0); //No calibration data found
}

void save_calibration(void)
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
		delay();

		LED_PINGBUT_ON;
		LED_REV1_ON;
		LED_REV2_ON;
		LED_LOOP1_ON;
		LED_LOOP2_ON;
		LED_INF1_ON;
		LED_INF2_ON;

		delay();
	}
}

void check_calibration_mode(void)
{
	global_mode[CALIBRATE] = 0;

    if (!load_calibration()){
    	auto_calibrate();
    	set_firmware_version();

    	global_mode[CALIBRATE] = 1;
    //	factory_reset();
	}

    if (ENTER_CALIBRATE_BUTTONS)
    	global_mode[CALIBRATE] = 1;

}
