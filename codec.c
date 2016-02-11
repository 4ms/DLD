/*
 * codec.c
 */

#include "codec.h"
#include "globals.h"
#include "dig_inouts.h"

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


/* Mask for the bit EN of the I2S CFGR register */
#define I2S_ENABLE_MASK                 0x0400

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

#define W8731_ADDR_0 0x1A
#define W8731_ADDR_1 0x1B
#define W8731_NUM_REGS 10

#define WM8731_REG_LLINEIN	0
#define WM8731_REG_RLINEIN	1
#define WM8731_REG_LHEADOUT	2
#define WM8731_REG_RHEADOUT	3
#define WM8731_REG_ANALOG	4
#define WM8731_REG_DIGITAL	5
#define WM8731_REG_POWERDOWN	6
#define WM8731_REG_INTERFACE	7
#define WM8731_REG_SAMPLING	8
#define WM8731_REG_ACTIVE	9
#define WM8731_REG_RESET	15


//Reg 0 and 1:
#define VOL_p12dB	0b11111 /*+12dB*/
#define VOL_0dB		0b10111 /*0dB*/
#define VOL_n1dB	0b10110 /*-1.5dB*/
#define VOL_n3dB	0b10101 /*-3dB*/
#define VOL_n6dB	0b10011 /*-6dB*/
#define VOL_n12dB	15 		/*-12dB*/
#define VOL_n15dB	13 		/*-15dB*/
/*1.5dB steps down to..*/
#define VOL_n34dB	0b00000 /*-34.5dB*/
#define INMUTE		(1<<7)
#define INBOTH		(1<<8)


//Register 4: Analogue Audio Path Control
#define MICBOOST 		(1 << 0)	/* Boost Mic level */
#define MUTEMIC			(1 << 1)	/* Mute Mic to ADC */
#define INSEL_mic		(1 << 2)	/* Mic Select*/
#define INSEL_line		(0 << 2)	/* LineIn Select*/
#define BYPASS			(1 << 3)	/* Bypass Enable */
#define DACSEL			(1 << 4)	/* Select DAC */
#define SIDETONE		(1 << 5)	/* Enable Sidetone */
#define SIDEATT_neg15dB	(0b11 << 6)
#define SIDEATT_neg12dB	(0b10 << 6)
#define SIDEATT_neg9dB	(0b01 << 6)
#define SIDEATT_neg6dB	(0b00 << 6)


//Reg 5: Digital Audio Path
#define ADCHPFDisable (1<<0)
#define ADCHPFEnable (0<<0)
#define DEEMP_48k (3<<1)
#define DEEMP_44k (2<<1)
#define DEEMP_32k (1<<1)
#define DEEMP_Disable (0<<1)
#define DACMUTE (1<<3)
#define HPOFFSETSTORE (1<<4)

//Reg 6: Power Down module when bit is set
#define LINEINPD (1<<0)	/*Line Input*/
#define MICPD (1<<1)	/*Mic Input*/
#define ADCPD (1<<2)	/*ADC*/
#define DACPD (1<<3) 	/*DAC*/
#define OUTPD (1<<4) 	/*Outputs*/
#define OSCPD (1<<5) 	/*Internal Oscillator*/
#define CLKOUTPD (1<<6)	/*Clock Out*/
#define POWEROFF (1<<7)	/*Power off*/

//Reg 7: Digital Audio Interface Format
#define format_MSB_Right (0<<0)
#define format_MSB_Left	(1<<0)
#define format_I2S (2<<0)
#define format_DSP (3<<0)
#define format_16b (0<<2)
#define format_20b (1<<2)
#define format_24b (2<<2)
#define format_32b (3<<2)
#define LRPhase (1<<4) /*DACLRC Phase Control*/
#define LRSWAP (1<<5) /*DAC Left Right Clock Swap*/
#define Master_Mode (1<<6)
#define Slave_Mode (0<<6)
#define BCLKINV (1<<7) /*Bit Clock Invert*/

