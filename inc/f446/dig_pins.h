/*
 * dig_pins.h - initialize general purpose input/output pins
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

#ifndef INOUTS_H_
#define INOUTS_H_
#include <stm32f4xx.h>

//INPUTS

#define PINGBUT_pin GPIO_PIN_4
#define PINGBUT_GPIO GPIOB
#define PINGBUT (!(PINGBUT_GPIO->IDR & PINGBUT_pin))

#define PINGJACK_pin GPIO_PIN_11
#define PINGJACK_GPIO GPIOA
#define PINGJACK (PINGJACK_GPIO->IDR & PINGJACK_pin)

//Same pin as WS clock of I2S2 (LRCLK), as defined in codec.h
#define EXTI_CLOCK_GPIO GPIOE
#define EXTI_CLOCK_pin GPIO_PIN_4
#define EXTI_CLOCK_IRQ EXTI4_IRQn
#define EXTI_Handler EXTI4_IRQHandler

// Infinite Repeat Button and Jack

#define INF1BUT_pin GPIO_PIN_12
#define INF1BUT_GPIO GPIOB
#define INF1BUT (!(INF1BUT_GPIO->IDR & INF1BUT_pin))

#define INF1JACK_pin GPIO_PIN_12
#define INF1JACK_GPIO GPIOC
#define INF1JACK ((INF1JACK_GPIO->IDR & INF1JACK_pin))

#define INF2BUT_pin GPIO_PIN_7
#define INF2BUT_GPIO GPIOD
#define INF2BUT (!(INF2BUT_GPIO->IDR & INF2BUT_pin))

#define INF2JACK_pin GPIO_PIN_8
#define INF2JACK_GPIO GPIOA
#define INF2JACK ((INF2JACK_GPIO->IDR & INF2JACK_pin))

#define REV1JACK_pin GPIO_PIN_9
#define REV1JACK_GPIO GPIOA
#define REV1JACK ((REV1JACK_GPIO->IDR & REV1JACK_pin))

#define REV2JACK_pin GPIO_PIN_11
#define REV2JACK_GPIO GPIOG
#define REV2JACK ((REV2JACK_GPIO->IDR & REV2JACK_pin))

#define REV1BUT_pin GPIO_PIN_12
#define REV1BUT_GPIO GPIOG
#define REV1BUT (!(REV1BUT_GPIO->IDR & REV1BUT_pin))

#define REV2BUT_pin GPIO_PIN_13
#define REV2BUT_GPIO GPIOB
#define REV2BUT (!(REV2BUT_GPIO->IDR & REV2BUT_pin))

#define SWITCH_CENTER 0b11
#define SWITCH_UP 0b10
#define SWITCH_DOWN 0b01
#define SWITCH_INVALID 0b00

#define TIMESW_CH1_T1_pin GPIO_PIN_4
#define TIMESW_CH1_T1_GPIO GPIOD
#define TIMESW_CH1_T2_pin GPIO_PIN_5
#define TIMESW_CH1_T2_GPIO GPIOD
#define TIMESW_CH1                                                                                                     \
	(((TIMESW_CH1_T2_GPIO->IDR & TIMESW_CH1_T2_pin) ? 0b10 : 0b00) |                                                   \
	 ((TIMESW_CH1_T1_GPIO->IDR & TIMESW_CH1_T1_pin) ? 0b01 : 0b00))

#define TIMESW_CH2_T1_pin GPIO_PIN_15
#define TIMESW_CH2_T1_GPIO GPIOB
#define TIMESW_CH2_T2_pin GPIO_PIN_7
#define TIMESW_CH2_T2_GPIO GPIOC
#define TIMESW_CH2                                                                                                     \
	(((TIMESW_CH2_T2_GPIO->IDR & TIMESW_CH2_T2_pin) ? 0b10 : 0b00) |                                                   \
	 ((TIMESW_CH2_T1_GPIO->IDR & TIMESW_CH2_T1_pin) ? 0b01 : 0b00))

// Jumper 1 (aka DEBUG3) is not connected, so init its pins as JUMPER 2
// And always return 0 (no jumper installed)
#define JUMPER_1_GPIO GPIOD
#define JUMPER_1_pin GPIO_PIN_12
#define JUMPER_1 (0)

// DEBUG2
#define JUMPER_2_GPIO GPIOD
#define JUMPER_2_pin GPIO_PIN_12
#define JUMPER_2 (!(JUMPER_2_GPIO->IDR & JUMPER_2_pin))

#define DCINPUT_JUMPER JUMPER_1
#define MODE_24BIT_JUMPER JUMPER_2

//OUTPUTS

//CLK OUT

#define CLKOUT_pin GPIO_PIN_12
#define CLKOUT_GPIO GPIOA
#define CLKOUT_ON CLKOUT_GPIO->BSRR = CLKOUT_pin
#define CLKOUT_OFF CLKOUT_GPIO->BSRR = (CLKOUT_pin << 16)
#define CLKOUT_TRIG_TIME 960 /*20ms*/

