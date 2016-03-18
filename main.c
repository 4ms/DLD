/*
 * to-do:
 *
 * Software:
 *
 * Auto calibrate CV jacks on first boot, store in EEPROM
 * --add key combo to re-do calibration
 *
 * How fast can we CV the jacks?
 * Can we reduce noise w/hardware, w/software
 *
 * How fast can we ping?
 *
 * Hardware:
 * Get the output stage resistor values right for unity gain and near 0 DC
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


uint32_t g_error=0;

__IO uint16_t potadc_buffer[NUM_POT_ADCS];
__IO uint16_t cvadc_buffer[NUM_CV_ADCS];


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

	if (RAMTEST_JUMPER) RAM_startup_test();

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

	Codec_Register_Setup();

	init_adc_param_update_timer();

    if (!load_calibration()){
    	auto_calibrate();
    	factory_reset();
	}

	Start_I2SDMA_Channel1();
	Start_I2SDMA_Channel2();

	while(1){//-O1 roughly 400kHz and takes 1us
		check_errors();
		
		update_ping_ledbut();

		//process_audio();
		update_channel_leds(0);
		update_channel_leds(1);

		update_inf_ledbut(0);
		update_inf_ledbut(1);


		update_instant_params(0);
		update_instant_params(1);
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
