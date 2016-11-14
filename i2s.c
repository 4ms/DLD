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

#include "codec_CS4271.h"
#include "timekeeper.h"
#include "looping_delay.h"


DMA_InitTypeDef dma_ch1tx, dma_ch1rx;
DMA_InitTypeDef dma_ch2tx, dma_ch2rx;

NVIC_InitTypeDef nvic_i2s2ext, nvic_i2s3ext, NVIC_InitStructure;


volatile int16_t ch1tx_buffer[codec_BUFF_LEN];
volatile int16_t ch1rx_buffer[codec_BUFF_LEN];

volatile int16_t ch2tx_buffer[codec_BUFF_LEN];
volatile int16_t ch2rx_buffer[codec_BUFF_LEN];

uint32_t ch1tx_buffer_start, ch1rx_buffer_start;
uint32_t ch2tx_buffer_start, ch2rx_buffer_start;

void init_audio_dma(void)
{
	RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);
	RCC_PLLI2SCmd(ENABLE);

	//Codec_GPIO_Init();

	Init_I2SDMA_Channel2();
	Init_I2SDMA_Channel1();

}



void Start_I2SDMA_Channel1(void)
{
	NVIC_EnableIRQ(AUDIO_I2S3_EXT_DMA_IRQ);
}
void Start_I2SDMA_Channel2(void)
{
	NVIC_EnableIRQ(AUDIO_I2S2_EXT_DMA_IRQ);
}

void DeInit_I2S_Clock(void){

	RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);
	RCC_PLLI2SCmd(DISABLE);

}

void DeInit_I2SDMA_Channel1(void)
{
	RCC_AHB1PeriphClockCmd(AUDIO_I2S3_DMA_CLOCK, DISABLE);



	DMA_Cmd(AUDIO_I2S3_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S3_DMA_STREAM);

	DMA_Cmd(AUDIO_I2S3_EXT_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S3_EXT_DMA_STREAM);

}
void DeInit_I2SDMA_Channel2(void)
{
	RCC_AHB1PeriphClockCmd(AUDIO_I2S2_DMA_CLOCK, DISABLE);
	DMA_Cmd(AUDIO_I2S2_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S2_DMA_STREAM);

	DMA_Cmd(AUDIO_I2S2_EXT_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S2_EXT_DMA_STREAM);

}

