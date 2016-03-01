/*  * globals.h
 *
 *  Created on: Jun 8, 2014
 *      Author: design
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

//codec_BUFF_LEN = Size (in samples) of the DMA rx/tx buffers:
//Each channel = codec_BUFF_LEN/2 samples
//We process the rx buffer when it's half-full and 100% full, so codec_BUFF_LEN/4 samples are processed at one time
#define codec_BUFF_LEN 16
//32 is 6kHz (48kHz / 32/4)
//16 is 12kHz
//8 is 24kHz but seems glitchy...


//#define PCB_PROTO_VERSION_4
//#define PCB_PROTO_VERSION_5
//#define PCB_PROTO_VERSION_6
#define PCB_PROTO_VERSION_7

//Error codes for g_error
#define OUT_OF_MEM 1
#define OUT_OF_SAMPLES 2
#define SPIERROR_1 4
#define SPIERROR_2 8
#define DMA_OVR_ERROR 16
#define sFLASH_BAD_ID 32
#define WRITE_BUFF_OVERRUN 64
#define READ_BUFF1_OVERRUN 128
#define READ_BUFF2_OVERRUN 256
#define READ_MEM_ERROR 512

//Number of channels
#define NUM_CHAN 2

#define TRIG_TIME 196

//#define USE_VCXO

//About 50ms delay
#define delay()						\
do {							\
  register unsigned int i;				\
  for (i = 0; i < 1000000; ++i)				\
    __asm__ __volatile__ ("nop\n\t":::"memory");	\
} while (0)


//extern volatile uint32_t sys_time;
//#define delay_sys(x) do{register uint32_t donetime=x+systime;__asm__ __volatile__ ("nop\n\t":::"memory");}while(sys_time!=donetime;)


#endif /* GLOBALS_H_ */
