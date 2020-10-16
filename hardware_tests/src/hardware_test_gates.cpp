#include "GateInChecker.h"
#include "GateOutput.h"
#include "CodecCallbacks.h"
#include "hardware_test_gates.h"
#include "hardware_test_util.h"
extern "C" {
#include "globals.h"
#include "dig_pins.h"
#include "leds.h"
#include "i2s.h"
}


class DLDGateInChecker : public IGateInChecker {
	static const uint8_t kNumGateIns = 5;
	const unsigned kNumRepeats = 100;

public:
	DLDGateInChecker() 
		: IGateInChecker(5)
	{
		out_B_val = 0;
		set_codec_callback(manual_control_audio_outs_cb);
		set_num_toggles(kNumRepeats);
	}

private:
	static inline uint16_t out_B_val;
	static void manual_control_audio_outs_cb(int16_t *src, int16_t *dst, uint16_t sz, uint8_t channel) {
		if (channel==1) {
			for (uint16_t i=0; i<sz/2; i++) {
				*dst++ = channel ? out_B_val : 0;
				*dst++ = 0;
				*dst++ = 0;
				*dst++ = 0;
			}
			(void)(*src);//unused
		}
	}

protected:
	virtual void _set_test_signal(bool newstate) {
		out_B_val = newstate ? 31000 : 0;
		if (newstate) {
			LED_LOOP1_ON;
			LED_LOOP2_OFF;
		}
		else {
			LED_LOOP1_OFF;
			LED_LOOP2_ON;
		}
		delay_ms(1); //allow for latency of DAC output
	}

	virtual bool _read_gate(uint8_t gate_num) {
		if (gate_num==0) 
			return (PINGJACK!=0);
		else if (gate_num==1)
			return (REV1JACK!=0);
		else if (gate_num==2)
			return (INF1JACK!=0);
		else if (gate_num==3)
			return (INF2JACK!=0);
		else if (gate_num==4)
			return (REV2JACK!=0);
		else
			return false;
	}

	virtual void _set_indicator(uint8_t indicator_num, bool newstate) {
		if (newstate) {
			if (indicator_num==0) 
				LED_PINGBUT_ON;
			else if (indicator_num==1)
				LED_REV1_ON;
			else if (indicator_num==2)
				LED_INF1_ON;
			else if (indicator_num==3)
				LED_INF2_ON;
			else if (indicator_num==4)
				LED_REV1_ON;
		} else {
			if (indicator_num==0) 
				LED_PINGBUT_OFF;
			else if (indicator_num==1)
				LED_REV1_OFF;
			else if (indicator_num==2)
				LED_INF1_OFF;
			else if (indicator_num==3)
				LED_INF2_OFF;
			else if (indicator_num==4)
				LED_REV1_OFF;
		}
	}

	virtual void _set_error_indicator(uint8_t channel, ErrorType err) {
		switch (err) {
			case ErrorType::None:
				break;

			case ErrorType::MultipleHighs:
				blink_all_lights(100);
				blink_all_lights(100);
				blink_all_lights(100);
				blink_all_lights(100);
				blink_all_lights(100);
				all_leds_off();
				LED_LOOP1_ON;
				LED_LOOP2_ON;
				flash_ping_until_pressed();
				delay_ms(150);
				break;

			case ErrorType::StuckHigh:
				all_leds_on();
				LED_LOOP1_ON;
				LED_LOOP2_OFF;
				flash_ping_until_pressed();
				delay_ms(150);
				break;

			case ErrorType::StuckLow:
				all_leds_on();
				LED_LOOP1_OFF;
				LED_LOOP2_ON;
				flash_ping_until_pressed();
				delay_ms(150);
				break;
		}
	}
};

void test_gate_ins() {
	DLDGateInChecker checker;

	checker.reset();
	while (checker.check()) {;}

	if (checker.get_error() != DLDGateInChecker::ErrorType::None) {
		while (!hardwaretest_continue_button()) {
			blink_all_lights(400);
			blink_all_lights(400);
			blink_all_lights(400);
			blink_all_lights(400);
			flash_ping_until_pressed();
			delay_ms(150);
		}
	}

	all_leds_off();
	delay_ms(150);
}


///GATE OUTS
static void output_gate(uint8_t gatenum, bool turn_on);
static void clock_out_onoff(bool newstate);
static void loopA_out_onoff(bool newstate);
static void loopB_out_onoff(bool newstate);


void test_gate_outs() {
	uint32_t kEmpiricalSampleRate = 2658;
	GateOutput loopA_out {80, 0.25, 0, kEmpiricalSampleRate};
	GateOutput clock_out {100, 0.5, 0, kEmpiricalSampleRate};
	GateOutput loopB_out {120, 0.75, 0, kEmpiricalSampleRate};

	clock_out.assign_gate_onoff_func(clock_out_onoff);
	loopA_out.assign_gate_onoff_func(loopA_out_onoff);
	loopB_out.assign_gate_onoff_func(loopB_out_onoff);

	LED_LOOP1_ON;
	LED_LOOP2_ON;
	LED_PINGBUT_ON;
	pause_until_button_released();

	while (!hardwaretest_continue_button()) {
		clock_out.update();
		loopA_out.update();
		loopB_out.update();
	}

	pause_until_button_released();
}



static void output_gate(uint8_t gatenum, bool turn_on) {
	if (turn_on) {
		if (gatenum==0)
			CLKOUT1_ON;
		else if (gatenum==1)
			CLKOUT_ON;
		else if (gatenum==2)
			CLKOUT2_ON;
	} 
	else {
		if (gatenum==0)
			CLKOUT1_OFF;
		else if (gatenum==1)
			CLKOUT_OFF;
		else if (gatenum==2)
			CLKOUT2_OFF;
	}
}



static void clock_out_onoff(bool newstate) {
	output_gate(1, newstate);
}

static void loopA_out_onoff(bool newstate) {
	output_gate(0, newstate);
}

static void loopB_out_onoff(bool newstate) {
	output_gate(2, newstate);
}

