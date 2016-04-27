/*
 * leds.c
 */

#include "globals.h"
#include "leds.h"
#include "dig_inouts.h"
#include "timekeeper.h"
#include "params.h"

extern volatile uint32_t ping_ledbut_tmr;
extern volatile uint32_t pingled_tmr[2];
extern volatile uint32_t divmult_time[2];
extern volatile uint32_t ping_time;
extern uint8_t mode[NUM_CHAN][NUM_CHAN_MODES];

uint8_t loop_led_state[NUM_CHAN]={0,0};


void update_ping_ledbut(void)
{
	if (ping_ledbut_tmr>=ping_time)
	{
		LED_PINGBUT_ON;
		reset_ping_ledbut_tmr();
	}
	else if (ping_ledbut_tmr >= (ping_time>>1))
	{
		LED_PINGBUT_OFF;
	}
}

/*

  			|| (mode[channel][WINDOWMODE_POT]==WINDOW && (flicker_ctr & 0x3FFFF) <= 0x5000)
			|| (mode[channel][TIMEMODE_POT]==MOD_READWRITE_TIME_NOQ && (flicker_ctr & 0xFFFF) > 0xC000)

 */
void update_channel_leds(uint8_t channel)
{
	if (pingled_tmr[channel] >= divmult_time[channel]){
		reset_pingled_tmr(channel);
	}

	else if (pingled_tmr[channel] >= (divmult_time[channel]>>1))
	{

		loop_led_state[channel]=0;

		if (channel==0) {
			CLKOUT1_OFF;
		} else {
			CLKOUT2_OFF;
		}
	}

	else if (mode[channel][LOOP_CLOCK_GATETRIG] == TRIG_MODE && pingled_tmr[channel] >= TRIG_TIME)
	{
		if (channel==0)
			CLKOUT1_OFF;
		 else
			CLKOUT2_OFF;

	}

}

void update_inf_ledbut(uint8_t channel){
	static uint32_t flicker_ctr=0;

	//0xFF = 256 = 500us
	//125000 = 250ms
	flicker_ctr++;


	if (!mode[channel][INF])
	{
		if (channel==0)	LED_INF1_OFF;
		else			LED_INF2_OFF;
	}
	else
	{
		if (channel==0)	LED_INF1_ON;
		else 			LED_INF2_ON;
	}

}
