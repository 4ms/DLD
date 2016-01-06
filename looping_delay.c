/*
 * looping_delay.c
 */

#include "looping_delay.h"
#include "globals.h"
#include "dig_inouts.h"
#include "sdram.h"
#include "adc.h"
#include "params.h"
#include "audio_memory.h"


extern float param[NUM_CHAN][NUM_PARAMS];
extern uint8_t mode[NUM_CHAN][NUM_MODES];

extern uint8_t g_error;

volatile uint32_t ping_time;
volatile uint32_t divmult_time[NUM_CHAN];

volatile uint32_t write_addr[NUM_CHAN];
volatile uint32_t read_addr[NUM_CHAN];

uint32_t loop_start[NUM_CHAN];
uint32_t loop_end[NUM_CHAN];

const uint32_t LOOP_RAM_BASE[NUM_CHAN] = {SDRAM_BASE, SDRAM_BASE + LOOP_SIZE};


uint32_t fade_queued_dest_divmult_time[NUM_CHAN];
uint32_t fade_dest_read_addr[NUM_CHAN];
float fade_pos[NUM_CHAN];

#define FADE_INCREMENT 0.01



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
		fade_dest_read_addr[i] = LOOP_RAM_BASE[i];
		divmult_time[i]=ping_time;
		set_divmult_time(i);

		loop_start[i] = LOOP_RAM_BASE[i];
		loop_end[i] = LOOP_RAM_BASE[i] + LOOP_SIZE;
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

// Here we are saying that divmult_time * 2 = memory offset.
//
// If we use something other than a sample clock to increment our tmrs (ping_tmr and thus ping_time and divmult_time)
// then we need to use a scaled value other than 2
// However that can lead to sticky issues, if we are off by one sample every loop, that could lead to drift
//
inline uint32_t calculate_read_addr(uint8_t channel, uint32_t new_divmult_time){
	uint32_t t_read_addr;

	if (mode[channel][REV] == 0){

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


void swap_read_write(uint8_t channel){

	uint32_t prev_write_addr;

	prev_write_addr=write_addr[channel];

	write_addr[channel]=read_addr[channel];

	// If we're not cross-fading read head then
	//	-Initiate a read head fade to the write head's old address

	if (fade_pos[channel] < FADE_INCREMENT){

		fade_dest_read_addr[channel] = prev_write_addr;
		fade_pos[channel] = FADE_INCREMENT;

		fade_queued_dest_divmult_time[channel] = 0;

	} else {
		fade_queued_dest_divmult_time[channel]=divmult_time[channel];
	}

}



inline uint32_t inc_addr(uint32_t addr, uint8_t channel)
{
	if (mode[channel][REV] == 0)
	{
		addr+=2;
		if (addr >= loop_end[channel])
			addr = loop_start[channel];
	}
	else
	{
		addr-=2;
		if (addr <= loop_start[channel])
			addr = loop_end[channel] - 2;
	}

	return(addr & 0xFFFFFFFE);
}



inline void set_divmult_time(uint8_t channel){
	uint32_t t_divmult_time;
	static uint32_t old_divmult_time[2]={0,0};

	t_divmult_time = ping_time * param[channel][TIME];

	// Check for valid divmult time range
	if (t_divmult_time > LOOP_SIZE>>1)
		t_divmult_time = LOOP_SIZE>>1;

	// If we are not cross-fading the read head:
	//	-See if the new divmult_time is different than the existing one
	//		-If so, initiate a cross-fade.
	//	-Set divmult_time to the destination divmult_time
	//
	// Otherwise, if we are in the middle of a cross-fade, then just queue the new divmult_time

	if (fade_pos[channel] < FADE_INCREMENT){

		if (old_divmult_time[channel] != t_divmult_time){
			old_divmult_time[channel] = t_divmult_time;

			divmult_time[channel] = t_divmult_time;

			fade_pos[channel] = FADE_INCREMENT;
			fade_queued_dest_divmult_time[channel] = 0;
			fade_dest_read_addr[channel] = calculate_read_addr(channel, divmult_time[channel]);

			if (mode[channel][INF])
				loop_start[channel]=fade_dest_read_addr[channel];

		}

	} else {

		fade_queued_dest_divmult_time[channel]=t_divmult_time;

	}
}

inline void process_read_addr_fade(uint8_t channel){

	//If we're fading, increment the fade position
	//Otherwise, do nothing

	if (fade_pos[channel]>0.0){
		fade_pos[channel] += FADE_INCREMENT;

		//If we've cross-faded 100%:
		//	-Stop the cross-fade
		//	-Set read_addr to the destination
		//	-Load the next queued fade (if it exists)

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
void process_audio_block_codec(int16_t *src, int16_t *dst, int16_t sz, uint8_t channel)
{
	int32_t mainin, mix, dry, wr, rd;
	float regen;
	float mainin_atten;
	int32_t auxin;

	uint16_t i;

	int16_t rd_buff[codec_BUFF_LEN/4];
	int16_t rd_buff_dest[codec_BUFF_LEN/4];

	int16_t wr_buff[codec_BUFF_LEN/4];

	//if (channel==0)
		DEBUG2_ON;

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


		//In INF mode, REGEN and LEVEL have been set to 1.0 and 0.0 in params.c, but we can just shortcut this completely.
		//Also, we must ignore auxin in INF mode

		if (mode[channel][INF] == 0){
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



		// Wet/dry mix, as determined by the MIX parameter
		mix = ( (float)dry * param[channel][MIX_DRY] + (float)rd * param[channel][MIX_WET] );
		asm("ssat %[dst], #16, %[src]" : [dst] "=r" (mix) : [src] "r" (mix));

		// Combine stereo: Left<=Mix, Right<=Wet
		*dst++ = mix; //left
		*dst++ = rd; //right
		wr_buff[i]=wr;

	}

	//Write a block to memory
	if (mode[channel][INF] == 0)
		write_addr[channel] = sdram_write(write_addr[channel], channel, wr_buff, (sz/2));

	//Handle new cross-fade position
	process_read_addr_fade(channel);

	//if (channel==0)
		DEBUG2_OFF;
}
