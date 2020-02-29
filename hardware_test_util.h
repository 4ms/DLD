#pragma once
#include <stdint.h>
#include "dig_pins.h"
#include "globals.h"

static inline uint8_t hardwaretest_continue_button(void) 	{ return PINGBUT; }

static inline void pause_until_button_pressed(void) {
	delay_ms(80);
	while (!hardwaretest_continue_button()) {;}
}

static inline void pause_until_button_released(void) {
	delay_ms(80);
	while (hardwaretest_continue_button()) {;}
}

static inline void flash_ping_until_pressed(void) {
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

