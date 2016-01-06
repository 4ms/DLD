/*
 * audio_memory.h
 *
 *  Created on: Apr 6, 2015
 *      Author: design
 */

#ifndef AUDIO_MEMORY_H_
#define AUDIO_MEMORY_H_
#include <stm32f4xx.h>

uint32_t sdram_read(uint32_t addr, uint8_t channel, int16_t *rd_buff, uint8_t num_samples);
uint32_t sdram_write(uint32_t addr, uint8_t channel, int16_t *wr_buff, uint8_t num_samples);

#endif /* AUDIO_MEMORY_H_ */
