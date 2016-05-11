/*
 * to-do:
 *
 * Software:
 *
 * Would it be more efficient to have a routine to change INF/REV/Time modes, instead of setting a flag and polling a process_() routine to check the flag:
 * i.e.:
 * if (trigger detected on inf jack)
 * 		set_inf_mode(!mode[channel][INF]);
 *
 * 	if (inf button detected falling edge w/o pot wiggle)
 * 		set_inf_mode(!mode[channel][INF]);
 *
 *
 *
 * Make a nvic_control.c file, which has a table of all nvics we use.
 * Then enable each nvic in the current place.
 * ...Some central place to compare all priorities.
 * ...This makes us a bit less portable? Unless we implement the nvic_control.c file on all projects
 *
 *
 * Initialize all arrays with size NUM_POT_ADCS OR NUM_CV_ADCS or NUM_GLOBAL_MODES etc. with a init_() function, not in the declaration
 *
 *
 * Change mode[0][MAIN_CLOCK_GATETRIG] to global_mode[MAIN_CLOCK_GATETRIG]
 *
 *
 * Create a system settings input to turn on/off global_mode[AUTO_UNQ]
 *
 * change name of loopled_tmr to channel_tmr
 */

#include <stm32f4xx.h>

#include "globals.h"
#include "codec.h"
#include "i2s.h"
#include "adc.h"
#include "looping_delay.h"
#include "dig_inouts.h"
#include "params.h"
#include "timekeeper.h"
#include "sdram.h"
#include "si5153a.h"
#include "calibration.h"
#include "flash_user.h"
#include "leds.h"


uint32_t g_error=0;

__IO uint16_t potadc_buffer[NUM_POT_ADCS];
__IO uint16_t cvadc_buffer[NUM_CV_ADCS];

extern uint8_t global_mode[NUM_GLOBAL_MODES];
extern uint8_t flag_time_param_changed[2];
extern uint32_t flash_firmware_version;

void check_errors(void){

}


int main(void)
{
	uint32_t do_factory_reset=0;

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);

    Codecs_Deinit();

    init_dig_inouts();

    DeInit_I2S_Clock();
	DeInit_I2SDMA_Channel2();
	DeInit_I2SDMA_Channel1();

#ifdef HAS_VCXO
	init_VCXO();

#ifdef USE_VCXO
	setupPLLInt(SI5351_PLL_A, 15); //375Mhz
	setupPLLInt(SI5351_PLL_B, 15);
	setupMultisynth(0, SI5351_PLL_A, 36, 265, 512);
	setupMultisynth(1, SI5351_PLL_A, 36, 265, 512);

	delay();

#else
	Si5351a_enableOutputs(0);
#endif
#endif

	delay();

	init_timekeeper();
	init_inputread_timer();

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
#ifdef HAS_VCXO
#ifdef USE_VCXO
	Si5351a_enableOutputs(1);
#endif
#endif

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
//    else if (load_flash_params() == -1 ) //This line is for pre-production. For the production release, change FW_VERSION to 2, and replace this line with the one below:
  else if (flash_firmware_version <=1 ) //If we detect an early version of firmware, then check the RAM and do a factory reset
    {
    	if (RAM_test()==0)
    	{
    		global_mode[CALIBRATE] = 1;
    		do_factory_reset = 960000; //about 6 seconds
    	}
    	else
    		while (1) blink_all_lights(50); //It's on the fritz!

    }

	Start_I2SDMA_Channel1();
	Start_I2SDMA_Channel2();

	while(1){

		check_entering_system_mode();

		process_mode_flags();

		check_errors();
		
		update_ping_ledbut();
		update_INF_REV_ledbut(0);
		update_INF_REV_ledbut(1);

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
