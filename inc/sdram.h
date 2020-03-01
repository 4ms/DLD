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

//SDRAM Bank 2 is 4 x 64MB, from 0xD000 0000 to 0xDFFF FFFF
//Thus, we can access 0x10000000 addresses, or 256M addresses

#define SDRAM_BASE 0xD0000000
#define SDRAM_SIZE 0x02000000

void SDRAM_Init(void);

#endif /* SDRAM_H_ */