//Reg 8: Sampling Control
#define USB_MODE (1<<0)
#define NO_USB_MODE (0<<0)
#define BOSR (1<<1) /*Base Oversample Rate, see datasheet*/
#define SR0 (1<<2) /*Sample Rate Selectors*/
#define SR1 (1<<3)
#define SR2 (1<<4)
#define SR3 (1<<5)
#define CLKIDIV2 (1<<6) /*Core Clock is MCLK input divided by 2*/
#define CLKODIV2 (1<<7) /*Clock output is core clock divided by 2*/

//Reg 9: Active Control
#define I_ACTIVE 1
#define I_INACTIVE 0

// Oddness:
// format_I2S does not work with I2S2 on the STM32F427Z (works on the 427V) in Master TX mode (I2S2ext is RX)
// The RX data is shifted left 2 bits (x4) as it comes in, causing digital wrap-around clipping.
// Using format_MSB_Left works (I2S periph has to be set up I2S_Standard = I2S_Standard_LSB or I2S_Standard_MSB).
// Also, format_MSB_Right does not seem to work at all (with the I2S set to LSB or MSB)


const uint16_t w8731_init_data_slave[] =
{
	VOL_0dB,			// Reg 00: Left Line In (0dB, mute off)
	VOL_0dB,			// Reg 01: Right Line In (0dB, mute off)
	0b0101111,			// Reg 02: Left Headphone out (Mute)
	0b0101111,			// Reg 03: Right Headphone out (Mute)

	(MUTEMIC 			// Reg 04: Analog Audio Path Control (maximum attenuation on sidetone, sidetone disabled, DAC selected, Mute Mic, no bypass)
	| INSEL_line
	| DACSEL
	| SIDEATT_neg15dB),

	(DEEMP_Disable
	| ADCHPFEnable),	// Reg 05: Digital Audio Path Control: HPF, De-emp at 48kHz on DAC, do not soft mute dac

	(MICPD
	| OSCPD
	| CLKOUTPD),		// Reg 06: Power Down Control (Osc, Mic Off)

	(format_16b
	| format_MSB_Left
	| Slave_Mode),		// Reg 07: Digital Audio Interface Format (16-bit, slave)

	(0),				// Reg 08: Sampling Control (Normal, 256x, 48k ADC/DAC)
	0x001				// Reg 09: Active Control
};

const uint16_t w8731_init_data_master[] =
{
	VOL_0dB,			// Reg 00: Left Line In (0dB, mute off)
	VOL_0dB,			// Reg 01: Right Line In (0dB, mute off)
	0b0101111,			// Reg 02: Left Headphone out (Mute)
	0b0101111,			// Reg 03: Right Headphone out (Mute)

	(MUTEMIC 			// Reg 04: Analog Audio Path Control (maximum attenuation on sidetone, sidetone disabled, DAC selected, Mute Mic, no bypass)
	| INSEL_line
	| DACSEL
	| SIDEATT_neg6dB),

	(DEEMP_48k
	| ADCHPFEnable),	// Reg 05: Digital Audio Path Control: HPF, De-emp at 48kHz on DAC, do not soft mute dac

	(MICPD | OSCPD),	// Reg 06: Power Down Control (Osc, Mic Off)

	(format_16b
	| format_MSB_Left
	| Master_Mode),		// Reg 07: Digital Audio Interface Format (16-bit, master)

	0x000,				// Reg 08: Sampling Control (Normal, 256x, 48k ADC/DAC)
	0x001				// Reg 09: Active Control
};


/* The 7 bits Codec address (sent through I2C interface) */
#define CODEC_ADDRESS           (W8731_ADDR_0<<1)

/* local vars */
__IO uint32_t  CODECTimeout = CODEC_LONG_TIMEOUT;   



#ifdef USE_DEFAULT_TIMEOUT_CALLBACK

