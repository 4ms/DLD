/*
 * system_settings.h
 *
 *  Created on: Apr 26, 2016
 *      Author: design
 */

#ifndef SYSTEM_SETTINGS_H_
#define SYSTEM_SETTINGS_H_

#include <stm32f4xx.h>

void check_entering_system_mode(void);
void update_system_settings(void);
void set_default_system_settings(void);

void update_system_settings_button_leds(void);

#endif /* SYSTEM_SETTINGS_H_ */
