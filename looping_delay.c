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

extern __IO uint16_t potadc_buffer[NUM_POT_ADCS];

extern const float epp_lut[4096];
extern float param[NUM_CHAN][NUM_PARAMS];
extern uint8_t mode[NUM_CHAN][NUM_CHAN_MODES];
extern uint8_t global_mode[NUM_GLOBAL_MODES];

//extern int16_t i_smoothed_cvadc[NUM_CV_ADCS];
//extern int16_t i_smoothed_potadc[NUM_POT_ADCS];

extern uint8_t SAMPLESIZE;


extern uint8_t flag_pot_changed_revdown[NUM_POT_ADCS];

extern int16_t CODEC_DAC_CALIBRATION_DCOFFSET[4];
//extern int16_t CODEC_ADC_CALIBRATION_DCOFFSET[4];

volatile uint32_t ping_time;
volatile uint32_t divmult_time[NUM_CHAN];

uint32_t write_addr[NUM_CHAN];
uint32_t read_addr[NUM_CHAN];

uint32_t loop_start[NUM_CHAN];
uint32_t loop_end[NUM_CHAN];

const uint32_t LOOP_RAM_BASE[NUM_CHAN] = {SDRAM_BASE, SDRAM_BASE + LOOP_SIZE};


uint32_t fade_queued_dest_divmult_time[NUM_CHAN];
uint8_t fade_queued_doing_inf_fade[NUM_CHAN];
uint32_t fade_queued_dest_read_addr[NUM_CHAN];
uint32_t fade_queued_dest_write_addr[NUM_CHAN];
uint32_t fade_dest_read_addr[NUM_CHAN];
uint32_t fade_dest_write_addr[NUM_CHAN];
float fade_pos[NUM_CHAN];

uint8_t doing_reverse_fade[NUM_CHAN] = {0,0};
uint8_t doing_inf_fade[NUM_CHAN] = {0,0};

uint8_t flag_reset_loopled_tmr_on_queueadvance[NUM_CHAN]={0,0};



void audio_buffer_init(void)
{
	uint32_t i;


	if (!ping_time)
		ping_time=0x00004000;

	//ping_time=7376;

	for(i=0;i<NUM_CHAN;i++){
		memory_clear(i);

		write_addr[i]=LOOP_RAM_BASE[i] + ping_time;
		read_addr[i] = LOOP_RAM_BASE[i];
		fade_dest_read_addr[i] = LOOP_RAM_BASE[i];
		fade_dest_write_addr[i] = write_addr[i];
		divmult_time[i]=ping_time;

		set_divmult_time(i);

		loop_start[i] = LOOP_RAM_BASE[i];
		loop_end[i] = LOOP_RAM_BASE[i] + LOOP_SIZE;
		doing_reverse_fade[i]=0;
	}
}

inline uint32_t offset_samples(uint8_t channel, uint32_t base_addr, uint32_t offset, uint8_t subtract)
{
	uint32_t t_addr;

	//convert samples to addresses
	offset*=SAMPLESIZE;

	if (subtract == 0){

		t_addr = base_addr + offset;

		while (t_addr >= (LOOP_RAM_BASE[channel] + LOOP_SIZE))
			t_addr = t_addr - LOOP_SIZE;

	} else {

		t_addr = base_addr - offset;

		while (t_addr < LOOP_RAM_BASE[channel])
			t_addr = t_addr + LOOP_SIZE;

	}

	if (SAMPLESIZE==2)
		t_addr = t_addr & 0xFFFFFFFE; //addresses must be even
	else
		t_addr = t_addr & 0xFFFFFFFC; //addresses must end in 00

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

	t_read_addr = offset_samples(channel, write_addr[channel], new_divmult_time, 1-mode[channel][REV]);
	return (t_read_addr);
}

void pivot_read_addr(uint8_t channel)
{

	if (fade_pos[channel] < FADE_INCREMENT)
	{
		fade_dest_read_addr[channel] = calculate_read_addr(channel, divmult_time[channel]);

		if (fade_dest_read_addr[channel] != read_addr[channel])
			fade_pos[channel] = FADE_INCREMENT;
	}
//	doing_reverse_fade[channel]=1;
}


