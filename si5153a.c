#include "si5153a.h"
#include "globals.h"

si5351Config_t m_si5351Config;



/**************************************************************************/
/*!
  @brief  Sets the multiplier for the specified PLL using integer values

  @param  pll   The PLL to configure, which must be one of the following:
                - SI5351_PLL_A
                - SI5351_PLL_B
  @param  mult  The PLL integer multiplier (must be between 15 and 90)
*/
/**************************************************************************/
uint32_t setupPLLInt(si5351PLL_t pll, uint8_t mult)
{
  return setupPLL(pll, mult, 0, 1);
}

/**************************************************************************/
/*!
    @brief  Sets the multiplier for the specified PLL

    @param  pll   The PLL to configure, which must be one of the following:
                  - SI5351_PLL_A
                  - SI5351_PLL_B
    @param  mult  The PLL integer multiplier (must be between 15 and 90)
    @param  num   The 20-bit numerator for fractional output (0..1,048,575).
                  Set this to '0' for integer output.
    @param  denom The 20-bit denominator for fractional output (1..1,048,575).
                  Set this to '1' or higher to avoid divider by zero errors.

    @section PLL Configuration

    fVCO is the PLL output, and must be between 600..900MHz, where:

        fVCO = fXTAL * (a+(b/c))

    fXTAL = the crystal input frequency
    a     = an integer between 15 and 90
    b     = the fractional numerator (0..1,048,575)
    c     = the fractional denominator (1..1,048,575)

    NOTE: Try to use integers whenever possible to avoid clock jitter
    (only use the a part, setting b to '0' and c to '1').

    See: http://www.silabs.com/Support%20Documents/TechnicalDocs/AN619.pdf
*/
/**************************************************************************/
uint32_t setupPLL(si5351PLL_t pll, uint8_t mult, uint32_t num, uint32_t denom)
{
	uint32_t P1;		/* PLL config register P1 */
	uint32_t P2;		/* PLL config register P2 */
	uint32_t P3;		/* PLL config register P3 */
	uint8_t baseaddr;
	float fvco;

	/* Basic validation */
	if (!(m_si5351Config.initialised))
			return Si5153a_ERROR_DEVICENOTINITIALISED;

	if (!((mult > 14) && (mult < 91)		/* mult = 15..90 */
			&& (denom > 0)					/* Avoid divide by zero */
			&& (num <= 0xFFFFF)				/* 20-bit limit */
			&& (denom <= 0xFFFFF)			/* 20-bit limit */
	)) 		 return Si5153a_ERROR_INVALIDPARAMETER;


	/* Feedback Multisynth Divider Equation
	*
	* where: a = mult, b = num and c = denom
	*
	* P1 register is an 18-bit value using following formula:
	*
	* 	P1[17:0] = 128 * mult + floor(128*(num/denom)) - 512
	*
	* P2 register is a 20-bit value using the following formula:
	*
	* 	P2[19:0] = 128 * num - denom * floor(128*(num/denom))
	*
	* P3 register is a 20-bit value using the following formula:
	*
	* 	P3[19:0] = denom
	*/

	/* Set the main PLL config registers */
	if (num == 0) {
		/* Integer mode */
		P1 = 128 * mult - 512;
		P2 = num;
		P3 = denom;
	} else {
		/* Fractional mode */
		P1 = (uint32_t)(128 * mult + (uint32_t)(128 * ((float)num/(float)denom)) - 512);
		P2 = (uint32_t)(128 * num - denom * (uint32_t)(128 * ((float)num/(float)denom)));
		P3 = denom;
	}

	/* Get the appropriate starting point for the PLL registers */
	baseaddr = (pll == SI5351_PLL_A ? 26 : 34);

	/* The datasheet is a nightmare of typos and inconsistencies here! */
	I2C_WriteRegister( baseaddr,   (P3 & 0x0000FF00) >> 8);
	I2C_WriteRegister( baseaddr+1, (P3 & 0x000000FF));
	I2C_WriteRegister( baseaddr+2, (P1 & 0x00030000) >> 16);
	I2C_WriteRegister( baseaddr+3, (P1 & 0x0000FF00) >> 8);
	I2C_WriteRegister( baseaddr+4, (P1 & 0x000000FF));
	I2C_WriteRegister( baseaddr+5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16) );
	I2C_WriteRegister( baseaddr+6, (P2 & 0x0000FF00) >> 8);
	I2C_WriteRegister( baseaddr+7, (P2 & 0x000000FF));

	/* Reset both PLLs */
	I2C_WriteRegister(SI5351_REGISTER_177_PLL_RESET, (1<<7) | (1<<5) );

	/* Store the frequency settings for use with the Multisynth helper */
	if (pll == SI5351_PLL_A)
	{
		fvco = m_si5351Config.crystalFreq * (mult + ( (float)num / (float)denom ));
		m_si5351Config.plla_configured = 1;
		m_si5351Config.plla_freq = (uint32_t)fvco;
	}
	else
	{
		fvco = m_si5351Config.crystalFreq * (mult + ( (float)num / (float)denom ));
		m_si5351Config.pllb_configured = 1;
		m_si5351Config.pllb_freq = (uint32_t)fvco;
	}

	return Si5153a_ERROR_NONE;
}

