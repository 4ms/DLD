/*
 * i2s.h - I2S feeder routines
 */

#ifndef __i2s__
#define __i2s__

#include <stm32f4xx.h>

void Init_I2SDMA_Channel1(void);
void Init_I2SDMA_Channel2(void);
void DeInit_I2S_Clock(void);
void DeInit_I2SDMA_Channel1(void);
void DeInit_I2SDMA_Channel2(void);
void Start_I2SDMA_Channel1(void);
void Start_I2SDMA_Channel2(void);
void init_audio_dma(void);

#endif

