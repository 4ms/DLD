/*
 * flash_user.h
 *
 *  Created on: Mar 4, 2016
 *      Author: design
 */

#ifndef FLASH_USER_H_
#define FLASH_USER_H_
#include <stm32f4xx.h>

void store_params_into_sram(void);
void write_all_params_to_FLASH(void);

void factory_reset(void);

#endif /* FLASH_USER_H_ */
