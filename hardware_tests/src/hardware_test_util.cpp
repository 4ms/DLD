#include "hardware_test_util.h"
extern "C" {
#include "dig_pins.h"
#include "globals.h"
}


uint8_t hardwaretest_continue_button(void) {
	return PINGBUT;
}

void pause_until_button_pressed(void) {
	delay_ms(80);
	while (!hardwaretest_continue_button()) {;}
}

void pause_until_button_released(void) {
	delay_ms(80);
	while (hardwaretest_continue_button()) {;}
}

void flash_ping_until_pressed(void) {
	while (1) {
		LED_PINGBUT_OFF;
		delay_ms(200);
		if (hardwaretest_continue_button()) break;
		LED_PINGBUT_ON;
		delay_ms(200);
	}
	LED_PINGBUT_ON;
	pause_until_button_released();
	LED_PINGBUT_OFF;
}

bool check_for_longhold_button(void) {
	uint32_t press_tmr = 0;
	bool longhold_detected = false;
	LED_PINGBUT_OFF;
	while (hardwaretest_continue_button()) {
		if (++press_tmr > 20000000) {
			longhold_detected = true;
			LED_PINGBUT_ON;
		}
	}
	return longhold_detected;
}

uint8_t read_switch_state(uint8_t sw_num) {
	if (sw_num==0) return TIMESW_CH1;
	if (sw_num==1) return TIMESW_CH2;
	else return 0;
}
bool read_button_state(uint8_t button_num) {
	if (button_num==0) return PINGBUT ? 1 : 0;
	if (button_num==1) return REV1BUT ? 1 : 0;
	if (button_num==2) return INF1BUT ? 1 : 0;
	if (button_num==3) return INF2BUT ? 1 : 0;
	if (button_num==4) return REV2BUT ? 1 : 0;
	else return 0;
}

void set_button_led(uint8_t button_num, bool turn_on) {
	if (turn_on) {
		if (button_num==0) LED_PINGBUT_ON;
		if (button_num==1) LED_REV1_ON;
		if (button_num==2) LED_INF1_ON;
		if (button_num==3) LED_INF2_ON;
		if (button_num==4) LED_REV2_ON;
	}
	else {
		if (button_num==0) LED_PINGBUT_OFF;
		if (button_num==1) LED_REV1_OFF;
		if (button_num==2) LED_INF1_OFF;
		if (button_num==3) LED_INF2_OFF;
		if (button_num==4) LED_REV2_OFF;
	}
}

void set_led(uint8_t led_num, bool turn_on) {
	if (turn_on) {
		if (led_num==0)
			LED_PINGBUT_ON;
		else if (led_num==1)
			LED_REV1_ON;
		else if (led_num==2)
			LED_INF1_ON;
		else if (led_num==3)
			LED_INF2_ON;
		else if (led_num==4)
			LED_REV2_ON;
		else if (led_num==5)
			LED_LOOP1_ON;
		else if (led_num==6)
			LED_LOOP2_ON;
	} 
	else {
		if (led_num==0)
			LED_PINGBUT_OFF;
		else if (led_num==1)
			LED_REV1_OFF;
		else if (led_num==2)
			LED_INF1_OFF;
		else if (led_num==3)
			LED_INF2_OFF;
		else if (led_num==4)
			LED_REV2_OFF;
		else if (led_num==5)
			LED_LOOP1_OFF;
		else if (led_num==6)
			LED_LOOP2_OFF;
	}
}

void all_leds_on() {
	LED_PINGBUT_ON;
	LED_REV1_ON;
	LED_INF1_ON;
	LED_INF2_ON;
	LED_REV2_ON;
	LED_LOOP1_ON;
	LED_LOOP2_ON;
}