uint32_t Codec_TIMEOUT_UserCallback(void)
{
	/* Block communication and all processes */
	while (1)
	{   
	}
}
#else
uint32_t Codec_TIMEOUT_UserCallback(void)
{
	return 1;
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */


uint32_t Codec_Init(uint32_t AudioFreq)
{
	uint32_t err = 0;

	Codec_GPIO_Init();   

	Codec_CtrlInterface_Init();

	//init_i2s_clkin();

	//SWAP 1: Norm 1 2, Swapped 2 1
	err=Codec_Reset(CODEC_I2C1, CODECA_MODE);
	err=Codec_Reset(CODEC_I2C2, CODECB_MODE);

	Codec_AudioInterface_Init(AudioFreq);

	return err;
}



uint32_t Codec_Reset(I2C_TypeDef *CODEC, uint8_t master_slave)
{
	uint8_t i;
	uint32_t err=0;
	
	err=Codec_WriteRegister(0b1111, 0, CODEC); //Reset Device by writing 0 to register 0b1111
	
	err+=Codec_WriteRegister(0b1001, 0, CODEC); //inactivate device

	if (master_slave==CODEC_IS_MASTER)
	{
			//for(i=0;i<W8731_NUM_REGS;i++)
			//err+=Codec_WriteRegister(i, w8731_init_data_master[i], CODEC);
		/*
			WM8731_REG_INTERFACE, 0x42); // I2S, 16 bit, MCLK master
			write(WM8731_REG_SAMPLING, 0x20);  // 256*Fs, 44.1 kHz, MCLK/1

			// In order to prevent pops, the DAC should first be soft-muted (DACMU),
			// the output should then be de-selected from the line and headphone output
			// (DACSEL), then the DAC powered down (DACPD).

			write(WM8731_REG_DIGITAL, 0x08);   // DAC soft mute
			write(WM8731_REG_ANALOG, 0x00);    // disable all

			write(WM8731_REG_POWERDOWN, 0x00); // codec powerdown

			write(WM8731_REG_LHEADOUT, 0x80);      // volume off
			write(WM8731_REG_RHEADOUT, 0x80);

			delay(100); // how long to power up?

			write(WM8731_REG_ACTIVE, 1);
			delay(5);
			write(WM8731_REG_DIGITAL, 0x00);   // DAC unmuted
			write(WM8731_REG_ANALOG, 0x10);    // DAC selected
*/


		err+=Codec_WriteRegister(WM8731_REG_INTERFACE, w8731_init_data_master[WM8731_REG_INTERFACE], CODEC);
		err+=Codec_WriteRegister(WM8731_REG_SAMPLING, w8731_init_data_master[WM8731_REG_SAMPLING], CODEC);

		err+=Codec_WriteRegister(WM8731_REG_DIGITAL, DACMUTE, CODEC); // DAC soft mute
		err+=Codec_WriteRegister(WM8731_REG_ANALOG, 0x00, CODEC); // disable all

		err+=Codec_WriteRegister(WM8731_REG_POWERDOWN, 0x00, CODEC); //codec power down

		err+=Codec_WriteRegister(WM8731_REG_LLINEIN, INMUTE, CODEC); //mute line in
		err+=Codec_WriteRegister(WM8731_REG_RLINEIN, INMUTE, CODEC);

		delay();
		delay();

		err+=Codec_WriteRegister(WM8731_REG_ACTIVE, I_ACTIVE, CODEC);

		delay();

		err+=Codec_WriteRegister(WM8731_REG_DIGITAL, w8731_init_data_master[WM8731_REG_DIGITAL], CODEC);
		err+=Codec_WriteRegister(WM8731_REG_ANALOG, w8731_init_data_master[WM8731_REG_ANALOG], CODEC);

		err+=Codec_WriteRegister(WM8731_REG_LLINEIN, VOL_0dB, CODEC); //unmute line in
		err+=Codec_WriteRegister(WM8731_REG_RLINEIN, VOL_0dB, CODEC);


	}
	else
	{

		for(i=0;i<W8731_NUM_REGS;i++)
			err+=Codec_WriteRegister(i, w8731_init_data_slave[i], CODEC);

		err+=Codec_WriteRegister(0b1001, 1, CODEC); //set active interface bit to activate device

	}


	return err;
}


uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue, I2C_TypeDef *CODEC)
{
	uint32_t result = 0;
	
	/* Assemble 2-byte data in WM8731 format */
	uint8_t Byte1 = ((RegisterAddr<<1)&0xFE) | ((RegisterValue>>8)&0x01);
	uint8_t Byte2 = RegisterValue&0xFF;
	
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

	/* Enable the CODEC_I2C peripheral clock */
	RCC_APB1PeriphClockCmd(CODEC_I2C2_CLK, ENABLE);
	RCC_APB1PeriphClockCmd(CODEC_I2C1_CLK, ENABLE);

	/* CODEC_I2C peripheral configuration */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x33;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
	
	/* Enable the I2C peripheral */
	I2C_DeInit(CODEC_I2C2);
	I2C_Init(CODEC_I2C2, &I2C_InitStructure);
	I2C_Cmd(CODEC_I2C2, ENABLE);


	/* CODEC_I2C peripheral configuration */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x33;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;

	I2C_DeInit(CODEC_I2C1);
	I2C_Init(CODEC_I2C1, &I2C_InitStructure);
	I2C_Cmd(CODEC_I2C1, ENABLE);


}

