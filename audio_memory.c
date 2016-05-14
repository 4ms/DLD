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
/*
#define MEM_SIZE 16000

int16_t memory[MEM_SIZE];

uint32_t test_read(uint32_t *addr, uint8_t channel, int16_t *rd_buff, uint8_t num_samples, uint32_t loop_addr)
{
	uint8_t i;
	uint32_t j;

	uint32_t heads_crossed=0;

	for (i=0;i<num_samples;i++){

		j=addr[channel] - LOOP_RAM_BASE[channel];
		if (j<0)
			j=0;

		if (j>=MEM_SIZE)
			j=MEM_SIZE;

		rd_buff[i] = memory[j];

		addr[channel]++;
		if (addr[channel] >= (MEM_SIZE + LOOP_RAM_BASE[channel]))
			addr[channel] = LOOP_RAM_BASE[channel];

		if (addr[channel] == loop_addr)
			heads_crossed=1;
	}
	return(heads_crossed);
}

uint32_t test_write(uint32_t *addr, uint8_t channel, int16_t *wr_buff, uint8_t num_samples)
{
	uint8_t i;
	uint32_t j;

	for (i=0;i<num_samples;i++){
		j=addr[channel] - LOOP_RAM_BASE[channel];

		if (j<0)
			j=0;

		if (j>=MEM_SIZE)
			j=MEM_SIZE;

		memory[j] = wr_buff[i];

		addr[channel]++;
		if (addr[channel] >= (MEM_SIZE + LOOP_RAM_BASE[channel]))
			addr[channel] = LOOP_RAM_BASE[channel];

	}

	return 0;
}
*/

uint32_t sdram_read(uint32_t *addr, uint8_t channel, int16_t *rd_buff, uint8_t num_samples, uint32_t loop_addr, uint8_t decrement){
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

		rd_buff[i] = *((int16_t *)(addr[channel]));

		if (decrement)
			addr[channel] = dec_addr(addr[channel], channel);
		else
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

uint32_t sdram_fade_write(uint32_t *addr, uint8_t channel, int16_t *wr_buff, uint8_t num_samples, float fade){
	uint8_t i;
	int16_t rd;
	int16_t mix;

	for (i=0;i<num_samples;i++){

		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		//Enforce valid addr range
		if ((addr[channel]<SDRAM_BASE) || (addr[channel] > (SDRAM_BASE + SDRAM_SIZE)))
			addr[channel]=SDRAM_BASE;

		//even addresses only
		addr[channel] = (addr[channel] & 0xFFFFFFFE);

		//read from address
		rd = *((int16_t *)(addr[channel]));

		mix = ((float)wr_buff[i] * fade) + ((float)rd * (1.0-fade));

		*((int16_t *)addr[channel]) = mix;

		addr[channel] = inc_addr(addr[channel], channel);

	}

	return 0;

}


