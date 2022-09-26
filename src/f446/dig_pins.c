/*
 * dig_pins.c - initialize general purpose input/output pins
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

#include "dig_pins.h"
#include "adc.h"
#include "audio_memory.h"
#include "globals.h"
#include "looping_delay.h"
#include "params.h"
#include "timekeeper.h"

#include "buttons_jacks.h"

#include "stm32f4xx.h"

void init_dig_inouts(void) {

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	GPIO_InitTypeDef gpio = {0};

	// Configure outputs
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
	gpio.Pull = GPIO_NOPULL;

	// LEDs
	gpio.Pin = LED_LOOP1;
	HAL_GPIO_Init(LED_LOOP1_GPIO, &gpio);
	gpio.Pin = LED_LOOP2;
	HAL_GPIO_Init(LED_LOOP2_GPIO, &gpio);

	gpio.Pin = LED_PINGBUT_pin;
	HAL_GPIO_Init(LED_PINGBUT_GPIO, &gpio);

	gpio.Pin = LED_INF1_pin;
	HAL_GPIO_Init(LED_INF1_GPIO, &gpio);

	gpio.Pin = LED_INF2_pin;
	HAL_GPIO_Init(LED_INF2_GPIO, &gpio);

	gpio.Pin = LED_REV1_pin;
	HAL_GPIO_Init(LED_REV1_GPIO, &gpio);

	gpio.Pin = LED_REV2_pin;
	HAL_GPIO_Init(LED_REV2_GPIO, &gpio);

	// CLKOUT Jacks
	gpio.Pin = CLKOUT_pin;
	HAL_GPIO_Init(CLKOUT_GPIO, &gpio);
	gpio.Pin = CLKOUT1_pin;
	HAL_GPIO_Init(CLKOUT1_GPIO, &gpio);
	gpio.Pin = CLKOUT2_pin;
	HAL_GPIO_Init(CLKOUT2_GPIO, &gpio);
	CLKOUT_OFF;
	CLKOUT1_OFF;
	CLKOUT2_OFF;

	// DEBUG pins

	gpio.Pin = DEBUG0;
	HAL_GPIO_Init(DEBUG0_GPIO, &gpio);
	gpio.Pin = DEBUG1;
	HAL_GPIO_Init(DEBUG1_GPIO, &gpio);
	gpio.Pin = DEBUG2;
	HAL_GPIO_Init(DEBUG2_GPIO, &gpio);
	gpio.Pin = DEBUG3;
	HAL_GPIO_Init(DEBUG3_GPIO, &gpio);
	DEBUG0_OFF;
	DEBUG1_OFF;
	DEBUG2_OFF;
	DEBUG3_OFF;

	// Configure inputs
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pull = GPIO_PULLUP;

	// Div/Mult switches
	gpio.Pin = TIMESW_CH1_T1_pin;
	HAL_GPIO_Init(TIMESW_CH1_T1_GPIO, &gpio);
	gpio.Pin = TIMESW_CH1_T2_pin;
	HAL_GPIO_Init(TIMESW_CH1_T2_GPIO, &gpio);
	gpio.Pin = TIMESW_CH2_T1_pin;
	HAL_GPIO_Init(TIMESW_CH2_T1_GPIO, &gpio);
	gpio.Pin = TIMESW_CH2_T2_pin;
	HAL_GPIO_Init(TIMESW_CH2_T2_GPIO, &gpio);

	// Reverse buttons
	gpio.Pin = REV2BUT_pin;
	HAL_GPIO_Init(REV2BUT_GPIO, &gpio);
	gpio.Pin = REV1BUT_pin;
	HAL_GPIO_Init(REV1BUT_GPIO, &gpio);

	// Ping button and jack
	gpio.Pin = PINGBUT_pin;
	HAL_GPIO_Init(PINGBUT_GPIO, &gpio);

	gpio.Pull = GPIO_NOPULL;
	gpio.Pin = PINGJACK_pin;
	HAL_GPIO_Init(PINGJACK_GPIO, &gpio);

	// Inf Repeat buttons and jacks
	gpio.Pull = GPIO_PULLUP;

	gpio.Pin = INF1BUT_pin;
	HAL_GPIO_Init(INF1BUT_GPIO, &gpio);
	gpio.Pin = INF2BUT_pin;
	HAL_GPIO_Init(INF2BUT_GPIO, &gpio);

	gpio.Pull = GPIO_PULLDOWN;

	gpio.Pin = INF1JACK_pin;
	HAL_GPIO_Init(INF1JACK_GPIO, &gpio);
	gpio.Pin = INF2JACK_pin;
	HAL_GPIO_Init(INF2JACK_GPIO, &gpio);

	gpio.Pin = REV1JACK_pin;
	HAL_GPIO_Init(REV1JACK_GPIO, &gpio);
	gpio.Pin = REV2JACK_pin;
	HAL_GPIO_Init(REV2JACK_GPIO, &gpio);

	gpio.Pull = GPIO_PULLUP;

	gpio.Pin = JUMPER_1_pin;
	HAL_GPIO_Init(JUMPER_1_GPIO, &gpio);
	gpio.Pin = JUMPER_2_pin;
	HAL_GPIO_Init(JUMPER_2_GPIO, &gpio);
}
