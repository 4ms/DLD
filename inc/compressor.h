/*
 * compressor.h - Fast and lightweight soft limiter
 * This algorithm produces no distortion below a threshold, and increasingly reduces gain above the threshold
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

#ifndef COMPRESSOR_H_
#define COMPRESSOR_H_
#include <stm32f4xx.h>

/*  ((1<<15)*(1<<15)*0.75*0.75 - 0.75*(1<<15)*(1<<15))  */
#define C_THRESHOLD_75_16BIT 201326592

/*
((1<<31)*(1<<31)*0.75*0.75 - 0.75*(1<<31)*(1<<31))
(1<<58 * 9) - (1<<60 * 3)
2594073385365405696 - 3458764513820540928
=-864691128455135232
*/

#define C_THRESHOLD_75_32BIT 864691128455135232

int32_t compress(int32_t val);

void init_compressor(uint32_t max_sample_val, float threshold_percent);

#endif /* COMPRESSOR_H_ */
