
#include "globals.h"
#include "flash.h"
#include "flash_user.h"
#include "calibration.h"

extern int16_t CV_CALIBRATION_OFFSET[6];
extern int16_t CODEC_DAC_CALIBRATION_DCOFFSET[4];
extern int16_t CODEC_ADC_CALIBRATION_DCOFFSET[4];


#define FLASH_ADDR_userparams 0x08004000
//Globals:
#define FLASH_ADDR_firmware_version			FLASH_ADDR_userparams							/* 0  ..3	*/
#define SZ_FV 4

#define FLASH_ADDR_cv_calibration_offset	(FLASH_ADDR_firmware_version	+ SZ_FV)		/* 4  ..27	*/
#define SZ_CVCAL 24

#define FLASH_ADDR_adc_calibration_dcoffset	(FLASH_ADDR_cv_calibration_offset 	+ SZ_CVCAL)	/* 28  ..43	*/
#define SZ_ADCCAL 16

#define FLASH_ADDR_dac_calibration_dcoffset	(FLASH_ADDR_adc_calibration_dcoffset 	+ SZ_ADCCAL)	/* 44  ..59	*/
#define SZ_DACCAL 16


#define FLASH_SYMBOL_bankfilled 0x01
#define FLASH_SYMBOL_startupoffset 0xAA
#define FLASH_SYMBOL_firmwareoffset 0xAA550000

uint32_t flash_firmware_version=0;
int32_t flash_CV_CALIBRATION_OFFSET[6];
int32_t flash_CODEC_DAC_CALIBRATION_DCOFFSET[4];
int32_t flash_CODEC_ADC_CALIBRATION_DCOFFSET[4];

void factory_reset(void)
{
	flash_firmware_version = FW_VERSION;

	//copy global variables to flash_* variables (SRAM staging area)
	store_params_into_sram();

	//copy SRAM variables to FLASH
	write_all_params_to_FLASH();

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

	flash_end_open_program();
}