void Init_I2SDMA_Channel1(void)
{
	uint32_t Size = codec_BUFF_LEN;

	/* Enable the DMA clock */
	RCC_AHB1PeriphClockCmd(AUDIO_I2S3_DMA_CLOCK, ENABLE);

	/* Configure the TX DMA Stream */
	DMA_Cmd(AUDIO_I2S3_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S3_DMA_STREAM);

	dma_ch1tx.DMA_Channel = AUDIO_I2S3_DMA_CHANNEL;
	dma_ch1tx.DMA_PeripheralBaseAddr = AUDIO_I2S3_DMA_DREG;
	dma_ch1tx.DMA_Memory0BaseAddr = (uint32_t)&ch1tx_buffer;
	dma_ch1tx.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	dma_ch1tx.DMA_BufferSize = (uint32_t)Size;
	dma_ch1tx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_ch1tx.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_ch1tx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dma_ch1tx.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	dma_ch1tx.DMA_Mode = DMA_Mode_Circular;
	dma_ch1tx.DMA_Priority = DMA_Priority_High;
	dma_ch1tx.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_ch1tx.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma_ch1tx.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma_ch1tx.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(AUDIO_I2S3_DMA_STREAM, &dma_ch1tx);

//Try TX error checking:
	DMA_ITConfig(AUDIO_I2S3_DMA_STREAM, DMA_IT_FE | DMA_IT_TE | DMA_IT_DME, ENABLE);

	/* Enable the I2S DMA request */
	SPI_I2S_DMACmd(CODECA_I2S, SPI_I2S_DMAReq_Tx, ENABLE);

	/* Configure the RX DMA Stream */
	DMA_Cmd(AUDIO_I2S3_EXT_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S3_EXT_DMA_STREAM);

	/* Set the parameters to be configured */
	dma_ch1rx.DMA_Channel = AUDIO_I2S3_EXT_DMA_CHANNEL;
	dma_ch1rx.DMA_PeripheralBaseAddr = AUDIO_I2S3_EXT_DMA_DREG;
	dma_ch1rx.DMA_Memory0BaseAddr = (uint32_t)&ch1rx_buffer;
	dma_ch1rx.DMA_DIR = DMA_DIR_PeripheralToMemory;
	dma_ch1rx.DMA_BufferSize = (uint32_t)Size;
	dma_ch1rx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_ch1rx.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_ch1rx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dma_ch1rx.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	dma_ch1rx.DMA_Mode = DMA_Mode_Circular;
	dma_ch1rx.DMA_Priority = DMA_Priority_High;
	dma_ch1rx.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_ch1rx.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma_ch1rx.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma_ch1rx.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(AUDIO_I2S3_EXT_DMA_STREAM, &dma_ch1rx);

	DMA_ITConfig(AUDIO_I2S3_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT | DMA_IT_FE | DMA_IT_TE | DMA_IT_DME, ENABLE);

	// I2S RX DMA IRQ Channel configuration
	nvic_i2s3ext.NVIC_IRQChannel = AUDIO_I2S3_EXT_DMA_IRQ;
	nvic_i2s3ext.NVIC_IRQChannelPreemptionPriority = 1; //was 2
	nvic_i2s3ext.NVIC_IRQChannelSubPriority = 0;
	nvic_i2s3ext.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvic_i2s3ext);

	//NVIC_EnableIRQ(AUDIO_I2S3_EXT_DMA_IRQ);
	NVIC_DisableIRQ(AUDIO_I2S3_EXT_DMA_IRQ);

	SPI_I2S_DMACmd(CODECA_I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);

	ch1tx_buffer_start = (uint32_t)&ch1tx_buffer;
	ch1rx_buffer_start = (uint32_t)&ch1rx_buffer;

	DMA_Init(AUDIO_I2S3_DMA_STREAM, &dma_ch1tx);
	DMA_Init(AUDIO_I2S3_EXT_DMA_STREAM, &dma_ch1rx);

	DMA_Cmd(AUDIO_I2S3_DMA_STREAM, ENABLE);
	DMA_Cmd(AUDIO_I2S3_EXT_DMA_STREAM, ENABLE);

	I2S_Cmd(CODECA_I2S, ENABLE);
	I2S_Cmd(CODECA_I2S_EXT, ENABLE);

}



