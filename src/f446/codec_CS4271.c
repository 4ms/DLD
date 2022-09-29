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

SAI_HandleTypeDef hsai_BlockA1;
SAI_HandleTypeDef hsai_BlockB1;
SAI_HandleTypeDef hsai_BlockA2;
SAI_HandleTypeDef hsai_BlockB2;

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
	__HAL_RCC_I2C2_CLK_ENABLE();
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
}

void Codec_A_CtrlInterface_Init(void) {
	__HAL_RCC_I2C1_CLK_ENABLE();
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

#define PROTOCOL SAI_PROTOCOL_DATASIZE_16BITEXTENDED
#define STANDARD SAI_I2S_STANDARD

void Codec_B_AudioInterface_Init(uint32_t AudioFreq) {
	__HAL_RCC_SAI2_CLK_ENABLE();
	hsai_BlockA2.Instance = SAI2_Block_A;
	hsai_BlockA2.Init.AudioMode = SAI_MODESLAVE_TX;
	hsai_BlockA2.Init.Synchro = SAI_SYNCHRONOUS_EXT_SAI1;
	hsai_BlockA2.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
	hsai_BlockA2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
	hsai_BlockA2.Init.MonoStereoMode = SAI_STEREOMODE;
	hsai_BlockA2.Init.CompandingMode = SAI_NOCOMPANDING;
	hsai_BlockA2.Init.TriState = SAI_OUTPUT_NOTRELEASED;
	HAL_SAI_DeInit(&hsai_BlockA2);

	if (HAL_SAI_InitProtocol(&hsai_BlockA2, STANDARD, PROTOCOL, 2) != HAL_OK) {
		__BKPT();
	}

	hsai_BlockB2.Instance = SAI2_Block_B;
	hsai_BlockB2.Init.AudioMode = SAI_MODESLAVE_RX;
	hsai_BlockB2.Init.Synchro = SAI_SYNCHRONOUS;
	hsai_BlockB2.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
	hsai_BlockB2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
	hsai_BlockB2.Init.MonoStereoMode = SAI_STEREOMODE;
	hsai_BlockB2.Init.CompandingMode = SAI_NOCOMPANDING;
	hsai_BlockB2.Init.TriState = SAI_OUTPUT_NOTRELEASED;
	HAL_SAI_DeInit(&hsai_BlockB2);

	if (HAL_SAI_InitProtocol(&hsai_BlockB2, STANDARD, PROTOCOL, 2) != HAL_OK) {
		__BKPT();
	}
}

void Codec_A_AudioInterface_Init(uint32_t AudioFreq) {
	// CODEC A: DLD Left Channel

	__HAL_RCC_SAI1_CLK_ENABLE();

	/* Peripheral DMA init*/

	hsai_BlockA1.Instance = SAI1_Block_A;
	hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_TX;
	hsai_BlockA1.Init.Synchro = SAI_ASYNCHRONOUS;
	hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
	hsai_BlockA1.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
	hsai_BlockA1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
	hsai_BlockA1.Init.ClockSource = SAI_CLKSOURCE_NA;
	hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
	hsai_BlockA1.Init.SynchroExt = SAI_SYNCEXT_OUTBLOCKA_ENABLE;
	hsai_BlockA1.Init.MonoStereoMode = SAI_STEREOMODE;
	hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
	hsai_BlockA1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
	HAL_SAI_DeInit(&hsai_BlockA1);

	if (HAL_SAI_InitProtocol(&hsai_BlockA1, STANDARD, PROTOCOL, 2) != HAL_OK) {
		__BKPT();
	}

	hsai_BlockB1.Instance = SAI1_Block_B;
	hsai_BlockB1.Init.AudioMode = SAI_MODESLAVE_RX;
	hsai_BlockB1.Init.Synchro = SAI_SYNCHRONOUS;
	hsai_BlockB1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
	hsai_BlockB1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
	hsai_BlockB1.Init.SynchroExt = SAI_SYNCEXT_OUTBLOCKA_ENABLE;
	hsai_BlockB1.Init.MonoStereoMode = SAI_STEREOMODE;
	hsai_BlockB1.Init.CompandingMode = SAI_NOCOMPANDING;
	hsai_BlockB1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
	HAL_SAI_DeInit(&hsai_BlockB1);

	if (HAL_SAI_InitProtocol(&hsai_BlockB1, STANDARD, PROTOCOL, 2) != HAL_OK) {
		__BKPT();
	}
}

void Codec_GPIO_Init(void) {
	GPIO_InitTypeDef gpio;

	// PB9     ------> I2C1_SDA
	// PB8     ------> I2C1_SCL
	gpio.Pin = GPIO_PIN_9 | GPIO_PIN_8;
	gpio.Mode = GPIO_MODE_AF_OD;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &gpio);

	// PB10     ------> I2C2_SCL
	// PB11     ------> I2C2_SDA
	gpio.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	gpio.Mode = GPIO_MODE_AF_OD;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate = GPIO_AF4_I2C2;
	HAL_GPIO_Init(GPIOB, &gpio);

	// PE2     ------> SAI1_MCLK_A
	// PE4     ------> SAI1_FS_A
	// PE5     ------> SAI1_SCK_A
	// PE6     ------> SAI1_SD_A
	gpio.Pin = GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	gpio.Alternate = GPIO_AF6_SAI1;
	HAL_GPIO_Init(GPIOE, &gpio);

	// PE3     ------> SAI1_SD_B
	gpio.Pin = GPIO_PIN_3;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	gpio.Alternate = GPIO_AF6_SAI1;
	HAL_GPIO_Init(GPIOE, &gpio);

	// PD11     ------> SAI2_SD_A
	gpio.Pin = GPIO_PIN_11;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	gpio.Alternate = GPIO_AF10_SAI2;
	HAL_GPIO_Init(GPIOD, &gpio);

	// PG10     ------> SAI2_SD_B
	gpio.Pin = GPIO_PIN_10;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	gpio.Alternate = GPIO_AF10_SAI2;
	HAL_GPIO_Init(GPIOG, &gpio);
}
