#pragma once
#include <stdint.h>

struct AdcRangeCheckerBounds {
	const uint16_t center_val;
	const uint16_t center_width;
	const uint32_t center_check_counts;
	const uint16_t min_val;
	const uint16_t max_val;
};

enum AdcCheck_State {
	ADCCHECK_NO_COVERAGE,
	ADCCHECK_AT_MIN,
	ADCCHECK_AT_MAX,
	ADCCHECK_AT_CENTER,
	ADCCHECK_ELSEWHERE,
	ADCCHECK_FULLY_COVERED,
};

class AdcRangeChecker {
public:
	AdcRangeChecker(const AdcRangeCheckerBounds &init);
	void reset();
	void set_adcval(uint16_t adcval);
	AdcCheck_State check();

private:
	AdcRangeCheckerBounds _bounds;
	uint32_t _coverage;
	uint32_t _countdown;
	uint16_t _cur_val;
};

