/*
 * i2s.c - i2s and DMA setup and interrupts
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

#include "globals.h"
#include "i2s.h"
#include "stm32f4xx.h"

#include "codec_CS4271.h"
#include "looping_delay.h"
#include "timers.h"

extern SAI_HandleTypeDef hsai_BlockA1;
extern SAI_HandleTypeDef hsai_BlockB1;
extern SAI_HandleTypeDef hsai_BlockA2;
extern SAI_HandleTypeDef hsai_BlockB2;
DMA_HandleTypeDef hdma_sai1_a;
DMA_HandleTypeDef hdma_sai1_b;
DMA_HandleTypeDef hdma_sai2_a;
DMA_HandleTypeDef hdma_sai2_b;

volatile int16_t ch1tx_buffer[codec_BUFF_LEN];
volatile int16_t ch1rx_buffer[codec_BUFF_LEN];

volatile int16_t ch2tx_buffer[codec_BUFF_LEN];
volatile int16_t ch2rx_buffer[codec_BUFF_LEN];

uint32_t ch1tx_buffer_start, ch1rx_buffer_start;
uint32_t ch2tx_buffer_start, ch2rx_buffer_start;

static void (*audio_callback)(int16_t *, int16_t *, uint16_t, uint8_t);

void set_codec_callback(void (*cb)(int16_t *, int16_t *, uint16_t, uint8_t)) {
	audio_callback = cb;
}

void init_audio_dma(void) {
	// RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);
	// RCC_PLLI2SCmd(ENABLE);
	Init_I2SDMA_Channel2();
	Init_I2SDMA_Channel1();
}

void Start_I2SDMA_Channel1(void) {
	HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
}

void Start_I2SDMA_Channel2(void) {
	HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
}

void DeInit_I2S_Clock(void) {
	// RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);
	// RCC_PLLI2SCmd(DISABLE);
}

void DeInit_I2SDMA_Channel1(void) {
	// __HAL_RCC_SAI1_CLK_DISABLE();
	// __HAL_RCC_DMA2_CLK_DISABLE();
	// HAL_DMA_DeInit(&hdma_sai1_a);
	// HAL_DMA_DeInit(&hdma_sai1_b);
	// DMA_Cmd(AUDIO_I2S3_DMA_STREAM, DISABLE);
	// DMA_Cmd(AUDIO_I2S3_EXT_DMA_STREAM, DISABLE);
}

void DeInit_I2SDMA_Channel2(void) {
	// __HAL_RCC_SAI2_CLK_DISABLE();
	// __HAL_RCC_DMA2_CLK_DISABLE();
	// HAL_DMA_DeInit(&hdma_sai2_a);
	// HAL_DMA_DeInit(&hdma_sai2_b);
	// DMA_Cmd(CODECB_TX_DMA_STREAM, DISABLE);
	// DMA_Cmd(CODECB_RX_DMA_STREAM, DISABLE);
}

void HAL_SAI_MspInit(SAI_HandleTypeDef *saiHandle) {
	__HAL_RCC_DMA2_CLK_ENABLE();

	if (saiHandle->Instance == SAI1_Block_A) {
		hdma_sai1_a.Instance = DMA2_Stream1;
		hdma_sai1_a.Init.Channel = DMA_CHANNEL_0;
		hdma_sai1_a.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_sai1_a.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_sai1_a.Init.MemInc = DMA_MINC_ENABLE;
		hdma_sai1_a.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_sai1_a.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_sai1_a.Init.Mode = DMA_CIRCULAR;
		hdma_sai1_a.Init.Priority = DMA_PRIORITY_HIGH;
		hdma_sai1_a.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_DeInit(&hdma_sai1_a);

		if (HAL_DMA_Init(&hdma_sai1_a) != HAL_OK) {
			__BKPT();
		}

		// TODO: mdrivlib links before calling deinit
		__HAL_LINKDMA(saiHandle, hdmarx, hdma_sai1_a);
		__HAL_LINKDMA(saiHandle, hdmatx, hdma_sai1_a);
	}

	if (saiHandle->Instance == SAI1_Block_B) {
		hdma_sai1_b.Instance = DMA2_Stream5;
		hdma_sai1_b.Init.Channel = DMA_CHANNEL_0;
		hdma_sai1_b.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_sai1_b.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_sai1_b.Init.MemInc = DMA_MINC_ENABLE;
		hdma_sai1_b.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_sai1_b.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_sai1_b.Init.Mode = DMA_CIRCULAR;
		hdma_sai1_b.Init.Priority = DMA_PRIORITY_HIGH;
		hdma_sai1_b.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_DeInit(&hdma_sai1_b);

		if (HAL_DMA_Init(&hdma_sai1_b) != HAL_OK) {
			__BKPT();
		}

		// TODO: mdrivlib links before calling deinit
		__HAL_LINKDMA(saiHandle, hdmarx, hdma_sai1_b);
		__HAL_LINKDMA(saiHandle, hdmatx, hdma_sai1_b);
	}

	if (saiHandle->Instance == SAI2_Block_A) {
		hdma_sai2_a.Instance = DMA2_Stream4;
		hdma_sai2_a.Init.Channel = DMA_CHANNEL_3;
		hdma_sai2_a.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_sai2_a.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_sai2_a.Init.MemInc = DMA_MINC_ENABLE;
		hdma_sai2_a.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_sai2_a.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_sai2_a.Init.Mode = DMA_CIRCULAR;
		hdma_sai2_a.Init.Priority = DMA_PRIORITY_HIGH;
		hdma_sai2_a.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_DeInit(&hdma_sai2_a);

		if (HAL_DMA_Init(&hdma_sai2_a) != HAL_OK) {
			__BKPT();
		}
		__HAL_LINKDMA(saiHandle, hdmarx, hdma_sai2_a);
		__HAL_LINKDMA(saiHandle, hdmatx, hdma_sai2_a);
	}

	if (saiHandle->Instance == SAI2_Block_B) {
		hdma_sai2_b.Instance = DMA2_Stream6;
		hdma_sai2_b.Init.Channel = DMA_CHANNEL_3;
		hdma_sai2_b.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_sai2_b.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_sai2_b.Init.MemInc = DMA_MINC_ENABLE;
		hdma_sai2_b.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_sai2_b.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_sai2_b.Init.Mode = DMA_CIRCULAR;
		hdma_sai2_b.Init.Priority = DMA_PRIORITY_HIGH;
		hdma_sai2_b.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_DeInit(&hdma_sai2_b);

		if (HAL_DMA_Init(&hdma_sai2_b) != HAL_OK) {
			__BKPT();
		}
		__HAL_LINKDMA(saiHandle, hdmarx, hdma_sai2_b);
		__HAL_LINKDMA(saiHandle, hdmatx, hdma_sai2_b);
	}
}

void Init_I2SDMA_Channel1(void) {

	ch1tx_buffer_start = (uint32_t)&ch1tx_buffer;
	ch1rx_buffer_start = (uint32_t)&ch1rx_buffer;
	HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 1, 0);
	HAL_NVIC_DisableIRQ(DMA2_Stream5_IRQn);

	__HAL_DMA_ENABLE_IT(&hdma_sai1_b, DMA_IT_DME);
	__HAL_DMA_ENABLE_IT(&hdma_sai1_b, DMA_IT_FE);
	__HAL_DMA_ENABLE_IT(&hdma_sai1_b, DMA_IT_TE);

	HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)ch1tx_buffer, codec_BUFF_LEN);
	HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)ch1rx_buffer, codec_BUFF_LEN);
}

void Init_I2SDMA_Channel2(void) {
	ch2tx_buffer_start = (uint32_t)&ch2tx_buffer;
	ch2rx_buffer_start = (uint32_t)&ch2rx_buffer;
	HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 1, 1);
	HAL_NVIC_DisableIRQ(DMA2_Stream6_IRQn);

	//uint32_t Size = codec_BUFF_LEN;

	__HAL_DMA_ENABLE_IT(&hdma_sai2_b, DMA_IT_DME);
	__HAL_DMA_ENABLE_IT(&hdma_sai2_b, DMA_IT_FE);
	__HAL_DMA_ENABLE_IT(&hdma_sai2_b, DMA_IT_TE);

	HAL_SAI_Transmit_DMA(&hsai_BlockA2, (uint8_t *)ch2tx_buffer, codec_BUFF_LEN);
	HAL_SAI_Receive_DMA(&hsai_BlockB2, (uint8_t *)ch2rx_buffer, codec_BUFF_LEN);
}

// DMA interrupt for RX data CodecB (sai2b)
void DMA2_Stream6_IRQHandler(void) {
	int16_t *src, *dst;
	uint16_t sz;
	uint32_t err = 0;

	if (__HAL_DMA_GET_FLAG(&hdma_sai2_b, DMA_FLAG_TEIF2_6) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai2_b, DMA_FLAG_TEIF2_6);

	if (__HAL_DMA_GET_FLAG(&hdma_sai2_b, DMA_FLAG_DMEIF2_6) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai2_b, DMA_FLAG_DMEIF2_6);

	if (__HAL_DMA_GET_FLAG(&hdma_sai2_b, DMA_FLAG_FEIF2_6) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai2_b, DMA_FLAG_FEIF2_6);

	//  Transfer complete interrupt
	if (__HAL_DMA_GET_FLAG(&hdma_sai2_b, DMA_FLAG_TCIF2_6) != RESET) {
		sz = codec_BUFF_LEN / 2;
		src = (int16_t *)(ch2rx_buffer_start) + sz;
		dst = (int16_t *)(ch2tx_buffer_start) + sz;

		audio_callback(src, dst, sz / 2, 1); //channel2

		__HAL_DMA_CLEAR_FLAG(&hdma_sai2_b, DMA_FLAG_TCIF2_6);
	}

	//  Half Transfer complete interrupt
	if (__HAL_DMA_GET_FLAG(&hdma_sai2_b, DMA_FLAG_HTIF2_6) != RESET) {
		// Point to 1st half of buffers
		sz = codec_BUFF_LEN / 2;
		src = (int16_t *)(ch2rx_buffer_start);
		dst = (int16_t *)(ch2tx_buffer_start);

		audio_callback(src, dst, sz / 2, 1); //channel2

		__HAL_DMA_CLEAR_FLAG(&hdma_sai2_b, DMA_FLAG_HTIF2_6);
	}
}

// DMA interrupt for TX data CodecB (sai2a)
void DMA2_Stream4_IRQHandler(void) {
	if (__HAL_DMA_GET_FLAG(&hdma_sai2_a, DMA_FLAG_TEIF0_4) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai2_a, DMA_FLAG_TEIF0_4);

	if (__HAL_DMA_GET_FLAG(&hdma_sai2_a, DMA_FLAG_DMEIF0_4) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai2_a, DMA_FLAG_DMEIF0_4);

	if (__HAL_DMA_GET_FLAG(&hdma_sai2_a, DMA_FLAG_FEIF0_4) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai2_a, DMA_FLAG_FEIF0_4);
}

// DMA interrupt for RX data Codec A
void DMA2_Stream5_IRQHandler(void) {
	int16_t *src, *dst;
	uint16_t sz;

	if (__HAL_DMA_GET_FLAG(&hdma_sai1_b, DMA_FLAG_TEIF1_5) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai1_b, DMA_FLAG_TEIF1_5);

	if (__HAL_DMA_GET_FLAG(&hdma_sai1_b, DMA_FLAG_DMEIF1_5) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai1_b, DMA_FLAG_DMEIF1_5);

	if (__HAL_DMA_GET_FLAG(&hdma_sai1_b, DMA_FLAG_FEIF1_5) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai1_b, DMA_FLAG_FEIF1_5);

	//  Transfer complete interrupt
	if (__HAL_DMA_GET_FLAG(&hdma_sai1_b, DMA_FLAG_TCIF1_5) != RESET) {
		sz = codec_BUFF_LEN / 2;
		src = (int16_t *)(ch1rx_buffer_start) + sz;
		dst = (int16_t *)(ch1tx_buffer_start) + sz;

		audio_callback(src, dst, sz / 2, 0); //channel1

		__HAL_DMA_CLEAR_FLAG(&hdma_sai1_b, DMA_FLAG_TCIF1_5);
	}

	//  Half Transfer complete interrupt
	if (__HAL_DMA_GET_FLAG(&hdma_sai1_b, DMA_FLAG_HTIF1_5) != RESET) {
		// Point to 1st half of buffers
		sz = codec_BUFF_LEN / 2;
		src = (int16_t *)(ch1rx_buffer_start);
		dst = (int16_t *)(ch1tx_buffer_start);

		audio_callback(src, dst, sz / 2, 0); //channel1

		__HAL_DMA_CLEAR_FLAG(&hdma_sai1_b, DMA_FLAG_HTIF1_5);
	}
}

// DMA interrupt for TX data CodecA
void DMA2_Stream1_IRQHandler(void) {
	if (__HAL_DMA_GET_FLAG(&hdma_sai1_a, DMA_FLAG_TEIF1_5) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai1_a, DMA_FLAG_TEIF1_5);

	if (__HAL_DMA_GET_FLAG(&hdma_sai1_a, DMA_FLAG_DMEIF1_5) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai1_a, DMA_FLAG_DMEIF1_5);

	if (__HAL_DMA_GET_FLAG(&hdma_sai1_a, DMA_FLAG_FEIF1_5) != RESET)
		__HAL_DMA_CLEAR_FLAG(&hdma_sai1_a, DMA_FLAG_FEIF1_5);
}
