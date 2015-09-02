/*
 * adc.h - adc setup
 */

#ifndef __adc__
#define __adc__

#include <stm32f4xx.h>

#define MIN_ADC_CHANGE 20

#define NUM_POT_ADCS 8

#define TIME1_POT 0
#define TIME2_POT 1
#define LVL1_POT 2
#define LVL2_POT 3
#define REGEN1_POT 4
#define REGEN2_POT 5
#define MIX1_POT 6
#define MIX2_POT 7

#define NUM_CV_ADCS 6
#define TIME1_CV 0
#define TIME2_CV 1
#define LVL1_CV 2
#define LVL2_CV 3
#define REGEN1_CV 4
#define REGEN2_CV 5


void Init_Pot_ADC(uint16_t *ADC_Buffer, uint8_t num_adcs);
void Init_CV_ADC(uint16_t *ADC_Buffer, uint8_t num_adcs);

#endif
