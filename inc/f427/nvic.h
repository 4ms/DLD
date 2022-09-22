#include "stm32f4xx.h"

inline void set_vect_table(void) {
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);
	// SCB->VTOR = NVIC_VectTab_FLASH | (0x8000 & (uint32_t)0x1FFFFF80);
}
