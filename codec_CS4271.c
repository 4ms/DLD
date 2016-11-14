/*
 * codec_CS4271.c - Setup for the Cirrus Logic CS4271 codec
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

#include "codec_CS4271.h"

#include "globals.h"
#include "i2s.h"

#define CODEC_IS_SLAVE 0
#define CODEC_IS_MASTER 1

#define MCLK_SRC_STM 0
#define MCLK_SRC_EXTERNAL 1
//#define MCLK_SRC_CODEC 2

//CODECA is left channel of the DLD (I2C1, I2S3)
//CODECB is right channel of the DLD (I2C2, I2S2)

#ifdef USE_VCXO

#define CODECA_MODE CODEC_IS_MASTER
#define CODECB_MODE CODEC_IS_MASTER

#define CODECA_MCLK_SRC MCLK_SRC_EXTERNAL
#define CODECB_MCLK_SRC MCLK_SRC_EXTERNAL

#else

#define CODECA_MODE CODEC_IS_SLAVE
#define CODECB_MODE CODEC_IS_SLAVE

#define CODECA_MCLK_SRC MCLK_SRC_STM
#define CODECB_MCLK_SRC MCLK_SRC_STM

#endif


/* Codec audio Standards */
#ifdef I2S_STANDARD_PHILLIPS
 #define  CODEC_STANDARD                0x04
 #define I2S_STANDARD                   I2S_Standard_Phillips         
#elif defined(I2S_STANDARD_MSB)
 #define  CODEC_STANDARD                0x00
 #define I2S_STANDARD                   I2S_Standard_MSB    
#elif defined(I2S_STANDARD_LSB)
 #define  CODEC_STANDARD                0x08
 #define I2S_STANDARD                   I2S_Standard_LSB    
#else 
 #error "Error: No audio communication standard selected !"
#endif /* I2S_STANDARD */



#define CS4271_ADDR_0 0b0010000
#define CS4271_ADDR_1 0b0010001

/*
 * The 7 bits Codec address (sent through I2C interface)
 * The 8th bit (LSB) is Read /Write
 */
#define CODEC_ADDRESS           (CS4271_ADDR_0<<1)

#define CS4271_NUM_REGS 6	/* we only initialize the first 6 registers, the 7th is for pre/post-init and the 8th is read-only */

#define CS4271_REG_MODECTRL1	1
#define CS4271_REG_DACCTRL		2
#define CS4271_REG_DACMIX		3
#define CS4271_REG_DACAVOL		4
#define CS4271_REG_DACBVOL		5
#define CS4271_REG_ADCCTRL		6
#define CS4271_REG_MODELCTRL2	7
#define CS4271_REG_CHIPID		8	/*Read-only*/

//Reg 1 (MODECTRL1):
#define SINGLE_SPEED		(0b00<<6)		/* 4-50kHz */
#define DOUBLE_SPEED		(0b10<<6)		/* 50-100kHz */
#define QUAD_SPEED			(0b11<<6)		/* 100-200kHz */
#define	RATIO0				(0b00<<4)		/* See table page 28 and 29 of datasheet */
#define	RATIO1				(0b01<<4)
#define	RATIO2				(0b10<<4)
#define	RATIO3				(0b11<<4)
#define	MASTER				(1<<3)
#define	SLAVE				(0<<3)
#define	DIF_LEFTJUST_24b	(0b000)
#define	DIF_I2S_24b			(0b001)
#define	DIF_RIGHTJUST_16b	(0b010)
#define	DIF_RIGHTJUST_24b	(0b011)
#define	DIF_RIGHTJUST_20b	(0b100)
#define	DIF_RIGHTJUST_18b	(0b101)

//Reg 2 (DACCTRL)
#define AUTOMUTE 		(1<<7)
#define SLOW_FILT_SEL	(1<<6)
#define FAST_FILT_SEL	(0<<6)
#define DEEMPH_OFF		(0<<4)
#define DEEMPH_44		(1<<4)
#define DEEMPH_48		(2<<4)
#define DEEMPH_32		(3<<4)
#define	SOFT_RAMPUP		(1<<3) /*An un-mute will be performed after executing a filter mode change, after a MCLK/LRCK ratio change or error, and after changing the Functional Mode.*/
#define	SOFT_RAMPDOWN	(1<<2) /*A mute will be performed prior to executing a filter mode change.*/
#define INVERT_SIGA_POL	(1<<1) /*When set, this bit activates an inversion of the signal polarity for the appropriate channel*/
#define INVERT_SIGB_POL	(1<<0)