void swap_read_write(uint8_t channel){

	fade_dest_read_addr[channel] = write_addr[channel];
	fade_dest_write_addr[channel] = read_addr[channel];

	fade_pos[channel] = FADE_INCREMENT;
	doing_reverse_fade[channel]=1;

	fade_queued_dest_divmult_time[channel] = 0;

}


inline uint32_t inc_addr(uint32_t addr, uint8_t channel)
{

	if (mode[channel][REV] == 0)
	{
		addr+=SAMPLESIZE;
		if (addr >= (LOOP_RAM_BASE[channel] + LOOP_SIZE))
			addr = LOOP_RAM_BASE[channel];
	}
	else
	{
		addr-=SAMPLESIZE;
		if (addr <= LOOP_RAM_BASE[channel])
			addr = LOOP_RAM_BASE[channel] + LOOP_SIZE - SAMPLESIZE;
	}

	return(addr & 0xFFFFFFFE);

	//return (offset_samples(channel, addr, 1, mode[channel][REV]));
}

inline uint32_t dec_addr(uint32_t addr, uint8_t channel)
{

	if (mode[channel][REV] != 0)
	{
		addr+=SAMPLESIZE;
		if (addr >= (LOOP_RAM_BASE[channel] + LOOP_SIZE))
			addr = LOOP_RAM_BASE[channel];
	}
	else
	{
		addr-=SAMPLESIZE;
		if (addr <= LOOP_RAM_BASE[channel])
			addr = LOOP_RAM_BASE[channel] + LOOP_SIZE - 2;
	}
	return(addr & 0xFFFFFFFE);

	//return (offset_samples(channel, addr, 1, 1-mode[channel][REV]));

}


