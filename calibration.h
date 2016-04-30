/*
 * calibrate.h
 *
 *  Created on: Mar 4, 2016
 *      Author: design
 */

#ifndef CALIBRATION_H_
#define CALIBRATION_H_
#include <stm32f4xx.h>


void set_default_calibration_values(void);
void update_calibration(void);
void auto_calibrate(void);
void update_calibrate_leds(void);

#endif /* CALIBRATION_H_ */
