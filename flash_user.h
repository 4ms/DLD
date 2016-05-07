/*
 * flash_user.h
 *
 *  Created on: Mar 4, 2016
 *      Author: design
 */

#ifndef FLASH_USER_H_
#define FLASH_USER_H_
#include <stm32f4xx.h>

void set_firmware_version(void);
void factory_reset(uint8_t loop_afterwards);
uint32_t load_flash_params(void);
void save_flash_params(void);
void store_params_into_sram(void);
void write_all_params_to_FLASH(void);
void read_all_params_from_FLASH(void);

#endif /* FLASH_USER_H_ */
