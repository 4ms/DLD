#include "hardware_test_adc_check.h"

void AdcCheck_init(struct AdcCheck *obj)
{
	obj->countdown = obj->center_check_counts;
	obj->coverage = 0;
}

enum AdcCheck_State AdcCheck_check(struct AdcCheck *obj)
{
	enum AdcCheck_State state;

	if (obj->cur_val < obj->min_val) {
		state = ADCCHECK_AT_MIN;
		obj->coverage |= (0b10UL);
	}
	if (obj->cur_val > obj->max_val) {
		state = ADCCHECK_AT_MAX;
		obj->coverage |= (0b01UL);
	}
	if (obj->cur_val>(obj->center_val - obj->center_width) \
		&& obj->cur_val<(obj->center_val + obj->center_width)) {
		state = ADCCHECK_AT_CENTER;
		obj->countdown--;
		if (obj->countdown==0 && obj->coverage==0b11)
			state = ADCCHECK_FULLY_COVERED;
	}
	else {
		obj->countdown = obj->center_check_counts;
		state = ADCCHECK_ELSEWHERE;
	}

	return state;
}

void AdcCheck_set_adcval(struct AdcCheck *obj, uint16_t adcval)
{
	obj->cur_val = adcval;
}

