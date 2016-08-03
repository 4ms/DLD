/*
 * Idea for algoritm from https://www.kvraudio.com/forum/viewtopic.php?t=195315
 * Optimized for int32_t by Dan Green
 */

#include "globals.h"
#include "compressor.h"

float MAX_SAMPLEVAL;
float THRESHOLD_COMPILED;
int32_t THRESHOLD_VALUE;

void init_compressor(uint32_t max_sample_val, float threshold_percent)
{
	MAX_SAMPLEVAL=max_sample_val;

	float m = (float)max_sample_val;
	THRESHOLD_COMPILED = m * m * threshold_percent * (1.0 - threshold_percent);

	THRESHOLD_VALUE = threshold_percent*max_sample_val;
}

inline int32_t compress(int32_t val)
{
	float tv = THRESHOLD_COMPILED / ((float)val);
	if (val > THRESHOLD_VALUE) return (MAX_SAMPLEVAL - tv);
	else if (val < -THRESHOLD_VALUE) return (-MAX_SAMPLEVAL - tv);
	else return val;

}
