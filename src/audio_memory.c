/*
 * audio_memory.c - audio buffer SDRAM access functions
 *
 * Author: Dan Green (danngreen1@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
 */

#include "globals.h"
#include "audio_memory.h"
#include "looping_delay.h"
#include "params.h"

extern const uint32_t LOOP_RAM_BASE[NUM_CHAN];

extern uint8_t SAMPLESIZE;

void memory_clear(uint8_t channel)
{

	uint32_t i;

	//Takes 700ms to clear the channel buffer in 32-bit chunks, roughly 83ns per write
	for(i = LOOP_RAM_BASE[channel]; i < (LOOP_RAM_BASE[channel] + LOOP_SIZE); i += 4)
			*((uint32_t *)i) = 0x00000000;


}

uint32_t memory_read(uint32_t *addr, uint8_t channel, int32_t *rd_buff, uint8_t num_samples, uint32_t loop_addr, uint8_t decrement){
	uint8_t i;
	uint32_t heads_crossed=0;

	//Loop of 8 takes 2.5us
	//read from SDRAM. first one takes 200us, subsequent reads take 50ns
	for (i=0;i<num_samples;i++){

		//Enforce valid addr range
		if ((addr[channel]<SDRAM_BASE) || (addr[channel] > (SDRAM_BASE + SDRAM_SIZE)))
		addr[channel]=SDRAM_BASE;

		//even addresses only
		addr[channel] = (addr[channel] & 0xFFFFFFFE);

		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		if (SAMPLESIZE==2)
			rd_buff[i] = *((int16_t *)(addr[channel]));
		else
			rd_buff[i] = *((int32_t *)(addr[channel]));

		if (decrement)
			addr[channel] = dec_addr(addr[channel], channel);
		else
			addr[channel] = inc_addr(addr[channel], channel);

		if (addr[channel]==loop_addr) heads_crossed=1;

	}

	return(heads_crossed);
}


void memory_write(uint32_t *addr, uint8_t channel, int32_t *wr_buff, uint8_t num_samples, uint8_t decrement)
{
	uint8_t i;

	for (i=0;i<num_samples;i++){

		//Enforce valid addr range
		if ((addr[channel]<SDRAM_BASE) || (addr[channel] > (SDRAM_BASE + SDRAM_SIZE)))
			addr[channel]=SDRAM_BASE;

		//even addresses only
		addr[channel] = (addr[channel] & 0xFFFFFFFE);

		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		if (SAMPLESIZE==2)
			*((int16_t *)addr[channel]) = wr_buff[i];
		else
			*((int32_t *)addr[channel]) = wr_buff[i];

		if (decrement)
			addr[channel] = dec_addr(addr[channel], channel);
		else
			addr[channel] = inc_addr(addr[channel], channel);


	}

}

//
// reads from the addr, and mixes that value with the value in wr_buff
// fade=1.0 means write 100% wr_buff and 0% read.
// fade=0.5 means write 50% wr_buff and 50% read.
// fade=0.0 means write 0% wr_buff and 100% read.
//
void memory_fade_write(uint32_t *addr, uint8_t channel, int32_t *wr_buff, uint8_t num_samples, uint8_t decrement, float fade){
	uint8_t i;
	int32_t rd;
	int32_t mix;

	for (i=0;i<num_samples;i++){

		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		//Enforce valid addr range
		if ((addr[channel]<SDRAM_BASE) || (addr[channel] > (SDRAM_BASE + SDRAM_SIZE)))
			addr[channel]=SDRAM_BASE;

		//even addresses only
		addr[channel] = (addr[channel] & 0xFFFFFFFE);

		//read from address
		if (SAMPLESIZE==2)
			rd = *((int16_t *)(addr[channel]));
		else
			rd = *((int32_t *)(addr[channel]));

		mix = ((float)wr_buff[i] * fade) + ((float)rd * (1.0-fade));

		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		if (SAMPLESIZE==2)
			*((int16_t *)addr[channel]) = mix;
		else
			*((int32_t *)addr[channel]) = mix;

		if (decrement)
			addr[channel] = dec_addr(addr[channel], channel);
		else
			addr[channel] = inc_addr(addr[channel], channel);

	}

}