/**************************************************************************/
/*!
    @brief  Configures the Multisynth divider using integer output.

    @param  output    The output channel to use (0..2)
    @param  pllSource	The PLL input source to use, which must be one of:
                      - SI5351_PLL_A
                      - SI5351_PLL_B
    @param  div       The integer divider for the Multisynth output,
                      which must be one of the following values:
                      - SI5351_MULTISYNTH_DIV_4
                      - SI5351_MULTISYNTH_DIV_6
                      - SI5351_MULTISYNTH_DIV_8
*/
/**************************************************************************/
uint32_t setupMultisynthInt(uint8_t output, si5351PLL_t pllSource, si5351MultisynthDiv_t div)
{
  return setupMultisynth(output, pllSource, div, 0, 1);
}


uint32_t setupRdiv(uint8_t output, si5351RDiv_t div)
{
	if(output < 3) return Si5153a_ERROR_INVALIDPARAMETER;  /* Channel range */

	uint8_t Rreg, regval;
	uint8_t divider;

	if (output == 0) Rreg = SI5351_REGISTER_44_MULTISYNTH0_PARAMETERS_3;
	if (output == 1) Rreg = SI5351_REGISTER_52_MULTISYNTH1_PARAMETERS_3;
	if (output == 2) Rreg = SI5351_REGISTER_60_MULTISYNTH2_PARAMETERS_3;

	//read8(Rreg, &regval); //is this necessary?
	//regval &= 0x0F;

	divider = div;
	divider &= 0x07;
	divider <<= 4;

	regval = divider;
	return I2C_WriteRegister(Rreg, regval);
}

