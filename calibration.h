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
void read_all_params_from_FLASH(void);
void update_calibration(void);
uint32_t load_calibration(void);
void save_calibration(void);
void auto_calibrate(void);
void check_calibration_mode(void);

#endif /* CALIBRATION_H_ */
