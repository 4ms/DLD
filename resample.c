/*
 * resample.c
 *
 *  Created on: Apr 6, 2015
 *      Author: design
 */

#include "stm32f4xx.h"
#include "resample.h"

#include "looping_delay.h"
#include "globals.h"
#include "dig_inouts.h"
#include "params.h"
extern const uint32_t LOOP_RAM_BASE[NUM_CHAN];


float r_rs=0.5; //read resample rate: 0.5 = read 16 bits and convert to 8 bits
float w_rs=2.0; //write resample rate: 2.0 = convert 8 bits to 16 bits which are written


uint32_t resampling_read(uint32_t addr, uint8_t channel, uint16_t *rd_buff, uint8_t num_samples_out){
	uint8_t i;
	uint32_t num_samples_in;

	//num_samples_in = (uint32_t) (r_rs * (float)num_samples_out);
	num_samples_in = num_samples_out;

	//Loop of 8 takes 2.5us
	//read from SDRAM. first one takes 200us, subsequent reads take 50ns

	for (i=0;i<num_samples_in;i++){
		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		rd_buff[i] = *((int16_t *)addr);

		addr+=2;
		if (addr >= (LOOP_RAM_BASE[channel] + LOOP_SIZE))
			addr = LOOP_RAM_BASE[channel];
	}

	return(addr);
}

/*
 uint32_t resampling_read(uint32_t addr, uint8_t channel, uint16_t *rd_buff, uint16 last_samp, uint8_t num_samples_out){

	int16_t xm1;
	int16_t x0,x1,x2;
	float a,b,c;

//to upsample (add samples):
xm1=last_samp;
for (i=0;i<num_samples_in - 2;i++) {
	x0  = rd_buff[i];
	x1  = rd_buff[i+1];
	x2  = rd_buff[i+2];
	a = (3 * (x0-x1) - xm1 + x2) / 2;
	b = 2*x1 + xm1 - (5*x0 + x2) / 2;
	c = (x1 - xm1) / 2;

	for (finpos=0;...){
		y [outpos] = (((a * finpos) + b) * finpos + c) * finpos + x0;
	}


	xm1=rd_buff[i];
}



 */



uint32_t resampling_write(uint32_t addr, uint8_t channel, uint16_t *wr_buff, uint8_t num_samples_in){
	uint8_t i;

	for (i=0;i<num_samples_in;i++){

		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}
		*((int16_t *)addr) = wr_buff[i];

		addr+=2;
		if (addr >= LOOP_RAM_BASE[channel] + LOOP_SIZE)
			addr = LOOP_RAM_BASE[channel];
	}

	return(addr);

}
