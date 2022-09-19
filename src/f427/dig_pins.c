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

#include "globals.h"
#include "adc.h"
#include "params.h"
#include "looping_delay.h"
#include "timekeeper.h"
#include "audio_memory.h"
#include "dig_pins.h"

#include "buttons_jacks.h"




void init_dig_inouts(void){
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);


	//Configure outputs
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//LEDs
	RCC_AHB1PeriphClockCmd(LED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_LOOP1 | LED_LOOP2;	GPIO_Init(LED_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(PINGBUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_PINGBUT_pin;	GPIO_Init(LED_PINGBUT_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(INF1_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_INF1_pin;	GPIO_Init(LED_INF1_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(INF2_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_INF2_pin;	GPIO_Init(LED_INF2_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(REV1_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_REV1_pin;	GPIO_Init(LED_REV1_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(REV2_BUTLED_RCC, ENABLE);
	gpio.GPIO_Pin = LED_REV2_pin;	GPIO_Init(LED_REV2_GPIO, &gpio);

	//CLKOUT Jacks
	RCC_AHB1PeriphClockCmd(CLKOUT_RCC, ENABLE);
	gpio.GPIO_Pin = CLKOUT_pin;	GPIO_Init(CLKOUT_GPIO, &gpio);
	gpio.GPIO_Pin = CLKOUT1_pin;	GPIO_Init(CLKOUT1_GPIO, &gpio);
	gpio.GPIO_Pin = CLKOUT2_pin;	GPIO_Init(CLKOUT2_GPIO, &gpio);
	CLKOUT_OFF;
	CLKOUT1_OFF;
	CLKOUT2_OFF;

	//DEBUG pins
	RCC_AHB1PeriphClockCmd(DEBUG_RCC, ENABLE);

	gpio.GPIO_Pin = DEBUG0;	GPIO_Init(DEBUG0_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG1;	GPIO_Init(DEBUG1_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG2;	GPIO_Init(DEBUG2_GPIO, &gpio);
	gpio.GPIO_Pin = DEBUG3;	GPIO_Init(DEBUG3_GPIO, &gpio);
//	gpio.GPIO_Pin = DEBUG4;	GPIO_Init(DEBUG4_GPIO, &gpio);
	DEBUG0_OFF;
	DEBUG1_OFF;
	DEBUG2_OFF;
	DEBUG3_OFF;
//	DEBUG4_OFF;

	//Configure inputs
	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_PuPd = GPIO_PuPd_UP;

	//Div/Mult switches
	RCC_AHB1PeriphClockCmd(TIMESW_RCC, ENABLE);

	gpio.GPIO_Pin = TIMESW_CH1_T1_pin;	GPIO_Init(TIMESW_CH1_T1_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH1_T2_pin;	GPIO_Init(TIMESW_CH1_T2_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH2_T1_pin;	GPIO_Init(TIMESW_CH2_T1_GPIO, &gpio);
	gpio.GPIO_Pin = TIMESW_CH2_T2_pin;	GPIO_Init(TIMESW_CH2_T2_GPIO, &gpio);

	//Reverse buttons
	RCC_AHB1PeriphClockCmd(REVBUT_RCC, ENABLE);

	gpio.GPIO_Pin = REV2BUT_pin;	GPIO_Init(REV2BUT_GPIO, &gpio);
	gpio.GPIO_Pin = REV1BUT_pin;	GPIO_Init(REV1BUT_GPIO, &gpio);


	//Ping button and jack
	RCC_AHB1PeriphClockCmd(PING_RCC, ENABLE);

	gpio.GPIO_Pin = PINGBUT_pin;	GPIO_Init(PINGBUT_GPIO, &gpio);

	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio.GPIO_Pin = PINGJACK_pin;	GPIO_Init(PINGJACK_GPIO, &gpio);


	// Inf Repeat buttons and jacks
	RCC_AHB1PeriphClockCmd(INF_RCC, ENABLE);

	gpio.GPIO_PuPd = GPIO_PuPd_UP;

	gpio.GPIO_Pin = INF1BUT_pin;	GPIO_Init(INF1BUT_GPIO, &gpio);
	gpio.GPIO_Pin = INF2BUT_pin;	GPIO_Init(INF2BUT_GPIO, &gpio);

	gpio.GPIO_PuPd = GPIO_PuPd_DOWN;

	gpio.GPIO_Pin = INF1JACK_pin;	GPIO_Init(INF1JACK_GPIO, &gpio);
	gpio.GPIO_Pin = INF2JACK_pin;	GPIO_Init(INF2JACK_GPIO, &gpio);

	RCC_AHB1PeriphClockCmd(REV_RCC, ENABLE);

	gpio.GPIO_Pin = REV1JACK_pin;	GPIO_Init(REV1JACK_GPIO, &gpio);
	gpio.GPIO_Pin = REV2JACK_pin;	GPIO_Init(REV2JACK_GPIO, &gpio);

	gpio.GPIO_PuPd = GPIO_PuPd_UP;
	RCC_AHB1PeriphClockCmd(JUMPER_RCC, ENABLE);

	gpio.GPIO_Pin = JUMPER_1_pin;	GPIO_Init(JUMPER_1_GPIO, &gpio);
	gpio.GPIO_Pin = JUMPER_2_pin;	GPIO_Init(JUMPER_2_GPIO, &gpio);
//	gpio.GPIO_Pin = JUMPER_3_pin;	GPIO_Init(JUMPER_3_GPIO, &gpio);
//	gpio.GPIO_Pin = JUMPER_4_pin;	GPIO_Init(JUMPER_4_GPIO, &gpio);


}

