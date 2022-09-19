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

#define PINGBUT_pin GPIO_PIN_5
#define PINGBUT_GPIO GPIOE
#define PINGBUT (!(PINGBUT_GPIO->IDR & PINGBUT_pin))

#define PINGJACK_pin GPIO_PIN_2
#define PINGJACK_GPIO GPIOE
#define PINGJACK (PINGJACK_GPIO->IDR & PINGJACK_pin)
//#define PINGJACK 0

//Same pin as WS clock of I2S2 (LRCLK), as defined in codec.h
#define EXTI_CLOCK_GPIO EXTI_PortSourceGPIOB
#define EXTI_CLOCK_pin EXTI_PinSource12
#define EXTI_CLOCK_line EXTI_Line12
#define EXTI_CLOCK_IRQ EXTI15_10_IRQn
#define EXTI_Handler EXTI15_10_IRQHandler

// Infinite Repeat Button and Jack

#define INF1BUT_pin GPIO_PIN_13
#define INF1BUT_GPIO GPIOC
#define INF1BUT (!(INF1BUT_GPIO->IDR & INF1BUT_pin))

#define INF1JACK_pin GPIO_PIN_7
#define INF1JACK_GPIO GPIOD
#define INF1JACK ((INF1JACK_GPIO->IDR & INF1JACK_pin))

#define INF2BUT_pin GPIO_PIN_9
#define INF2BUT_GPIO GPIOA
#define INF2BUT (!(INF2BUT_GPIO->IDR & INF2BUT_pin))

#define INF2JACK_pin GPIO_PIN_6
#define INF2JACK_GPIO GPIOG
#define INF2JACK ((INF2JACK_GPIO->IDR & INF2JACK_pin))

#define REV1JACK_pin GPIO_PIN_11
#define REV1JACK_GPIO GPIOD
#define REV1JACK ((REV1JACK_GPIO->IDR & REV1JACK_pin))

#define REV2JACK_pin GPIO_PIN_3
#define REV2JACK_GPIO GPIOD
#define REV2JACK ((REV2JACK_GPIO->IDR & REV2JACK_pin))

#define REV1BUT_pin GPIO_PIN_10
#define REV1BUT_GPIO GPIOG
#define REV1BUT (!(REV1BUT_GPIO->IDR & REV1BUT_pin))

#define REV2BUT_pin GPIO_PIN_1
#define REV2BUT_GPIO GPIOA
#define REV2BUT (!(REV2BUT_GPIO->IDR & REV2BUT_pin))

#define SWITCH_CENTER 0b11
#define SWITCH_UP 0b10
#define SWITCH_DOWN 0b01
#define SWITCH_INVALID 0b00

#define TIMESW_CH1_T1_pin GPIO_PIN_11
#define TIMESW_CH1_T1_GPIO GPIOG
#define TIMESW_CH1_T2_pin GPIO_PIN_12
#define TIMESW_CH1_T2_GPIO GPIOG
#define TIMESW_CH1                                                                                                     \
	(((TIMESW_CH1_T2_GPIO->IDR & TIMESW_CH1_T2_pin) ? 0b10 : 0b00) |                                                   \
	 ((TIMESW_CH1_T1_GPIO->IDR & TIMESW_CH1_T1_pin) ? 0b01 : 0b00))

#define TIMESW_CH2_T1_pin GPIO_PIN_2
#define TIMESW_CH2_T1_GPIO GPIOA
#define TIMESW_CH2_T2_pin GPIO_PIN_2
#define TIMESW_CH2_T2_GPIO GPIOB
#define TIMESW_CH2                                                                                                     \
	(((TIMESW_CH2_T2_GPIO->IDR & TIMESW_CH2_T2_pin) ? 0b10 : 0b00) |                                                   \
	 ((TIMESW_CH2_T1_GPIO->IDR & TIMESW_CH2_T1_pin) ? 0b01 : 0b00))

#define JUMPER_1_GPIO GPIOC
#define JUMPER_1_pin GPIO_PIN_15
#define JUMPER_1 (!(JUMPER_1_GPIO->IDR & JUMPER_1_pin))

#define JUMPER_2_GPIO GPIOC
#define JUMPER_2_pin GPIO_PIN_14
#define JUMPER_2 (!(JUMPER_2_GPIO->IDR & JUMPER_2_pin))

/*
#define JUMPER_3_GPIO GPIOE
#define JUMPER_3_pin GPIO_PIN_6
#define JUMPER_3 (!(JUMPER_3_GPIO->IDR & JUMPER_3_pin))


#define JUMPER_4_GPIO GPIOB
#define JUMPER_4_pin GPIO_PIN_7
#define JUMPER_4 (!(JUMPER_4_GPIO->IDR & JUMPER_4_pin))
*/

#define DCINPUT_JUMPER JUMPER_1
#define MODE_24BIT_JUMPER JUMPER_2

//OUTPUTS

