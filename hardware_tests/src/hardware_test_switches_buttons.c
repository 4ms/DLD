#include "hardware_test_util.h"
#include "dig_pins.h"
#include "leds.h"
#include "globals.h"

static uint8_t read_switch_state(uint8_t sw_num) {
	if (sw_num==0) return TIMESW_CH1;
	if (sw_num==1) return TIMESW_CH2;
	else return 0;
}
static uint8_t read_button_state(uint8_t button_num) {
	if (button_num==0) return PINGBUT ? 1 : 0;
	if (button_num==1) return REV1BUT ? 1 : 0;
	if (button_num==2) return INF1BUT ? 1 : 0;
	if (button_num==3) return INF2BUT ? 1 : 0;
	if (button_num==4) return REV2BUT ? 1 : 0;
	else return 0;
}
static void button_led_off(uint8_t button_num) {
	if (button_num==0) LED_PINGBUT_OFF;
	if (button_num==1) LED_REV1_OFF;
	if (button_num==2) LED_INF1_OFF;
	if (button_num==3) LED_INF2_OFF;
	if (button_num==4) LED_REV2_OFF;
}

//
// Press each button to turn the lights off
//
void test_buttons(void)
{
	LED_PINGBUT_ON;
	LED_INF1_ON;
	LED_INF2_ON;
	LED_REV1_ON;
	LED_REV2_ON;
	LED_LOOP1_OFF;
	LED_LOOP2_OFF;

	uint32_t buttons_pressed = 0;
	uint8_t buttons_state = 0b11111;

	while (buttons_state)
	{
		//Check button presses and turn off buttons
		//Must press buttons one at a time
		buttons_pressed = 0;
		for (uint8_t button_num=0; button_num<5; button_num++) {
			if (read_button_state(button_num))
				buttons_pressed++;
		}
		if (buttons_pressed == 1) {
			for (uint8_t button_num=0; button_num<5; button_num++) {
				if (read_button_state(button_num)) {
					button_led_off(button_num);
					buttons_state &= ~(1<<button_num);
				}
			}
		}
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


