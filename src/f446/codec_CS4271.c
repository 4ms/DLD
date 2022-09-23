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
#include "codec_CS4271_regs.h"
#include "globals.h"
#include "i2s.h"

I2C_HandleTypeDef hal_i2c1;
I2C_HandleTypeDef hal_i2c2;

static volatile uint32_t CODECTimeout = CODEC_LONG_TIMEOUT;
static uint32_t Codec_TIMEOUT_UserCallback(void) {
	return 1;
}

void Codecs_Deinit(void) {
	GPIO_InitTypeDef gpio;

	CODECA_RESET_RCC_ENABLE();
	CODECB_RESET_RCC_ENABLE();

	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Pull = GPIO_NOPULL;

	gpio.Pin = CODECA_RESET_pin;
	HAL_GPIO_Init(CODECA_RESET_GPIO, &gpio);
	gpio.Pin = CODECB_RESET_pin;
	HAL_GPIO_Init(CODECB_RESET_GPIO, &gpio);

	CODECA_RESET_LOW;
	CODECB_RESET_LOW;
}

static uint32_t _check_errors(uint32_t retries) {
	if (retries == 0)
		return 0;
	if (HAL_I2C_GetError(&hal_i2c1) != HAL_I2C_ERROR_AF)
		return 0;
	return --retries;
}

uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue, I2C_TypeDef *CODEC) {
	I2C_HandleTypeDef *i2c;
	if (CODEC == hal_i2c1.Instance)
		i2c = &hal_i2c1;
	else if (CODEC == hal_i2c2.Instance)
		i2c = &hal_i2c2;

	uint32_t retries = 16;
	while (retries) {
		if (HAL_I2C_Mem_Write(
				i2c, CODEC_ADDRESS, RegisterAddr, I2C_MEMADD_SIZE_8BIT, &RegisterValue, 1, CODEC_FLAG_TIMEOUT) ==
			HAL_OK)
			return 0;
		retries = _check_errors(retries);
	}
	return Codec_TIMEOUT_UserCallback();
}

/*
 * Initializes I2C peripheral for both codecs
 */
void Codec_B_CtrlInterface_Init(void) {
	hal_i2c1.Instance = I2C2;
	hal_i2c1.Init.ClockSpeed = 50000;
	hal_i2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hal_i2c1.Init.OwnAddress1 = 0x33;
	hal_i2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hal_i2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hal_i2c1.Init.OwnAddress2 = 0;
	hal_i2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hal_i2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	HAL_I2C_DeInit(&hal_i2c1);
	HAL_I2C_Init(&hal_i2c1);
	__HAL_I2C_ENABLE(&hal_i2c1);
}

void Codec_A_CtrlInterface_Init(void) {
	hal_i2c1.Instance = I2C1;
	hal_i2c1.Init.ClockSpeed = 50000;
	hal_i2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hal_i2c1.Init.OwnAddress1 = 0x33;
	hal_i2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hal_i2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hal_i2c1.Init.OwnAddress2 = 0;
	hal_i2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hal_i2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	HAL_I2C_DeInit(&hal_i2c1);
	HAL_I2C_Init(&hal_i2c1);
	__HAL_I2C_ENABLE(&hal_i2c1);
}

void Codec_B_AudioInterface_Init(uint32_t AudioFreq) {
	// I2S_InitTypeDef I2S_InitStructure;

	//CODEC B: Right channel of DLD (I2S2, I2C2)

	/* Enable the CODEC_I2S peripheral clock */
	// RCC_APB1PeriphClockCmd(CODECB_I2S_CLK, ENABLE);

	/* CODEC_I2S peripheral configuration for master TX */
	// SPI_I2S_DeInit(CODECB_I2S);
	// I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	// I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	// I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_24b;
	// I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;

	// if (CODECB_MODE == CODEC_IS_MASTER)
	// 	I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveTx;
	// else
	// 	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;

	// if (CODECB_MODE == CODEC_IS_MASTER)
	// 	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
	// else {
	// 	if (CODECB_MCLK_SRC == MCLK_SRC_STM)
	// 		I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	// 	else
	// 		I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
	// }
	// /* Initialize the I2S main channel for TX */
	// I2S_Init(CODECB_I2S, &I2S_InitStructure);

	// /* Initialize the I2S extended channel for RX */
	// I2S_FullDuplexConfig(CODECB_I2S_EXT, &I2S_InitStructure);

	// I2S_Cmd(CODECB_I2S, ENABLE);
	// I2S_Cmd(CODECB_I2S_EXT, ENABLE);
}

