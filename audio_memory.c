/*
 * audio_memory.c
 *
 *  Created on: Apr 6, 2015
 *      Author: design
 */

#include "globals.h"
#include "audio_memory.h"
#include "looping_delay.h"
#include "dig_inouts.h"
#include "params.h"

extern const uint32_t LOOP_RAM_BASE[NUM_CHAN];


uint32_t sdram_read(uint32_t *addr, uint8_t channel, int16_t *rd_buff, uint8_t num_samples, uint32_t loop_addr){
	uint8_t i;
	uint32_t heads_crossed=0;

	//Loop of 8 takes 2.5us
	//read from SDRAM. first one takes 200us, subsequent reads take 50ns
	for (i=0;i<num_samples;i++){
		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		//Enforce valid addr range
		if ((addr[channel]<SDRAM_BASE) || (addr[channel] > (SDRAM_BASE + SDRAM_SIZE)))
		addr[channel]=SDRAM_BASE;

		//even addresses only
		addr[channel] = (addr[channel] & 0xFFFFFFFE);

		rd_buff[i] = *((int16_t *)(addr[channel] & 0xFFFFFFFE));

		addr[channel] = inc_addr(addr[channel], channel);

		if (addr[channel]==loop_addr) heads_crossed=1;

	}

	return(heads_crossed);
}


uint32_t sdram_write(uint32_t *addr, uint8_t channel, int16_t *wr_buff, uint8_t num_samples){
	uint8_t i;

	for (i=0;i<num_samples;i++){

		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		//Enforce valid addr range
		if ((addr[channel]<SDRAM_BASE) || (addr[channel] > (SDRAM_BASE + SDRAM_SIZE)))
			addr[channel]=SDRAM_BASE;

		//even addresses only
		addr[channel] = (addr[channel] & 0xFFFFFFFE);

		*((int16_t *)addr[channel]) = wr_buff[i];

		addr[channel] = inc_addr(addr[channel], channel);

	}

	return 0;
//	return(addr);

}
