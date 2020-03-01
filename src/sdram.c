//=================================================================================================
// STM32F429I-Discovery SDRAM configuration
// Author : Radoslaw Kwiecien
// e-mail : radek@dxp.pl
// http://en.radzio.dxp.pl/stm32f429idiscovery/
// Date : 24.11.2013
//=================================================================================================
//
// Adpated for 256MBit SDRAM and commented by Dan Green <danngreen1@gmail.com>
//
#include "globals.h"
#include "gpiof4.h"
#include "sdram.h"


//=================================================================================================
// Macros for SDRAM Timing Register
//=================================================================================================
#define TMRD(x) (x << 0)
#define TXSR(x) (x << 4)
#define TRAS(x) (x << 8)
#define TRC(x)  (x << 12)
#define TWR(x)  (x << 16)
#define TRP(x)  (x << 20)
#define TRCD(x) (x << 24)
//=================================================================================================
// GPIO configuration data
//=================================================================================================
static  GPIO_TypeDef * const GPIOInitTable[] = {
		GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF,
		GPIOG, GPIOG, GPIOG,
		GPIOD, GPIOD, GPIOD, GPIOD,
		GPIOE, GPIOE, GPIOE, GPIOE, GPIOE, GPIOE, GPIOE, GPIOE, GPIOE,
		GPIOD, GPIOD, GPIOD,
		GPIOB, GPIOB,
		GPIOC,
		GPIOE, GPIOE,
		GPIOF,
		GPIOG, GPIOG, GPIOG, GPIOG,
		0
};
static uint8_t const PINInitTable[] = {
/* F */	0, 1, 2, 3, 4, 5, 12, 13, 14, 15,
/* G */	0, 1, 2,
/* D */	14, 15, 0, 1,
/* E */ 7, 8, 9, 10, 11, 12, 13, 14, 15,
/* D */	8, 9, 10,
/* B */	5, 6,
/* C */	0,
/* E */	0, 1,
/* F */	11,
/* G */	4, 5, 8, 15,
		0
};
//=================================================================================================
// SDRAM_Init function
//=================================================================================================
void SDRAM_Init(void)
{
	volatile uint32_t i = 0;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIOGEN;

	while(GPIOInitTable[i] != 0){
		gpio_conf(GPIOInitTable[i], PINInitTable[i],  MODE_AF, TYPE_PUSHPULL, SPEED_100MHz, PULLUP_NONE, 12);
		i++;
	}

	RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;

	// Initialization step 1

		//SDCLK period = 2 x HCLK periods
		//RBURST: single read requests are always managed as bursts
		//RPIPE: (delay, in HCLK clock cycles, for reading data after CAS latency) = Two HCLK clock cycle delay
	FMC_Bank5_6->SDCR[0] = FMC_SDCR1_SDCLK_1  | FMC_SDCR1_RBURST | FMC_SDCR1_RPIPE_1;

		//64Mbit (8MByte):
		//NR_0: Num Rows=12
		//NC (not present): Num Columns=8
		//MWID_0: Memory width=16 bits
		//NB: Num Banks=4
		//CAS: CAS latency=3 cycles
		//FMC_Bank5_6->SDCR[1] = FMC_SDCR1_NR_0	  | FMC_SDCR1_MWID_0 | FMC_SDCR1_NB | FMC_SDCR1_CAS;

		//256Mbit (32MByte):
		//NR_1: Num Rows=13
		//NC_0: Num Columns=9
		//MWID_1: Memory width=16
		//NB: Num Banks=4
		//CAS: CAS latency=3 cycles
	FMC_Bank5_6->SDCR[1] = FMC_SDCR1_NR_1 | FMC_SDCR1_NC_0 | FMC_SDCR1_MWID_0 | FMC_SDCR1_NB | FMC_SDCR1_CAS;

	// Initialization step 2
	// Timing characteristics, expressed in number of clock cycles (@180MHz, SDCLK is HCLK/2 means a value of 1 = 11.1ns, 2 = 22.2ns etc..)
	FMC_Bank5_6->SDTR[0] = TRC(6)  | TRP(2);
	FMC_Bank5_6->SDTR[1] = TMRD(2) | TXSR(6) | TRAS(4) | TWR(2) | TRCD(2);

	// Initialization step 3
	while(FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);
	//PALL command (Pre-charge all banks)
	FMC_Bank5_6->SDCMR 	 = 1 | FMC_SDCMR_CTB2 | (1 << 5);

	// Initialization step 4
	delay();
	delay();


	// Initialization step 5
	while(FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);
	// 2 = PALL (All Bank Precharge) command
	// SDRAM bank 2
	// 1<<5 = 2 Auto-refresh cycles
	FMC_Bank5_6->SDCMR 	 = 2 | FMC_SDCMR_CTB2 | (1 << 5);


	// Initialization step 6
	while(FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);
	// 3 = Auto-refresh command
	// SDRAM bank 2
	// 4<<5 = 5 Auto-refresh cycles
	FMC_Bank5_6->SDCMR 	 = 3 | FMC_SDCMR_CTB2 | (4 << 5);

	// Initialization step 7
	while(FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);
	// 4 = Load Mode Register
	// SDRAM bank 2
	// 1<<5 = 2 Auto-refresh cycles
	// Mode Register = 0x231: burst length 2, burst type sequential, CAS latency 3 clocks, Write burst mode single bit, normal operation mode
	// Mode Register = 0x030: burst length 1, burst type sequential, CAS latency 3 clocks, Write burst mode = single location access, normal operation mode
	FMC_Bank5_6->SDCMR 	 = 4 | FMC_SDCMR_CTB2 | (1 << 5) | (0x231 << 9);


	// Initialization step 8
	while(FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);

	// refresh rate in number of SDCLK clock cycles between the refresh cycles
	// 683 = 7.6uS x 90Mhz
	// 7.6uS x 8196 rows = 62ms refresh rate
	FMC_Bank5_6->SDRTR |= (663 << 1);



	while(FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);

}