#define CLKOUT1_pin GPIO_PIN_6
#define CLKOUT1_GPIO GPIOD
#define CLKOUT1_ON CLKOUT1_GPIO->BSRR = CLKOUT1_pin
#define CLKOUT1_OFF CLKOUT1_GPIO->BSRR = (CLKOUT1_pin << 16)

#define CLKOUT2_pin GPIO_PIN_6
#define CLKOUT2_GPIO GPIOC
#define CLKOUT2_ON CLKOUT2_GPIO->BSRR = CLKOUT2_pin
#define CLKOUT2_OFF CLKOUT2_GPIO->BSRR = (CLKOUT2_pin << 16)

//PING Button LED

#define LED_PINGBUT_pin GPIO_PIN_15
#define LED_PINGBUT_GPIO GPIOA
#define LED_PINGBUT_ON LED_PINGBUT_GPIO->BSRR = LED_PINGBUT_pin
#define LED_PINGBUT_OFF LED_PINGBUT_GPIO->BSRR = (LED_PINGBUT_pin << 16)

//INF REPEAT Button LED
#define LED_INF1_pin GPIO_PIN_3
#define LED_INF1_GPIO GPIOD
#define LED_INF1_ON LED_INF1_GPIO->BSRR = LED_INF1_pin
#define LED_INF1_OFF LED_INF1_GPIO->BSRR = (LED_INF1_pin << 16)

#define LED_INF2_pin GPIO_PIN_14
#define LED_INF2_GPIO GPIOC
#define LED_INF2_ON LED_INF2_GPIO->BSRR = LED_INF2_pin
#define LED_INF2_OFF LED_INF2_GPIO->BSRR = (LED_INF2_pin << 16)

#define LED_REV1_pin GPIO_PIN_15
#define LED_REV1_GPIO GPIOC
#define LED_REV1_ON LED_REV1_GPIO->BSRR = LED_REV1_pin
#define LED_REV1_OFF LED_REV1_GPIO->BSRR = (LED_REV1_pin << 16)

#define LED_REV2_pin GPIO_PIN_14
#define LED_REV2_GPIO GPIOG
#define LED_REV2_ON LED_REV2_GPIO->BSRR = LED_REV2_pin
#define LED_REV2_OFF LED_REV2_GPIO->BSRR = (LED_REV2_pin << 16)

//LOOP LEDs

#define LED_LOOP1 GPIO_PIN_13
#define LED_LOOP1_GPIO GPIOC
#define LED_LOOP1_ON LED_LOOP1_GPIO->BSRR = LED_LOOP1
#define LED_LOOP1_OFF LED_LOOP1_GPIO->BSRR = (LED_LOOP1 << 16)

#define LED_LOOP2 GPIO_PIN_10
#define LED_LOOP2_GPIO GPIOF
#define LED_LOOP2_ON LED_LOOP2_GPIO->BSRR = LED_LOOP2
#define LED_LOOP2_OFF LED_LOOP2_GPIO->BSRR = (LED_LOOP2 << 16)

//USART TX
#define DEBUG0 GPIO_PIN_10
#define DEBUG0_GPIO GPIOC
#define DEBUG0_ON DEBUG0_GPIO->BSRR = DEBUG0
#define DEBUG0_OFF DEBUG0_GPIO->BSRR = (DEBUG0 << 16)

//USART RX
#define DEBUG1 GPIO_PIN_11
#define DEBUG1_GPIO GPIOC
#define DEBUG1_ON DEBUG1_GPIO->BSRR = DEBUG1
#define DEBUG1_OFF DEBUG1_GPIO->BSRR = (DEBUG1 << 16)

//DEBUG0
#define DEBUG2 GPIO_PIN_14
#define DEBUG2_GPIO GPIOB
#define DEBUG2_ON DEBUG2_GPIO->BSRR = DEBUG2
#define DEBUG2_OFF DEBUG2_GPIO->BSRR = (DEBUG2 << 16)

//DEBUG1
#define DEBUG3 GPIO_PIN_10
#define DEBUG3_GPIO GPIOA
#define DEBUG3_ON DEBUG3_GPIO->BSRR = DEBUG3
#define DEBUG3_OFF DEBUG3_GPIO->BSRR = (DEBUG3 << 16)

#define INFREVBUTTONJACK_PINGBUT_TIM TIM4
#define INFREVBUTTONJACK_PINGBUT_TIM_RCC __HAL_RCC_TIM4_CLK_ENABLE
#define INFREVBUTTONJACK_PINGBUT_TIM_IRQn TIM4_IRQn
#define INFREVBUTTONJACK_PINGBUT_IRQHandler TIM4_IRQHandler

void init_dig_inouts(void);

#endif /* INOUTS_H_ */
