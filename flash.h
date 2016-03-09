/*
 * flash.h
 *
 *  Created on: Mar 8, 2016
 *      Author: design
 */

#ifndef FLASH_H_
#define FLASH_H_
#include <stm32f4xx.h>

FLASH_Status flash_erase_sector(uint32_t address);
FLASH_Status flash_open_erase_sector(uint32_t address);
void flash_begin_open_program(void);
FLASH_Status flash_open_program_byte(uint8_t byte, uint32_t address);
FLASH_Status flash_open_program_word(uint32_t word, uint32_t address);
void flash_end_open_program(void);
FLASH_Status flash_open_program_array(uint8_t* arr, uint32_t address, uint32_t size);
void flash_read_array(uint8_t* arr, uint32_t address, uint32_t size);
uint32_t flash_read_word(uint32_t address);
uint8_t flash_read_byte(uint32_t address);



#endif /* FLASH_H_ */
