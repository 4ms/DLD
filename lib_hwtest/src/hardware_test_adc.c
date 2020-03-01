#include "hardware_test_util.h"
#include "hardware_test_adc_check.h"
#include "dig_pins.h"
#include "leds.h"
#include "globals.h"

#include "adc.h"

extern uint16_t potadc_buffer[NUM_POT_ADCS];
extern uint16_t cvadc_buffer[NUM_CV_ADCS];

void setup_adc(void) {
	Deinit_Pot_ADC();
	Deinit_CV_ADC();

	Init_Pot_ADC((uint16_t *)potadc_buffer, NUM_POT_ADCS);
	Init_CV_ADC((uint16_t *)cvadc_buffer, NUM_CV_ADCS);
}

void test_pots_and_CV(void) {
	LED_LOOP1_ON;
	LED_LOOP2_ON;
	LED_PINGBUT_ON;
	LED_INF1_ON;
	LED_REV1_ON;

	setup_adc();
	LED_INF1_OFF;
	LED_REV1_OFF;

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
	struct AdcCheck adc_check = {
		.center_val = 2048,
		.center_width = 200,
		.min_val = 10,
		.max_val = 4000,
		.center_check_counts = 1000
	};
	for (uint32_t adc_i=0; adc_i<NUM_POT_ADCS+NUM_CV_ADCS; adc_i++) {
		AdcCheck_init(&adc_check);
		LED_LOOP1_ON;
		LED_LOOP2_ON;
		LED_PINGBUT_ON;

		uint8_t done = 0;
		uint32_t cur_adc = adc_map[adc_i];
		while (!done && !hardwaretest_continue_button()) {
			uint16_t adcval = (adc_i<NUM_POT_ADCS) ? potadc_buffer[cur_adc]: cvadc_buffer[cur_adc];
			AdcCheck_set_adcval(&adc_check, adcval);

			switch(AdcCheck_check(&adc_check)) {
				case (ADCCHECK_AT_MIN):
					LED_LOOP1_OFF;
					break;
				case (ADCCHECK_AT_MAX):
					LED_LOOP2_OFF;
					break;
				case (ADCCHECK_AT_CENTER):
					LED_PINGBUT_OFF;
					break;
				case (ADCCHECK_ELSEWHERE):
					LED_PINGBUT_ON;
					break;
				case (ADCCHECK_FULLY_COVERED):
					done = 1;
					break;
				default:
					break;
			}
		}

		LED_INF1_ON;
		LED_INF2_ON;
		delay_ms(150);
		LED_INF1_OFF;
		LED_INF2_OFF;

		pause_until_button_released();
	}
}