void Codec_A_AudioInterface_Init(uint32_t AudioFreq) {
	// I2S_InitTypeDef I2S_InitStructure;

	// CODEC A: DLD Left Channel
	// Enable the CODEC_I2S peripheral clock
	// RCC_APB1PeriphClockCmd(CODECA_I2S_CLK, ENABLE);

	// CODECA_I2S peripheral configuration for master TX
	// SPI_I2S_DeInit(CODECA_I2S);
	// I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	// I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	// I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_24b;
	// I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;

	// if (CODECA_MODE == CODEC_IS_MASTER)
	// 	I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveTx;
	// else
	// 	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;

	// if (CODECA_MODE == CODEC_IS_MASTER)
	// 	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
	// else {
	// 	if (CODECA_MCLK_SRC == MCLK_SRC_STM)
	// 		I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	// 	else
	// 		I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
	// }

	// // Initialize the I2S main channel for TX
	// I2S_Init(CODECA_I2S, &I2S_InitStructure);

	// // Initialize the I2S extended channel for RX
	// I2S_FullDuplexConfig(CODECA_I2S_EXT, &I2S_InitStructure);

	// I2S_Cmd(CODECA_I2S, ENABLE);
	// I2S_Cmd(CODECA_I2S_EXT, ENABLE);
}

