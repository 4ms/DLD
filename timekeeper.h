/*
 * timekeeper.h
 *
 *  Created on: Mar 29, 2015
 *      Author: design
 */

#ifndef TIMEKEEPER_H_
#define TIMEKEEPER_H_
#include <stm32f4xx.h>

void init_timekeeper(void);

inline void inc_tmrs(void);
inline void reset_ping_ledbut_tmr(void);
inline void reset_ping_tmr(void);
inline void reset_clkout_trigger_tmr(void);
inline void reset_loopled_tmr(uint8_t channel);
void init_adc_param_update_timer(void);


#endif /* TIMEKEEPER_H_ */