/*
 * Initializes I2S2 and I2S3 peripherals on the STM32
 */
void Codec_AudioInterface_Init(uint32_t AudioFreq)
{
	I2S_InitTypeDef I2S_InitStructure;


	//CODEC B: Right channel of DLD (I2S2, I2C2)
	/* Enable the CODEC_I2S peripheral clock */
	RCC_APB1PeriphClockCmd(CODEC_I2S2_CLK, ENABLE);

	/* CODEC_I2S peripheral configuration for master TX */
	SPI_I2S_DeInit(CODEC_I2S2);
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16bextended;//extended;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;


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
	I2S_Init(CODEC_I2S2, &I2S_InitStructure);

	/* Initialize the I2S extended channel for RX */
	I2S_FullDuplexConfig(CODEC_I2S2_EXT, &I2S_InitStructure);

	I2S_Cmd(CODEC_I2S2, ENABLE);
	I2S_Cmd(CODEC_I2S2_EXT, ENABLE);


	// CODEC A: DLD Left Channel
	// Enable the CODEC_I2S peripheral clock
	RCC_APB1PeriphClockCmd(CODEC_I2S3_CLK, ENABLE);

	// CODEC_I2S3 peripheral configuration for master TX
	SPI_I2S_DeInit(CODEC_I2S3);
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16bextended;//extended;
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
	I2S_Init(CODEC_I2S3, &I2S_InitStructure);

	// Initialize the I2S extended channel for RX
	I2S_FullDuplexConfig(CODEC_I2S3_EXT, &I2S_InitStructure);

	I2S_Cmd(CODEC_I2S3, ENABLE);
	I2S_Cmd(CODEC_I2S3_EXT, ENABLE);


}