/**************************************************************************/
/*!
    @brief  Configures the Multisynth divider, which determines the
            output clock frequency based on the specified PLL input.

    @param  output    The output channel to use (0..2)
    @param  pllSource	The PLL input source to use, which must be one of:
                      - SI5351_PLL_A
                      - SI5351_PLL_B
    @param  div       The integer divider for the Multisynth output.
                      If pure integer values are used, this value must
                      be one of:
                      - SI5351_MULTISYNTH_DIV_4
                      - SI5351_MULTISYNTH_DIV_6
                      - SI5351_MULTISYNTH_DIV_8
                      If fractional output is used, this value must be
                      between 8 and 900.
    @param  num       The 20-bit numerator for fractional output
                      (0..1,048,575). Set this to '0' for integer output.
    @param  denom     The 20-bit denominator for fractional output
                      (1..1,048,575). Set this to '1' or higher to
                      avoid divide by zero errors.

    @section Output Clock Configuration

    The multisynth dividers are applied to the specified PLL output,
    and are used to reduce the PLL output to a valid range (500kHz
    to 160MHz). The relationship can be seen in this formula, where
    fVCO is the PLL output frequency and MSx is the multisynth
    divider:

        fOUT = fVCO / MSx

    Valid multisynth dividers are 4, 6, or 8 when using integers,
    or any fractional values between 8 + 1/1,048,575 and 900 + 0/1

    The following formula is used for the fractional mode divider:

        a + b / c

    a = The integer value, which must be 4, 6 or 8 in integer mode (MSx_INT=1)
        or 8..900 in fractional mode (MSx_INT=0).
    b = The fractional numerator (0..1,048,575)
    c = The fractional denominator (1..1,048,575)

    @note   Try to use integers whenever possible to avoid clock jitter

    @note   For output frequencies > 150MHz, you must set the divider
            to 4 and adjust to PLL to generate the frequency (for example
            a PLL of 640 to generate a 160MHz output clock). This is not
            yet supported in the driver, which limits frequencies to
            500kHz .. 150MHz.

    @note   For frequencies below 500kHz (down to 8kHz) Rx_DIV must be
            used, but this isn't currently implemented in the driver.
*/
/**************************************************************************/
uint32_t setupMultisynth(uint8_t output, si5351PLL_t pllSource, uint32_t div, uint32_t num, uint32_t denom)
{
  uint32_t P1;       /* Multisynth config register P1 */
  uint32_t P2;	     /* Multisynth config register P2 */
  uint32_t P3;	     /* Multisynth config register P3 */

  uint8_t baseaddr;
  uint8_t clkControlReg;

  /* Basic validation */
	if (!(m_si5351Config.initialised))
			return Si5153a_ERROR_DEVICENOTINITIALISED;

	if (!((output < 3)               /* Channel range */
		&& (div > 3)                  /* Divider integer value */
		&& (div < 901) 				/* Divider integer value */
		&& (denom > 0)				/* Avoid divide by zero */
		&& (num <= 0xFFFFF) 			/* 20-bit limit */
		&& (denom <= 0xFFFFF)			/* 20-bit limit */
  ))		return Si5153a_ERROR_INVALIDPARAMETER;


  /* Make sure the requested PLL has been initialised */
  if (pllSource == SI5351_PLL_A)
  {
    if(!(m_si5351Config.plla_configured))
    	return Si5153a_ERROR_INVALIDPARAMETER;
  }
  else
  {
    if(!(m_si5351Config.pllb_configured))
    	return Si5153a_ERROR_INVALIDPARAMETER;
  }

  /* Output Multisynth Divider Equations
   *
   * where: a = div, b = num and c = denom
   *
   * P1 register is an 18-bit value using following formula:
   *
   * 	P1[17:0] = 128 * a + floor(128*(b/c)) - 512
   *
   * P2 register is a 20-bit value using the following formula:
   *
   * 	P2[19:0] = 128 * b - c * floor(128*(b/c))
   *
   * P3 register is a 20-bit value using the following formula:
   *
   * 	P3[19:0] = c
   */

  /* Set the main PLL config registers */
  if (num == 0)
  {
    /* Integer mode */
    P1 = 128 * div - 512;
    P2 = num;
    P3 = denom;
  }
  else
  {
    /* Fractional mode */
    P1 = (uint32_t)(128 * div + (uint32_t)(128 * ((float)num/(float)denom)) - 512);
    P2 = (uint32_t)(128 * num - denom * (uint32_t)(128 * ((float)num/(float)denom)));
    P3 = denom;
  }

  /* Get the appropriate starting point for the PLL registers */
  baseaddr = 0;
  switch (output)
  {
    case 0:
      baseaddr = SI5351_REGISTER_42_MULTISYNTH0_PARAMETERS_1;
      break;
    case 1:
      baseaddr = SI5351_REGISTER_50_MULTISYNTH1_PARAMETERS_1;
      break;
    case 2:
      baseaddr = SI5351_REGISTER_58_MULTISYNTH2_PARAMETERS_1;
      break;
  }

  /* Set the MSx config registers */
  I2C_WriteRegister( baseaddr,   (P3 & 0x0000FF00) >> 8);
  I2C_WriteRegister( baseaddr+1, (P3 & 0x000000FF));
  I2C_WriteRegister( baseaddr+2, (P1 & 0x00030000) >> 16);	/* ToDo: Add DIVBY4 (>150MHz) and R0 support (<500kHz) later */
  I2C_WriteRegister( baseaddr+3, (P1 & 0x0000FF00) >> 8);
  I2C_WriteRegister( baseaddr+4, (P1 & 0x000000FF));
  I2C_WriteRegister( baseaddr+5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16) );
  I2C_WriteRegister( baseaddr+6, (P2 & 0x0000FF00) >> 8);
  I2C_WriteRegister( baseaddr+7, (P2 & 0x000000FF));

  /* Configure the clk control and enable the output */
  clkControlReg = 0x0F;                             /* 8mA drive strength, MS0 as CLK0 source, Clock not inverted, powered up */
  if (pllSource == SI5351_PLL_B) clkControlReg |= (1 << 5); /* Uses PLLB */
  if (num == 0) clkControlReg |= (1 << 6);                  /* Integer mode */
  switch (output)
  {
    case 0:
    	I2C_WriteRegister(SI5351_REGISTER_16_CLK0_CONTROL, clkControlReg);
      break;
    case 1:
    	I2C_WriteRegister(SI5351_REGISTER_17_CLK1_CONTROL, clkControlReg);
      break;
    case 2:
    	I2C_WriteRegister(SI5351_REGISTER_18_CLK2_CONTROL, clkControlReg);
      break;
  }

  return Si5153a_ERROR_NONE;
}


