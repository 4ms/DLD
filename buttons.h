/*
 * buttons.h
 *
 *  Created on: Aug 18, 2016
 *      Author: design
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_
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



#endif /* BUTTONS_H_ */
