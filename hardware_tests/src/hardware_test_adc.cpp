#include "AdcRangeChecker.hh"
#include "CodecCallbacks.h"
#include "hardware_test_adc.h"
#include "hardware_test_util.h"
extern "C" {
#include "dig_pins.h"
#include "leds.h"
#include "globals.h"
#include "adc.h"
#include "i2s.h"
#include "skewed_tri.h"
}
static void show_multiple_nonzeros_error();
extern uint16_t potadc_buffer[NUM_POT_ADCS];
extern uint16_t cvadc_buffer[NUM_CV_ADCS];


static CenterFlatRamp flatRampWaveBiPolar; 
static CenterFlatRamp flatRampWaveUniPolar; 

static void test_audio_outs_as_lfos_cb(int16_t *src, int16_t *dst, uint16_t sz, uint8_t channel) {
	if (channel==0) 
		return;

	uint16_t i;
	for (i=0; i<sz/2; i++)
	{
		float leftOut = flatRampWaveBiPolar.update();
		*dst++ = (int16_t)leftOut;
		*dst++ = 0;

		float rightOut = flatRampWaveUniPolar.update();
		*dst++ = (int16_t)rightOut;
		*dst++ = 0;
	}
	(void)(*src);//unused
}

void send_LFOs_to_audio_outs() {
	flatRampWaveBiPolar.init(2.f, 0.1f, five_volts, neg_five_volts, 0.f, 48000.f);
	flatRampWaveUniPolar.init(2.f, 0.1f, five_volts, zero_volts, 0.f, 48000.f);
	set_codec_callback(test_audio_outs_as_lfos_cb);
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

bool check_max_one_cv_is_nonzero(uint16_t width) {
	uint8_t num_nonzero = 0;

	if (cvadc_buffer[REGEN1_CV] > width)
		num_nonzero++;
	if (cvadc_buffer[LVL1_CV] > width)
		num_nonzero++;
	if (cvadc_buffer[LVL2_CV] > width)
		num_nonzero++;
	if (cvadc_buffer[REGEN2_CV] > width)
		num_nonzero++;
	if (cvadc_buffer[TIME1_CV] < (2048-width/2))
		num_nonzero++;
	if (cvadc_buffer[TIME2_CV] < (2048-width/2))
		num_nonzero++;
	if (cvadc_buffer[TIME1_CV] > (2048+width/2))
		num_nonzero++;
	if (cvadc_buffer[TIME2_CV] > (2048+width/2))
		num_nonzero++;

	return (num_nonzero <= 1); 
}

void test_pots_and_CV() {
	send_LFOs_to_audio_outs();

	struct AdcRangeCheckerBounds bounds = {
		.center_val = 2048,
		.center_width = 200,
		.center_check_counts = 100000,
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

	for (uint32_t adc_i=0; adc_i<NUM_POT_ADCS+NUM_CV_ADCS; adc_i++) {
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
				zeroes_ok = check_max_one_cv_is_nonzero(300);
				if (!zeroes_ok) {
					show_multiple_nonzeros_error();
					checker.reset();
					LED_LOOP1_ON;
					LED_LOOP2_ON;
					LED_PINGBUT_ON;
				}
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

			// if (hardwaretest_continue_button())
			// 	done = true;
		}

		LED_INF1_ON;
		LED_INF2_ON;
		delay_ms(150);
		LED_INF1_OFF;
		LED_INF2_OFF;
	}
}

static void show_multiple_nonzeros_error() {
	blink_all_lights(200);
	blink_all_lights(200);
	blink_all_lights(200);
	blink_all_lights(200);
	blink_all_lights(200);
	flash_ping_until_pressed();
	delay_ms(150);
}

