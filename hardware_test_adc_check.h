#pragma once
#include <stdint.h>

struct AdcCheck {
	const uint16_t center_val;
	const uint16_t center_width;
	const uint16_t min_val;
	const uint16_t max_val;
	const uint32_t center_check_rate;
	uint16_t cur_val;
	uint32_t status;
};

enum AdcCheck_State {
	ADCCHECK_NO_COVERAGE,
	ADCCHECK_AT_MIN,
	ADCCHECK_AT_MAX,
	ADCCHECK_AT_CENTER,
	ADCCHECK_ELSEWHERE,
	ADCCHECK_FULLY_COVERED,
};

enum AdcCheck_State AdcCheck_check(struct AdcCheck *adc_check);
void AdcCheck_reset(struct AdcCheck *adc_check);
void AdcCheck_set_adcval(struct AdcCheck *adc_check, uint16_t adcval);

