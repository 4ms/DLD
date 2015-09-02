/*
 * dac.c

 *
 *  Created on: Jul 28, 2014
 *      Author: design
 */

#include "globals.h"
#include "dig_inouts.h"
#include "dac.h"


#define INITIAL_PLAYBACK_PERIOD 1750
#define BUFF_LEN 64

uint16_t play_buffer1[BUFF_LEN];
uint16_t play_buffer2[BUFF_LEN];

uint16_t play2_buffer1[BUFF_LEN];
uint16_t play2_buffer2[BUFF_LEN];

uint32_t playback_timer_period1=INITIAL_PLAYBACK_PERIOD;
uint32_t playback_timer_period2=INITIAL_PLAYBACK_PERIOD;

extern uint32_t g_error;

extern __IO uint16_t adc_buffer[8];

DMA_InitTypeDef dac_dma1;
DAC_InitTypeDef dac1;
DMA_InitTypeDef dac_dma2;
DAC_InitTypeDef dac2;

/*
 * DAC
 * DAC1: PA4
 * DMA1 Stream 5 Channel 7
 *
 * DAC2: PA5 (MSByte)
 * DMA1 Stream 6 Channel 7
 *
 */
#define DAC2_DMA_Stream DMA1_Stream6
#define DAC2_DMA_Channel DMA_Channel_7
#define DAC2_DMA_IRQ DMA1_Stream6_IRQn
#define DAC2_DHR12L2_ADDRESS    0x40007418
#define DAC2_TIM TIM6


/*DAC channel2 12-bit left aligned data holding register*/
//#define DAC_DHR12L2_ADDRESS (DAC_BASE + 0x18)


void Audio_DAC2_Init(void){
	GPIO_InitTypeDef   gpio;
	TIM_TimeBaseInitTypeDef tim;
	NVIC_InitTypeDef nvic_play;


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_5;
	gpio.GPIO_Mode = GPIO_Mode_AN;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &gpio);

	TIM_TimeBaseStructInit(&tim);
	tim.TIM_Period = INITIAL_PLAYBACK_PERIOD; //  874 = 96kHz  from 84 MHz TIM6CLK (ie APB1 = HCLK/4, TIM6CLK = HCLK/2)
	tim.TIM_Prescaler = 0;
	tim.TIM_ClockDivision = 0;
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(DAC2_TIM, &tim);

	TIM_SelectOutputTrigger(DAC2_TIM, TIM_TRGOSource_Update);
	TIM_Cmd(DAC2_TIM, ENABLE);

	dac2.DAC_Trigger = DAC_Trigger_T6_TRGO;
	dac2.DAC_WaveGeneration = DAC_WaveGeneration_None;
	dac2.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_2, &dac2);

	DMA_DeInit(DMA1_Stream6);
	dac_dma2.DMA_Channel = DMA_Channel_7;
	dac_dma2.DMA_PeripheralBaseAddr = (uint32_t)DAC2_DHR12L2_ADDRESS;
	dac_dma2.DMA_Memory0BaseAddr = (uint32_t)&play_buffer1;
	dac_dma2.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	dac_dma2.DMA_BufferSize = BUFF_LEN;
	dac_dma2.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dac_dma2.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dac_dma2.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dac_dma2.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	dac_dma2.DMA_Mode = DMA_Mode_Circular;
	dac_dma2.DMA_Priority = DMA_Priority_High;
	dac_dma2.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dac_dma2.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	dac_dma2.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dac_dma2.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DAC2_DMA_Stream, &dac_dma2);

	DMA_DoubleBufferModeConfig(DAC2_DMA_Stream,(uint32_t)&play_buffer2,DMA_Memory_0);
	DMA_DoubleBufferModeCmd(DAC2_DMA_Stream, ENABLE);

//	DMA_ITConfig(DAC_DMA_Stream, DMA_IT_TC , ENABLE);
	DMA_ITConfig(DAC2_DMA_Stream, DMA_IT_TC | DMA_IT_FE | DMA_IT_TE | DMA_IT_DME, ENABLE);
	nvic_play.NVIC_IRQChannel = DAC2_DMA_IRQ;
	nvic_play.NVIC_IRQChannelPreemptionPriority = 2;
	nvic_play.NVIC_IRQChannelSubPriority = 1;
	nvic_play.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvic_play);

	NVIC_EnableIRQ(DAC2_DMA_IRQ);

	DMA_Cmd(DAC2_DMA_Stream, ENABLE);

	DAC_Cmd(DAC_Channel_2, ENABLE);

	DAC_DMACmd(DAC_Channel_2, ENABLE);

}

#define DAC1_DMA_Stream DMA1_Stream5
#define DAC1_DMA_Channel DMA_Channel_7
#define DAC1_DMA_IRQ DMA1_Stream5_IRQn
#define DAC1_DHR12L1_ADDRESS    0x4000740C
#define DAC1_TIM TIM7

NVIC_InitTypeDef nvic_play;

