/*
 * looping_delay.h
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

#ifndef __audio__
#define __audio__

//#define ARM_MATH_CM4

#include <stm32f4xx.h>
#include "sdram.h"

#define LOOP_SIZE (SDRAM_SIZE/2)


void audio_buffer_init(void);
void process_audio_block_codec(int16_t *src, int16_t *dst, int16_t sz, uint8_t channel);
void update_write_time(uint8_t channel);
void set_divmult_time(uint8_t channel);
void swap_read_write(uint8_t channel);
uint32_t inc_addr(uint32_t addr, uint8_t channel);
uint32_t dec_addr(uint32_t addr, uint8_t channel);
uint32_t offset_samples(uint8_t channel, uint32_t base_addr, uint32_t offset, uint8_t subtract);

uint32_t calculate_read_addr(uint8_t channel, uint32_t new_divmult_time);
void scroll_loop(uint8_t channel, float scroll_amount, uint8_t scroll_subtract);

void change_inf_mode(uint8_t channel);
void reverse_loop(uint8_t channel);

#endif

