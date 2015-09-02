/*
 * resample.h
 *
 *  Created on: Apr 6, 2015
 *      Author: design
 */

#ifndef RESAMPLE_H_
#define RESAMPLE_H_
#include <stm32f4xx.h>

uint32_t resampling_read(uint32_t addr, uint8_t channel, uint16_t *rd_buff, uint8_t num_samples_out);
uint32_t resampling_write(uint32_t addr, uint8_t channel, uint16_t *wr_buff, uint8_t num_samples_in);

#endif /* RESAMPLE_H_ */
