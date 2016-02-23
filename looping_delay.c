/*
 * looping_delay.c

 */
#include "globals.h"
#include "looping_delay.h"
#include "dig_inouts.h"
#include "sdram.h"
#include "adc.h"
#include "params.h"
#include "audio_memory.h"


extern float param[NUM_CHAN][NUM_PARAMS];
extern uint8_t mode[NUM_CHAN][NUM_CHAN_MODES];

extern uint8_t g_error;

volatile uint32_t ping_time;
volatile uint32_t divmult_time[NUM_CHAN];

extern uint8_t flag_timepot_changed_revdown[2];

uint32_t write_addr[NUM_CHAN];
uint32_t read_addr[NUM_CHAN];

uint32_t loop_start[NUM_CHAN];
uint32_t loop_end[NUM_CHAN];

const uint32_t LOOP_RAM_BASE[NUM_CHAN] = {SDRAM_BASE, SDRAM_BASE + LOOP_SIZE};


uint32_t fade_queued_dest_divmult_time[NUM_CHAN];
uint32_t fade_queued_dest_read_addr[NUM_CHAN];
uint32_t fade_dest_read_addr[NUM_CHAN];
float fade_pos[NUM_CHAN];

#define FADE_INCREMENT 0.01
//#define FADE_INCREMENT 0.1
//#define FADE_INCREMENT 0.05
//#define FADE_INCREMENT 0.025

//FADE_ADDRESSES should equal ((1/FADE_INCREMENT)-1) * codec_BUFF_LEN/2
///???
//100 --> 1600 --> 1584
//20 --> 320 --> 300

//#define FADE_ADDRESSES 1584 /* this worked with 0.01 */
#define FADE_ADDRESSES 1440
//#define FADE_ADDRESSES 144
//#define FADE_ADDRESSES 288
//#define FADE_ADDRESSES 620

//0.01 ===> 16.5 ms fade time
//0.05 ===> 3.0 ms fade time
//0.1  ===> 1.5ms fade time


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

inline uint32_t calculate_addr_offset(uint8_t channel, uint32_t base_addr, uint32_t offset, uint8_t subtract){
	uint32_t t_addr;

	if (subtract == 0){

		t_addr = base_addr + offset;

		while (t_addr >= (LOOP_RAM_BASE[channel] + LOOP_SIZE))
			t_addr = t_addr - LOOP_SIZE;

	} else {

		t_addr = base_addr - offset;

		while (t_addr < LOOP_RAM_BASE[channel])
			t_addr = t_addr + LOOP_SIZE;

	}

	t_addr = t_addr & 0xFFFFFFFE; //addresses must be even!
	return (t_addr);
}


// Here we are saying that divmult_time * 2 = memory offset.
//
// If we use something other than a sample clock to increment our tmrs (ping_tmr and thus ping_time and divmult_time)
// then we need to use a scaled value other than 2
// However that can lead to sticky issues, if we are off by one sample every loop, that could lead to drift
//
inline uint32_t calculate_read_addr(uint8_t channel, uint32_t new_divmult_time){
	uint32_t t_read_addr;
/*
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
*/
	t_read_addr = calculate_addr_offset(channel, write_addr[channel], new_divmult_time*2, 1-mode[channel][REV]);
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
		if (addr >= (LOOP_RAM_BASE[channel] + LOOP_SIZE))
			addr = LOOP_RAM_BASE[channel];
	}
	else
	{
		addr-=2;
		if (addr <= LOOP_RAM_BASE[channel])
			addr = LOOP_RAM_BASE[channel] + LOOP_SIZE - 2;
	}

	return(addr & 0xFFFFFFFE);

	//return (calculate_addr_offset(channel, addr, 2, mode[channel][REV]));
}


// Utility function to determine if address mid is in between addresses beg and end in a circular (ring) buffer.
// To Do: draw a truth table and condense this into one or two boolean logic functions
uint8_t in_between(uint32_t mid, uint32_t beg, uint32_t end, uint8_t reverse)
{
	uint32_t t;

	if (beg==end) //zero length, trivial case?
	{
		if (mid!=beg) return(0);
		else return(1);
	}

	if (reverse) {
		t=end;
		end=beg;
		beg=t;
	}

	if (end>beg) //not wrapped around
	{
		if ((mid>=beg) && (mid<=end)) return(1);
		else return(0);

	}
	else //end has wrapped around
	{
		if ((mid<=end) || (mid>=beg)) return(1);
		else return(0);
	}
}


