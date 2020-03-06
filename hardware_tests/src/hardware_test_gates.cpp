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

const uint8_t kNumGateIns = 5;

static bool read_gate(uint8_t gatenum);
static void output_gate(uint8_t gatenum, bool turn_on);
static void show_multiple_highs_error();
static void send_gates_to_audio_outs();

static void clock_out_onoff(bool newstate);
static void loopA_out_onoff(bool newstate);
static void loopB_out_onoff(bool newstate);

//Each gate input needs to get a low and a high gate signal to pass.
//The corresponding button will turn on/off when the gate is high/low.
//If two gates are detected high at the same time, flash all lights 5 times,
//then turn on red and blue lights. Ping will flash, press it to start the test over.
//
void test_gate_ins() {
	GateInChecker checker{kNumGateIns};

	send_gates_to_audio_outs();

	LED_LOOP1_OFF;
	LED_LOOP2_OFF;

	checker.assign_read_gate_func(read_gate);
	checker.assign_indicator_func(set_led);
	
	checker.reset();
	while (checker.check()) {
		if (checker.num_gates_high()>1) {
			show_multiple_highs_error();
			checker.reset();
		}
	}

	all_leds_off();
	flash_ping_until_pressed();
}

void test_gate_outs() {
	uint32_t kEmpiricalSampleRate = 2658;
	GateOutput loopA_out {80, 0.25, 0, kEmpiricalSampleRate};
	GateOutput clock_out {100, 0.5, 0, kEmpiricalSampleRate};
	GateOutput loopB_out {120, 0.75, 0, kEmpiricalSampleRate};

	clock_out.assign_gate_onoff_func(clock_out_onoff);
	loopA_out.assign_gate_onoff_func(loopA_out_onoff);
	loopB_out.assign_gate_onoff_func(loopB_out_onoff);

	pause_until_button_released();

	while (!hardwaretest_continue_button()) {
		clock_out.update();
		loopA_out.update();
		loopB_out.update();
	}

	pause_until_button_released();
}


static bool read_gate(uint8_t gatenum) {
	if (gatenum==0) 
		return (PINGJACK!=0);
	else if (gatenum==1)
		return (REV1JACK!=0);
	else if (gatenum==2)
		return (INF1JACK!=0);
	else if (gatenum==3)
		return (INF2JACK!=0);
	else if (gatenum==4)
		return (REV2JACK!=0);
	else
		return false;
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


void show_multiple_highs_error() {
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


static TestGateOscillator gateWave; 

static void test_audio_outs_as_lfos_cb(int16_t *src, int16_t *dst, uint16_t sz, uint8_t channel) {
	if (channel==0) 
		return;

	uint16_t i;
	for (i=0; i<sz/2; i++)
	{
		float leftOut = gateWave.update();
		*dst++ = (int16_t)leftOut;
		*dst++ = 0;

		*dst++ = 0;
		*dst++ = 0;
	}
	(void)(*src);//unused
}

static void send_gates_to_audio_outs() {
	gateWave.init(2.f, 0.1f, five_volts, zero_volts, 0.f, 48000.f);
	set_codec_callback(test_audio_outs_as_lfos_cb);
}

