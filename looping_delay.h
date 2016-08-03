/*
 * looping_delay.h
 */

#ifndef __audio__
#define __audio__

//#define ARM_MATH_CM4

#include <stm32f4xx.h>
#include "sdram.h"

#define LOOP_SIZE (SDRAM_SIZE/2)


void audio_buffer_init(void);
void process_audio_block_codec(int16_t *src, int16_t *dst, int16_t sz, uint8_t channel);
inline void update_write_time(uint8_t channel);
inline void set_divmult_time(uint8_t channel);
void swap_read_write(uint8_t channel);
uint32_t inc_addr(uint32_t addr, uint8_t channel);
uint32_t dec_addr(uint32_t addr, uint8_t channel);
inline uint32_t offset_samples(uint8_t channel, uint32_t base_addr, uint32_t offset, uint8_t subtract);

inline uint32_t calculate_read_addr(uint8_t channel, uint32_t new_divmult_time);
void scroll_loop(uint8_t channel, float scroll_amount, uint8_t scroll_subtract);

void change_inf_mode(uint8_t channel);

#endif

