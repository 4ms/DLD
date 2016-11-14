/*
 * buttons_jacks.h - handles button presses and trigger/gate/clock input jacks
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

#ifndef BUTTONS_JACKS_H_
#define BUTTONS_JACKS_H_
#include <stm32f4xx.h>

//RAM Test: On boot: Up/Down + Left three buttons
#define RAMTEST_BUTTONS (\
		(TIMESW_CH1==SWITCH_UP) &&\
		(TIMESW_CH2==SWITCH_DOWN) &&\
		!PINGBUT &&\
		REV1BUT &&\
		INF1BUT &&\
		!REV2BUT &&\
		INF2BUT)

//Save Calibration: Center/Center + four buttons
#define SAVE_CALIBRATE_BUTTONS (\
		(TIMESW_CH1==SWITCH_CENTER) &&\
		(TIMESW_CH2==SWITCH_CENTER) &&\
		!PINGBUT &&\
		REV1BUT &&\
		INF1BUT &&\
		REV2BUT &&\
		INF2BUT)

//Calibrate mode: on boot: Down/Up + Right three buttons
#define ENTER_CALIBRATE_BUTTONS (\
		(TIMESW_CH1==SWITCH_DOWN) &&\
		(TIMESW_CH2==SWITCH_UP) &&\
		!PINGBUT &&\
		!REV1BUT &&\
		INF1BUT &&\
		REV2BUT &&\
		INF2BUT)

//System Mode: Up/Up + Five buttons
#define ENTER_SYSMODE_BUTTONS (\
		(TIMESW_CH1==SWITCH_UP) &&\
		(TIMESW_CH2==SWITCH_UP) &&\
		PINGBUT &&\
		REV1BUT &&\
		INF1BUT &&\
		REV2BUT &&\
		INF2BUT)

//Save System settings: (not implemented)
#define SAVE_SYSMODE_BUTTONS (\
		PINGBUT &&\
		REV1BUT &&\
		INF1BUT &&\
		REV2BUT &&\
		INF2BUT)

//Factory Reset: Boot into Calibration mode (D/U + right 3), then Up/Down + 5 buttons for ten seconds
#define FACTORY_RESET_BUTTONS (\
		(TIMESW_CH1==SWITCH_UP) &&\
		(TIMESW_CH2==SWITCH_DOWN) &&\
		PINGBUT &&\
		REV1BUT &&\
		INF1BUT &&\
		REV2BUT &&\
		INF2BUT)


#define BOOTLOADER_BUTTONS (\
		PINGBUT &&\
		REV1BUT &&\
		!INF1BUT &&\
		REV2BUT &&\
		!INF2BUT)


#define RAM_CLEAR_CH1_BUTTONS (\
		!PINGBUT &&\
		REV1BUT &&\
		INF1BUT &&\
		!REV2BUT &&\
		!INF2BUT)

#define RAM_CLEAR_CH2_BUTTONS (\
		!PINGBUT &&\
		!REV1BUT &&\
		!INF1BUT &&\
		REV2BUT &&\
		INF2BUT)

#define RAM_CLEAR_BOTHCHAN_BUTTONS (\
		!PINGBUT &&\
		REV1BUT &&\
		INF1BUT &&\
		REV2BUT &&\
		INF2BUT)

#define CONTINUOUS_REV1_BUTTONS (\
		!PINGBUT &&\
		REV1BUT &&\
		!INF1BUT &&\
		!REV2BUT &&\
		!INF2BUT)

#define CONTINUOUS_REV2_BUTTONS (\
		!PINGBUT &&\
		!REV1BUT &&\
		!INF1BUT &&\
		REV2BUT &&\
		!INF2BUT)




#define INFREVBUTTONJACK_PINGBUT_TIM TIM4
#define INFREVBUTTONJACK_PINGBUT_TIM_RCC RCC_APB1Periph_TIM4
#define INFREVBUTTONJACK_PINGBUT_TIM_IRQn TIM4_IRQn
#define INFREVBUTTONJACK_PINGBUT_IRQHandler TIM4_IRQHandler

void init_inputread_timer(void);



#endif /* BUTTONS_JACKS_H_ */
