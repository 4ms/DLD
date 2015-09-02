/*
 * i2s.c - I2S+DMA routines
 *
 * */

#include "i2s.h"
#include "timekeeper.h"
#include "codec.h"
#include "dig_inouts.h"
#include "globals.h"

extern uint32_t g_error;


DMA_InitTypeDef dma_ch1tx, dma_ch1rx;
DMA_InitTypeDef dma_ch2tx, dma_ch2rx;

NVIC_InitTypeDef nvic_i2s2ext, nvic_i2s3ext, NVIC_InitStructure;


volatile int16_t ch1tx_buffer[codec_BUFF_LEN];
volatile int16_t ch1rx_buffer[codec_BUFF_LEN];

volatile int16_t ch2tx_buffer[codec_BUFF_LEN];
volatile int16_t ch2rx_buffer[codec_BUFF_LEN];

uint32_t ch1tx_buffer_start, ch1rx_buffer_start;
uint32_t ch2tx_buffer_start, ch2rx_buffer_start;


void Init_I2S_Channel1(void)
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

	/* Enable the I2S DMA request */
	SPI_I2S_DMACmd(CODEC_I2S3, SPI_I2S_DMAReq_Tx, ENABLE);

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
	nvic_i2s3ext.NVIC_IRQChannelPreemptionPriority = 2;
	nvic_i2s3ext.NVIC_IRQChannelSubPriority = 3;
	nvic_i2s3ext.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvic_i2s3ext);

	NVIC_EnableIRQ(AUDIO_I2S3_EXT_DMA_IRQ);

	SPI_I2S_DMACmd(CODEC_I2S3_EXT, SPI_I2S_DMAReq_Rx, ENABLE);

	ch1tx_buffer_start = (uint32_t)&ch1tx_buffer;
	ch1rx_buffer_start = (uint32_t)&ch1rx_buffer;

	DMA_Init(AUDIO_I2S3_DMA_STREAM, &dma_ch1tx);
	DMA_Init(AUDIO_I2S3_EXT_DMA_STREAM, &dma_ch1rx);

	DMA_Cmd(AUDIO_I2S3_DMA_STREAM, ENABLE);
	DMA_Cmd(AUDIO_I2S3_EXT_DMA_STREAM, ENABLE);

	I2S_Cmd(CODEC_I2S3, ENABLE);
	I2S_Cmd(CODEC_I2S3_EXT, ENABLE);

}

void Init_I2S_Channel2(void)
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

	/* Enable the I2S DMA request */
	SPI_I2S_DMACmd(CODEC_I2S2, SPI_I2S_DMAReq_Tx, ENABLE);


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
	nvic_i2s2ext.NVIC_IRQChannelPreemptionPriority = 2;
	nvic_i2s2ext.NVIC_IRQChannelSubPriority = 2;
	nvic_i2s2ext.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvic_i2s2ext);

	NVIC_EnableIRQ(AUDIO_I2S2_EXT_DMA_IRQ);

	SPI_I2S_DMACmd(CODEC_I2S2_EXT, SPI_I2S_DMAReq_Rx, ENABLE);

	ch2tx_buffer_start = (uint32_t)&ch2tx_buffer;
	ch2rx_buffer_start = (uint32_t)&ch2rx_buffer;

	DMA_Init(AUDIO_I2S2_DMA_STREAM, &dma_ch2tx);
	DMA_Init(AUDIO_I2S2_EXT_DMA_STREAM, &dma_ch2rx);

	DMA_Cmd(AUDIO_I2S2_DMA_STREAM, ENABLE);
	DMA_Cmd(AUDIO_I2S2_EXT_DMA_STREAM, ENABLE);

	I2S_Cmd(CODEC_I2S2, ENABLE);
	I2S_Cmd(CODEC_I2S2_EXT, ENABLE);

}

/**
  * I2S2 DMA interrupt for RX data
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

	/* Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_TC) != RESET)
	{
		/* Point to 2nd half of buffers */
		sz = codec_BUFF_LEN/2;
		src = (int16_t *)(ch2rx_buffer_start) +sz;
		dst = (int16_t *)(ch2tx_buffer_start) +sz;

		process_audio_block(src, dst, sz, 1); //channel2

		DMA_ClearFlag(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_TC);
	}

	/* Half Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_HT) != RESET)
	{
		/* Point to 1st half of buffers */
		sz = codec_BUFF_LEN/2;
		src = (int16_t *)(ch2rx_buffer_start);
		dst = (int16_t *)(ch2tx_buffer_start);

		/* Handle 1st half */
		process_audio_block(src, dst, sz, 1); //channel2

		/* Clear the Interrupt flag */
		DMA_ClearFlag(AUDIO_I2S2_EXT_DMA_STREAM, AUDIO_I2S2_EXT_DMA_FLAG_HT);
	}

}


/**
  * I2S3 DMA interrupt for RX data
  */
void DMA1_Stream2_IRQHandler(void)
{
	int16_t *src, *dst, sz;
	uint32_t err=0;
	uint32_t i;

	if (DMA_GetFlagStatus(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_FE) != RESET)
		err=AUDIO_I2S3_EXT_DMA_FLAG_FE;

	if (DMA_GetFlagStatus(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_TE) != RESET)
		err=AUDIO_I2S3_EXT_DMA_FLAG_TE;

	if (DMA_GetFlagStatus(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_DME) != RESET)
		err=AUDIO_I2S3_EXT_DMA_FLAG_DME;

	/* Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_TC) != RESET)
	{

		/* Point to 2nd half of buffers */
		sz = codec_BUFF_LEN/2;
		src = (int16_t *)(ch1rx_buffer_start) + sz;
		dst = (int16_t *)(ch1tx_buffer_start) + sz;

		process_audio_block(src, dst, sz, 0); //channel 1

		DMA_ClearFlag(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_TC);
	}

	/* Half Transfer complete interrupt */
	if (DMA_GetFlagStatus(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_HT) != RESET)
	{
		/* Point to 1st half of buffers */
		sz = codec_BUFF_LEN/2;
		src = (int16_t *)(ch1rx_buffer_start);
		dst = (int16_t *)(ch1tx_buffer_start);

		/* Handle 1st half */
		process_audio_block(src, dst, sz, 0); //channel 1

		/* Clear the Interrupt flag */
		DMA_ClearFlag(AUDIO_I2S3_EXT_DMA_STREAM, AUDIO_I2S3_EXT_DMA_FLAG_HT);
	}

}
