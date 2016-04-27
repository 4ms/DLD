/*
 * leds.h
 *
 *  Created on: Apr 21, 2016
 *      Author: design
 */

#ifndef LEDS_H_
#define LEDS_H_
#include <stm32f4xx.h>


void update_ping_ledbut(void);
void update_channel_leds(uint8_t channel);
void update_inf_ledbut(uint8_t channel);

#endif /* LEDS_H_ */