void Init_I2SDMA_Channel2(void)
{
	uint32_t Size = codec_BUFF_LEN;

	/* Enable the DMA clock */
	RCC_AHB1PeriphClockCmd(AUDIO_I2S2_DMA_CLOCK, ENABLE);

	/* Configure the TX DMA Stream */
	DMA_Cmd(AUDIO_I2S2_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S2_DMA_STREAM);

	dma_ch2tx.DMA_Channel = AUDIO_I2S2_DMA_CHANNEL;
	dma_ch2tx.DMA_PeripheralBaseAddr = AUDIO_I2S2_DMA_DREG;
	dma_ch2tx.DMA_Memory0BaseAddr = (uint32_t)&ch2tx_buffer;
	dma_ch2tx.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	dma_ch2tx.DMA_BufferSize = (uint32_t)Size;
	dma_ch2tx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_ch2tx.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_ch2tx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dma_ch2tx.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	dma_ch2tx.DMA_Mode = DMA_Mode_Circular;
	dma_ch2tx.DMA_Priority = DMA_Priority_High;
	dma_ch2tx.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_ch2tx.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma_ch2tx.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma_ch2tx.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(AUDIO_I2S2_DMA_STREAM, &dma_ch2tx);

	//Try TX error checking:
	DMA_ITConfig(AUDIO_I2S2_DMA_STREAM, DMA_IT_FE | DMA_IT_TE | DMA_IT_DME, ENABLE);

	/* Enable the I2S DMA request */
	SPI_I2S_DMACmd(CODECB_I2S, SPI_I2S_DMAReq_Tx, ENABLE);


	/* Configure the RX DMA Stream */
	DMA_Cmd(AUDIO_I2S2_EXT_DMA_STREAM, DISABLE);
	DMA_DeInit(AUDIO_I2S2_EXT_DMA_STREAM);

	/* Set the parameters to be configured */
	dma_ch2rx.DMA_Channel = AUDIO_I2S2_EXT_DMA_CHANNEL;
	dma_ch2rx.DMA_PeripheralBaseAddr = AUDIO_I2S2_EXT_DMA_DREG;
	dma_ch2rx.DMA_Memory0BaseAddr = (uint32_t)&ch2rx_buffer;
	dma_ch2rx.DMA_DIR = DMA_DIR_PeripheralToMemory;
	dma_ch2rx.DMA_BufferSize = (uint32_t)Size;
	dma_ch2rx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma_ch2rx.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_ch2rx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dma_ch2rx.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	dma_ch2rx.DMA_Mode = DMA_Mode_Circular;
	dma_ch2rx.DMA_Priority = DMA_Priority_High;
	dma_ch2rx.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma_ch2rx.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma_ch2rx.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma_ch2rx.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(AUDIO_I2S2_EXT_DMA_STREAM, &dma_ch2rx);

	DMA_ITConfig(AUDIO_I2S2_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT | DMA_IT_FE | DMA_IT_TE | DMA_IT_DME, ENABLE);

	/* I2S RX DMA IRQ Channel configuration */

	nvic_i2s2ext.NVIC_IRQChannel = AUDIO_I2S2_EXT_DMA_IRQ;
	nvic_i2s2ext.NVIC_IRQChannelPreemptionPriority = 1; //was 2
	nvic_i2s2ext.NVIC_IRQChannelSubPriority = 1;
	nvic_i2s2ext.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvic_i2s2ext);

	//NVIC_EnableIRQ(AUDIO_I2S2_EXT_DMA_IRQ);
	NVIC_DisableIRQ(AUDIO_I2S2_EXT_DMA_IRQ);

	SPI_I2S_DMACmd(CODECB_I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);

	ch2tx_buffer_start = (uint32_t)&ch2tx_buffer;
	ch2rx_buffer_start = (uint32_t)&ch2rx_buffer;

	DMA_Init(AUDIO_I2S2_DMA_STREAM, &dma_ch2tx);
	DMA_Init(AUDIO_I2S2_EXT_DMA_STREAM, &dma_ch2rx);

	DMA_Cmd(AUDIO_I2S2_DMA_STREAM, ENABLE);
	DMA_Cmd(AUDIO_I2S2_EXT_DMA_STREAM, ENABLE);

	I2S_Cmd(CODECB_I2S, ENABLE);
	I2S_Cmd(CODECB_I2S_EXT, ENABLE);

}

/**
  * I2S2 DMA interrupt for RX data CodecB
  */
void DMA1_Stream3_IRQHandler(void)
{
	int16_t *src, *dst, sz;
	uint32_t err=0;

	if (DMA_GetFlagStatus(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_FE) != RESET)
		err=AUDIO_I2S2_EXT_DMA_FLAG_FE;

	if (DMA_GetFlagStatus(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_TE) != RESET)
		err=AUDIO_I2S2_EXT_DMA_FLAG_TE;

	if (DMA_GetFlagStatus(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_DME) != RESET)
		err=AUDIO_I2S2_EXT_DMA_FLAG_DME;

	if (err)
		err++; //debug breakpoint

	/* Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_TC) != RESET)
	{
		/* Point to 2nd half of buffers */
		sz = codec_BUFF_LEN/2;
		src = (int16_t *)(ch2rx_buffer_start) +sz;
		dst = (int16_t *)(ch2tx_buffer_start) +sz;

		process_audio_block_codec(src, dst, sz/2, 1); //channel2

		DMA_ClearFlag(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_TC);
	}

	/* Half Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_HT) != RESET)
	{
		/* Point to 1st half of buffers */
		sz = codec_BUFF_LEN/2;
		src = (int16_t *)(ch2rx_buffer_start);
		dst = (int16_t *)(ch2tx_buffer_start);

		process_audio_block_codec(src, dst, sz/2, 1); //channel2

		DMA_ClearFlag(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_HT);
	}

}

/*
 * I2S2 DMA interrupt for TX data CodecB
 */
