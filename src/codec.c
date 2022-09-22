#include "codec_CS4271.h"
#include "codec_CS4271_regs.h"
#include "globals.h"

const uint8_t codec_init_data_slave_DCinput[] = {

	SINGLE_SPEED | RATIO0 | SLAVE | DIF_I2S_24b, //MODECTRL1

	SLOW_FILT_SEL | DEEMPH_OFF, //DACCTRL

	ATAPI_aLbR, //DACMIX

	0b00000000, //DACAVOL
	0b00000000, //DACBVOL

	ADC_DIF_I2S | HPFDisableA | HPFDisableB //ADCCTRL

};

const uint8_t codec_init_data_slave[] =
{

		SINGLE_SPEED
		| RATIO0
		| SLAVE
		| DIF_I2S_24b,		//MODECTRL1

		SLOW_FILT_SEL
		| DEEMPH_OFF,		//DACCTRL

		ATAPI_aLbR,			//DACMIX

		0b00000000,			//DACAVOL
		0b00000000,			//DACBVOL

		ADC_DIF_I2S
		/*| HPFDisableA
		| HPFDisableB */	//ADCCTRL

};
const uint8_t codec_init_data_master[] =
{

		SINGLE_SPEED
		| RATIO0
		| MASTER
		| DIF_I2S_24b,//MODECTRL1

		FAST_FILT_SEL
		| DEEMPH_OFF
		| SOFT_RAMPUP
		| SOFT_RAMPDOWN,	//DACCTRL

		ATAPI_aLbR,			//DACMIX

		0b00000000,			//DACAVOL
		0b00000000,			//DACBVOL

		ADC_DIF_I2S
	/*	| HPFDisableA
		| HPFDisableB */	//ADCCTRL

};

static uint32_t Codec_Reset(I2C_TypeDef *CODEC, uint8_t master_slave, uint8_t enable_DCinput);

uint32_t Codec_A_Register_Setup(uint8_t enable_DCinput) {
	uint32_t err = 0;
	Codec_A_CtrlInterface_Init();

	CODECA_RESET_HIGH;
	delay_ms(2);

	err = Codec_Reset(CODECA_I2C, CODECA_MODE, enable_DCinput);

	return err;
}

uint32_t Codec_B_Register_Setup(uint8_t enable_DCinput) {
	uint32_t err = 0;
	Codec_B_CtrlInterface_Init();

	CODECB_RESET_HIGH;
	delay_ms(2);

	err = Codec_Reset(CODECB_I2C, CODECB_MODE, enable_DCinput);

	return err;
}

static uint32_t Codec_Reset(I2C_TypeDef *CODEC, uint8_t master_slave, uint8_t enable_DCinput) {
	uint8_t i;
	uint32_t err = 0;

	err += Codec_WriteRegister(CS4271_REG_MODELCTRL2, CPEN | PDN, CODEC); //Control Port Enable and Power Down Enable

	if (master_slave == CODEC_IS_MASTER) {
		for (i = 0; i < CS4271_NUM_REGS; i++)
			err += Codec_WriteRegister(i + 1, codec_init_data_master[i], CODEC);

	} else {
		if (enable_DCinput) {
			for (i = 0; i < CS4271_NUM_REGS; i++)
				err += Codec_WriteRegister(i + 1, codec_init_data_slave_DCinput[i], CODEC);
		} else {
			for (i = 0; i < CS4271_NUM_REGS; i++)
				err += Codec_WriteRegister(i + 1, codec_init_data_slave[i], CODEC);
		}
	}

	err += Codec_WriteRegister(CS4271_REG_MODELCTRL2, CPEN, CODEC); //Power Down disable

	return err;
}
