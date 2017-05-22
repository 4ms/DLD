/*
 * main.c - Initialization and main loop for Dual Looping Delay
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

#include <stm32f4xx.h>

#include "globals.h"
#include "i2s.h"
#include "adc.h"
#include "looping_delay.h"
#include "params.h"
#include "timekeeper.h"
#include "sdram.h"
#include "calibration.h"
#include "flash_user.h"
#include "leds.h"
#include "system_settings.h"
#include "buttons_jacks.h"
#include "dig_pins.h"
#include "codec_CS4271.h"
#include "RAM_test.h"


uint32_t g_error=0;

__IO uint16_t potadc_buffer[NUM_POT_ADCS];
__IO uint16_t cvadc_buffer[NUM_CV_ADCS];

extern uint8_t global_mode[NUM_GLOBAL_MODES];

extern uint32_t flash_firmware_version;

void check_errors(void){

}

uint8_t check_bootloader_keys(void)
{
	uint32_t dly;
	uint32_t button_debounce=0;

	dly=32000;
	while(dly--){
		if (BOOTLOADER_BUTTONS) button_debounce++;
		else button_debounce=0;
	}
	return (button_debounce>15000);

}

typedef void (*EntryPoint)(void);

void JumpTo(uint32_t address) {
  uint32_t application_address = *(__IO uint32_t*)(address + 4);
  EntryPoint application = (EntryPoint)(application_address);
  __set_MSP(*(__IO uint32_t*)address);
  application();
}


int main(void)
{
	uint32_t do_factory_reset=0;

    if (check_bootloader_keys())
    	JumpTo(0x08000000);

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);

    Codecs_Deinit();

    init_dig_inouts();

    DeInit_I2S_Clock();
	DeInit_I2SDMA_Channel2();
	DeInit_I2SDMA_Channel1();

	delay();

	init_timekeeper();

	Deinit_Pot_ADC();
	Deinit_CV_ADC();

	Init_Pot_ADC((uint16_t *)potadc_buffer, NUM_POT_ADCS);
	Init_CV_ADC((uint16_t *)cvadc_buffer, NUM_CV_ADCS);

	delay();

	init_LowPassCoefs();

	SDRAM_Init();

	if (RAMTEST_BUTTONS) RAM_startup_test();

	init_LED_PWM_IRQ();

	audio_buffer_init();

	delay();

	Codec_GPIO_Init();
	Codec_AudioInterface_Init(I2S_AudioFreq_48k);

	init_audio_dma();

	global_mode[DCINPUT] = DCINPUT_JUMPER;

	Codec_Register_Setup(global_mode[DCINPUT]);

	global_mode[CALIBRATE] = 0;

	init_adc_param_update_timer();

	init_params();
	init_modes();

	flash_firmware_version = load_flash_params();

    if (ENTER_CALIBRATE_BUTTONS)
    {
    	global_mode[CALIBRATE] = 1;
    }
    else if (flash_firmware_version <= 1 ) //If we detect a pre-production version of firmware, then check the RAM and do a factory reset
    {
    	if (RAM_test()==0)
    	{
    		global_mode[CALIBRATE] = 1;
    		do_factory_reset = 960000; //run normally for about 6 seconds before calibrating the CV jacks
    	}
    	else
    		while (1) blink_all_lights(50); //RAM Test failed: It's on the fritz!

    }
    else if (flash_firmware_version < FW_VERSION ) //If we detect a recently upgraded firmware version
    {
    	set_firmware_version();
    	store_params_into_sram();
    	write_all_params_to_FLASH();

    }

	init_inputread_timer();

	Start_I2SDMA_Channel1();
	Start_I2SDMA_Channel2();

	while(1){


		if (global_mode[QUANTIZE_MODE_CHANGES]==0)
		{
			process_mode_flags(0);
			process_mode_flags(1);
		}
		else
		{
			process_ping_changed(0);
			process_ping_changed(1);
		}


		check_errors();
		

    	if (do_factory_reset)
    		if (!(--do_factory_reset))
    			factory_reset(1);

	}

	return(1);
}

//#define USE_FULL_ASSERT

#ifdef  USE_FULL_ASSERT

#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

#if 1
/* exception handlers - so we know what's failing */
void NMI_Handler(void)
{ 
	while(1){};
}

void HardFault_Handler(void)
{ 
	uint8_t foobar;
	uint32_t hfsr,dfsr,afsr,bfar,mmfar,cfsr;

	foobar=0;
	mmfar=SCB->MMFAR;
	bfar=SCB->BFAR;

	hfsr=SCB->HFSR;
	afsr=SCB->AFSR;
	dfsr=SCB->DFSR;
	cfsr=SCB->CFSR;


	if (foobar){
		return;
	} else {
		while(1){};
	}
}

void MemManage_Handler(void)
{ 
	while(1){};
}

void BusFault_Handler(void)
{ 
	while(1){};
}

void UsageFault_Handler(void)
{ 
	while(1){};
}

void SVC_Handler(void)
{ 
	while(1){};
}

void DebugMon_Handler(void)
{ 
	while(1){};
}

void PendSV_Handler(void)
{ 
	while(1){};
}
#endif
