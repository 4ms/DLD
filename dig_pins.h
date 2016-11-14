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

#define PING_RCC RCC_AHB1Periph_GPIOE

#define PINGBUT_pin GPIO_Pin_5
#define PINGBUT_GPIO GPIOE
#define PINGBUT (!(PINGBUT_GPIO->IDR & PINGBUT_pin))

#define PINGJACK_pin GPIO_Pin_2
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
#define INF_RCC RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG

#define INF1BUT_pin GPIO_Pin_13
#define INF1BUT_GPIO GPIOC
#define INF1BUT (!(INF1BUT_GPIO->IDR & INF1BUT_pin))

#define INF1JACK_pin GPIO_Pin_7
#define INF1JACK_GPIO GPIOD
#define INF1JACK ((INF1JACK_GPIO->IDR & INF1JACK_pin))


#define INF2BUT_pin GPIO_Pin_9
#define INF2BUT_GPIO GPIOA
#define INF2BUT (!(INF2BUT_GPIO->IDR & INF2BUT_pin))

#define INF2JACK_pin GPIO_Pin_6
#define INF2JACK_GPIO GPIOG
#define INF2JACK ((INF2JACK_GPIO->IDR & INF2JACK_pin))


#define REV_RCC RCC_AHB1Periph_GPIOD

#define REV1JACK_pin GPIO_Pin_11
#define REV1JACK_GPIO GPIOD
#define REV1JACK ((REV1JACK_GPIO->IDR & REV1JACK_pin))

#define REV2JACK_pin GPIO_Pin_3
#define REV2JACK_GPIO GPIOD
#define REV2JACK ((REV2JACK_GPIO->IDR & REV2JACK_pin))



#define REVBUT_RCC RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOG

#define REV1BUT_pin GPIO_Pin_10
#define REV1BUT_GPIO GPIOG
#define REV1BUT (!(REV1BUT_GPIO->IDR & REV1BUT_pin))

#define REV2BUT_pin GPIO_Pin_1
#define REV2BUT_GPIO GPIOA
#define REV2BUT (!(REV2BUT_GPIO->IDR & REV2BUT_pin))


#define TIMESW_RCC RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE

#define SWITCH_CENTER 0b11
#define SWITCH_UP 0b10
#define SWITCH_DOWN 0b01
#define SWITCH_INVALID 0b00

#define TIMESW_CH1_T1_pin GPIO_Pin_11
#define TIMESW_CH1_T1_GPIO GPIOG
#define TIMESW_CH1_T2_pin GPIO_Pin_12
#define TIMESW_CH1_T2_GPIO GPIOG
#define TIMESW_CH1 (((TIMESW_CH1_T2_GPIO->IDR & TIMESW_CH1_T2_pin) ? 0b10:0b00) | ((TIMESW_CH1_T1_GPIO->IDR & TIMESW_CH1_T1_pin) ? 0b01:0b00))

#define TIMESW_CH2_T1_pin GPIO_Pin_2
#define TIMESW_CH2_T1_GPIO GPIOA
#define TIMESW_CH2_T2_pin GPIO_Pin_2
#define TIMESW_CH2_T2_GPIO GPIOB
#define TIMESW_CH2 (((TIMESW_CH2_T2_GPIO->IDR & TIMESW_CH2_T2_pin) ? 0b10:0b00) | ((TIMESW_CH2_T1_GPIO->IDR & TIMESW_CH2_T1_pin) ? 0b01:0b00))


#define JUMPER_RCC (RCC_AHB1Periph_GPIOC /*| RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOB*/)

#define JUMPER_1_GPIO GPIOC
#define JUMPER_1_pin GPIO_Pin_15
#define JUMPER_1 (!(JUMPER_1_GPIO->IDR & JUMPER_1_pin))

#define JUMPER_2_GPIO GPIOC
#define JUMPER_2_pin GPIO_Pin_14
#define JUMPER_2 (!(JUMPER_2_GPIO->IDR & JUMPER_2_pin))

/*
#define JUMPER_3_GPIO GPIOE
#define JUMPER_3_pin GPIO_Pin_6
#define JUMPER_3 (!(JUMPER_3_GPIO->IDR & JUMPER_3_pin))


#define JUMPER_4_GPIO GPIOB
#define JUMPER_4_pin GPIO_Pin_7
#define JUMPER_4 (!(JUMPER_4_GPIO->IDR & JUMPER_4_pin))
*/

#define DCINPUT_JUMPER JUMPER_1
#define MODE_24BIT_JUMPER JUMPER_2


//OUTPUTS

//CLK OUT
#define CLKOUT_RCC RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC

#define CLKOUT_pin GPIO_Pin_3
#define CLKOUT_GPIO GPIOE
#define CLKOUT_ON CLKOUT_GPIO->BSRRL = CLKOUT_pin
#define CLKOUT_OFF CLKOUT_GPIO->BSRRH = CLKOUT_pin
#define CLKOUT_TRIG_TIME 960 /*20ms*/

