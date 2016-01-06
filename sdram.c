#include "globals.h"
#include "gpiof4.h"
#include "sdram.h"
#include "dig_inouts.h"

uint8_t RAM_test(void){

	uint32_t addr;
	uint32_t i;

	addr=SDRAM_BASE;
	for (i=0;i<5000;i++){
		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		*((int16_t *)addr) = (int16_t)(i);

		addr+=2;
	}

	addr=SDRAM_BASE;
	for (i=0;i<5000;i++){
		while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET){;}

		//t = *((int16_t *)addr);

		addr+=2;
	}

	return(0);
}





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
		0, 1, 2, 3, 4, 5, 12, 13, 14, 15,
		0, 1, 2,
		14, 15, 0, 1,
		7, 8, 9, 10, 11, 12, 13, 14, 15,
		8, 9, 10,
		5, 6,
		0,
		0, 1,
		11,
		4, 5, 8, 15,
		0
};
//=================================================================================================
// SDRAM_Init function
//=================================================================================================
void SDRAM_Init(void)
{
	//volatile uint32_t ptr = 0;
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
	//FMC_Bank5_6->SDCR[0] = FMC_SDCR1_SDCLK_1   | FMC_SDCR1_RPIPE_1;

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
/*	do {
	  register unsigned int i;
	  for (i = 0; i < 1000000; ++i)
	    __asm__ __volatile__ ("nop\n\t":::"memory");
	} while (0);
*/
	delay();
	delay();


	// Initialization step 5
	while(FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);
	// 2 = PALL (“All Bank Precharge”) command
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

	// Clear SDRAM
	//for(ptr = SDRAM_BASE; ptr < (SDRAM_BASE + SDRAM_SIZE - 1); ptr += 4)
		//	*((uint32_t *)ptr) = 0xFFFFFFFF;

}




void FMC_Config(void){

  GPIO_InitTypeDef            GPIO_InitStructure;
  FMC_SDRAMInitTypeDef        FMC_SDRAMInitStructure;
  FMC_SDRAMTimingInitTypeDef  FMC_SDRAMTimingInitStructure;
  FMC_SDRAMCommandTypeDef     FMC_SDRAMCommandStructure;

  uint32_t tmpr = 0;
  uint32_t timeout = SDRAM_TIMEOUT;

  /* GPIO configuration ------------------------------------------------------*/
  /* Enable GPIOs clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE |
                         RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG , ENABLE);

  /* Common GPIO configuration */
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  /* GPIOD configuration */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  |GPIO_Pin_1  |GPIO_Pin_8 |GPIO_Pin_9 |
                                GPIO_Pin_10 |GPIO_Pin_14 |GPIO_Pin_15;

  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* GPIOE configuration */
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_7 | GPIO_Pin_8  |
                                GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11| GPIO_Pin_12 |
                                GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* GPIOF configuration */
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource11 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  |
                                GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_11 | GPIO_Pin_12 |
                                GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

  GPIO_Init(GPIOF, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource2 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource8 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource15 , GPIO_AF_FMC);


  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 |GPIO_Pin_2 |GPIO_Pin_4 |GPIO_Pin_5 |
                                GPIO_Pin_8 | GPIO_Pin_15;

  GPIO_Init(GPIOG, &GPIO_InitStructure);


  /* Enable FMC clock */
  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);

  /* FMC SDRAM device initialization sequence --------------------------------*/
  /* Step 1 ----------------------------------------------------*/
  /* Timing configuration for 84 Mhz of SD clock frequency (168Mhz/2) */
  /* TMRD: 2 Clock cycles */
  FMC_SDRAMTimingInitStructure.FMC_LoadToActiveDelay    = 3;
  /* TXSR: min=70ns (6x11.90ns) */
  FMC_SDRAMTimingInitStructure.FMC_ExitSelfRefreshDelay = 7;
  /* TRAS: min=42ns (4x11.90ns) max=120k (ns) */
  FMC_SDRAMTimingInitStructure.FMC_SelfRefreshTime      = 5;
  /* TRC:  min=70 (6x11.90ns) */
  FMC_SDRAMTimingInitStructure.FMC_RowCycleDelay        = 7;
  /* TWR:  min=1+ 7ns (1+1x11.90ns) */
  FMC_SDRAMTimingInitStructure.FMC_WriteRecoveryTime    = 3;
  /* TRP:  20ns => 2x11.90ns */
  FMC_SDRAMTimingInitStructure.FMC_RPDelay              = 3;
  /* TRCD: 20ns => 2x11.90ns */
  FMC_SDRAMTimingInitStructure.FMC_RCDDelay             = 3;

  /* Step 2 ----------------------------------------------------*/

  /* FMC SDRAM control configuration */
  FMC_SDRAMInitStructure.FMC_Bank = FMC_Bank2_SDRAM;

  FMC_SDRAMInitStructure.FMC_ColumnBitsNumber   = FMC_ColumnBits_Number_9b;
  FMC_SDRAMInitStructure.FMC_RowBitsNumber      = FMC_RowBits_Number_13b;
  FMC_SDRAMInitStructure.FMC_SDMemoryDataWidth  = SDRAM_MEMORY_WIDTH;
  FMC_SDRAMInitStructure.FMC_InternalBankNumber = FMC_InternalBank_Number_4;

  /* CL: Cas Latency = 3 clock cycles */
  FMC_SDRAMInitStructure.FMC_CASLatency         = FMC_CAS_Latency_3;
  FMC_SDRAMInitStructure.FMC_WriteProtection    = FMC_Write_Protection_Disable;
  FMC_SDRAMInitStructure.FMC_SDClockPeriod      = SDCLOCK_PERIOD;
  FMC_SDRAMInitStructure.FMC_ReadBurst          = FMC_Read_Burst_Enable;
  FMC_SDRAMInitStructure.FMC_ReadPipeDelay      = FMC_ReadPipe_Delay_1;
  FMC_SDRAMInitStructure.FMC_SDRAMTimingStruct  = &FMC_SDRAMTimingInitStructure;
  /* FMC SDRAM bank initialization */
  FMC_SDRAMInit(&FMC_SDRAMInitStructure);

