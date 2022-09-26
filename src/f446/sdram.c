#include "globals.h"
#include "stm32f4xx.h"

static SDRAM_HandleTypeDef hsdram1;

void SDRAM_Init(void) {
	FMC_SDRAM_TimingTypeDef SdramTiming = {0};

	hsdram1.Instance = FMC_SDRAM_DEVICE;

	hsdram1.Init.SDBank = FMC_SDRAM_BANK2;
	hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
	hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
	hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
	hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
	hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
	hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
	hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
	hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
	hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_2;

	SdramTiming.LoadToActiveDelay = 2;	  //MRD
	SdramTiming.ExitSelfRefreshDelay = 6; //XSR
	SdramTiming.SelfRefreshTime = 4;	  //RAS
	SdramTiming.RowCycleDelay = 6;		  //RC
	SdramTiming.WriteRecoveryTime = 2;	  //WR
	SdramTiming.RPDelay = 2;			  //RP
	SdramTiming.RCDDelay = 2;			  //RCD

	HAL_StatusTypeDef err = HAL_SDRAM_Init(&hsdram1, &SdramTiming);
	if (err != HAL_OK)
		__BKPT();

	// Initialization step 3
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY)
		;
	//PALL command (Pre-charge all banks)
	FMC_Bank5_6->SDCMR = 1 | FMC_SDCMR_CTB2 | (1 << 5);

	// Initialization step 4
	delay_ms(1);

	// Initialization step 5
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY)
		;
	// 2 = PALL (All Bank Precharge) command
	// SDRAM bank 2
	// 1<<5 = 2 Auto-refresh cycles
	FMC_Bank5_6->SDCMR = 2 | FMC_SDCMR_CTB2 | (1 << 5);

	// Initialization step 6
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY)
		;
	// 3 = Auto-refresh command
	// SDRAM bank 2
	// 4<<5 = 5 Auto-refresh cycles
	FMC_Bank5_6->SDCMR = 3 | FMC_SDCMR_CTB2 | (4 << 5);

	// Initialization step 7
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY)
		;
	// 4 = Load Mode Register
	// SDRAM bank 2
	// 1<<5 = 2 Auto-refresh cycles
	// Mode Register = 0x231: burst length 2, burst type sequential, CAS latency 3 clocks, Write burst mode single bit, normal operation mode
	// Mode Register = 0x030: burst length 1, burst type sequential, CAS latency 3 clocks, Write burst mode = single location access, normal operation mode
	FMC_Bank5_6->SDCMR = 4 | FMC_SDCMR_CTB2 | (1 << 5) | (0x231 << 9);

	// Initialization step 8
	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY)
		;

	// refresh rate in number of SDCLK clock cycles between the refresh cycles
	// 683 = 7.6uS x 90Mhz
	// 7.6uS x 8196 rows = 62ms refresh rate
	FMC_Bank5_6->SDRTR |= (663 << 1);

	while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY)
		;
}

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if (FMC_Initialized) {
		return;
	}
	FMC_Initialized = 1;

	__HAL_RCC_FMC_CLK_ENABLE();

	/** FMC GPIO Configuration
  PE1   ------> FMC_NBL1
  PE0   ------> FMC_NBL0
  PB5   ------> FMC_SDCKE1
  PG15   ------> FMC_SDNCAS
  PF0   ------> FMC_A0
  PF1   ------> FMC_A1
  PB6   ------> FMC_SDNE1
  PF2   ------> FMC_A2
  PD1   ------> FMC_D3
  PF3   ------> FMC_A3
  PF4   ------> FMC_A4
  PF5   ------> FMC_A5
  PD0   ------> FMC_D2
  PG8   ------> FMC_SDCLK
  PC0   ------> FMC_SDNWE
  PE11   ------> FMC_D8
  PG5   ------> FMC_BA1
  PG1   ------> FMC_A11
  PE10   ------> FMC_D7
  PE12   ------> FMC_D9
  PD10   ------> FMC_D15
  PG4   ------> FMC_BA0
  PG2   ------> FMC_A12
  PF13   ------> FMC_A7
  PG0   ------> FMC_A10
  PE9   ------> FMC_D6
  PE13   ------> FMC_D10
  PD9   ------> FMC_D14
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PF12   ------> FMC_A6
  PF15   ------> FMC_A9
  PE8   ------> FMC_D5
  PE14   ------> FMC_D11
  PD8   ------> FMC_D13
  PF11   ------> FMC_SDNRAS
  PF14   ------> FMC_A8
  PE7   ------> FMC_D4
  PE15   ------> FMC_D12
  */

	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_9 | GPIO_PIN_13 |
						  GPIO_PIN_8 | GPIO_PIN_14 | GPIO_PIN_7 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_8 | GPIO_PIN_5 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_2 | GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_13 |
						  GPIO_PIN_12 | GPIO_PIN_15 | GPIO_PIN_11 | GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *sdramHandle) {
	HAL_FMC_MspInit();
}

void SDRAM_Wait(void) {
	// while (HAL_IS_BIT_SET(Device->SDSR, FMC_SDSR_BUSY))
	// 	;
}
