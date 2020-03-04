#include "LEDButtonChecker.h"
extern "C" {
#include "hardware_test_util.h"
#include "dig_pins.h"
#include "leds.h"
#include "globals.h"
}

//
// Press each button to turn the lights off
//
void test_buttons(void)
{
	all_leds_off();

	LEDButtonChecker check{5};
	check.assign_button_read_func(read_button_state);
	check.assign_button_led_func(set_button_led);
	check.reset();

	while (check.run_check()) {
	}
}

void test_switches(void) {
	uint8_t switch_state[2] = {0};
	uint8_t switch_last_state[2] = {0};
	uint8_t pos0[2]; 
	uint8_t pos1[2];
	uint8_t pos2[2];

	for (uint8_t sw_num=0; sw_num<2; sw_num++)
		switch_last_state[sw_num] = read_switch_state(sw_num);

	LED_INF1_ON;
	LED_INF2_ON;
	LED_REV1_ON;
	LED_REV2_ON;
	LED_LOOP1_ON;
	LED_LOOP2_ON;
	LED_PINGBUT_OFF;

	uint8_t bad_switch=0;
	for (uint8_t sw_num=0; sw_num<2; sw_num++) {
		pos0[sw_num] = 0;
		pos1[sw_num] = 0;
		pos2[sw_num] = 0;
	}
	for (uint8_t sw_num=0; sw_num<2; sw_num++) {
		pos0[sw_num] |= (1<<sw_num);
		pos1[sw_num] |= (1<<sw_num);
		pos2[sw_num] |= (1<<sw_num);
	}

	while(1) {
		//Check 3-pos switches
		for (uint8_t sw_num=0; sw_num<2; sw_num++) {
			switch_state[sw_num] = read_switch_state(sw_num);

			if (switch_state[sw_num]==1 && switch_last_state[sw_num]!=1) {
				if (sw_num) LED_INF2_OFF;
				else LED_INF1_OFF;
				delay_ms(100);
				pos0[sw_num] &= ~(1<<sw_num);
			}
			if (switch_state[sw_num]==3 && switch_last_state[sw_num]!=3) {
				if (sw_num) LED_REV2_OFF;
				else LED_REV1_OFF;
				delay_ms(100);
				pos1[sw_num] &= ~(1<<sw_num);
			}
			if (switch_state[sw_num]==2 && switch_last_state[sw_num]!=2) {
				if (sw_num) LED_LOOP2_OFF;
				else LED_LOOP1_OFF;
				delay_ms(100);
				pos2[sw_num] &= ~(1<<sw_num);
			}
			if (switch_state[sw_num]==0 && switch_last_state[sw_num]!=0) {
				LED_PINGBUT_ON;
				delay_ms(100);
				bad_switch |= 1<<sw_num;
			}
			switch_last_state[sw_num] = switch_state[sw_num];
		}

		uint8_t tot = 0;
		for (uint8_t sw_num=0; sw_num<2; sw_num++) {
			tot += pos0[sw_num] + pos1[sw_num] + pos2[sw_num];
		}
		if (tot==0)
			break;

	}

	if (bad_switch) {
		while (1) {
			if (bad_switch & 0b01) LED_LOOP1_ON;
			if (bad_switch & 0b10) LED_LOOP2_ON;
			delay_ms(150);
			LED_LOOP1_OFF;
			LED_LOOP2_OFF;
			delay_ms(150);
		}
	}

	flash_ping_until_pressed();
}


