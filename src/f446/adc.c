/*
 * adc.c - adc setup
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
#include "adc.h"
#include "globals.h"

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;

static uint32_t HAL_RCC_ADC12_CLK_ENABLED = 0;

// Definitions
typedef struct builtinAdcSetup {
	GPIO_TypeDef *gpio;
	uint16_t pin;
	uint32_t channel;
	uint32_t sample_time;
} builtinAdcSetup;
void ADC_Init(ADC_TypeDef *ADCx, uint16_t *adc_buffer, uint32_t num_channels, const builtinAdcSetup *adc_setup);

// Exported functions:
void Init_Pot_ADC(uint16_t *ADC_Buffer, uint8_t num_adcs) {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	const builtinAdcSetup pot_config[8] = {
		{.gpio = GPIOC, .pin = GPIO_PIN_1, .channel = ADC_CHANNEL_11, .sample_time = ADC_SAMPLETIME_144CYCLES}, //TIME1
		{.gpio = GPIOC, .pin = GPIO_PIN_2, .channel = ADC_CHANNEL_12, .sample_time = ADC_SAMPLETIME_144CYCLES}, //TIME2
		{.gpio = GPIOB, .pin = GPIO_PIN_0, .channel = ADC_CHANNEL_8, .sample_time = ADC_SAMPLETIME_144CYCLES},	//LVL1
		{.gpio = GPIOB, .pin = GPIO_PIN_1, .channel = ADC_CHANNEL_9, .sample_time = ADC_SAMPLETIME_144CYCLES},	//LVL2
		{.gpio = GPIOA, .pin = GPIO_PIN_5, .channel = ADC_CHANNEL_5, .sample_time = ADC_SAMPLETIME_144CYCLES},	//REGEN1
		{.gpio = GPIOA, .pin = GPIO_PIN_6, .channel = ADC_CHANNEL_6, .sample_time = ADC_SAMPLETIME_144CYCLES},	//REGEN2
		{.gpio = GPIOA, .pin = GPIO_PIN_7, .channel = ADC_CHANNEL_7, .sample_time = ADC_SAMPLETIME_144CYCLES},	//MIX1
		{.gpio = GPIOC, .pin = GPIO_PIN_3, .channel = ADC_CHANNEL_13, .sample_time = ADC_SAMPLETIME_144CYCLES}, //MIX2
	};
	ADC_Init(ADC1, ADC_Buffer, num_adcs, pot_config);
}

void Init_CV_ADC(uint16_t *ADC_Buffer, uint8_t num_adcs) {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	const builtinAdcSetup cv_config[6] = {
		{.gpio = GPIOA, .pin = GPIO_PIN_3, .channel = ADC_CHANNEL_3, .sample_time = ADC_SAMPLETIME_144CYCLES},	//TIME1
		{.gpio = GPIOA, .pin = GPIO_PIN_2, .channel = ADC_CHANNEL_2, .sample_time = ADC_SAMPLETIME_144CYCLES},	//TIME2
		{.gpio = GPIOC, .pin = GPIO_PIN_4, .channel = ADC_CHANNEL_14, .sample_time = ADC_SAMPLETIME_144CYCLES}, //LEVEL1
		{.gpio = GPIOA, .pin = GPIO_PIN_4, .channel = ADC_CHANNEL_4, .sample_time = ADC_SAMPLETIME_144CYCLES},	//LEVEL2
		{.gpio = GPIOA, .pin = GPIO_PIN_1, .channel = ADC_CHANNEL_1, .sample_time = ADC_SAMPLETIME_144CYCLES},	//REGEN1
		{.gpio = GPIOA, .pin = GPIO_PIN_0, .channel = ADC_CHANNEL_0, .sample_time = ADC_SAMPLETIME_144CYCLES},	//REGEN2
	};
	ADC_Init(ADC2, ADC_Buffer, num_adcs, cv_config);
}

void ADC_Init(ADC_TypeDef *ADCx, uint16_t *adc_buffer, uint32_t num_channels, const builtinAdcSetup *adc_setup) {
	GPIO_InitTypeDef gpio;
	HAL_StatusTypeDef err;
	uint8_t i;

	ADC_HandleTypeDef *hadc = ADCx == ADC1 ? &hadc1 : ADCx == ADC1 ? &hadc2 : NULL;
	if (!hadc)
		return;

	__HAL_RCC_DMA2_CLK_ENABLE();

	for (i = 0; i < num_channels; i++) {
		gpio.Pin = adc_setup[i].pin;
		gpio.Mode = GPIO_MODE_ANALOG;
		gpio.Pull = GPIO_NOPULL;
		// Assume all GPIO RCC are enabled at this point
		//mdrivlib::RCC_Enable::GPIO::enable(adc_setup[i].gpio);
		HAL_GPIO_Init(adc_setup[i].gpio, &gpio);
	}

	hadc->Instance = ADCx;
	hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
	hadc->Init.Resolution = ADC_RESOLUTION_12B;
	hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc->Init.ScanConvMode = ENABLE;
	hadc->Init.EOCSelection = ADC_EOC_SEQ_CONV;
	hadc->Init.ContinuousConvMode = ENABLE;
	hadc->Init.NbrOfConversion = num_channels;
	hadc->Init.DiscontinuousConvMode = DISABLE;
	hadc->Init.NbrOfDiscConversion = 0;
	hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc->Init.DMAContinuousRequests = ENABLE;
	err = HAL_ADC_Init(hadc);

	ADC_ChannelConfTypeDef sConfig;
	for (i = 0; i < num_channels; i++) {
		sConfig.Channel = adc_setup[i].channel;
		sConfig.Rank = i + 1;
		sConfig.SamplingTime = adc_setup[i].sample_time;
		err = HAL_ADC_ConfigChannel(hadc, &sConfig);
	}

	err = HAL_ADC_Start_DMA(hadc, (uint32_t *)adc_buffer, num_channels);
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle) {
	if (adcHandle->Instance == ADC1) {
		__HAL_RCC_ADC1_CLK_ENABLE();
		__HAL_RCC_DMA2_CLK_ENABLE();

		hdma_adc1.Instance = DMA2_Stream0;
		hdma_adc1.Init.Channel = DMA_CHANNEL_0;
		hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
		hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_adc1.Init.Mode = DMA_CIRCULAR;
		hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
		hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_adc1);

		__HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);
		// HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
		// HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
	}
	if (adcHandle->Instance == ADC2) {
		__HAL_RCC_ADC2_CLK_ENABLE();
		__HAL_RCC_DMA2_CLK_ENABLE();

		hdma_adc1.Instance = DMA2_Stream2;
		hdma_adc1.Init.Channel = DMA_CHANNEL_1;
		hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
		hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_adc1.Init.Mode = DMA_CIRCULAR;
		hdma_adc1.Init.Priority = DMA_PRIORITY_MEDIUM;
		hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_adc1);

		__HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);
		// HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
		// HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
	}
}

// void DMA2_Stream0_IRQHandler(void) {
// 	HAL_DMA_IRQHandler(&hdma_adc1);
// }

// void DMA2_Stream2_IRQHandler(void) {
// 	HAL_DMA_IRQHandler(&hdma_adc2);
// }

void Deinit_Pot_ADC(void) {
	__HAL_RCC_ADC1_CLK_DISABLE();
	__HAL_RCC_DMA2_CLK_DISABLE();
	HAL_DMA_DeInit(&hdma_adc1);
	HAL_ADC_DeInit(&hadc1);

	// ADC_Cmd(ADC1, DISABLE);
	// ADC_DMACmd(ADC1, DISABLE);
	// ADC_DMARequestAfterLastTransferCmd(ADC1, DISABLE);
	// DMA_Cmd(DMA2_Stream4, DISABLE);
}

void Deinit_CV_ADC(void) {
	__HAL_RCC_ADC2_CLK_DISABLE();
	__HAL_RCC_DMA2_CLK_DISABLE();
	HAL_DMA_DeInit(&hdma_adc2);
	HAL_ADC_DeInit(&hadc2);

	// ADC_Cmd(ADC3, DISABLE);
	// ADC_DMACmd(ADC3, DISABLE);
	// ADC_DMARequestAfterLastTransferCmd(ADC3, DISABLE);
	// DMA_Cmd(DMA2_Stream0, DISABLE);
}
