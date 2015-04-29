/*
 * codec.c
 */

  
#include "codec.h"

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

#define INBOTH 8
#define INMUTE 7
#define INVOL 0

#define VOL_p12dB	0b11111 /*+12dB*/
#define VOL_0dB		0b10111 /*0dB*/
#define VOL_n1dB	0b10110 /*-1.5dB*/
#define VOL_n3dB	0b10101 /*-3dB*/
#define VOL_n6dB	0b10011 /*-6dB*/
#define VOL_n12dB	15 		/*-12dB*/
#define VOL_n15dB	13 		/*-15dB*/
/*1.5dB steps down to..*/
#define VOL_n34dB	0b00000 /*-34.5dB*/

#define MICBOOST 0
#define MUTEMIC 1
#define INSEL 2
#define BYPASS 3
#define DACSEL 4
#define SIDETONE 5
#define SIDEATT0 6
#define SIDEATT1 7

#define format_MSB_Right 0
#define format_MSB_Left 1
#define format_I2S 2
#define format_DSP 3

#define format_16b (0<<2)
#define format_20b (1<<2)
#define format_24b (2<<2)
#define format_32b (3<<2)

#define ADCHPFDisable 1
#define ADCHPFEnable 0

// Oddness:
// format_I2S does not work with I2S2 on the STM32F427Z (works on the 427V) in Master TX mode (I2S2ext is RX)
// The RX data is shifted left 2 bits (x4) as it comes in, causing digital wrap-around clipping.
// Using format_MSB_Left works (I2S periph has to be set up I2S_Standard_LSB or I2S_Standard_MSB).
// Also, format_MSB_Right does not seem to work at all (with the I2S set to LSB or MSB)

const uint16_t w8731_init_data[] = 
{
	VOL_0dB,			// Reg 00: Left Line In (0dB, mute off)
	VOL_0dB,			// Reg 01: Right Line In (0dB, mute off)
	0b0101111,			// Reg 02: Left Headphone out (Mute)
	0b0101111,			// Reg 03: Right Headphone out (Mute)
	0b00010010,			// Reg 04: Analog Audio Path Control (maximum attenuation on sidetone, sidetone disabled, DAC selected, Mute Mic, no bypass)
	(0b00000110 | ADCHPFEnable),			// Reg 05: Digital Audio Path Control: HPF, De-emp at 48kHz on DAC, do not soft mute dac
	0x062,				// Reg 06: Power Down Control (Clkout, Osc, Mic Off)
	(format_16b
	| format_MSB_Left),	// Reg 07: Digital Audio Interface Format (16-bit, slave)
	0x000,				// Reg 08: Sampling Control (Normal, 256x, 48k ADC/DAC)
	0x001				// Reg 09: Active Control
};

/* The 7 bits Codec address (sent through I2C interface) */
#define CODEC_ADDRESS           (W8731_ADDR_0<<1)

/* local vars */
__IO uint32_t  CODECTimeout = CODEC_LONG_TIMEOUT;   


/**
  * @brief  Inserts a delay time (not accurate timing).
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void Delay( __IO uint32_t nCount)
{	for (; nCount != 0; nCount--);
}

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
	/* nothing */
	return 1;
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */


uint32_t Codec_Init(uint32_t AudioFreq)
{
	uint32_t err = 0;

	Codec_GPIO_Init();   

	Codec_CtrlInterface_Init();

	//init_i2s_clkin();

	Codec_AudioInterface_Init(AudioFreq);  

	err=Codec_Reset(CODEC_I2C2);
	err=Codec_Reset(CODEC_I2C1);


	return err;
}


