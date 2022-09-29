/*
 * leds.c - driver for all LEDs (buttons and discrete)
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

#include "leds.h"
#include "calibration.h"
#include "dig_pins.h"
#include "globals.h"
#include "params.h"
#include "system_settings.h"
#include "timers.h"

extern volatile uint32_t ping_ledbut_tmr;
extern volatile uint32_t loopled_tmr[2];
extern volatile uint32_t divmult_time[2];
extern volatile uint32_t ping_time;
extern uint8_t mode[NUM_CHAN][NUM_CHAN_MODES];
extern uint8_t global_mode[NUM_GLOBAL_MODES];
extern float global_param[NUM_GLOBAL_PARAMS];

extern uint32_t flag_acknowlegde_qcm;

uint8_t loop_led_state[NUM_CHAN] = {0, 0};

void update_ping_ledbut(void) {
	if (global_mode[CALIBRATE])
		LED_PINGBUT_OFF;
	else if (global_mode[SYSTEM_SETTINGS]) {
		//LED_PINGBUT_OFF;
	} else if (flag_acknowlegde_qcm) {
		flag_acknowlegde_qcm--;
		if ((flag_acknowlegde_qcm & (1 << 8)) ||
			(!global_mode[QUANTIZE_MODE_CHANGES] && (flag_acknowlegde_qcm & (1 << 6))))
		{
			LED_PINGBUT_ON;
			LED_REV1_ON;
			LED_REV2_ON;
		} else {
			LED_PINGBUT_OFF;
			LED_REV1_OFF;
			LED_REV2_OFF;
		}
	} else {
		if (ping_ledbut_tmr >= ping_time) {
			LED_PINGBUT_ON;
			reset_ping_ledbut_tmr();
		} else if (ping_ledbut_tmr >= (ping_time >> 1)) {
			LED_PINGBUT_OFF;
		}
	}
}

void chase_all_lights(uint32_t delaytime) {
	LED_LOOP1_ON;
	delay_ms(delaytime);
	LED_LOOP1_OFF;
	delay_ms(delaytime);

	LED_PINGBUT_ON;
	delay_ms(delaytime);
	LED_PINGBUT_OFF;
	delay_ms(delaytime);

	LED_LOOP2_ON;
	delay_ms(delaytime);
	LED_LOOP2_OFF;
	delay_ms(delaytime);

	LED_REV1_ON;
	delay_ms(delaytime);
	LED_REV1_OFF;
	delay_ms(delaytime);

	LED_INF1_ON;
	delay_ms(delaytime);
	LED_INF1_OFF;
	delay_ms(delaytime);

	LED_INF2_ON;
	delay_ms(delaytime);
	LED_INF2_OFF;
	delay_ms(delaytime);

	LED_REV2_ON;
	delay_ms(delaytime);
	LED_REV2_OFF;
	delay_ms(delaytime);
}

void blink_all_lights(uint32_t delaytime) {
	LED_LOOP1_ON;
	LED_PINGBUT_ON;
	LED_LOOP2_ON;
	LED_REV1_ON;
	LED_INF1_ON;
	LED_INF2_ON;
	LED_REV2_ON;
	delay_ms(delaytime);

	LED_LOOP1_OFF;
	LED_PINGBUT_OFF;
	LED_LOOP2_OFF;
	LED_REV1_OFF;
	LED_INF1_OFF;
	LED_INF2_OFF;
	LED_REV2_OFF;
	delay_ms(delaytime);
}

void all_leds_off(void) {
	LED_PINGBUT_OFF;
	LED_INF1_OFF;
	LED_INF2_OFF;
	LED_REV1_OFF;
	LED_REV2_OFF;
	LED_LOOP1_OFF;
	LED_LOOP2_OFF;
}

void update_channel_leds(void) {
	uint8_t channel;

	for (channel = 0; channel < NUM_CHAN; channel++) {
		//if (!mode[channel][CONTINUOUS_REVERSE])
		//{
		if (loopled_tmr[channel] >= divmult_time[channel] && (mode[channel][INF] == INF_OFF)) {
			reset_loopled_tmr(channel);
		}

		else if (loopled_tmr[channel] >= (divmult_time[channel] >> 1))
		{

			loop_led_state[channel] = 0;

			if (channel == 0) {
				CLKOUT1_OFF;
			} else {
				CLKOUT2_OFF;
			}
		}

		else if (mode[channel][LOOP_CLOCK_GATETRIG] == TRIG_MODE && loopled_tmr[channel] >= TRIG_TIME)
		{
			if (channel == 0)
				CLKOUT1_OFF;
			else
				CLKOUT2_OFF;
		}
		//}
	}
}

void update_INF_REV_ledbut(uint8_t channel) {
	static uint32_t flicker_ctr = 0;
	uint8_t t;

	flicker_ctr -= (1 << 25);

	if (global_mode[CALIBRATE]) {
		update_calibration_button_leds();
	} else if (global_mode[SYSTEM_SETTINGS]) {
		update_system_settings_button_leds();
	} else {

		//let the ping button function handle the rev lights blinking in ack_qcm state
		if (!flag_acknowlegde_qcm) {

			//create a flicker by inverting the led state
			t = mode[channel][CONTINUOUS_REVERSE] && (flicker_ctr < (1 << 23));

			if (mode[channel][REV] == t) {
				if (channel == 0)
					LED_REV1_OFF;
				else
					LED_REV2_OFF;
			} else {
				if (channel == 0)
					LED_REV1_ON;
				else
					LED_REV2_ON;
			}
		}
		//create a flicker by inverting the state
		t = mode[channel][PING_LOCKED] && (flicker_ctr < (1 << 28));
		if ((mode[channel][INF] != INF_ON && mode[channel][INF] != INF_TRANSITIONING_ON) == t) {
			if (channel == 0)
				LED_INF1_ON;
			else
				LED_INF2_ON;
		} else {
			if (channel == 0)
				LED_INF1_OFF;
			else
				LED_INF2_OFF;
		}
	}
}

// 19.2kHz
void update_led_pwm_handler(void) {
	static uint32_t loop_led_PWM_ctr = 0;
	//begin1: F427: 300ns - 450ns; F446: 250ns - 420ns

	if (loop_led_state[0] && (loop_led_PWM_ctr < global_param[LOOP_LED_BRIGHTNESS]))
		LED_LOOP1_ON;
	else
		LED_LOOP1_OFF;

	if (loop_led_state[1] && (loop_led_PWM_ctr < global_param[LOOP_LED_BRIGHTNESS]))
		LED_LOOP2_ON;
	else
		LED_LOOP2_OFF;

	if (loop_led_PWM_ctr++ > 32)
		loop_led_PWM_ctr = 0;
}
