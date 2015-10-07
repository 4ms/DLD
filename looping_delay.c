/*
 * looping_delay.c
 */

#include "looping_delay.h"
#include "globals.h"
#include "dig_inouts.h"
#include "sdram.h"
#include "adc.h"
#include "params.h"

extern float param[NUM_CHAN][NUM_PARAMS];

extern uint8_t g_error;

volatile uint32_t ping_time;
volatile uint32_t divmult_time[NUM_CHAN];

volatile uint32_t write_addr[NUM_CHAN];
volatile uint32_t read_addr[NUM_CHAN];

uint32_t fade_queued_dest_divmult_time[NUM_CHAN];
uint32_t fade_dest_read_addr[NUM_CHAN];
float fade_pos[NUM_CHAN];

#define FADE_INCREMENT 0.01

const uint32_t LOOP_RAM_BASE[NUM_CHAN] = {SDRAM_BASE, SDRAM_BASE + LOOP_SIZE};



void Audio_Init(void)
{
	uint32_t i;

	//Takes 1.4 seconds to clear it in 32-bit chunks, roughly 83ns per write
	for(i = SDRAM_BASE; i < (SDRAM_BASE + SDRAM_SIZE); i += 4)
			*((uint32_t *)i) = 0x00000000;

	if (!ping_time)
		ping_time=0x00004000;


	for(i=0;i<NUM_CHAN;i++){
		write_addr[i]=LOOP_RAM_BASE[i] + ping_time;
		read_addr[i] = LOOP_RAM_BASE[i];
		set_divmult_time(i);

	}
}

/*
inline void update_write_time(uint8_t channel){
	uint32_t t_write_addr;

	t_write_addr=read_addr[channel] + (divmult_time[channel]*2);

	if (t_write_addr >= LOOP_RAM_BASE[channel] + LOOP_SIZE)
		t_write_addr = LOOP_RAM_BASE[channel];

	write_addr[channel] = t_write_addr;

}
*/

void reverse_read_head(uint8_t channel){
	uint32_t t;



}

void swap_read_write(uint8_t channel){

	uint32_t t;

	t=write_addr[channel];
	write_addr[channel]=read_addr[channel];
	read_addr[channel]=t;

}

inline uint32_t calculate_read_addr(uint8_t channel, uint32_t new_divmult_time){
	uint32_t t_read_addr;

	if (param[channel][REV] == 0){

		t_read_addr=write_addr[channel] - ((int32_t)new_divmult_time*2);

		while (t_read_addr < LOOP_RAM_BASE[channel])
			t_read_addr = t_read_addr + LOOP_SIZE;

	} else {

		t_read_addr=write_addr[channel] + ((int32_t)new_divmult_time*2);

		while (t_read_addr >= (LOOP_RAM_BASE[channel] + LOOP_SIZE))
			t_read_addr = t_read_addr - LOOP_SIZE;

	}

	t_read_addr = t_read_addr & 0xFFFFFFFE; //addresses must be even!
	return (t_read_addr);
}


inline void set_divmult_time(uint8_t channel){
	uint32_t t_divmult_time;
	static uint32_t old_divmult_time[2]={0,0};

	t_divmult_time = ping_time * param[channel][TIME];

	// Check for valid divmult time range
	if (t_divmult_time > LOOP_SIZE>>1)
		t_divmult_time = LOOP_SIZE>>1;
		//OVLD LED comes on?

/*
	if (old_divmult_time[channel] != t_divmult_time){
		old_divmult_time[channel] = t_divmult_time;

		divmult_time[channel] = t_divmult_time;

		read_addr[channel] = calculate_read_addr(channel, divmult_time[channel]);

	}
	*/
	//If we are not cross-fading the read head currently, then see if the new divmult_time is different than the existing one
	//If so, initiate a cross-fade.
	//Set divmult_time to the destination divmult_time (we don't cross fade this value)
	if (fade_pos[channel] < FADE_INCREMENT){

		if (old_divmult_time[channel] != t_divmult_time){
			old_divmult_time[channel] = t_divmult_time;

			divmult_time[channel] = t_divmult_time;

			fade_pos[channel] = FADE_INCREMENT;

			fade_queued_dest_divmult_time[channel] = 0;

			fade_dest_read_addr[channel] = calculate_read_addr(channel, divmult_time[channel]);

		}

	//Otherwise, if we are in the middle of a cross-fade, then just queue the new divmult_time
	} else {

		fade_queued_dest_divmult_time[channel]=t_divmult_time;

	}




}

