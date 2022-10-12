#include "flash_layout.h"
#include "stm32f4xx.h"

inline void set_vect_table(void) {
	SCB->VTOR = FLASH_ADDR_app;
}
