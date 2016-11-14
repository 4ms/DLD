/*
 * leds.c - driver for all LEDs (buttons and discrete)
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

#ifndef LEDS_H_
#define LEDS_H_
#include <stm32f4xx.h>

#define LED_PWM_IRQHandler TIM2_IRQHandler
#define LED_TIM	TIM2
#define LED_TIM_IRQn TIM2_IRQn
#define LED_TIM_RCC RCC_APB1Periph_TIM2


void update_ping_ledbut(void);
void update_INF_REV_ledbut(uint8_t channel);
void init_LED_PWM_IRQ(void);

void blink_all_lights(uint32_t delaytime);
void chase_all_lights(uint32_t delaytime);
void update_channel_leds(void);


#endif /* LEDS_H_ */