inline void process_read_addr_fade(uint8_t channel){

	if (fade_pos[channel]>0.0){
		fade_pos[channel] += FADE_INCREMENT;

		//If we've cross-faded 100%:
		//	-Stop the cross-fade
		//	-Set read_addr to the destination
		//	-Load the next queued fade (if it exists)
		//
		if (fade_pos[channel] >= 1.0){

			fade_pos[channel] = 0.0;

			read_addr[channel] = fade_dest_read_addr[channel];

			if (fade_queued_dest_divmult_time[channel]){

				divmult_time[channel] = fade_queued_dest_divmult_time[channel];

				fade_dest_read_addr[channel] = calculate_read_addr(channel, divmult_time[channel]);
			}

		}
	}
}


void process_audio(void){

//Implement this function:
	//Called from main loop when audio_is_queued_flag[channel] is set
	//I2S DMA sets the flag
	//Calls process_audio_block, and knows what src and dst are (perhaps audio_is_queued == 1 for first half, 2 for second half?)
}


// process_audio_block
// This is called by the RX DMA interrupt for both codecs.
// To do:
// -optimize for writing in larger blocks (SDRAM burst mode may be faster than individual writes)
// -run this via the main loop so we spend less time in the DMA interrupt, but make sure we don't under-buffer

//sz is 16
void process_audio_block(int16_t *src, int16_t *dst, int16_t sz, uint8_t channel)
{
	//if (channel==0) DEBUG0_ON;	//12us at -O0

	int32_t mainin, mix, dry, wr, rd;
	float regen;
	float mainin_atten;
	int32_t auxin;

	float a,b,c;
	uint16_t i;

	int16_t rd_buff[codec_BUFF_LEN/4];
	int16_t rd_buff_dest[codec_BUFF_LEN/4];

	int16_t wr_buff[codec_BUFF_LEN/4];

	//Read a block from memory
	read_addr[channel] = sdram_read(read_addr[channel], channel, rd_buff, sz/2);

	fade_dest_read_addr[channel] = sdram_read(fade_dest_read_addr[channel], channel, rd_buff_dest, sz/2);

	for (i=0;i<(sz/2);i++){

		// Split incoming stereo audio into the two channels: Left=>Main input (clean), Right=>Aux Input
		mainin = *src++;
		auxin = *src++;

		// The Dry signal is just the clean signal, without any attenuation from LEVEL
		dry = mainin;

		// Read from the loop and save this value so we can output it to the Delay Out jack
		rd=(rd_buff[i] * (1.0-fade_pos[channel])) + (rd_buff_dest[i] * fade_pos[channel]);
		//rd = rd_buff[i];

#ifndef INF_WP_MODE

		//In INF mode, REGEN and LEVEL have been set to 1.0 and 0.0 in params.c, but we can just shortcut this completely.
		//Also, we must ignore auxin in INF mode
		if (param[channel][INF]==0.0){
			// Attenuate the delayed signal with REGEN
			regen = ((float)rd) * param[channel][REGEN];

			// Attenuate the clean signal by the LEVEL parameter
			mainin_atten = ((float)mainin) * param[channel][LEVEL];

			// Add the loop contents to the input signal, as well as the auxin signal
			wr = (int32_t)(regen + mainin_atten + (float)auxin);
			asm("ssat %[dst], #16, %[src]" : [dst] "=r" (wr) : [src] "r" (wr));

		} else {
			//In INF mode, just write what we read (copy data from read_addr to write_addr, ignore mainin/auxin/regen)
			wr = rd;
		}


#else		// Disabled: Write to the loop only if we're not in infinite repeat mode

		if (param[channel][INF]==0.0){

			// Attenuate the delayed signal with REGEN
			regen = ((float)rd) * param[channel][REGEN];

			// Attenuate the clean signal by the LEVEL parameter
			mainin_atten = ((float)mainin) * param[channel][LEVEL];

			// Add the loop contents to the input signal, as well as the auxin signal
			wr = (int32_t)(regen + mainin_atten + (float)auxin);
			asm("ssat %[dst], #16, %[src]" : [dst] "=r" (wr) : [src] "r" (wr));


		}
#endif


		// Wet/dry mix, as determined by the MIX parameter
		mix = ( (float)dry * param[channel][MIX_DRY] + (float)rd * param[channel][MIX_WET] );
		asm("ssat %[dst], #16, %[src]" : [dst] "=r" (mix) : [src] "r" (mix));

		// Combine stereo: Left<=Mix, Right<=Wet
		*dst++ = mix; //left
		*dst++ = rd; //right

		wr_buff[i]=wr;

	}

	//Write a block to memory
	write_addr[channel] = sdram_write(write_addr[channel], channel, wr_buff, (sz/2));

	//Handle new cross-fade position
	process_read_addr_fade(channel);

//	if (channel==0) DEBUG0_OFF;
}