// Utility function to determine if address mid is in between addresses beg and end in a circular (ring) buffer.
// To Do: draw a truth table and condense this into one or two boolean logic functions
uint8_t in_between(uint32_t mid, uint32_t beg, uint32_t end, uint8_t reverse)
{
	uint32_t t;

	if (beg==end) //zero length, trivial case
	{
		if (mid!=beg) return(0);
		else return(1);
	}

	if (reverse) { //swap beg and end if we're reversed
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

	//t_divmult_time = t_divmult_time & 0xFFFFFFFC; //force it to be a multiple of 4

	// Check for valid divmult time range
	if (t_divmult_time > LOOP_SIZE>>1)
		t_divmult_time = LOOP_SIZE>>1;

	if (mode[channel][INF])
	{
		if (old_divmult_time[channel] != t_divmult_time){

			old_divmult_time[channel] = t_divmult_time;
			divmult_time[channel] = t_divmult_time;

			/*
			t=flag_pot_changed_revdown[TIME*2+channel];
			if (mode[REVTIME_POLARITY]==NORMAL_LOOPEND) t=1-t;
			if (t)
				loop_end[channel] = offset_samples(channel, loop_start[channel], divmult_time[channel], mode[channel][REV]);
			else
				loop_start[channel] = offset_samples(channel, loop_end[channel], divmult_time[channel], 1-mode[channel][REV]);
			 */
			if (flag_pot_changed_revdown[TIME*2+channel])
				loop_end[channel] = offset_samples(channel, loop_start[channel], divmult_time[channel], mode[channel][REV]);
			else
				loop_start[channel] = offset_samples(channel, loop_end[channel], divmult_time[channel], 1-mode[channel][REV]);


			// If the read addr is not in between the loop start and end, then fade to the loop start
			if (!in_between(read_addr[channel], loop_start[channel], loop_end[channel],mode[channel][REV]))
			{
				if (fade_pos[channel] < FADE_INCREMENT)
				{
					fade_pos[channel] = FADE_INCREMENT;
					fade_queued_dest_divmult_time[channel] = 0;

					fade_dest_read_addr[channel] = loop_start[channel];
					reset_loopled_tmr(channel);
				}
				else
				{
					fade_queued_dest_read_addr[channel]=loop_start[channel];
					flag_reset_loopled_tmr_on_queueadvance[channel]=1;
				}
			}
		}
	}
	else
	{
		if (fade_pos[channel] < FADE_INCREMENT)
		{

			divmult_time[channel] = t_divmult_time;

			fade_queued_dest_divmult_time[channel] = 0;

			fade_dest_read_addr[channel] = calculate_read_addr(channel, divmult_time[channel]);

			if (fade_dest_read_addr[channel] != read_addr[channel])
				fade_pos[channel] = FADE_INCREMENT;


		} else
			fade_queued_dest_divmult_time[channel]=t_divmult_time;

	}

}


/* ******************
 *  scroll_loop()
 * ******************
 *
 * Move loop_start and loop_end the same amount.
 *
 * scroll_amount specifies the amount to move it, as expressed as a fraction of the loop legnth
 * scroll_subtract flag means to subtract from loop_start and loop_end, otherwise add
 *    Thus, if loop_start is 500 and loop_end is 750, and scroll_amount is 0.4
 *    then add 0.4 * (750 - 500) = 100 to loop_start and loop_end
 *
 */

void scroll_loop(uint8_t channel, float scroll_amount, uint8_t scroll_subtract)
{
	uint32_t loop_length;
	uint32_t loop_shift;

	// Get loop length
	if (!mode[channel][REV]){
		if (loop_end[channel] > loop_start[channel])
			loop_length = loop_end[channel] - loop_start[channel];
		else
			loop_length = loop_end[channel] + LOOP_SIZE - loop_start[channel];
	}
	else
	{
		if (loop_start[channel] > loop_end[channel])
			loop_length = loop_start[channel] - loop_end[channel];
		else
			loop_length = loop_start[channel] + LOOP_SIZE - loop_end[channel];
	}

	//Calculate amount to shift
	loop_shift = (uint32_t)(scroll_amount * (float)loop_length);

	//convert the units from addresses to samples
	loop_shift = loop_shift / SAMPLESIZE;

	//Add (or subtract) to the loop points.
	loop_start[channel] = offset_samples(channel, loop_start[channel], loop_shift, scroll_subtract);
	loop_end[channel] = offset_samples(channel, loop_end[channel], loop_shift, scroll_subtract);
}



inline void process_read_addr_fade(uint8_t channel){


	//If we're fading, increment the fade position
	//Otherwise, do nothing

//	if (divmult_time[channel]<=420)
//			fade_pos[channel]=1.0;

	if (fade_pos[channel]>0.0){
		fade_pos[channel] += FADE_INCREMENT;

		//If we've cross-faded 100%:
		//	-Stop the cross-fade
		//	-Set read_addr to the destination
		//	-Load the next queued fade (if it exists)

		if (fade_pos[channel] > 1.0){
			fade_pos[channel] = 0.0;

			doing_reverse_fade[channel] = 0;
			doing_inf_fade[channel]=0;

			read_addr[channel] = fade_dest_read_addr[channel];
			write_addr[channel] = fade_dest_write_addr[channel];

			if (fade_queued_dest_divmult_time[channel])
			{
				divmult_time[channel] = fade_queued_dest_divmult_time[channel];
				fade_queued_dest_divmult_time[channel]=0;
				fade_dest_read_addr[channel] = calculate_read_addr(channel, divmult_time[channel]);
				fade_pos[channel] = FADE_INCREMENT;
			}
			else
			{
				if (fade_queued_dest_read_addr[channel])
				{
					fade_dest_read_addr[channel] = fade_queued_dest_read_addr[channel];
					fade_queued_dest_read_addr[channel]=0;
					fade_pos[channel] = FADE_INCREMENT;
				}

				if (fade_queued_dest_write_addr[channel])
				{
					fade_dest_write_addr[channel] = fade_queued_dest_write_addr[channel];
					fade_queued_dest_write_addr[channel]=0;
					fade_pos[channel] = FADE_INCREMENT;

					if (fade_queued_doing_inf_fade[channel])
						doing_inf_fade[channel] = 1;

				}

				//if(flag_reset_loopled_tmr_on_queueadvance[channel])
				//{
					//disabled
					//reset_loopled_tmr(channel);
				//}
				//flag_reset_loopled_tmr_on_queueadvance[channel]=0;
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

//parameter sz is 16 and immediately is divided by 2

//takes about 15us
void process_audio_block_codec(int16_t *src, int16_t *dst, int16_t sz, uint8_t channel)
{
	static uint32_t mute_on_boot_ctr=96000;

	int32_t mainin, mix, dry, wr, rd;
	float regen;
	float mainin_atten;
	int32_t auxin;

	float f_wr;
	float f_mix;

	float lpf_coef;
	int32_t min_vol;

	static float mainin_lpf[2]={0.0,0.0}, auxin_lpf[2]={0.0,0.0};

	uint16_t i,t;

	int16_t rd_buff[codec_BUFF_LEN/4];
	int16_t rd_buff_dest[codec_BUFF_LEN/4];

	int16_t wr_buff[codec_BUFF_LEN/4];
	int16_t wr_buff_combined[codec_BUFF_LEN/4];

	uint32_t passed_end_of_loop;
	uint32_t start_fade_addr;

	int32_t dummy;

	uint32_t t32;

	uint32_t fade_addresses;
	uint8_t keep_reading_in_same_dir;



	sz=sz/2;

//	if (doing_reverse_fade[0])
//		DEBUG2_ON;
//	else
//		DEBUG2_OFF;


//	if (fade_pos[0]<FADE_INCREMENT)
//		DEBUG0_OFF;
//	else
//		DEBUG0_ON;


	//Sanity check to made sure the read_addr is inside the loop.
	//We shouldn't have to do this. The likely reason the read_addr escapes the loop
	//is that it passes the start_fade_addr and triggers the passed_end_of_loop block,
	//while at the same time already in the middle of a cross fade due to change in divmult_time or reverse
	//What to do? If we queue to passed_end_of_loop fade then we risk overflowing out of the loop
	//We could set start_fade_addr to be much earlier than the loop_end (by a factor of 2?) so that we won't go
	//past the loop_end even if we have to do two cross fades. Of course, this means usually our loop will be earlier by
	//one crossfade period, maybe 3ms or so. Acceptable??

	if (mode[channel][INF] && (!in_between(read_addr[channel], loop_start[channel], loop_end[channel], mode[channel][REV])))
//	if (mode[channel][INF] && (!in_between(fade_dest_read_addr[channel], loop_start[channel], loop_end[channel], mode[channel][REV])))
	{
		if (fade_pos[channel] < FADE_INCREMENT)
		{

			fade_pos[channel] = FADE_INCREMENT;
			fade_queued_dest_divmult_time[channel] = 0;

			fade_dest_read_addr[channel] = loop_start[channel];

			reset_loopled_tmr(channel);

		}
		else
		{
			fade_queued_dest_read_addr[channel] = loop_start[channel];

			flag_reset_loopled_tmr_on_queueadvance[channel]=1;
		}
	}

	/*
	 * Sanity check: make sure read_addr and write_addr are spaced properly
	 */
	if ((mode[channel][INF]==0) && (fade_pos[channel] < FADE_INCREMENT))
	{
		t32 = calculate_read_addr(channel, divmult_time[channel]);
		if (t32 != read_addr[channel])
		{
			//if (channel==0) DEBUG2_ON;
			set_divmult_time(channel);
			//if (channel==0) DEBUG2_OFF;
		}

	}


	//For short periods (audio rate), disble crossfading before the end of the loop
	if (divmult_time[channel]<420)
		start_fade_addr = loop_end[channel];
	else
		start_fade_addr = offset_samples(channel, loop_end[channel], FADE_SAMPLES, 1-mode[channel][REV]);

	if (doing_reverse_fade[channel])
		keep_reading_in_same_dir = 1;
	else
		keep_reading_in_same_dir = 0;

	// passed_end_of_loop is true if read addr crosses the end of the loop, and then we will reset it to the beginning of the loop
	// if doing_reverse_fade is true, then we should read in the opposite direction as mode[][REV] dictates (this is because we just
	// made REV=!REV, and are doing a cross fade from rd_buff going !REV to dest_rd_buff going REV)
	passed_end_of_loop = memory_read(read_addr, channel, rd_buff, sz/2, start_fade_addr, keep_reading_in_same_dir);


	if (mode[channel][INF] && divmult_time[channel]<420 && passed_end_of_loop)
	{
		reset_loopled_tmr(channel);
		read_addr[channel]=loop_start[channel];
		fade_pos[channel] = 0.0;
		fade_dest_read_addr[channel] = offset_samples(channel, read_addr[channel], sz/SAMPLESIZE, 1-mode[channel][REV]);

	}
	else if (mode[channel][INF] && passed_end_of_loop)
	{
		fade_pos[channel] = FADE_INCREMENT;
		fade_queued_dest_divmult_time[channel]=0;

		if (mode[channel][REV])
			fade_dest_read_addr[channel] = offset_samples(channel, read_addr[channel], ((loop_start[channel]-loop_end[channel])+sz)/SAMPLESIZE, 0);
		else
			fade_dest_read_addr[channel] = offset_samples(channel, read_addr[channel], ((loop_end[channel]-loop_start[channel])+sz)/SAMPLESIZE, 1);

		reset_loopled_tmr(channel);
	}

	memory_read(fade_dest_read_addr, channel, rd_buff_dest, sz/2, 0, 0);

	for (i=0;i<(sz/2);i++){

		// Split incoming stereo audio into the two channels: Left=>Main input (clean), Right=>Aux Input

		mainin = (*src++) /*+ CODEC_ADC_CALIBRATION_DCOFFSET[channel+0]*/;
		dummy=*src++;

		auxin = (*src++) /*+ CODEC_ADC_CALIBRATION_DCOFFSET[channel+2]*/;
		dummy=*src++;

		if (mute_on_boot_ctr)
		{
			mute_on_boot_ctr--;
			mainin=0;
			auxin=0;
		}

		if (global_mode[AUTO_MUTE]){

			//debug: lpf_coef = 1.0 / ((i_smoothed_potadc[5]+1.0)*10.0);
			lpf_coef = 0.0002;
			//debug: min_vol = (i_smoothed_potadc[3])>>4;
			min_vol = 10;

			mainin_lpf[channel] = (mainin_lpf[channel]*(1.0-lpf_coef)) + (((mainin>0)?mainin:(-1*mainin))*lpf_coef);

			if (mainin_lpf[channel]<min_vol)
				mainin=0;

			auxin_lpf[channel] = (auxin_lpf[channel]*(1.0-lpf_coef)) + (((auxin>0)?auxin:(-1*auxin))*lpf_coef);

			if (auxin_lpf[channel]<min_vol)
				auxin=0;
		}


		// The Dry signal is just the clean signal, without any attenuation from LEVEL
		dry = mainin;


		// Read from the loop and save this value so we can output it to the Delay Out jack
//		rd=(rd_buff[i] * (1.0-fade_pos[channel])) + (rd_buff_dest[i] * fade_pos[channel]);

		t = (uint16_t)(4095.0 * fade_pos[channel]);
		asm("usat %[dst], #12, %[src]" : [dst] "=r" (t) : [src] "r" (t));
		rd=((float)rd_buff[i] * epp_lut[t]) + ((float)rd_buff_dest[i] * epp_lut[4095-t]);

		//In INF mode, REGEN and LEVEL have been set to 1.0 and 0.0 in params.c, but we can just shortcut this completely.
		//Also, we must ignore auxin in INF mode

		if (mode[channel][INF] == 0){
			// Attenuate the delayed signal with REGEN
			regen = ((float)rd) * param[channel][REGEN];

			// Attenuate the clean signal by the LEVEL parameter
			//t_f = param[channel][LEVEL];
			mainin_atten = ((float)mainin) * param[channel][LEVEL];

			// Add the loop contents to the input signal, as well as the auxin signal
			wr = (int32_t)(regen + mainin_atten + (float)auxin);

			// T = 0.8
			// -T + T^2 = -0.16

			//This could be optimized!
			if (global_mode[SOFTCLIP]){
				f_wr=(float)wr / 32768.0;
				if(f_wr > 0.75)
				{
					f_wr = 0.75 / f_wr;
					f_wr = (1.0f - f_wr) * (1.0f - 0.75) + 0.75;
					f_wr = f_wr * 32768.0;
					wr = (int32_t)f_wr;
				}
				else if(f_wr < -0.75)
				{
					f_wr = -0.75 / f_wr;
					f_wr = -((1.0f - f_wr) * (1.0f - 0.75) + 0.75);
					f_wr = f_wr * 32768.0;

					wr = (int32_t)f_wr;
				}
			}

			asm("ssat %[dst], #16, %[src]" : [dst] "=r" (wr) : [src] "r" (wr));


		} else {
			//In INF mode, just write what we read (copy data from read_addr to write_addr, ignore mainin/auxin/regen)
			wr = rd;
		}


		// Wet/dry mix, as determined by the MIX parameter
		mix = ( ((float)dry) * param[channel][MIX_DRY] ) + ( ((float)rd) * param[channel][MIX_WET] );

		//This could be optimized!
		if (global_mode[SOFTCLIP]){
			f_mix=(float)mix / 32768.0;
			if(f_mix > 0.75)
			{
				f_mix = 0.75 / f_mix;
				f_mix = (1.0f - f_mix) * (1.0f - 0.75) + 0.75;
				f_mix = f_mix * 32768.0;
				mix = (int32_t)f_mix;

			}
			else if(f_mix < -0.75)
			{
				f_mix = -0.75 / f_mix;
				f_mix = -((1.0f - f_mix) * (1.0f - 0.75) + 0.75);
				f_mix = f_mix * 32768.0;

				mix = (int32_t)f_mix;

			}
		}

		asm("ssat %[dst], #16, %[src]" : [dst] "=r" (mix) : [src] "r" (mix));

		if (global_mode[CALIBRATE]){
			*dst++ = CODEC_DAC_CALIBRATION_DCOFFSET[0+channel];
			*dst++ = 0;

			*dst++ = CODEC_DAC_CALIBRATION_DCOFFSET[2+channel];
			*dst++ = 0;
		}
		else
		{

#ifdef DEBUG_ADC_TO_CODEC
			*dst++ = potadc_buffer[channel+0]*4;
			*dst++ = 0;

			if (TIMESW_CH1==SWITCH_CENTER) *dst++ = potadc_buffer[channel+2]*4;
			else if (TIMESW_CH1==SWITCH_UP) *dst++ = potadc_buffer[channel+4]*4;
			else *dst++ = potadc_buffer[channel+6]*4;
			*dst++ = 0;
#else
			//Main out
			*dst++ = mix + CODEC_DAC_CALIBRATION_DCOFFSET[0+channel];
			*dst++ = 0;

			//Send out
			*dst++ = rd + CODEC_DAC_CALIBRATION_DCOFFSET[2+channel];
			*dst++ = 0;
#endif

		}



		wr_buff[i]=wr;

	}

	//Write a block to memory
	if (mode[channel][INF] == 0)
	{
		if (write_addr[channel]==fade_dest_write_addr[channel] || fade_pos[channel]<FADE_INCREMENT)
		{
			memory_write(write_addr, channel, wr_buff, sz/2, 0);
			fade_dest_write_addr[channel] = write_addr[channel];

		}
		else {
			//memory_fade_write(write_addr, channel, wr_buff, sz/2, doing_reverse_fade[channel], 1-fade_pos[channel]);
			memory_fade_write(fade_dest_write_addr, channel, wr_buff, sz/2, 0, fade_pos[channel]);
		}
	}



	//Handle new cross-fade position
	process_read_addr_fade(channel);


	//	if ((write_addr[channel] - read_addr[channel]) != divmult_time[channel]*2)
	//		DEBUG0_ON;
	//	else
	//		DEBUG0_OFF;


//	DEBUG0_OFF;
//	DEBUG2_OFF;

}