/* Step 3 --------------------------------------------------------------------*/
  /* Configure a clock configuration enable command */
  FMC_SDRAMCommandStructure.FMC_CommandMode            = FMC_Command_Mode_CLK_Enabled;
  FMC_SDRAMCommandStructure.FMC_CommandTarget          = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber      = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
  /* Wait until the SDRAM controller is ready */
  while((FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
  {
    timeout--;
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

/* Step 4 --------------------------------------------------------------------*/
  /* Insert 100 ms delay */
 delay();
 delay();

/* Step 5 --------------------------------------------------------------------*/
  /* Configure a PALL (precharge all) command */
  FMC_SDRAMCommandStructure.FMC_CommandMode            = FMC_Command_Mode_PALL;
  FMC_SDRAMCommandStructure.FMC_CommandTarget          = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber      = 2; //was 1
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;

  /* Wait until the SDRAM controller is ready */
  timeout = SDRAM_TIMEOUT;
  while((FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
  {
    timeout--;
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

/* Step 6 --------------------------------------------------------------------*/
  /* Configure a Auto-Refresh command */
  FMC_SDRAMCommandStructure.FMC_CommandMode            = FMC_Command_Mode_AutoRefresh;
  FMC_SDRAMCommandStructure.FMC_CommandTarget          = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber      = 5; //was 8
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;

  /* Wait until the SDRAM controller is ready */
  timeout = SDRAM_TIMEOUT;
  while((FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
  {
    timeout--;
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

/* Step 7 --------------------------------------------------------------------*/
  /* Program the external memory mode register */
  tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |
                   SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                   SDRAM_MODEREG_CAS_LATENCY_3           |
                   SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                   SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  /* Configure a load Mode register command*/
  FMC_SDRAMCommandStructure.FMC_CommandMode            = FMC_Command_Mode_LoadMode;
  FMC_SDRAMCommandStructure.FMC_CommandTarget          = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber      = 2;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = tmpr;

  /* Wait until the SDRAM controller is ready */
  timeout = SDRAM_TIMEOUT;
  while((FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
  {
    timeout--;
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

/* Step 8 --------------------------------------------------------------------*/

  /* Set the refresh rate counter */
  /* (7.81 us x Freq) - 20 */
  /* Set the device refresh counter */
  FMC_SetRefreshCount(683); //was 636

  /* Wait until the SDRAM controller is ready */
  timeout = SDRAM_TIMEOUT;
  while((FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
  {
    timeout--;
  }
}