void Audio_DAC1_Init(void){
	GPIO_InitTypeDef   gpio;
	TIM_TimeBaseInitTypeDef tim;


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_4;
	gpio.GPIO_Mode = GPIO_Mode_AN;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &gpio);

	TIM_TimeBaseStructInit(&tim);
	tim.TIM_Period = INITIAL_PLAYBACK_PERIOD; //  874 = 96kHz  from 84 MHz TIM6CLK (ie APB1 = HCLK/4, TIM6CLK = HCLK/2)
	tim.TIM_Prescaler = 0;
	tim.TIM_ClockDivision = 0;
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(DAC1_TIM, &tim);

	TIM_SelectOutputTrigger(DAC1_TIM, TIM_TRGOSource_Update);
	TIM_Cmd(DAC1_TIM, ENABLE);

	dac1.DAC_Trigger = DAC_Trigger_T7_TRGO;
	dac1.DAC_WaveGeneration = DAC_WaveGeneration_None;
	dac1.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &dac1);

	DMA_DeInit(DAC1_DMA_Stream);
	dac_dma1.DMA_Channel = DAC1_DMA_Channel;
	dac_dma1.DMA_PeripheralBaseAddr = (uint32_t)DAC1_DHR12L1_ADDRESS;
	dac_dma1.DMA_Memory0BaseAddr = (uint32_t)&play2_buffer1;
	dac_dma1.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	dac_dma1.DMA_BufferSize = BUFF_LEN;
	dac_dma1.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dac_dma1.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dac_dma1.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dac_dma1.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	dac_dma1.DMA_Mode = DMA_Mode_Circular;
	dac_dma1.DMA_Priority = DMA_Priority_High;
	dac_dma1.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dac_dma1.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	dac_dma1.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dac_dma1.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DAC1_DMA_Stream, &dac_dma1);

	DMA_DoubleBufferModeConfig(DAC1_DMA_Stream,(uint32_t)&play2_buffer2,DMA_Memory_0);
	DMA_DoubleBufferModeCmd(DAC1_DMA_Stream, ENABLE);

//	DMA_ITConfig(DAC1_DMA_Stream, DMA_IT_TC , ENABLE);
	DMA_ITConfig(DAC1_DMA_Stream, DMA_IT_TC | DMA_IT_FE | DMA_IT_TE | DMA_IT_DME, ENABLE);
	nvic_play.NVIC_IRQChannel = DAC1_DMA_IRQ;
	nvic_play.NVIC_IRQChannelPreemptionPriority = 2;
	nvic_play.NVIC_IRQChannelSubPriority = 2;
	nvic_play.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvic_play);

	NVIC_EnableIRQ(DAC1_DMA_IRQ);

	DMA_Cmd(DAC1_DMA_Stream, ENABLE);

	DAC_Cmd(DAC_Channel_1, ENABLE);

	DAC_DMACmd(DAC_Channel_1, ENABLE);

}


uint32_t adjust_playback(int16_t offset){
	playback_timer_period1+=offset;

	if (playback_timer_period1>0xFFFF) playback_timer_period1=0xFFFF;
	if (playback_timer_period1<0x0100) playback_timer_period1=0x0100;

	DAC1_TIM->ARR=playback_timer_period1;

	return playback_timer_period1;
}

uint32_t set_playback(uint8_t chan, int16_t playbackrate){
	uint32_t t;


	playbackrate=4096-playbackrate;

	if (playbackrate>3500) playbackrate=playbackrate*2;
	if (playbackrate>4000) playbackrate=playbackrate*4;

	if (playbackrate<600) t=600;
	else t= playbackrate;

	if (chan==1){
		playback_timer_period1=t;

		DAC1_TIM->ARR=playback_timer_period1;
	} else {
		playback_timer_period2=t;

		DAC2_TIM->ARR=playback_timer_period2;

	}

	return t;
}


void DMA1_Stream6_IRQHandler(void){
	int i, current_dac_buffer_is_pb2;


	if (DMA_GetITStatus(DAC2_DMA_Stream, DMA_IT_FEIF6) != RESET)
		g_error|=DMA_OVR_ERROR;
	if (DMA_GetITStatus(DAC2_DMA_Stream, DMA_IT_TEIF6) != RESET)
		g_error|=DMA_OVR_ERROR;
	if (DMA_GetITStatus(DAC2_DMA_Stream, DMA_IT_DMEIF6) != RESET)
		g_error|=DMA_OVR_ERROR;


	if (DMA_GetFlagStatus(DAC2_DMA_Stream, DMA_FLAG_TCIF6) != RESET)
	{
		//DEBUG_ON(DEBUG2); //40us


		current_dac_buffer_is_pb2=(DAC2_DMA_Stream->CR & DMA_SxCR_CT);
		/* if this bit is set, then the DMA channel
		 * is currently transferring play_buffer2 (Memory 1)
		 * so we should fill play_buffer1 (Memory 0)
		 */




		DMA_ClearFlag(DAC2_DMA_Stream, DMA_FLAG_TCIF6);
	}

	//DEBUG_OFF(DEBUG2); //40us
}

void DMA1_Stream5_IRQHandler(void){
	int i, current_dac_buffer_is_pb2;


	if (DMA_GetITStatus(DAC1_DMA_Stream, DMA_IT_FEIF5) != RESET)
		g_error|=DMA_OVR_ERROR;
	if (DMA_GetITStatus(DAC1_DMA_Stream, DMA_IT_TEIF5) != RESET)
		g_error|=DMA_OVR_ERROR;
	if (DMA_GetITStatus(DAC1_DMA_Stream, DMA_IT_DMEIF5) != RESET)
		g_error|=DMA_OVR_ERROR;


	if (DMA_GetFlagStatus(DAC1_DMA_Stream, DMA_FLAG_TCIF5) != RESET)
	{
		//DEBUG_ON(DEBUG1); //40us


		current_dac_buffer_is_pb2=(DAC1_DMA_Stream->CR & DMA_SxCR_CT);
		/* if this bit is set, then the DMA channel
		 * is currently transferring play_buffer2 (Memory 1)
		 * so we should fill play_buffer1 (Memory 0)
		 */



		DMA_ClearFlag(DAC1_DMA_Stream, DMA_FLAG_TCIF5);

	}

	//DEBUG_OFF(DEBUG1);
}


