/*
 * audio_memory.c
 *
 *  Created on: Apr 6, 2015
 *      Author: design
 */

#include "audio_memory.h"
#include "looping_delay.h"
#include "globals.h"
#include "dig_inouts.h"
#include "params.h"

extern float param[NUM_CHAN][NUM_PARAMS];

extern const uint32_t LOOP_RAM_BASE[NUM_CHAN];


uint32_t sdram_read(uint32_t addr, uint8_t channel, uint16_t *rd_buff, uint8_t num_samples){
	uint8_t i;

	//Loop of 8 takes 2.5us
	//read from SDRAM. first one takes 200us, subsequent reads take 50ns
	for (i=0;i<num_samples;i++){
		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		rd_buff[i] = *((int16_t *)addr);

		if (param[channel][REV] == 0){
			addr+=2;
			if (addr >= (LOOP_RAM_BASE[channel] + LOOP_SIZE))
				addr = LOOP_RAM_BASE[channel];
		} else {
			addr-=2;
			if (addr <= LOOP_RAM_BASE[channel])
				addr = LOOP_RAM_BASE[channel] + LOOP_SIZE - 2;
		}
	}

	return(addr);
}


uint32_t sdram_write(uint32_t addr, uint8_t channel, uint16_t *wr_buff, uint8_t num_samples){
	uint8_t i;

	for (i=0;i<num_samples;i++){

		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}
		*((int16_t *)addr) = wr_buff[i];

		if (param[channel][REV] == 0.0){
			addr+=2;
			if (addr >= LOOP_RAM_BASE[channel] + LOOP_SIZE)
				addr = LOOP_RAM_BASE[channel];

		} else{
			addr-=2;

			if (addr <= LOOP_RAM_BASE[channel])
				addr = LOOP_RAM_BASE[channel] + LOOP_SIZE - 2;

		}

	}

	return(addr);

}