//Reg 3 (DACMIX)
#define BEQA			(1<<6) /*If set, ignore AOUTB volume setting, and instead make channel B's volume equal channel A's volume as set by AOUTA */
#define SOFTRAMP		(1<<5) /*Allows level changes, both muting and attenuation, to be implemented by incrementally ramping, in 1/8 dB steps, from the current level to the new level at a rate of 1 dB per 8 left/right clock periods */
#define	ZEROCROSS		(1<<4) /*Dictates that signal level changes, either by attenuation changes or muting, will occur on a signal zero crossing to minimize audible artifacts*/
#define ATAPI_aLbR		(0b1001) /*channel A==>Left, channel B==>Right*/

//Reg 4: DACAVOL
//Reg 5: DACBVOL

//Reg 6 (ADCCTRL)
#define DITHER16		(1<<5) /*activates the Dither for 16-Bit Data feature*/
#define ADC_DIF_I2S		(1<<4) /*I2S, up to 24-bit data*/
#define ADC_DIF_LJUST	(0<<4) /*Left Justified, up to 24-bit data (default)*/
#define MUTEA			(1<<3)
#define MUTEB			(1<<2)
#define HPFDisableA		(1<<1)
#define HPFDisableB		(1<<0)


//Reg 7 (MODECTRL2)
#define PDN		(1<<0)		/* Power Down Enable */
#define CPEN	(1<<1)		/* Control Port Enable */
#define FREEZE	(1<<2)		/* Freezes effects of register changes */
#define MUTECAB	(1<<3)		/* Internal AND gate on AMUTEC and BMUTEC */
#define LOOP	(1<<4)		/* Digital loopback (ADC->DAC) */

//Reg 8 (CHIPID) (Read-only)
#define PART_mask	(0b11110000)
#define REV_mask	(0b00001111)

const uint8_t codec_init_data_slave_DCinput[] =
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
		| HPFDisableA
		| HPFDisableB 	//ADCCTRL

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


__IO uint32_t  CODECTimeout = CODEC_LONG_TIMEOUT;   



#ifdef USE_DEFAULT_TIMEOUT_CALLBACK

uint32_t Codec_TIMEOUT_UserCallback(void)
{
	while (1)
	{   
	}
}
#else
uint32_t Codec_TIMEOUT_UserCallback(void)
{
	return 1;
}
#endif


void Codecs_Deinit(void)
{
	GPIO_InitTypeDef gpio;

	RCC_AHB1PeriphClockCmd(CODECB_RESET_RCC, ENABLE);
	RCC_AHB1PeriphClockCmd(CODECA_RESET_RCC, ENABLE);

	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = CODECA_RESET_pin; GPIO_Init(CODECA_RESET_GPIO, &gpio);
	gpio.GPIO_Pin = CODECB_RESET_pin; GPIO_Init(CODECB_RESET_GPIO, &gpio);

	CODECA_RESET_LOW;
	CODECB_RESET_LOW;


}


uint32_t Codec_Register_Setup(uint8_t enable_DCinput)
{
	uint32_t err = 0;

	Codec_CtrlInterface_Init();


	CODECA_RESET_HIGH;
	delay_ms(2);

	err+=Codec_Reset(CODECA_I2C, CODECA_MODE, enable_DCinput);


	CODECB_RESET_HIGH;
	delay_ms(2);

	err+=Codec_Reset(CODECB_I2C, CODECB_MODE, enable_DCinput);

	return err;
}