/* ******************
 *  set_divmult_time
 * ******************
 *
 * Changing divmult (Time knob or jack, or Ping clock speed) results in moving the read addr
 * Unless we're in INF mode, then move the loop end
 *
 * To move the read addr, we have to pay attention to the cross-fading status:
 * If we are not cross-fading the read head then
 *  -See if the new divmult_time is different than the existing one
 *   	-If so, initiate a cross-fade.
 * -Set divmult_time to the destination divmult_time
 *
 * Otherwise, if we are in the middle of a cross-fade, then just queue the new divmult_time
 *
 */


inline void set_divmult_time(uint8_t channel){
	uint32_t t_divmult_time;
	static uint32_t old_divmult_time[2]={0,0};

	t_divmult_time = ping_time * param[channel][TIME];

	t_divmult_time = t_divmult_time & 0xFFFFFFFC; //force it to be a multiple of 4

	// Check for valid divmult time range
	if (t_divmult_time > LOOP_SIZE>>1)
		t_divmult_time = LOOP_SIZE>>1;

	if (mode[channel][INF])
	{
		if (old_divmult_time[channel] != t_divmult_time){
			old_divmult_time[channel] = t_divmult_time;
			divmult_time[channel] = t_divmult_time;

			/*
			t=flag_timepot_changed_revdown[channel];
			if (mode[REVTIME_POLARITY]==NORMAL_LOOPEND) t=1-t;
			if (t)
				loop_end[channel] = calculate_addr_offset(channel, loop_start[channel], divmult_time[channel]*2, mode[channel][REV]);
			else
				loop_start[channel] = calculate_addr_offset(channel, loop_end[channel], divmult_time[channel]*2, 1-mode[channel][REV]);
			 */
			if (flag_timepot_changed_revdown[channel])
				loop_end[channel] = calculate_addr_offset(channel, loop_start[channel], divmult_time[channel]*2, mode[channel][REV]);
			else
				loop_start[channel] = calculate_addr_offset(channel, loop_end[channel], divmult_time[channel]*2, 1-mode[channel][REV]);


			// If the read addr is not in between the loop start and end, then fade to the loop start
			if (!in_between(read_addr[channel], loop_start[channel], loop_end[channel],mode[channel][REV]))
			{
				if (fade_pos[channel] < FADE_INCREMENT)
				{
					fade_pos[channel] = FADE_INCREMENT;
					fade_queued_dest_divmult_time[channel] = 0;

					fade_dest_read_addr[channel] = loop_start[channel];
				}
				else
					fade_queued_dest_read_addr[channel]=loop_start[channel];

			}
		}
	}
	else
	{
		if (fade_pos[channel] < FADE_INCREMENT)
		{

		//	if (old_divmult_time[channel] != t_divmult_time){
		//		old_divmult_time[channel] = t_divmult_time;

				divmult_time[channel] = t_divmult_time;

				fade_queued_dest_divmult_time[channel] = 0;

				fade_dest_read_addr[channel] = calculate_read_addr(channel, divmult_time[channel]);

				if (fade_dest_read_addr[channel] != read_addr[channel])
					fade_pos[channel] = FADE_INCREMENT;

		//	}

		} else
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

		if (fade_pos[channel] > 1.0){
			fade_pos[channel] = 0.0;

			read_addr[channel] = fade_dest_read_addr[channel];

			if (fade_queued_dest_divmult_time[channel])
			{
				divmult_time[channel] = fade_queued_dest_divmult_time[channel];
				fade_queued_dest_divmult_time[channel]=0;
				fade_dest_read_addr[channel] = calculate_read_addr(channel, divmult_time[channel]);
				fade_pos[channel] = FADE_INCREMENT;
			}
			else if (fade_queued_dest_read_addr[channel])
			{
				fade_dest_read_addr[channel] = fade_queued_dest_read_addr[channel];
				fade_queued_dest_read_addr[channel]=0;
				fade_pos[channel] = FADE_INCREMENT;
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

	static float mainin_lpf[2]={0.0,0.0}, auxin_lpf[2]={0.0,0.0};

	uint16_t i;

	int16_t rd_buff[codec_BUFF_LEN/4];
	int16_t rd_buff_dest[codec_BUFF_LEN/4];

	int16_t wr_buff[codec_BUFF_LEN/4];

	uint32_t passed_end_of_loop;
	uint32_t start_fade_addr;



	sz=sz/2;
	//if (channel==0)
		//DEBUG0_ON;

	if (fade_pos[0]<FADE_INCREMENT)
		DEBUG0_OFF;
	else
		DEBUG0_ON;


	if (fade_pos[1]<FADE_INCREMENT)
		DEBUG1_OFF;
	else
		DEBUG1_ON;


									//Sanity check to made sure the read_addr is inside the loop.
									//We shouldn't have to do this. The likely reason the read_addr escapes the loop
									//is that it passes the start_fade_addr and triggers the passed_end_of_loop block,
									//while at the same time already in the middle of a cross fade due to change in divmult_time or reverse
									//What to do? If we queue to passed_end_of_loop fade then we risk overflowing out of the loop
									//We could set start_fade_addr to be much earlier than the loop_end (by a factor of 2?) so that we won't go
									//past the loop_end even if we have to do two cross fades. Of course, this means usually our loop will be earlier by
									//one crossfade period, maybe 3ms or so. Acceptable??

	if (mode[channel][INF] && (!in_between(read_addr[channel], loop_start[channel], loop_end[channel], mode[channel][REV])))
	{
		if (fade_pos[channel] < FADE_INCREMENT)
		{
			fade_pos[channel] = FADE_INCREMENT;
			fade_queued_dest_divmult_time[channel] = 0;

			fade_dest_read_addr[channel] = loop_start[channel];
		}
		else
			fade_queued_dest_read_addr[channel]=loop_start[channel];
	}


	//Read a block from memory
	start_fade_addr = calculate_addr_offset(channel, loop_end[channel], FADE_ADDRESSES, 1);

	passed_end_of_loop = sdram_read(read_addr, channel, rd_buff, sz/2, start_fade_addr);

	//if read addr crosses the end of the loop, reset it to the beginning of the loop
	if (mode[channel][INF] && passed_end_of_loop)
	{
		fade_pos[channel] = FADE_INCREMENT;
		fade_queued_dest_divmult_time[channel]=0;

		if (mode[channel][REV])
			fade_dest_read_addr[channel] = calculate_addr_offset(channel, read_addr[channel], (loop_end[channel]-loop_start[channel])-(read_addr[channel]-start_fade_addr)-8, 1);
		else
			fade_dest_read_addr[channel] = calculate_addr_offset(channel, read_addr[channel], (loop_end[channel]-loop_start[channel])-(read_addr[channel]-start_fade_addr)+8, 1);

	}

	sdram_read(fade_dest_read_addr, channel, rd_buff_dest, sz/2, 0);

	for (i=0;i<(sz/2);i++){

		//*dst++ = *src++;
		//*dst++ = *src++;
		//*dst++ = *src++;
		//*dst++ = *src++;

		// Split incoming stereo audio into the two channels: Left=>Main input (clean), Right=>Aux Input
		mainin = *src++;
		*src++;
		auxin = *src++;
		*src++;

#ifdef ENABLE_AUTOMUTE
		//0.0001 is 10k samples or about 1/4 second
		mainin_lpf[channel] = (mainin_lpf[channel]*0.9995) + (((mainin>0)?mainin:(-1*mainin))*0.0005);
		if (mainin_lpf[channel]<10)
			mainin=0;

		auxin_lpf[channel] = (auxin_lpf[channel]*0.9995) + (((auxin>0)?auxin:(-1*auxin))*0.0005);
		if (auxin_lpf[channel]<10)
			auxin=0;
#endif

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
		*dst++ = 0;

		*dst++ = rd; //right
		*dst++ = 0;

		//*dst++ = mainin;
		//*dst++ = 0;
		//*dst++ = auxin;
		//*dst++ = 0;

		wr_buff[i]=wr;

	}

	//Write a block to memory
	if (mode[channel][INF] == 0)
		sdram_write(write_addr, channel, wr_buff, sz/2);

	//Handle new cross-fade position
	process_read_addr_fade(channel);

	//if (channel==0)
		//DEBUG0_OFF;
}
