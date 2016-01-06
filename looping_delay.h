/*
 * looping_delay.h
 */

#ifndef __audio__
#define __audio__

//#define ARM_MATH_CM4

#include <stm32f4xx.h>
#include "sdram.h"

#define LOOP_SIZE (SDRAM_SIZE/2)


void Audio_Init(void);
void process_audio_block_codec(int16_t *src, int16_t *dst, int16_t sz, uint8_t channel);
inline void update_write_time(uint8_t channel);
inline void set_divmult_time(uint8_t channel);
void swap_read_write(uint8_t channel);


#endif

