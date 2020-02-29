/*
* Skewed Triangle class (c-style class)
* Dan Green (danngreen1@gmail.com)
*/

#pragma once

#include <stdint.h>

struct skewedTri {
	float rise_inc;
	float fall_inc;
	float max;
	float min;
	float cur_val;
	uint8_t dir;
};

float skewedTri_update(struct skewedTri *t);
void skewedTri_init(struct skewedTri *t, float freqHz, float skew, float max, float min, float samplerate);