void DMA1_Stream4_IRQHandler(void)
{
	uint32_t err=0;

	if (DMA_GetFlagStatus(AUDIO_I2S2_DMA_STREAM, AUDIO_I2S2_DMA_FLAG_FE) != RESET){
		err=AUDIO_I2S2_DMA_FLAG_FE;
		DMA_ClearFlag(AUDIO_I2S2_DMA_STREAM, AUDIO_I2S2_DMA_FLAG_FE);
	}

	if (DMA_GetFlagStatus(AUDIO_I2S2_DMA_STREAM, AUDIO_I2S2_DMA_FLAG_TE) != RESET){
		err=AUDIO_I2S2_DMA_FLAG_TE;
		DMA_ClearFlag(AUDIO_I2S2_DMA_STREAM, AUDIO_I2S2_DMA_FLAG_TE);
	}

	if (DMA_GetFlagStatus(AUDIO_I2S2_DMA_STREAM, AUDIO_I2S2_DMA_FLAG_DME) != RESET){
		err=AUDIO_I2S2_DMA_FLAG_DME;
		DMA_ClearFlag(AUDIO_I2S2_DMA_STREAM, AUDIO_I2S2_DMA_FLAG_DME);
	}
	if (err)
		err++; //debug breakpoint

}

/**
  * I2S3 DMA interrupt for RX data Codec A
  */
void DMA1_Stream2_IRQHandler(void)
{
	int16_t *src, *dst, sz;
	uint32_t err=0;
	//uint32_t i;

	if (DMA_GetFlagStatus(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_FE) != RESET)
		err=AUDIO_I2S3_EXT_DMA_FLAG_FE;

	if (DMA_GetFlagStatus(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_TE) != RESET)
		err=AUDIO_I2S3_EXT_DMA_FLAG_TE;

	if (DMA_GetFlagStatus(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_DME) != RESET)
		err=AUDIO_I2S3_EXT_DMA_FLAG_DME;

	if (err)
		err++; //debug breakpoint

	/* Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_TC) != RESET)
	{
		/* Point to 2nd half of buffers */
		sz = codec_BUFF_LEN/2;
		src = (int16_t *)(ch1rx_buffer_start) + sz;
		dst = (int16_t *)(ch1tx_buffer_start) + sz;

		process_audio_block_codec(src, dst, sz/2, 0); //channel 1

		DMA_ClearFlag(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_TC);
	}

	/* Half Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_HT) != RESET)
	{
		/* Point to 1st half of buffers */
		sz = codec_BUFF_LEN/2;
		src = (int16_t *)(ch1rx_buffer_start);
		dst = (int16_t *)(ch1tx_buffer_start);

		process_audio_block_codec(src, dst, sz/2, 0); //channel 1

		DMA_ClearFlag(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_HT);
	}
}

/*
 * I2S2 DMA interrupt for TX data CodecA
 */
void DMA1_Stream5_IRQHandler(void)
{
	uint32_t err=0;

	if (DMA_GetFlagStatus(AUDIO_I2S3_DMA_STREAM, AUDIO_I2S3_DMA_FLAG_FE) != RESET){
		err=AUDIO_I2S3_DMA_FLAG_FE;
		DMA_ClearFlag(AUDIO_I2S3_DMA_STREAM, AUDIO_I2S3_DMA_FLAG_FE);
	}

	if (DMA_GetFlagStatus(AUDIO_I2S3_DMA_STREAM, AUDIO_I2S3_DMA_FLAG_TE) != RESET){
		err=AUDIO_I2S3_DMA_FLAG_TE;
		DMA_ClearFlag(AUDIO_I2S3_DMA_STREAM, AUDIO_I2S3_DMA_FLAG_TE);
	}

	if (DMA_GetFlagStatus(AUDIO_I2S3_DMA_STREAM, AUDIO_I2S3_DMA_FLAG_DME) != RESET){
		err=AUDIO_I2S3_DMA_FLAG_DME;
		DMA_ClearFlag(AUDIO_I2S3_DMA_STREAM, AUDIO_I2S3_DMA_FLAG_DME);
	}

	if (err)
		err++; //debug breakpoint

}
