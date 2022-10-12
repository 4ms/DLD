/*
 * flash.c - mid-level Flash r/w fuctions
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

#include "flash.h"
#include "flash_layout.h"
#include "globals.h"
#include "stm32f4xx.h"

typedef unsigned FlashStatus;
static void _clear_error_codes();


static FlashStatus _erase_sector(uint32_t sector) {
	FLASH_EraseInitTypeDef erase_conf = {
		.TypeErase = FLASH_TYPEERASE_SECTORS,
		.Banks = FLASH_BANK_1,
		.Sector = sector,
		.NbSectors = 1,
		.VoltageRange = FLASH_VOLTAGE_RANGE_3,
	};

	uint32_t err;
	return HAL_FLASHEx_Erase(&erase_conf, &err);
}

FlashStatus flash_erase_sector(uint32_t address) {
	HAL_FLASH_Unlock();
	for (uint32_t i = 0; i < 8; ++i) {
		if (address == get_sector_addr(i)) {
			return _erase_sector(i);
		}
	}
	HAL_FLASH_Lock();

	return 99; // address out of range
}

FlashStatus flash_open_erase_sector(uint32_t address) {
	for (uint32_t i = 0; i < 8; ++i) {
		if (address == get_sector_addr(i)) {
			return _erase_sector(i);
		}
	}
	return 99; // address out of range
}

void flash_begin_open_program(void) {
	_clear_error_codes();
	HAL_FLASH_Unlock();
}

FlashStatus flash_open_program_byte(uint8_t byte, uint32_t address) {
	if (address < get_sector_addr(1))
		return HAL_FLASH_ERROR_PGP;
	else
		return HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, byte);
}

FlashStatus flash_open_program_word(uint32_t word, uint32_t address) {
	if (address < get_sector_addr(1))
		return HAL_FLASH_ERROR_PGP;
	else
		return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, word);
}

void flash_end_open_program(void) {
	HAL_FLASH_Lock();
}

// size is in # of bytes
FlashStatus flash_open_program_array(uint8_t *arr, uint32_t address, uint32_t size) {
	FlashStatus status;

	while (size--) {
		if (address >= get_sector_addr(1))
			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, *arr);
		*arr++;
		address++;
	}
	return status;
}

// size in # of bytes
void flash_read_array(uint8_t *arr, uint32_t address, uint32_t size) {

	while (size--) {
		*arr++ = (uint8_t)(*(__IO uint32_t *)address);
		address++;
	}
}

uint32_t flash_read_word(uint32_t address) {
	return (*(__IO uint32_t *)address);
}

uint8_t flash_read_byte(uint32_t address) {
	return ((uint8_t)(*(__IO uint32_t *)address));
}

static void _clear_error_codes() {
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGAERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGPERR);
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGSERR);
#if defined(FLASH_SR_RDERR)
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_RDERR);
#endif
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR);
}