uint32_t Codec_Reset(I2C_TypeDef *CODEC, uint8_t master_slave, uint8_t enable_DCinput)
{
	uint8_t i;
	uint32_t err=0;
	
	err+=Codec_WriteRegister(CS4271_REG_MODELCTRL2, CPEN | PDN, CODEC); //Control Port Enable and Power Down Enable
	

	if (master_slave==CODEC_IS_MASTER)
	{
		for(i=0;i<CS4271_NUM_REGS;i++)
			err+=Codec_WriteRegister(i+1, codec_init_data_master[i], CODEC);

	}
	else
	{
		if (enable_DCinput)
		{
			for(i=0;i<CS4271_NUM_REGS;i++)
				err+=Codec_WriteRegister(i+1, codec_init_data_slave_DCinput[i], CODEC);
		}
		else
		{
			for(i=0;i<CS4271_NUM_REGS;i++)
				err+=Codec_WriteRegister(i+1, codec_init_data_slave[i], CODEC);
		}
	}

	err+=Codec_WriteRegister(CS4271_REG_MODELCTRL2, CPEN, CODEC); //Power Down disable

	return err;
}


uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue, I2C_TypeDef *CODEC)
{
	uint32_t result = 0;
	
	uint8_t Byte1 = RegisterAddr;
	uint8_t Byte2 = RegisterValue;
	
	/*!< While the bus is busy */
	CODECTimeout = CODEC_LONG_TIMEOUT;
	while(I2C_GetFlagStatus(CODEC, I2C_FLAG_BUSY))
	{
		if((CODECTimeout--) == 0)
			return Codec_TIMEOUT_UserCallback();
	}

	/* Start the config sequence */
	I2C_GenerateSTART(CODEC, ENABLE);

	/* Test on EV5 and clear it */
	CODECTimeout = CODEC_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(CODEC, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((CODECTimeout--) == 0)
			return Codec_TIMEOUT_UserCallback();
	}

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(CODEC, CODEC_ADDRESS, I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	CODECTimeout = CODEC_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(CODEC, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((CODECTimeout--) == 0)
			return Codec_TIMEOUT_UserCallback();
	}

	/* Transmit the first address for write operation */
	I2C_SendData(CODEC, Byte1);

	/* Test on EV8 and clear it */
	CODECTimeout = CODEC_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(CODEC, I2C_EVENT_MASTER_BYTE_TRANSMITTING))
	{
		if((CODECTimeout--) == 0)
			return Codec_TIMEOUT_UserCallback();
	}

	/* Prepare the register value to be sent */
	I2C_SendData(CODEC, Byte2);

	/*!< Wait till all data have been physically transferred on the bus */
	CODECTimeout = CODEC_LONG_TIMEOUT;
	while(!I2C_GetFlagStatus(CODEC, I2C_FLAG_BTF))
	{
		if((CODECTimeout--) == 0)
			return Codec_TIMEOUT_UserCallback();
	}

	/* End the configuration sequence */
	I2C_GenerateSTOP(CODEC, ENABLE);

	/* Return the verifying value: 0 (Passed) or 1 (Failed) */
	return result;  
}

/*
 * Initializes I2C peripheral for both codecs
 */
void Codec_CtrlInterface_Init(void)
{
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB1PeriphClockCmd(CODECB_I2C_CLK, ENABLE);

	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x33;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
	
	I2C_DeInit(CODECB_I2C);
	I2C_Init(CODECB_I2C, &I2C_InitStructure);
	I2C_Cmd(CODECB_I2C, ENABLE);


	RCC_APB1PeriphClockCmd(CODECA_I2C_CLK, ENABLE);

	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x33;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;

	I2C_DeInit(CODECA_I2C);
	I2C_Init(CODECA_I2C, &I2C_InitStructure);
	I2C_Cmd(CODECA_I2C, ENABLE);
}

/*
 * Initializes I2S2 and I2S3 peripherals on the STM32
 */