void Codec_GPIO_Init(void)
{
	GPIO_InitTypeDef gpio;

	/* Enable I2S and I2C GPIO clocks */
	RCC_AHB1PeriphClockCmd(CODEC_I2C2_GPIO_CLOCK | CODEC_I2S2_GPIO_CLOCK, ENABLE);
	RCC_AHB1PeriphClockCmd(CODEC_I2C1_GPIO_CLOCK | CODEC_I2S3_GPIO_CLOCK, ENABLE);


	/* CODEC_I2C SCL and SDA pins configuration -------------------------------------*/
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_OType = GPIO_OType_OD;
	gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = CODEC_I2C2_SCL_PIN | CODEC_I2C2_SDA_PIN;	GPIO_Init(CODEC_I2C2_GPIO, &gpio);
	gpio.GPIO_Pin = CODEC_I2C1_SCL_PIN | CODEC_I2C1_SDA_PIN;	GPIO_Init(CODEC_I2C1_GPIO, &gpio);

	/* Connect pins to I2C peripheral */
	GPIO_PinAFConfig(CODEC_I2C1_GPIO, CODEC_I2C1_SCL_PINSRC, CODEC_I2C1_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2C1_GPIO, CODEC_I2C1_SDA_PINSRC, CODEC_I2C1_GPIO_AF);

	GPIO_PinAFConfig(CODEC_I2C2_GPIO, CODEC_I2C2_SCL_PINSRC, CODEC_I2C2_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2C2_GPIO, CODEC_I2C2_SDA_PINSRC, CODEC_I2C2_GPIO_AF);


	/* CODEC_I2S output pins configuration: WS, SCK SD0 SDI MCK pins ------------------*/
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = CODEC_I2S2_WS_PIN;	GPIO_Init(CODEC_I2S2_GPIO, &gpio);
	gpio.GPIO_Pin = CODEC_I2S2_SDI_PIN;	GPIO_Init(CODEC_I2S2_GPIO_SDI, &gpio);
	gpio.GPIO_Pin = CODEC_I2S2_SCK_PIN;	GPIO_Init(CODEC_I2S2_GPIO_CK, &gpio);
	gpio.GPIO_Pin = CODEC_I2S2_SDO_PIN;	GPIO_Init(CODEC_I2S2_GPIO_SD, &gpio);

	gpio.GPIO_Pin = CODEC_I2S3_WS_PIN;	GPIO_Init(CODEC_I2S3_GPIO_WS, &gpio);
	gpio.GPIO_Pin = CODEC_I2S3_SDI_PIN;	GPIO_Init(CODEC_I2S3_GPIO_SDI, &gpio);
	gpio.GPIO_Pin = CODEC_I2S3_SCK_PIN;	GPIO_Init(CODEC_I2S3_GPIO_SCK, &gpio);
	gpio.GPIO_Pin = CODEC_I2S3_SDO_PIN;	GPIO_Init(CODEC_I2S3_GPIO_SDO, &gpio);

	GPIO_PinAFConfig(CODEC_I2S2_GPIO, CODEC_I2S2_WS_PINSRC, CODEC_I2S2_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S2_GPIO_CK, CODEC_I2S2_SCK_PINSRC, CODEC_I2S2_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S2_GPIO_SD, CODEC_I2S2_SDO_PINSRC, CODEC_I2S2_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S2_GPIO_SDI, CODEC_I2S2_SDI_PINSRC, CODEC_I2S2ext_GPIO_AF);

	GPIO_PinAFConfig(CODEC_I2S3_GPIO_WS, CODEC_I2S3_WS_PINSRC, CODEC_I2S3_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S3_GPIO_SCK, CODEC_I2S3_SCK_PINSRC, CODEC_I2S3_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S3_GPIO_SDO, CODEC_I2S3_SDO_PINSRC, CODEC_I2S3_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S3_GPIO_SDI, CODEC_I2S3_SDI_PINSRC, CODEC_I2S3ext_GPIO_AF);

	//SWAP 2: norm A/3 2/B, swapped: 2/B A/3
	if (CODECA_MCLK_SRC==MCLK_SRC_STM){

		gpio.GPIO_Mode = GPIO_Mode_AF;
		gpio.GPIO_Speed = GPIO_Speed_100MHz;
		gpio.GPIO_OType = GPIO_OType_PP;
		gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

		gpio.GPIO_Pin = CODEC_I2S3_MCK_PIN; GPIO_Init(CODEC_I2S3_MCK_GPIO, &gpio);
		GPIO_PinAFConfig(CODEC_I2S3_MCK_GPIO, CODEC_I2S3_MCK_PINSRC, CODEC_I2S3_GPIO_AF);

	} else if (CODECA_MCLK_SRC==MCLK_SRC_EXTERNAL){

		gpio.GPIO_Mode = GPIO_Mode_IN;
		gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

		gpio.GPIO_Pin = CODEC_I2S3_MCK_PIN; GPIO_Init(CODEC_I2S3_MCK_GPIO, &gpio);

	}

	if (CODECB_MCLK_SRC==MCLK_SRC_STM){
		gpio.GPIO_Mode = GPIO_Mode_AF;
		gpio.GPIO_Speed = GPIO_Speed_100MHz;
		gpio.GPIO_OType = GPIO_OType_PP;
		gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

		gpio.GPIO_Pin = CODEC_I2S2_MCK_PIN; GPIO_Init(CODEC_I2S2_MCK_GPIO, &gpio);
		GPIO_PinAFConfig(CODEC_I2S2_MCK_GPIO, CODEC_I2S2_MCK_PINSRC, CODEC_I2S2_GPIO_AF);

	} else if (CODECB_MCLK_SRC==MCLK_SRC_EXTERNAL){

		gpio.GPIO_Mode = GPIO_Mode_IN;
		gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

		gpio.GPIO_Pin = CODEC_I2S2_MCK_PIN; GPIO_Init(CODEC_I2S2_MCK_GPIO, &gpio);

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

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, CODEC_I2S2_GPIO_AF);

}