/**************************************************************************/
/*!
    @brief  Enables or disables all clock outputs
*/
/**************************************************************************/
uint32_t Si5351a_enableOutputs(uint8_t enabled)
{
	/* Make sure we've called init first */
	if (!(m_si5351Config.initialised))
		  return Si5153a_ERROR_DEVICENOTINITIALISED;

	/* Enabled desired outputs (see Register 3) */
	I2C_WriteRegister(SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, enabled ? 0x00: 0xFF);

	return Si5153a_ERROR_NONE;
}


/**************************************************************************/
/*!
    @brief  Initialize the Si5153a VCXO
*/
/**************************************************************************/
void init_VCXO(void)
{
/*	GPIO_InitTypeDef gpio;

	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = Si5153A_SCL_PIN;	GPIO_Init(Si5153A_SCL_GPIO, &gpio);
	gpio.GPIO_Pin = Si5153A_SDA_PIN;	GPIO_Init(Si5153A_SDA_GPIO, &gpio);
*/

	init_Si5153a_gpio();

	init_Si5153a_i2c();

	init_Si5351aConfig();

	init_Si5351a_registers();

}

void init_Si5351a_registers(void)
{

	// Disable all outputs setting CLKx_DIS high
	I2C_WriteRegister(SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, 0xFF);

	// Power down all output drivers
	I2C_WriteRegister(SI5351_REGISTER_16_CLK0_CONTROL, 0x80);
	I2C_WriteRegister(SI5351_REGISTER_17_CLK1_CONTROL, 0x80);
	I2C_WriteRegister(SI5351_REGISTER_18_CLK2_CONTROL, 0x80);
	I2C_WriteRegister(SI5351_REGISTER_19_CLK3_CONTROL, 0x80);
	I2C_WriteRegister(SI5351_REGISTER_20_CLK4_CONTROL, 0x80);
	I2C_WriteRegister(SI5351_REGISTER_21_CLK5_CONTROL, 0x80);
	I2C_WriteRegister(SI5351_REGISTER_22_CLK6_CONTROL, 0x80);
	I2C_WriteRegister(SI5351_REGISTER_23_CLK7_CONTROL, 0x80);

	// Set the load capacitance for the XTAL
	I2C_WriteRegister(SI5351_REGISTER_183_CRYSTAL_INTERNAL_LOAD_CAPACITANCE, m_si5351Config.crystalLoad);

	// Set interrupt masks as required (see Register 2 description in AN619).
	// By default, ClockBuilder Desktop sets this register to 0x18.
	// Note that the least significant nibble must remain 0x8, but the most
	// significant nibble may be modified to suit your needs.

	// Reset the PLL config fields just in case we call init again */
	m_si5351Config.plla_configured = 0;
	m_si5351Config.plla_freq = 0;
	m_si5351Config.pllb_configured = 0;
	m_si5351Config.pllb_freq = 0;

	// All done!
	m_si5351Config.initialised = 1;

}

void init_Si5351aConfig(void)
{
	  m_si5351Config.initialised     = 0;
	  m_si5351Config.crystalFreq     = SI5351_CRYSTAL_FREQ_25MHZ;
	  m_si5351Config.crystalLoad     = SI5351_CRYSTAL_LOAD_10PF;
	  m_si5351Config.crystalPPM      = 30;
	  m_si5351Config.plla_configured = 0;
	  m_si5351Config.plla_freq       = 0;
	  m_si5351Config.pllb_configured = 0;
	  m_si5351Config.pllb_freq       = 0;
}