#define CLKOUT1_pin GPIO_Pin_8
#define CLKOUT1_GPIO GPIOC
#define CLKOUT1_ON CLKOUT1_GPIO->BSRRL = CLKOUT1_pin
#define CLKOUT1_OFF CLKOUT1_GPIO->BSRRH = CLKOUT1_pin

#define CLKOUT2_pin GPIO_Pin_5
#define CLKOUT2_GPIO GPIOC
#define CLKOUT2_ON CLKOUT2_GPIO->BSRRL = CLKOUT2_pin
#define CLKOUT2_OFF CLKOUT2_GPIO->BSRRH = CLKOUT2_pin

//PING Button LED
#define PINGBUTLED_RCC RCC_AHB1Periph_GPIOE

#define LED_PINGBUT_pin GPIO_Pin_4
#define LED_PINGBUT_GPIO GPIOE
#define LED_PINGBUT_ON LED_PINGBUT_GPIO->BSRRL = LED_PINGBUT_pin
#define LED_PINGBUT_OFF LED_PINGBUT_GPIO->BSRRH = LED_PINGBUT_pin

//INF REPEAT Button LED
#define INF1_BUTLED_RCC RCC_AHB1Periph_GPIOD
#define LED_INF1_pin GPIO_Pin_4
#define LED_INF1_GPIO GPIOD
#define LED_INF1_ON LED_INF1_GPIO->BSRRL = LED_INF1_pin
#define LED_INF1_OFF LED_INF1_GPIO->BSRRH = LED_INF1_pin

#define INF2_BUTLED_RCC RCC_AHB1Periph_GPIOA
#define LED_INF2_pin GPIO_Pin_10
#define LED_INF2_GPIO GPIOA
#define LED_INF2_ON LED_INF2_GPIO->BSRRL = LED_INF2_pin
#define LED_INF2_OFF LED_INF2_GPIO->BSRRH = LED_INF2_pin

#define REV1_BUTLED_RCC RCC_AHB1Periph_GPIOA
#define LED_REV1_pin GPIO_Pin_15
#define LED_REV1_GPIO GPIOA

#define LED_REV1_ON LED_REV1_GPIO->BSRRL = LED_REV1_pin
#define LED_REV1_OFF LED_REV1_GPIO->BSRRH = LED_REV1_pin

#define REV2_BUTLED_RCC RCC_AHB1Periph_GPIOD
#define LED_REV2_pin GPIO_Pin_2
#define LED_REV2_GPIO GPIOD
#define LED_REV2_ON LED_REV2_GPIO->BSRRL = LED_REV2_pin
#define LED_REV2_OFF LED_REV2_GPIO->BSRRH = LED_REV2_pin


//LOOP LEDs
#define LED_RCC RCC_AHB1Periph_GPIOA

#define LED_LOOP1 GPIO_Pin_11
#define LED_LOOP2 GPIO_Pin_12
#define LED_GPIO GPIOA
#define LED_LOOP1_ON LED_GPIO->BSRRL = LED_LOOP1
#define LED_LOOP1_OFF LED_GPIO->BSRRH = LED_LOOP1
#define LED_LOOP2_ON LED_GPIO->BSRRL = LED_LOOP2
#define LED_LOOP2_OFF LED_GPIO->BSRRH = LED_LOOP2


//DEBUG pins
#define DEBUG_RCC (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOA)

#define DEBUG0 GPIO_Pin_5
#define DEBUG0_GPIO GPIOD
#define DEBUG0_ON DEBUG0_GPIO->BSRRL = DEBUG0
#define DEBUG0_OFF DEBUG0_GPIO->BSRRH = DEBUG0

#define DEBUG1 GPIO_Pin_6
#define DEBUG1_GPIO GPIOD
#define DEBUG1_ON DEBUG1_GPIO->BSRRL = DEBUG1
#define DEBUG1_OFF DEBUG1_GPIO->BSRRH = DEBUG1

#define DEBUG2 GPIO_Pin_7
#define DEBUG2_GPIO GPIOB
#define DEBUG2_ON DEBUG2_GPIO->BSRRL = DEBUG2
#define DEBUG2_OFF DEBUG2_GPIO->BSRRH = DEBUG2

#define DEBUG3 GPIO_Pin_6
#define DEBUG3_GPIO GPIOE
#define DEBUG3_ON DEBUG3_GPIO->BSRRL = DEBUG3
#define DEBUG3_OFF DEBUG3_GPIO->BSRRH = DEBUG3

/*
#define DEBUG4 GPIO_Pin_5
#define DEBUG4_GPIO GPIOA
#define DEBUG4_ON DEBUG4_GPIO->BSRRL = DEBUG4
#define DEBUG4_OFF DEBUG4_GPIO->BSRRH = DEBUG4
*/

void init_dig_inouts(void);


#endif /* INOUTS_H_ */
