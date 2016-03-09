//=================================================================================================
// STM32F429I-Discovery SDRAM configuration
// Author : Radoslaw Kwiecien
// e-mail : radek@dxp.pl
// http://en.radzio.dxp.pl/stm32f429idiscovery/
// Date : 24.11.2013
//=================================================================================================
#ifndef SDRAM_H_
#define SDRAM_H_
#include <stm32f4xx.h>

#define SDRAM_BASE 0xD0000000


//SDRAM Bank 2 is 4 x 64MB, from 0xD000 0000 to 0xDFFF FFFF
//Thus, we can access 0x10000000 addresses, or 256M addresses
#define SDRAM_SIZE 0x02000000



//#define SDRAM_MEMORY_WIDTH            FMC_SDMemory_Width_8b
#define SDRAM_MEMORY_WIDTH            FMC_SDMemory_Width_16b
//#define SDRAM_MEMORY_WIDTH               FMC_SDMemory_Width_32b

//#define SDCLOCK_PERIOD	FMC_SDClock_Disable
#define SDCLOCK_PERIOD                   FMC_SDClock_Period_2
//#define SDCLOCK_PERIOD                FMC_SDClock_Period_3

#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)


void FMC_Config(void);

void SDRAM_Init(void);
uint32_t RAM_test(void);


#endif /* SDRAM_H_ */