uint32_t Codec_Reset(I2C_TypeDef *CODEC)
{
	uint8_t i;
	uint32_t err=0;
	
	err=Codec_WriteRegister(0x0f, 0, CODEC);
	
	for(i=0;i<W8731_NUM_REGS;i++)
		err=Codec_WriteRegister(i, w8731_init_data[i], CODEC);

	err=Codec_WriteRegister(0b1001, 0, CODEC); //reset active interface bit
	err=Codec_WriteRegister(0b1001, 1, CODEC); //set active interface bit

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
 * Initializes I2S2 and I2S3 for both codecs
 */
void Codec_AudioInterface_Init(uint32_t AudioFreq)
{
	I2S_InitTypeDef I2S_InitStructure;


	/* Enable the CODEC_I2S peripheral clock */
	RCC_APB1PeriphClockCmd(CODEC_I2S2_CLK, ENABLE);

	/* CODEC_I2S peripheral configuration for master TX */
	SPI_I2S_DeInit(CODEC_I2S2);
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;//extended;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;

	/* Initialize the I2S main channel for TX */
	I2S_Init(CODEC_I2S2, &I2S_InitStructure);

	/* Initialize the I2S extended channel for RX */
	I2S_FullDuplexConfig(CODEC_I2S2_EXT, &I2S_InitStructure);



	// Enable the CODEC_I2S peripheral clock
	RCC_APB1PeriphClockCmd(CODEC_I2S3_CLK, ENABLE);

	// CODEC_I2S3 peripheral configuration for master TX
	SPI_I2S_DeInit(CODEC_I2S3);
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;//extended;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;

	// Initialize the I2S main channel for TX
	I2S_Init(CODEC_I2S3, &I2S_InitStructure);

	// Initialize the I2S extended channel for RX
	I2S_FullDuplexConfig(CODEC_I2S3_EXT, &I2S_InitStructure);



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
	gpio.GPIO_Speed = GPIO_Speed_25MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = CODEC_I2S2_WS_PIN;	GPIO_Init(CODEC_I2S2_GPIO, &gpio);
	gpio.GPIO_Pin = CODEC_I2S2_SDI_PIN;	GPIO_Init(CODEC_I2S2_GPIO_SDI, &gpio);
	gpio.GPIO_Pin = CODEC_I2S2_SCK_PIN;	GPIO_Init(CODEC_I2S2_GPIO_CK, &gpio);
	gpio.GPIO_Pin = CODEC_I2S2_SDO_PIN;	GPIO_Init(CODEC_I2S2_GPIO_SD, &gpio);
	gpio.GPIO_Pin = CODEC_I2S2_MCK_PIN; GPIO_Init(CODEC_I2S2_MCK_GPIO, &gpio);

	gpio.GPIO_Pin = CODEC_I2S3_WS_PIN;	GPIO_Init(CODEC_I2S3_GPIO, &gpio);
	gpio.GPIO_Pin = CODEC_I2S3_SDI_PIN;	GPIO_Init(CODEC_I2S3_GPIO_SDI, &gpio);
	gpio.GPIO_Pin = CODEC_I2S3_SCK_PIN;	GPIO_Init(CODEC_I2S3_GPIO_CK, &gpio);
	gpio.GPIO_Pin = CODEC_I2S3_SDO_PIN;	GPIO_Init(CODEC_I2S3_GPIO_SD, &gpio);
	gpio.GPIO_Pin = CODEC_I2S3_MCK_PIN; GPIO_Init(CODEC_I2S3_MCK_GPIO, &gpio);

	GPIO_PinAFConfig(CODEC_I2S2_GPIO, CODEC_I2S2_WS_PINSRC, CODEC_I2S2_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S2_GPIO_CK, CODEC_I2S2_SCK_PINSRC, CODEC_I2S2_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S2_GPIO_SD, CODEC_I2S2_SDO_PINSRC, CODEC_I2S2_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S2_GPIO_SDI, CODEC_I2S2_SDI_PINSRC, CODEC_I2S2ext_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S2_MCK_GPIO, CODEC_I2S2_MCK_PINSRC, CODEC_I2S2_GPIO_AF);

	GPIO_PinAFConfig(CODEC_I2S3_GPIO, CODEC_I2S3_WS_PINSRC, CODEC_I2S3_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S3_GPIO_CK, CODEC_I2S3_SCK_PINSRC, CODEC_I2S3_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S3_GPIO_SD, CODEC_I2S3_SDO_PINSRC, CODEC_I2S3_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S3_GPIO_SDI, CODEC_I2S3_SDI_PINSRC, CODEC_I2S3ext_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S3_MCK_GPIO, CODEC_I2S3_MCK_PINSRC, CODEC_I2S3_GPIO_AF);

}


/*
 * This must be called before Codec_AudioInterface_Init() because the I2S2CLKConfig must happen before APB1 for I2S2 periph is turned on
 *
 * Must uncomment the line in stm32f4xx_conf.h:
 * #define I2S_EXTERNAL_CLOCK_VAL   7000000
 */
void init_i2s_clkin(void){

	TIM_TimeBaseInitTypeDef tim;
	GPIO_InitTypeDef gpio;
	TIM_OCInitTypeDef  tim_oc;

	int i;

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
