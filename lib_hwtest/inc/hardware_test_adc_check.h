#pragma once
#include <stdint.h>

struct AdcCheck {
	const uint16_t center_val;
	const uint16_t center_width;
	const uint16_t min_val;
	const uint16_t max_val;
	const uint32_t center_check_counts;
	uint16_t cur_val;
	uint32_t coverage;
	uint32_t countdown;
};

enum AdcCheck_State {
	ADCCHECK_NO_COVERAGE,
	ADCCHECK_AT_MIN,
	ADCCHECK_AT_MAX,
	ADCCHECK_AT_CENTER,
	ADCCHECK_ELSEWHERE,
	ADCCHECK_FULLY_COVERED,
};

void AdcCheck_init(struct AdcCheck *obj);
enum AdcCheck_State AdcCheck_check(struct AdcCheck *obj);
void AdcCheck_set_adcval(struct AdcCheck *obj, uint16_t adcval);

