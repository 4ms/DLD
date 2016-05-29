/*
 * audio_memory.h
 *
 *  Created on: Apr 6, 2015
 *      Author: design
 */

#ifndef AUDIO_MEMORY_H_
#define AUDIO_MEMORY_H_
#include <stm32f4xx.h>

uint32_t memory_read(uint32_t *addr, uint8_t channel, int16_t *rd_buff, uint8_t num_samples, uint32_t loop_addr, uint8_t decrement);
uint32_t memory_write(uint32_t *addr, uint8_t channel, int16_t *wr_buff, uint8_t num_samples, uint8_t decrement);
uint32_t memory_fade_write(uint32_t *addr, uint8_t channel, int16_t *wr_buff, uint8_t num_samples, uint8_t decrement, float fade);

//uint32_t test_read(uint32_t *addr, uint8_t channel, int16_t *rd_buff, uint8_t num_samples, uint32_t loop_addr);
//uint32_t test_write(uint32_t *addr, uint8_t channel, int16_t *wr_buff, uint8_t num_samples);

void memory_clear(uint8_t channel);

#endif /* AUDIO_MEMORY_H_ */
