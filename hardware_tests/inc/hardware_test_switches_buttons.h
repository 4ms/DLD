#pragma once
#include "ButtonChecker.h"
extern "C" {
#include "dig_pins.h"
#include "leds.h"
#include "globals.h"
}

const uint8_t kNumDLDButtons = 5;
class DLDButtonChecker : public IButtonChecker {
public:
	DLDButtonChecker()
		: IButtonChecker(kNumDLDButtons)
	{}
	~DLDButtonChecker() {}

	virtual bool _read_button(uint8_t button_num) {
		if (button_num==0) return PINGBUT ? true : false;
		if (button_num==1) return REV1BUT ? true : false;
		if (button_num==2) return INF1BUT ? true : false;
		if (button_num==3) return INF2BUT ? true : false;
		if (button_num==4) return REV2BUT ? true : false;
		return false;
	}

	virtual void _set_error_indicator(uint8_t channel, ErrorType err) {
		LED_LOOP1_OFF;
		LED_LOOP2_OFF;
		if (err == ErrorType::NoisyPress)
			LED_LOOP1_ON;
		if (err == ErrorType::NoisyRelease)
			LED_LOOP2_ON;
		if (err != ErrorType::None) {
			uint8_t flashes = 10;
			while (flashes--) {
				delay_ms(50);
				_set_indicator(channel, false);
				delay_ms(50);
				_set_indicator(channel, true);
			}
		}
	}

	virtual void _set_indicator(uint8_t indicate_num, bool turn_on) {
		if (turn_on) {
			if (indicate_num==0) LED_PINGBUT_ON;
			if (indicate_num==1) LED_REV1_ON;
			if (indicate_num==2) LED_INF1_ON;
			if (indicate_num==3) LED_INF2_ON;
			if (indicate_num==4) LED_REV2_ON;
		}
		else {
			if (indicate_num==0) LED_PINGBUT_OFF;
			if (indicate_num==1) LED_REV1_OFF;
			if (indicate_num==2) LED_INF1_OFF;
			if (indicate_num==3) LED_INF2_OFF;
			if (indicate_num==4) LED_REV2_OFF;
		}
	}
	virtual void _check_max_one_pin_changed() {
	}

};

void test_buttons(void);
void test_switches(void);