void Codec_AudioInterface_Init(uint32_t AudioFreq)
{
	I2S_InitTypeDef I2S_InitStructure;



	//CODEC B: Right channel of DLD (I2S2, I2C2)

	/* Enable the CODEC_I2S peripheral clock */
	RCC_APB1PeriphClockCmd(CODECB_I2S_CLK, ENABLE);

	/* CODEC_I2S peripheral configuration for master TX */
	SPI_I2S_DeInit(CODECB_I2S);
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_24b;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;


	if (CODECB_MODE==CODEC_IS_MASTER)
		I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveTx;
	else
		I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;

	if (CODECB_MODE==CODEC_IS_MASTER)
		I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
	else {
		if (CODECB_MCLK_SRC==MCLK_SRC_STM)
			I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
		else
			I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
	}
	/* Initialize the I2S main channel for TX */
	I2S_Init(CODECB_I2S, &I2S_InitStructure);

	/* Initialize the I2S extended channel for RX */
	I2S_FullDuplexConfig(CODECB_I2S_EXT, &I2S_InitStructure);

	I2S_Cmd(CODECB_I2S, ENABLE);
	I2S_Cmd(CODECB_I2S_EXT, ENABLE);


	// CODEC A: DLD Left Channel
	// Enable the CODEC_I2S peripheral clock
	RCC_APB1PeriphClockCmd(CODECA_I2S_CLK, ENABLE);

	// CODECA_I2S peripheral configuration for master TX
	SPI_I2S_DeInit(CODECA_I2S);
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_24b;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;

	if (CODECA_MODE==CODEC_IS_MASTER)
		I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveTx;
	else
		I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;

	if (CODECA_MODE==CODEC_IS_MASTER)
		I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
	else {
		if (CODECA_MCLK_SRC==MCLK_SRC_STM)
			I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
		else
			I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
	}


	// Initialize the I2S main channel for TX
	I2S_Init(CODECA_I2S, &I2S_InitStructure);

	// Initialize the I2S extended channel for RX
	I2S_FullDuplexConfig(CODECA_I2S_EXT, &I2S_InitStructure);

	I2S_Cmd(CODECA_I2S, ENABLE);
	I2S_Cmd(CODECA_I2S_EXT, ENABLE);


}


