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

void check_errors(void){

}


int main(void)
{

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

	Init_Pot_ADC((uint16_t *)potadc_buffer, NUM_POT_ADCS);
	Init_CV_ADC((uint16_t *)cvadc_buffer, NUM_CV_ADCS);

	delay();

	init_LowPassCoefs();
	init_params();
	init_modes();

	SDRAM_Init();

	if (RAMTEST_BUTTONS) RAM_startup_test();

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

	check_calibration_mode();

	init_adc_param_update_timer();

	Start_I2SDMA_Channel1();
	Start_I2SDMA_Channel2();

//	flag_time_param_changed[0]=1;
//	flag_time_param_changed[1]=1;
//	flag_ping_was_changed=1;

	while(1){//-O1 roughly 400kHz and takes 1us
		process_mode_flags();

		check_errors();
		
		update_ping_ledbut();

		//process_audio();
		update_channel_leds(0);
		update_channel_leds(1);

		update_inf_ledbut(0);
		update_inf_ledbut(1);


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
