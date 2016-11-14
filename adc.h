/*
 * adc.h - adc setup
 *
 * Author: Dan Green (danngreen1@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
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

void Deinit_Pot_ADC(void);
void Deinit_CV_ADC(void);

#endif
