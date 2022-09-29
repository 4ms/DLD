#include "stm32f4xx.h"
static inline void panic(void) {
	__BKPT();
}
