#include <stm32f4xx.h>
#include "ITM.h"

// Print a given string to ITM.
// This uses 8 bit writes, as that seems to be the most common way to write text
// through ITM. Otherwise there is no good way for the PC software to know what
// is text and what is some other data.
void ITM_Print(int port, const char *p)
{
    if ((ITM->TCR & ITM_TCR_ITMENA_Msk) && (ITM->TER & (1UL << port)))
    {
        while (*p)
        {
            while (ITM->PORT[port].u32 == 0);
            ITM->PORT[port].u8 = *p++;
        }
    }
}

// Write a 32-bit value to ITM.
// This can be used as a fast way to log important values from code.
void ITM_SendValue (int port, uint32_t value)
{
    if ((ITM->TCR & ITM_TCR_ITMENA_Msk) && (ITM->TER & (1UL << port)))
    {
        while (ITM->PORT[port].u32 == 0);
        ITM->PORT[port].u32 = value;
    }
}

void ITM_Init(uint32_t SWOSpeed)
{

	  uint32_t SWOPrescaler;

	   *((volatile unsigned *)0xE000EDFC) = 0x01000000;   // "Debug Exception and Monitor Control Register (DEMCR)"
	   *((volatile unsigned *)0xE0042004) = 0x00000027;
	   *((volatile unsigned *)0xE00400F0) = 0x00000002;   // "Selected PIN Protocol Register": Select which protocol to use for trace output (2: SWO)
	  SWOPrescaler = (168000000 / SWOSpeed) - 1;  // SWOSpeed in Hz
	   *((volatile unsigned *)0xE0040010) = SWOPrescaler; // "Async Clock Prescaler Register". Scale the baud rate of the asynchronous output
	   *((volatile unsigned *)0xE0000FB0) = 0xC5ACCE55;   // ITM Lock Access Register, C5ACCE55 enables more write access to Control Register 0xE00 :: 0xFFC
	   *((volatile unsigned *)0xE0000E80) = 0x0001000D;   // ITM Trace Control Register
	   *((volatile unsigned *)0xE0000E40) = 0x0000000F;   // ITM Trace Privilege Register
	   *((volatile unsigned *)0xE0000E00) = 0x00000001;   // ITM Trace Enable Register. Enabled tracing on stimulus ports. One bit per stimulus port.
	   *((volatile unsigned *)0xE0001000) = 0x400003FE;   // DWT_CTRL
	   *((volatile unsigned *)0xE0040304) = 0x00000100;   // Formatter and Flush Control Register

}

#define ITM_ENA (*(volatile unsigned int*)0xE0000E00) // ITM Enable
#define ITM_TPR (*(volatile unsigned int*)0xE0000E40) // Trace Privilege Register
#define ITM_TCR (*(volatile unsigned int*)0xE0000E80) // ITM Trace Control Reg.
#define ITM_LSR (*(volatile unsigned int*)0xE0000FB0) // ITM Lock Status Register
#define DHCSR (*(volatile unsigned int*)0xE000EDF0) // Debug register
#define DEMCR (*(volatile unsigned int*)0xE000EDFC) // Debug register
#define TPIU_ACPR (*(volatile unsigned int*)0xE0040010) // Async Clock presacler register
#define TPIU_SPPR (*(volatile unsigned int*)0xE00400F0) // Selected Pin Protocol Register
#define DWT_CTRL (*(volatile unsigned int*)0xE0001000) // DWT Control Register
#define FFCR (*(volatile unsigned int*)0xE0040304) // Formatter and flus


void ITM_Disable(void)
{
	ITM_ENA &= ~(1 << 0);
	ITM_TCR = 0;
}