//CLK OUT

#define CLKOUT_pin GPIO_PIN_3
#define CLKOUT_GPIO GPIOE
#define CLKOUT_ON CLKOUT_GPIO->BSRR = CLKOUT_pin
#define CLKOUT_OFF CLKOUT_GPIO->BSRR = (CLKOUT_pin << 16)
#define CLKOUT_TRIG_TIME 960 /*20ms*/

#define CLKOUT1_pin GPIO_PIN_8
#define CLKOUT1_GPIO GPIOC
#define CLKOUT1_ON CLKOUT1_GPIO->BSRR = CLKOUT1_pin
#define CLKOUT1_OFF CLKOUT1_GPIO->BSRR = (CLKOUT1_pin << 16)

#define CLKOUT2_pin GPIO_PIN_5
#define CLKOUT2_GPIO GPIOC
#define CLKOUT2_ON CLKOUT2_GPIO->BSRR = CLKOUT2_pin
#define CLKOUT2_OFF CLKOUT2_GPIO->BSRR = (CLKOUT2_pin << 16)

//PING Button LED

#define LED_PINGBUT_pin GPIO_PIN_4
#define LED_PINGBUT_GPIO GPIOE
#define LED_PINGBUT_ON LED_PINGBUT_GPIO->BSRR = LED_PINGBUT_pin
#define LED_PINGBUT_OFF LED_PINGBUT_GPIO->BSRR = (LED_PINGBUT_pin << 16)

//INF REPEAT Button LED
#define LED_INF1_pin GPIO_PIN_4
#define LED_INF1_GPIO GPIOD
#define LED_INF1_ON LED_INF1_GPIO->BSRR = LED_INF1_pin
#define LED_INF1_OFF LED_INF1_GPIO->BSRR = (LED_INF1_pin << 16)

#define LED_INF2_pin GPIO_PIN_10
#define LED_INF2_GPIO GPIOA
#define LED_INF2_ON LED_INF2_GPIO->BSRR = LED_INF2_pin
#define LED_INF2_OFF LED_INF2_GPIO->BSRR = (LED_INF2_pin << 16)

#define LED_REV1_pin GPIO_PIN_15
#define LED_REV1_GPIO GPIOA

#define LED_REV1_ON LED_REV1_GPIO->BSRR = LED_REV1_pin
#define LED_REV1_OFF LED_REV1_GPIO->BSRR = (LED_REV1_pin << 16)

#define LED_REV2_pin GPIO_PIN_2
#define LED_REV2_GPIO GPIOD
#define LED_REV2_ON LED_REV2_GPIO->BSRR = LED_REV2_pin
#define LED_REV2_OFF LED_REV2_GPIO->BSRR = (LED_REV2_pin << 16)

//LOOP LEDs

#define LED_LOOP1 GPIO_PIN_11
#define LED_LOOP2 GPIO_PIN_12
#define LED_GPIO GPIOA
#define LED_LOOP1_ON LED_GPIO->BSRR = LED_LOOP1
#define LED_LOOP1_OFF LED_GPIO->BSRR = (LED_LOOP1 << 16)
#define LED_LOOP2_ON LED_GPIO->BSRR = LED_LOOP2
#define LED_LOOP2_OFF LED_GPIO->BSRR = (LED_LOOP2 << 16)

#define DEBUG0 GPIO_PIN_5
#define DEBUG0_GPIO GPIOD
#define DEBUG0_ON DEBUG0_GPIO->BSRR = DEBUG0
#define DEBUG0_OFF DEBUG0_GPIO->BSRR = (DEBUG0 << 16)

#define DEBUG1 GPIO_PIN_6
#define DEBUG1_GPIO GPIOD
#define DEBUG1_ON DEBUG1_GPIO->BSRR = DEBUG1
#define DEBUG1_OFF DEBUG1_GPIO->BSRR = (DEBUG1 << 16)

#define DEBUG2 GPIO_PIN_7
#define DEBUG2_GPIO GPIOB
#define DEBUG2_ON DEBUG2_GPIO->BSRR = DEBUG2
#define DEBUG2_OFF DEBUG2_GPIO->BSRR = (DEBUG2 << 16)

#define DEBUG3 GPIO_PIN_6
#define DEBUG3_GPIO GPIOE
#define DEBUG3_ON DEBUG3_GPIO->BSRR = DEBUG3
#define DEBUG3_OFF DEBUG3_GPIO->BSRR = (DEBUG3 << 16)

#define INFREVBUTTONJACK_PINGBUT_TIM TIM4
#define INFREVBUTTONJACK_PINGBUT_TIM_RCC RCC_APB1Periph_TIM4
#define INFREVBUTTONJACK_PINGBUT_TIM_IRQn TIM4_IRQn
#define INFREVBUTTONJACK_PINGBUT_IRQHandler TIM4_IRQHandler

void init_dig_inouts(void);

#endif /* INOUTS_H_ */