void Codec_GPIO_Init(void) {
	GPIO_InitTypeDef gpio;

	/* Enable I2S and I2C GPIO clocks */
	// RCC_AHB1PeriphClockCmd(CODECB_I2C_GPIO_CLOCK | CODECB_SAI_GPIO_CLOCK, ENABLE);
	// RCC_AHB1PeriphClockCmd(CODECA_I2C_GPIO_CLOCK | CODECA_SAI_GPIO_CLOCK, ENABLE);

	// /* CODEC_I2C SCL and SDA pins configuration -------------------------------------*/
	// gpio.GPIO_Mode = GPIO_Mode_AF;
	// gpio.GPIO_Speed = GPIO_Speed_50MHz;
	// gpio.GPIO_OType = GPIO_OType_OD;
	// gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	// gpio.GPIO_Pin = CODECB_I2C_SCL_PIN | CODECB_I2C_SDA_PIN;
	// GPIO_Init(CODECB_I2C_GPIO, &gpio);
	// gpio.GPIO_Pin = CODECA_I2C_SCL_PIN | CODECA_I2C_SDA_PIN;
	// GPIO_Init(CODECA_I2C_GPIO, &gpio);

	// /* Connect pins to I2C peripheral */
	// GPIO_PinAFConfig(CODECA_I2C_GPIO, CODECA_I2C_SCL_PINSRC, CODECA_I2C_GPIO_AF);
	// GPIO_PinAFConfig(CODECA_I2C_GPIO, CODECA_I2C_SDA_PINSRC, CODECA_I2C_GPIO_AF);

	// GPIO_PinAFConfig(CODECB_I2C_GPIO, CODECB_I2C_SCL_PINSRC, CODECB_I2C_GPIO_AF);
	// GPIO_PinAFConfig(CODECB_I2C_GPIO, CODECB_I2C_SDA_PINSRC, CODECB_I2C_GPIO_AF);

	// /* CODEC_I2S output pins configuration: WS, SCK SD0 SDI MCK pins ------------------*/
	// gpio.GPIO_Mode = GPIO_Mode_AF;
	// gpio.GPIO_Speed = GPIO_Speed_100MHz;
	// gpio.GPIO_OType = GPIO_OType_PP;
	// gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	// gpio.GPIO_Pin = CODECB_SAI_WS_PIN;
	// GPIO_Init(CODECB_SAI_GPIO_WS, &gpio);
	// gpio.GPIO_Pin = CODECB_SAI_SDI_PIN;
	// GPIO_Init(CODECB_SAI_GPIO_SDI, &gpio);
	// gpio.GPIO_Pin = CODECB_SAI_SCK_PIN;
	// GPIO_Init(CODECB_SAI_GPIO_SCK, &gpio);
	// gpio.GPIO_Pin = CODECB_SAI_SDO_PIN;
	// GPIO_Init(CODECB_SAI_GPIO_SDO, &gpio);

	// gpio.GPIO_Pin = CODECA_SAI_WS_PIN;
	// GPIO_Init(CODECA_SAI_GPIO_WS, &gpio);
	// gpio.GPIO_Pin = CODECA_SAI_SDI_PIN;
	// GPIO_Init(CODECA_SAI_GPIO_SDI, &gpio);
	// gpio.GPIO_Pin = CODECA_SAI_SCK_PIN;
	// GPIO_Init(CODECA_SAI_GPIO_SCK, &gpio);
	// gpio.GPIO_Pin = CODECA_SAI_SDO_PIN;
	// GPIO_Init(CODECA_SAI_GPIO_SDO, &gpio);

	// GPIO_PinAFConfig(CODECB_SAI_GPIO_WS, CODECB_SAI_WS_PINSRC, CODECB_SAI_GPIO_AF);
	// GPIO_PinAFConfig(CODECB_SAI_GPIO_SCK, CODECB_SAI_SCK_PINSRC, CODECB_SAI_GPIO_AF);
	// GPIO_PinAFConfig(CODECB_SAI_GPIO_SDO, CODECB_SAI_SDO_PINSRC, CODECB_SAI_GPIO_AF);
	// GPIO_PinAFConfig(CODECB_SAI_GPIO_SDI, CODECB_SAI_SDI_PINSRC, CODECB_I2Sext_GPIO_AF);

	// GPIO_PinAFConfig(CODECA_SAI_GPIO_WS, CODECA_SAI_WS_PINSRC, CODECA_SAI_GPIO_AF);
	// GPIO_PinAFConfig(CODECA_SAI_GPIO_SCK, CODECA_SAI_SCK_PINSRC, CODECA_SAI_GPIO_AF);
	// GPIO_PinAFConfig(CODECA_SAI_GPIO_SDO, CODECA_SAI_SDO_PINSRC, CODECA_SAI_GPIO_AF);
	// GPIO_PinAFConfig(CODECA_SAI_GPIO_SDI, CODECA_SAI_SDI_PINSRC, CODECA_I2Sext_GPIO_AF);

	// if (CODECA_MCLK_SRC == MCLK_SRC_STM) {

	// 	gpio.GPIO_Mode = GPIO_Mode_AF;
	// 	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	// 	gpio.GPIO_OType = GPIO_OType_PP;
	// 	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	// 	gpio.GPIO_Pin = CODECA_SAI_MCK_PIN;
	// 	GPIO_Init(CODECA_SAI_MCK_GPIO, &gpio);
	// 	GPIO_PinAFConfig(CODECA_SAI_MCK_GPIO, CODECA_SAI_MCK_PINSRC, CODECA_SAI_GPIO_AF);

	// } else if (CODECA_MCLK_SRC == MCLK_SRC_EXTERNAL) {

	// 	gpio.GPIO_Mode = GPIO_Mode_IN;
	// 	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	// 	gpio.GPIO_Pin = CODECA_SAI_MCK_PIN;
	// 	GPIO_Init(CODECA_SAI_MCK_GPIO, &gpio);
	// }

	// if (CODECB_MCLK_SRC == MCLK_SRC_STM) {
	// 	gpio.GPIO_Mode = GPIO_Mode_AF;
	// 	gpio.GPIO_Speed = GPIO_Speed_100MHz;
	// 	gpio.GPIO_OType = GPIO_OType_PP;
	// 	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	// 	gpio.GPIO_Pin = CODECB_SAI_MCK_PIN;
	// 	GPIO_Init(CODECB_SAI_MCK_GPIO, &gpio);
	// 	GPIO_PinAFConfig(CODECB_SAI_MCK_GPIO, CODECB_SAI_MCK_PINSRC, CODECB_SAI_GPIO_AF);

	// } else if (CODECB_MCLK_SRC == MCLK_SRC_EXTERNAL) {

	// 	gpio.GPIO_Mode = GPIO_Mode_IN;
	// 	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	// 	gpio.GPIO_Pin = CODECB_SAI_MCK_PIN;
	// 	GPIO_Init(CODECB_SAI_MCK_GPIO, &gpio);
	// }
}
