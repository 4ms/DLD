/*
 * RAM_test.c - tests for SDRAM
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

#include "globals.h"
#include "gpiof4.h"
#include "sdram.h"
#include "leds.h"
#include "dig_pins.h"

uint32_t RAM_test(void){

	uint32_t addr;
	uint32_t i;
	uint16_t rd0;
	uint16_t rd1;
	volatile uint32_t fail=0;

	addr=SDRAM_BASE;
	for (i=0;i<(SDRAM_SIZE/2);i++){
		LED_REV1_ON;
		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		LED_REV1_OFF;

		rd1 = (uint16_t)((i) & 0x0000FFFF);
		*((uint16_t *)addr) = rd1;

		addr+=2;


	}

	addr=SDRAM_BASE;
	for (i=0;i<(SDRAM_SIZE/2);i++){
		LED_REV2_ON;
		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}
		LED_REV2_OFF;

		rd1 = *((uint16_t *)addr);

		rd0=(uint16_t)((i) & 0x0000FFFF);
		if (rd1 != rd0)
		{
			fail++;
		}

		addr+=2;

	}

	return(fail);
}


void RAM_startup_test(void)
{
	volatile register uint32_t ram_errors=0;

	LED_LOOP1_OFF;
	LED_LOOP2_OFF;
	LED_PINGBUT_OFF;
	LED_REV1_OFF;
	LED_REV2_OFF;
	LED_INF1_OFF;
	LED_INF2_OFF;

	ram_errors = RAM_test();

	LED_LOOP1_ON;
	LED_LOOP2_ON;
	LED_PINGBUT_ON;
	LED_REV1_ON;
	LED_REV2_ON;
	LED_INF1_ON;
	LED_INF2_ON;


	//Display the number of bad memory addresses using the seven lights (up to 127 can be shown)
	//If there's 128 or more bad memory addresses, then flash all the lights
	if (ram_errors & 1)
		LED_LOOP1_OFF;
	if (ram_errors & 2)
		LED_LOOP2_OFF;
	if (ram_errors & 4)
		LED_PINGBUT_OFF;
	if (ram_errors & 8)
		LED_REV1_OFF;
	if (ram_errors & 16)
		LED_INF1_OFF;
	if (ram_errors & 32)
		LED_INF2_OFF;
	if (ram_errors & 64)
		LED_REV2_OFF;

	while (1)
	{
		if (ram_errors >= 128)
		{
			blink_all_lights(50);
		}
	}


}