void Codec_GPIO_Init(void)
{
	GPIO_InitTypeDef gpio;


	/* Enable I2S and I2C GPIO clocks */
	RCC_AHB1PeriphClockCmd(CODECB_I2C_GPIO_CLOCK | CODECB_I2S_GPIO_CLOCK, ENABLE);
	RCC_AHB1PeriphClockCmd(CODECA_I2C_GPIO_CLOCK | CODECA_I2S_GPIO_CLOCK, ENABLE);


	/* CODEC_I2C SCL and SDA pins configuration -------------------------------------*/
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_OType = GPIO_OType_OD;
	gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = CODECB_I2C_SCL_PIN | CODECB_I2C_SDA_PIN;	GPIO_Init(CODECB_I2C_GPIO, &gpio);
	gpio.GPIO_Pin = CODECA_I2C_SCL_PIN | CODECA_I2C_SDA_PIN;	GPIO_Init(CODECA_I2C_GPIO, &gpio);

	/* Connect pins to I2C peripheral */
	GPIO_PinAFConfig(CODECA_I2C_GPIO, CODECA_I2C_SCL_PINSRC, CODECA_I2C_GPIO_AF);
	GPIO_PinAFConfig(CODECA_I2C_GPIO, CODECA_I2C_SDA_PINSRC, CODECA_I2C_GPIO_AF);

	GPIO_PinAFConfig(CODECB_I2C_GPIO, CODECB_I2C_SCL_PINSRC, CODECB_I2C_GPIO_AF);
	GPIO_PinAFConfig(CODECB_I2C_GPIO, CODECB_I2C_SDA_PINSRC, CODECB_I2C_GPIO_AF);


	/* CODEC_I2S output pins configuration: WS, SCK SD0 SDI MCK pins ------------------*/
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = CODECB_I2S_WS_PIN;	GPIO_Init(CODECB_I2S_GPIO_WS, &gpio);
	gpio.GPIO_Pin = CODECB_I2S_SDI_PIN;	GPIO_Init(CODECB_I2S_GPIO_SDI, &gpio);
	gpio.GPIO_Pin = CODECB_I2S_SCK_PIN;	GPIO_Init(CODECB_I2S_GPIO_SCK, &gpio);
	gpio.GPIO_Pin = CODECB_I2S_SDO_PIN;	GPIO_Init(CODECB_I2S_GPIO_SDO, &gpio);

	gpio.GPIO_Pin = CODECA_I2S_WS_PIN;	GPIO_Init(CODECA_I2S_GPIO_WS, &gpio);
	gpio.GPIO_Pin = CODECA_I2S_SDI_PIN;	GPIO_Init(CODECA_I2S_GPIO_SDI, &gpio);
	gpio.GPIO_Pin = CODECA_I2S_SCK_PIN;	GPIO_Init(CODECA_I2S_GPIO_SCK, &gpio);
	gpio.GPIO_Pin = CODECA_I2S_SDO_PIN;	GPIO_Init(CODECA_I2S_GPIO_SDO, &gpio);

	GPIO_PinAFConfig(CODECB_I2S_GPIO_WS, CODECB_I2S_WS_PINSRC, CODECB_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODECB_I2S_GPIO_SCK, CODECB_I2S_SCK_PINSRC, CODECB_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODECB_I2S_GPIO_SDO, CODECB_I2S_SDO_PINSRC, CODECB_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODECB_I2S_GPIO_SDI, CODECB_I2S_SDI_PINSRC, CODECB_I2Sext_GPIO_AF);

	GPIO_PinAFConfig(CODECA_I2S_GPIO_WS, CODECA_I2S_WS_PINSRC, CODECA_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODECA_I2S_GPIO_SCK, CODECA_I2S_SCK_PINSRC, CODECA_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODECA_I2S_GPIO_SDO, CODECA_I2S_SDO_PINSRC, CODECA_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODECA_I2S_GPIO_SDI, CODECA_I2S_SDI_PINSRC, CODECA_I2Sext_GPIO_AF);

	if (CODECA_MCLK_SRC==MCLK_SRC_STM){

		gpio.GPIO_Mode = GPIO_Mode_AF;
		gpio.GPIO_Speed = GPIO_Speed_100MHz;
		gpio.GPIO_OType = GPIO_OType_PP;
		gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

		gpio.GPIO_Pin = CODECA_I2S_MCK_PIN; GPIO_Init(CODECA_I2S_MCK_GPIO, &gpio);
		GPIO_PinAFConfig(CODECA_I2S_MCK_GPIO, CODECA_I2S_MCK_PINSRC, CODECA_I2S_GPIO_AF);

	} else if (CODECA_MCLK_SRC==MCLK_SRC_EXTERNAL){

		gpio.GPIO_Mode = GPIO_Mode_IN;
		gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

		gpio.GPIO_Pin = CODECA_I2S_MCK_PIN; GPIO_Init(CODECA_I2S_MCK_GPIO, &gpio);

	}

	if (CODECB_MCLK_SRC==MCLK_SRC_STM){
		gpio.GPIO_Mode = GPIO_Mode_AF;
		gpio.GPIO_Speed = GPIO_Speed_100MHz;
		gpio.GPIO_OType = GPIO_OType_PP;
		gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

		gpio.GPIO_Pin = CODECB_I2S_MCK_PIN; GPIO_Init(CODECB_I2S_MCK_GPIO, &gpio);
		GPIO_PinAFConfig(CODECB_I2S_MCK_GPIO, CODECB_I2S_MCK_PINSRC, CODECB_I2S_GPIO_AF);

	} else if (CODECB_MCLK_SRC==MCLK_SRC_EXTERNAL){

		gpio.GPIO_Mode = GPIO_Mode_IN;
		gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

		gpio.GPIO_Pin = CODECB_I2S_MCK_PIN; GPIO_Init(CODECB_I2S_MCK_GPIO, &gpio);

	}


}


/*
 * This must be called before Codec_AudioInterface_Init() because the I2S2CLKConfig must happen before APB1 for I2S2 periph is turned on
 *
 * Must uncomment the line in stm32f4xx_conf.h:
 * #define I2S_EXTERNAL_CLOCK_VAL   7000000
 */
void init_i2s_clkin(void){

	GPIO_InitTypeDef gpio;

	// Enable I2S2 clock in:
	RCC_I2SCLKConfig(RCC_I2S2CLKSource_Ext);

	//I2S2 CLK_IN is PC9
	gpio.GPIO_Pin = GPIO_Pin_9;
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &gpio);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, CODECB_I2S_GPIO_AF);

}
