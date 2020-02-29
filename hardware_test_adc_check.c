#include "hardware_test_adc_check.h"

enum AdcCheck_State AdcCheck_check(struct AdcCheck *adc_check)
{
	enum AdcCheck_State state;

	if (adc_check->cur_val < adc_check->min_val) {
		state = ADCCHECK_AT_MIN; //LED_LOOP1_OFF;
		adc_check->status &= ~(0b10UL);
	}
	if (adc_check->cur_val > adc_check->max_val) {
		state = ADCCHECK_AT_MAX; //LED_LOOP2_OFF;
		adc_check->status &= ~(0b01UL);
	}
	if (adc_check->cur_val>(adc_check->center_val - adc_check->center_width) \
		&& adc_check->cur_val<(adc_check->center_val + adc_check->center_width)) {
		state = ADCCHECK_AT_CENTER; //LED_PINGBUT_OFF;
		adc_check->status -= adc_check->center_check_rate; //count down
	}
	else {
		adc_check->status |= ~(0b11UL); //reset counter
		state = ADCCHECK_ELSEWHERE; //LED_PINGBUT_ON;
	}
	if ((adc_check->status & 0xFFFF0003)==0) //Todo: fix this magic number with two variables:countdown and coverage
		state = ADCCHECK_FULLY_COVERED; //return 1;

	return state;
}

void AdcCheck_reset(struct AdcCheck *adc_check)
{
	adc_check->status = 0xFFFFFFFF;
}

void AdcCheck_set_adcval(struct AdcCheck *adc_check, uint16_t adcval)
{
	adc_check->cur_val = adcval;
}
