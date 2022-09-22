#include "stm32f4xx.h"

inline void set_vect_table(void) {
	SCB->VTOR = 0x08000000; //reset_address & (uint32_t)0x1FFFFF80;
}