void init_Si5153a_i2c(void)
{
	I2C_InitTypeDef i2c;

	/* Enable the CODEC_I2C peripheral clock */
	RCC_APB1PeriphClockCmd(Si5153A_CLK, ENABLE);

	/* CODEC_I2C peripheral configuration */
	i2c.I2C_Mode = I2C_Mode_I2C;
	i2c.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c.I2C_OwnAddress1 = 0x33;
	i2c.I2C_Ack = I2C_Ack_Enable;
	i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	i2c.I2C_ClockSpeed = Si5153A_I2C_SPEED;

	/* Enable the I2C peripheral */
	I2C_DeInit(Si5153A_I2C);
	I2C_Init(Si5153A_I2C, &i2c);
	I2C_Cmd(Si5153A_I2C, ENABLE);
}

void init_Si5153a_gpio(void)
{
	GPIO_InitTypeDef gpio;

	RCC_AHB1PeriphClockCmd(Si5153A_GPIO_CLOCK, ENABLE);

	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_OType = GPIO_OType_OD;
	gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	gpio.GPIO_Pin = Si5153A_SCL_PIN;	GPIO_Init(Si5153A_SCL_GPIO, &gpio);
	gpio.GPIO_Pin = Si5153A_SDA_PIN;	GPIO_Init(Si5153A_SDA_GPIO, &gpio);

	GPIO_PinAFConfig(Si5153A_SCL_GPIO, Si5153A_SCL_PINSRC, Si5153A_GPIO_AF);
	GPIO_PinAFConfig(Si5153A_SDA_GPIO, Si5153A_SDA_PINSRC, Si5153A_GPIO_AF);

}


uint32_t I2C_TIMEOUT_UserCallback(void)
{
	/* nothing */
	return 1;
}

uint32_t  I2CTimeout = Si5153A_LONG_TIMEOUT;
uint32_t I2C_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue)
{
	uint32_t result = 0;

	// While the bus is busy
	I2CTimeout = Si5153A_LONG_TIMEOUT;
	while(I2C_GetFlagStatus(Si5153A_I2C, I2C_FLAG_BUSY))
	{
		if((I2CTimeout--) == 0)
			return I2C_TIMEOUT_UserCallback();
	}

	// Start the config sequence
	I2C_GenerateSTART(Si5153A_I2C, ENABLE);

	// Test on EV5 and clear it
	I2CTimeout = Si5153A_LONG_TIMEOUT;
	while (!I2C_CheckEvent(Si5153A_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((I2CTimeout--) == 0)
			return I2C_TIMEOUT_UserCallback();
	}

	// Transmit the slave address and enable writing operation
	I2C_Send7bitAddress(Si5153A_I2C, SI5351_ADDRESS, I2C_Direction_Transmitter);

	// Test on EV6 and clear it
	I2CTimeout = Si5153A_LONG_TIMEOUT;
	while (!I2C_CheckEvent(Si5153A_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((I2CTimeout--) == 0)
			return I2C_TIMEOUT_UserCallback();
	}

	// Transmit the first address for write operation
	I2C_SendData(Si5153A_I2C, RegisterAddr);

	// Test on EV8 and clear it
	I2CTimeout = Si5153A_LONG_TIMEOUT;
	while (!I2C_CheckEvent(Si5153A_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING))
	{
		if((I2CTimeout--) == 0)
			return I2C_TIMEOUT_UserCallback();
	}

	// Prepare the register value to be sent
	I2C_SendData(Si5153A_I2C, RegisterValue);

	// Wait till all data have been physically transferred on the bus
	I2CTimeout = Si5153A_LONG_TIMEOUT;
	while(!I2C_GetFlagStatus(Si5153A_I2C, I2C_FLAG_BTF))
	{
		if((I2CTimeout--) == 0)
			return I2C_TIMEOUT_UserCallback();
	}

	// End the configuration sequence
	I2C_GenerateSTOP(Si5153A_I2C, ENABLE);

	// Return the verifying value: 0 (Passed) or 1 (Failed)
	return result;
}
