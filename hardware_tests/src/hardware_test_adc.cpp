#include "AdcRangeChecker.hh"
extern "C" {
#include "hardware_test_adc.h"
#include "hardware_test_util.h"
#include "dig_pins.h"
#include "leds.h"
#include "globals.h"
#include "adc.h"
#include "i2s.h"
#include "skewed_tri.h"
}
extern uint16_t potadc_buffer[NUM_POT_ADCS];
extern uint16_t cvadc_buffer[NUM_CV_ADCS];

#define SAMPLERATE 48000

static skewedTri testWaves[4];

static void test_audio_outs_cb(int16_t *src, int16_t *dst, int16_t sz, uint8_t channel) {
	uint16_t i;
	for (i=0; i<sz/2; i++)
	{
		float leftOut = skewedTri_update(&testWaves[channel*2]);
		*dst++ = (int16_t)leftOut;
		*dst++ = 0;

		float rightOut = skewedTri_update(&testWaves[channel*2+1]);
		*dst++ = (int16_t)rightOut;
		*dst++ = 0;
	}
	(void)(*src);//unused
}

void send_LFOs_to_audio_outs() {
	float one_volt = (float)(1<<15) / 12.2f;

	float max = one_volt * 5.0f; 
	float min = -one_volt * 0.6f;
	float neg_max = -one_volt * 6.4f;

	skewedTri_init(&testWaves[0], 1, 0.5, 1, 0, SAMPLERATE);
	skewedTri_init(&testWaves[1], 0.5, 0.5, max, min, SAMPLERATE);
	skewedTri_init(&testWaves[2], 0.5, 0.5, max, neg_max, SAMPLERATE);
	skewedTri_init(&testWaves[3], 0.5, 0.5, max, min, SAMPLERATE);

	set_codec_callback(test_audio_outs_cb);
}

const uint8_t adc_map[NUM_POT_ADCS+NUM_CV_ADCS] = {
	TIME1_POT,
	REGEN1_POT,
	LVL1_POT,
	MIX1_POT,
	MIX2_POT,
	LVL2_POT,
	REGEN2_POT,
	TIME2_POT,

	TIME1_CV,
	REGEN1_CV,
	LVL1_CV,
	LVL2_CV,
	REGEN2_CV,
	TIME2_CV,
};

void setup_adc() {
	Deinit_Pot_ADC();
	Deinit_CV_ADC();

	Init_Pot_ADC((uint16_t *)potadc_buffer, NUM_POT_ADCS);
	Init_CV_ADC((uint16_t *)cvadc_buffer, NUM_CV_ADCS);
}

bool check_cvs_are_zero_except(uint8_t chan) {
	return ((cvadc_buffer[REGEN1_CV] < 30) || chan==REGEN1_CV)
		&& ((cvadc_buffer[LVL1_CV] < 30) || chan==LVL1_CV)
		&& ((cvadc_buffer[REGEN2_CV] < 30) || chan==REGEN2_CV)
		&& ((cvadc_buffer[LVL2_CV] < 30) || chan==LVL2_CV)
		&& ((cvadc_buffer[TIME1_CV] > 1950) || chan==TIME1_CV)
		&& ((cvadc_buffer[TIME1_CV] < 2150) || chan==TIME1_CV)
		&& ((cvadc_buffer[TIME2_CV] > 1950) || chan==TIME2_CV)
		&& ((cvadc_buffer[TIME2_CV] < 2150) || chan==TIME2_CV);
}

void test_pots_and_CV() {
	send_LFOs_to_audio_outs();

	struct AdcRangeCheckerBounds bounds = {
		.center_val = 2048,
		.center_width = 200,
		.center_check_counts = 10000,
		.min_val = 10,
		.max_val = 4080,
	};
	AdcRangeChecker checker {bounds};

	LED_LOOP1_ON;
	LED_LOOP2_ON;
	LED_PINGBUT_ON;

	LED_INF1_ON;
	LED_REV1_ON;
	setup_adc();
	LED_INF1_OFF;
	LED_REV1_OFF;

	for (uint32_t adc_i=NUM_POT_ADCS; adc_i<NUM_POT_ADCS+NUM_CV_ADCS; adc_i++) {
		pause_until_button_released();
		delay_ms(100);

		LED_LOOP1_ON;
		LED_LOOP2_ON;
		LED_PINGBUT_ON;

		bool done = false;
		bool zeroes_ok = true;
		uint8_t cur_adc = adc_map[adc_i];
		
		checker.reset();

		while (!done) {
			uint16_t adcval = (adc_i<NUM_POT_ADCS) ? potadc_buffer[cur_adc]: cvadc_buffer[cur_adc];
			checker.set_adcval(adcval);

			if (adc_i>=NUM_POT_ADCS) {
				zeroes_ok = check_cvs_are_zero_except(cur_adc);
				if (!zeroes_ok)
					done = true;
			}

			auto status = checker.check();

			if (status==ADCCHECK_AT_MIN){
				LED_LOOP1_OFF;
			}
			else if (status==ADCCHECK_AT_MAX){
				LED_LOOP2_OFF;
			}
			else if (status==ADCCHECK_AT_CENTER){
				LED_PINGBUT_OFF;
			}
			else if (status==ADCCHECK_ELSEWHERE){
				LED_PINGBUT_ON;
			}
			else if (status==ADCCHECK_FULLY_COVERED){
				done = true;
			}

			if (hardwaretest_continue_button())
				done = true;

		}
		if (!zeroes_ok) {
			int flashes=5;
			while (flashes--) 
				blink_all_lights(200);
			LED_PINGBUT_ON;
			pause_until_button_pressed();
			LED_PINGBUT_OFF;
			pause_until_button_released();
			delay_ms(150);
		}
		else {
			LED_INF1_ON;
			LED_INF2_ON;
			delay_ms(150);
			LED_INF1_OFF;
			LED_INF2_OFF;
		}
	}
}

