#include "AdcRangeChecker.hh"

AdcRangeChecker::AdcRangeChecker(const AdcRangeCheckerBounds &init)
: _bounds(init)
{
	reset();
}

void AdcRangeChecker::reset()
{
	_coverage = 0;
	_countdown = _bounds.center_check_counts;
}

AdcCheck_State AdcRangeChecker::check()
{
	enum AdcCheck_State state;

	if (_cur_val < _bounds.min_val) {
		state = ADCCHECK_AT_MIN;
		_coverage |= (0b10UL);
	}
	else if (_cur_val > _bounds.max_val) {
		state = ADCCHECK_AT_MAX;
		_coverage |= (0b01UL);
	}
	else if (_cur_val>(_bounds.center_val - _bounds.center_width) \
	 && _cur_val<(_bounds.center_val + _bounds.center_width))
	{
		_countdown--;
		if (_countdown==0 && _coverage==0b11)
			state = ADCCHECK_FULLY_COVERED;
		else
			state = ADCCHECK_AT_CENTER;
	}
	else {
		_countdown = _bounds.center_check_counts;
		state = ADCCHECK_ELSEWHERE;
	}

	return state;
}

void AdcRangeChecker::set_adcval(uint16_t adcval)
{
	_cur_val = adcval;
}

