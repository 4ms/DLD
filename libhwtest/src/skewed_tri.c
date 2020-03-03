#include "skewed_tri.h"

float skewedTri_update(struct skewedTri *t) {
	if (t->dir==1)
		t->cur_val += t->rise_inc;
	else
		t->cur_val -= t->fall_inc;

	if (t->cur_val >= t->max) 	{ t->dir = 1 - t->dir; t->cur_val = t->max; }
	if (t->cur_val <= t->min) 	{ t->dir = 1 - t->dir; t->cur_val = t->min; }

	return t->cur_val;
}


void skewedTri_init(struct skewedTri *t, float freqHz, float skew, float max, float min, float samplerate)
{
	t->max = max; //(float)((1UL<<15UL) - 1UL);
	t->min = min; //0.0f - (float)(1UL<<15UL);
	t->cur_val = 0.0f;
	t->dir = 0;
	float range = t->max - t->min;
	t->rise_inc = (range * freqHz) / (samplerate * skew);
	t->fall_inc = (range * freqHz) / (samplerate * (1.0f - skew));
}

