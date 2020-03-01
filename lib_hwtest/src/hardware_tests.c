/*
 * hardware_tests.c - test hardware with a scripted procedure
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

#include "hardware_tests.h"
#include "dig_pins.h"
#include "RAM_test.h"
#include "flash.h"
#include "globals.h"
#include "leds.h"
#include "sdram.h"
#include "hardware_test_audio.h"
#include "hardware_test_switches_buttons.h"
#include "hardware_test_adc.h"
#include "hardware_test_util.h"

uint16_t _abs(int16_t val) {return (val<0) ? -val : val;}

static void test_single_leds(void);
static void test_RAM(void);

static void test_input_jacks(void);
static void animate_success(void);

void do_hardware_test(void)
{
	pause_until_button_released();

	test_single_leds();
	test_codec_init();
	test_audio_out();
	test_audio_in();
//	test_RAM();
	test_buttons();
	test_switches();

	send_LFOs_to_audio_outs();
	test_pots_and_CV();

	//test_gate_inputs();
	//test_gate_outputs();

	pause_until_button_released();
	//Animate success and force a hard reboot
	while (1) {
		animate_success();
	}
}

void animate_success(void)
{
	chase_all_lights(100);
}


// Slider, button, and clip LED test
// The continue button is pressed to illuminate each LED, one at a time
// At the end, all leds turn on and pressing continue exits the test
void test_single_leds(void)
{
	LED_PINGBUT_OFF;
	LED_INF1_OFF;
	LED_INF2_OFF;
	LED_REV1_OFF;
	LED_REV2_OFF;
	LED_LOOP1_OFF;
	LED_LOOP2_OFF;
	pause_until_button_pressed();

	LED_PINGBUT_ON;
	pause_until_button_released();
	pause_until_button_pressed();
	LED_PINGBUT_OFF;

	LED_REV1_ON;
	pause_until_button_released();
	pause_until_button_pressed();
	LED_REV1_OFF;

	LED_INF1_ON;
	pause_until_button_released();
	pause_until_button_pressed();
	LED_INF1_OFF;

	LED_INF2_ON;
	pause_until_button_released();
	pause_until_button_pressed();
	LED_INF2_OFF;

	LED_REV2_ON;
	pause_until_button_released();
	pause_until_button_pressed();
	LED_REV2_OFF;

	LED_LOOP1_ON;
	pause_until_button_released();
	pause_until_button_pressed();
	LED_LOOP1_OFF;

	LED_LOOP2_ON;
	pause_until_button_released();
	pause_until_button_pressed();
	LED_LOOP2_OFF;

	LED_PINGBUT_ON;
	LED_INF1_ON;
	LED_INF2_ON;
	LED_REV1_ON;
	LED_REV2_ON;
	LED_LOOP1_ON;
	LED_LOOP2_ON;
	pause_until_button_released();
	pause_until_button_pressed();
	LED_PINGBUT_OFF;
	LED_INF1_OFF;
	LED_INF2_OFF;
	LED_REV1_OFF;
	LED_REV2_OFF;
	LED_LOOP1_OFF;
	LED_LOOP2_OFF;

	delay_ms(100);
}


void test_RAM(void) {
	LED_PINGBUT_ON;

	SDRAM_Init();

	uint32_t errs = RAM_test();

	if (errs>0) {
		while (1) {
			blink_all_lights(50);
		}
	}

	flash_ping_until_pressed();
}


//Patch a cable from Odds Out and cable from Evens Out: both jacks must remain plugged.
//Odds Out is -5V to +5V triangle (only 0V to 5V is used, so there is no response when the output is low)
//Evens Out is -10V to +10V
//Patch Evens Out into: Freq CV jacks, Morph CV, and Spread CV (Morph and Spread knobs must be at 0%)
//Patch Odds Out into all other CV and gate jacks
//
void test_input_jacks(void) {

//	uint8_t do_continue;
//	uint32_t led_id;
//	// uint8_t cur_displayed_led=0;
//	int8_t ring_pos=10;
//	uint32_t last_ring_pos_timestamp=0;
//	uint8_t ring_pos_active=0;
//	uint32_t continue_armed=0;
//	uint8_t range_tested[13] = {0};
//	uint8_t zero_tested[13] = {0};
//	int16_t last_adc_val[13] = {0};
//	int16_t adc_val;
//	uint8_t gate_jack_high[4] = {0};
//
//	set_envout_IRQ(&hardware_test_envout_LFO_IRQ);
//	set_audio_callback(&hardware_test_audio_LFO_IRQ);
//
//	//Turn on all lock LEDs, slider LEDs, and Env Out LEDs
//	LOCKLED_ALLON();
//	LED_SLIDER_ON(slider_led[0]);
//	LED_SLIDER_ON(slider_led[1]);
//	LED_SLIDER_ON(slider_led[2]);
//	LED_SLIDER_ON(slider_led[3]);
//	LED_SLIDER_ON(slider_led[4]);
//	LED_SLIDER_ON(slider_led[5]);
//
//	for (led_id=0; led_id<20; led_id++)
//		LEDDriver_setRGBLED(led_id, 0);
//	for (led_id=20; led_id<26; led_id++)
//		LEDDriver_setRGBLED(led_id, FULL_WHITE);
//
//	LEDDriver_set_one_LED(get_envoutled_green(0), 0);
//	LEDDriver_set_one_LED(get_envoutled_green(5), 0);
//
//	LED_CLIPL_ON;
//	LED_CLIPR_ON;
//
//	while (1) {
//
//		for (uint8_t cv_id=0; cv_id<13; cv_id++) { 
//			adc_val = get_cv_val(cv_id);
//
//			if ((cv_id<=6 || zero_tested[cv_id]) && !range_tested[cv_id] && adc_val > 3800) {
//				range_tested[cv_id] = 1;
//				if (cv_id<6) LEDDriver_set_one_LED(get_envoutled_red(cv_id), 0);
//				else if (cv_id==6) LED_CLIPL_OFF;
//				else LOCKLED_OFF(cv_id-7);
//			}
//
//			if ((cv_id>6 || range_tested[cv_id]) && !zero_tested[cv_id] && adc_val < 10) {
//				zero_tested[cv_id] = 1;
//				if (cv_id<6) LEDDriver_set_one_LED(get_envoutled_blue(cv_id), 0);
//				else if (cv_id==6) LED_CLIPR_OFF;
//				else LED_SLIDER_OFF(slider_led[cv_id-7]);
//			}
//
//			if (_abs(adc_val - last_adc_val[cv_id]) > 200)
//			{
//				// cur_displayed_led = cv_id;
//				// if (cv_id<6)  LEDDriver_set_one_LED(get_envoutled_green(cv_id), 500);
//				last_adc_val[cv_id] = adc_val;
//				LEDDriver_setRGBLED(ring_pos, 0);
//				ring_pos = flip_ring((adc_val * 20)>>12);
//				LEDDriver_setRGBLED(ring_pos, 1023<<cv_id);
//				last_ring_pos_timestamp = 5000;
//				ring_pos_active = 1;
//			}
//
//		}
//
//		//Turn off ring LED when no motion (timed out)
//		if (ring_pos_active && !(--last_ring_pos_timestamp)) {
//			ring_pos_active = 0;
//			LEDDriver_setRGBLED(ring_pos, 0);
//			// if (cur_displayed_led<6)  LEDDriver_set_one_LED(get_envoutled_green(cur_displayed_led), 0);
//		}
//
//		for (uint8_t gate_id=0; gate_id<4; gate_id++)
//		{
//			if (get_gate_jack(gate_id)) {
//				if (!gate_jack_high[gate_id])
//					LEDDriver_set_one_LED(get_envoutled_green(gate_id+1), 500);
//				gate_jack_high[gate_id] = 1;
//			}
//			else {
//				if (gate_jack_high[gate_id])
//					LEDDriver_set_one_LED(get_envoutled_green(gate_id+1), 0);
//			}
//		}
//
//		do_continue = 0;
//		if (hardwaretest_continue_button()) {
//			continue_armed++;
//			for (led_id=0; led_id<20; led_id++) {
//				if (continue_armed==((led_id*400) + 1)) {
//					LEDDriver_setRGBLED(led_id, 1023<<20);
//					if (led_id==19) do_continue = 1;
//				}
//			}
//		}
//		else {
//			if (continue_armed!=0) {
//				for (led_id=0; led_id<20; led_id++)
//					LEDDriver_setRGBLED(led_id, 0);
//				continue_armed=0;
//			}
//		}
//
//		if (do_continue) {
//			for (led_id=0; led_id<NUM_LEDS; led_id++)
//				LEDDriver_setRGBLED(led_id, 0);
//			break;
//		}
//	}
//	pause_until_button_released();
}


