/*
 * to-do:
 *
 * Software:
 *
 *  Weird bug that disappears and re-appears: right channel codec runs at +17dB (or so). Could we be running I2C too fast?
 *
 * Hardware:
 *
 *  - Isolated analog and digital power/ground, separate regulators, electrolytic caps
 *
 *  - Add switch/es for resampling?
 *
 *  - Pot and CV adc values jump up to 20mV, increase capacitance? R/C filter the pots? Or handle it in software...
 *
 */

#include "stm32f4xx.h"

#include "codec.h"
#include "i2s.h"
#include "adc.h"
#include "looping_delay.h"
#include "dig_inouts.h"
#include "globals.h"
#include "dac.h"
#include "params.h"
#include "timekeeper.h"
#include "sdram.h"

uint32_t g_error=0;

__IO uint16_t potadc_buffer[NUM_POT_ADCS];
__IO uint16_t cvadc_buffer[NUM_CV_ADCS];
uint8_t flag_ping_was_changed;



void check_errors(void){

}

/*

inline uint32_t diff_circular(uint32_t leader, uint32_t follower){
	if (leader==follower) return(0);
	else if (leader > follower) return(leader-follower);
	else return ((leader+BUFFER_SIZE)-(follower+1));
}
*/

#define ADC_POLL_TIME 1000

void main(void)
{
	uint8_t err=0;
	int16_t i;
	uint32_t adc_read_ctr=0;


	Codec_Init(I2S_AudioFreq_48k);

	init_timekeeper();
	init_dig_inouts();
	init_EXTI_inputs();
	init_inputread_timer();

	Init_Pot_ADC((uint16_t *)potadc_buffer, NUM_POT_ADCS);
	Init_CV_ADC((uint16_t *)cvadc_buffer, NUM_CV_ADCS);

	delay();

	SDRAM_Init();
//	FMC_Config();

	Audio_Init();

	Init_I2S_Channel2();
	Init_I2S_Channel1();

	while(1){ //-O0 roughly 100kHz

		DEBUG2_ON;

		check_errors();

		if (adc_read_ctr++ > ADC_POLL_TIME){
			adc_read_ctr=0;
			update_adc_params();
		}
		
		update_ping_ledbut();

		//process_audio();

		DEBUG2_OFF;

		update_channel_leds(0);
		update_channel_leds(1);

		update_inf_ledbut(0);
		update_inf_ledbut(1);

		if (flag_ping_was_changed){
			flag_ping_was_changed=0;
			set_divmult_time(0);
			set_divmult_time(1);
		}

		update_clkout_jack();
	}
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

	foobar=0;

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
