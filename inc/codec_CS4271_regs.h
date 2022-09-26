#pragma once
#include <stdint.h>

#define CODEC_IS_SLAVE 0
#define CODEC_IS_MASTER 1

#define MCLK_SRC_STM 0
#define MCLK_SRC_EXTERNAL 1
//#define MCLK_SRC_CODEC 2

//CODECA is left channel of the DLD (I2C1, I2S3)
//CODECB is right channel of the DLD (I2C2, I2S2)

#ifdef USE_VCXO

#define CODECA_MODE CODEC_IS_MASTER
#define CODECB_MODE CODEC_IS_MASTER

#define CODECA_MCLK_SRC MCLK_SRC_EXTERNAL
#define CODECB_MCLK_SRC MCLK_SRC_EXTERNAL

#else

#define CODECA_MODE CODEC_IS_SLAVE
#define CODECB_MODE CODEC_IS_SLAVE

#define CODECA_MCLK_SRC MCLK_SRC_STM
#define CODECB_MCLK_SRC MCLK_SRC_STM

#endif

/* Codec audio Standards */
// #ifdef USE_I2S_STANDARD_PHILLIPS
// #define CODEC_STANDARD 0x04
// #define I2S_STANDARD I2S_Standard_Phillips
// #elif defined(USE_I2S_STANDARD_MSB)
// #define CODEC_STANDARD 0x00
// #define I2S_STANDARD I2S_STANDARD_MSB
// #elif defined(USE_I2S_STANDARD_LSB)
// #define CODEC_STANDARD 0x08
// #define I2S_STANDARD I2S_Standard_LSB
// #else
// #error "Error: No audio communication standard selected !"
// #endif /* I2S_STANDARD */

#define CS4271_ADDR_0 0b0010000
#define CS4271_ADDR_1 0b0010001

/*
 * The 7 bits Codec address (sent through I2C interface)
 * The 8th bit (LSB) is Read /Write
 */
#define CODEC_ADDRESS (CS4271_ADDR_0 << 1)

#define CS4271_NUM_REGS 6
/* we only initialize the first 6 registers, the 7th is for pre/post-init and the 8th is read-only */

#define CS4271_REG_MODECTRL1 1
#define CS4271_REG_DACCTRL 2
#define CS4271_REG_DACMIX 3
#define CS4271_REG_DACAVOL 4
#define CS4271_REG_DACBVOL 5
#define CS4271_REG_ADCCTRL 6
#define CS4271_REG_MODELCTRL2 7
#define CS4271_REG_CHIPID 8 /*Read-only*/

//Reg 1 (MODECTRL1):
#define SINGLE_SPEED (0b00 << 6) /* 4-50kHz */
#define DOUBLE_SPEED (0b10 << 6) /* 50-100kHz */
#define QUAD_SPEED (0b11 << 6)	 /* 100-200kHz */
#define RATIO0 (0b00 << 4)		 /* See table page 28 and 29 of datasheet */
#define RATIO1 (0b01 << 4)
#define RATIO2 (0b10 << 4)
#define RATIO3 (0b11 << 4)
#define MASTER (1 << 3)
#define SLAVE (0 << 3)
#define DIF_LEFTJUST_24b (0b000)
#define DIF_I2S_24b (0b001)
#define DIF_RIGHTJUST_16b (0b010)
#define DIF_RIGHTJUST_24b (0b011)
#define DIF_RIGHTJUST_20b (0b100)
#define DIF_RIGHTJUST_18b (0b101)

//Reg 2 (DACCTRL)
#define AUTOMUTE (1 << 7)
#define SLOW_FILT_SEL (1 << 6)
#define FAST_FILT_SEL (0 << 6)
#define DEEMPH_OFF (0 << 4)
#define DEEMPH_44 (1 << 4)
#define DEEMPH_48 (2 << 4)
#define DEEMPH_32 (3 << 4)
#define SOFT_RAMPUP (1 << 3)
/*An un-mute will be performed after executing a filter mode change, after a MCLK/LRCK ratio change or error, and after changing the Functional Mode.*/
#define SOFT_RAMPDOWN (1 << 2)
/*A mute will be performed prior to executing a filter mode change.*/
#define INVERT_SIGA_POL (1 << 1)
/*When set, this bit activates an inversion of the signal polarity for the appropriate channel*/
#define INVERT_SIGB_POL (1 << 0)

//Reg 3 (DACMIX)
#define BEQA (1 << 6)
/*If set, ignore AOUTB volume setting, and instead make channel B's volume equal channel A's volume as set by AOUTA */
#define SOFTRAMP (1 << 5)
/*Allows level changes, both muting and attenuation, to be implemented by incrementally ramping, in 1/8 dB steps, from the current level to the new level at a rate of 1 dB per 8 left/right clock periods */
#define ZEROCROSS (1 << 4)
/*Dictates that signal level changes, either by attenuation changes or muting, will occur on a signal zero crossing to minimize audible artifacts*/
#define ATAPI_aLbR (0b1001)
/*channel A==>Left, channel B==>Right*/

//Reg 4: DACAVOL
//Reg 5: DACBVOL

//Reg 6 (ADCCTRL)
#define DITHER16 (1 << 5)	   /*activates the Dither for 16-Bit Data feature*/
#define ADC_DIF_I2S (1 << 4)   /*I2S, up to 24-bit data*/
#define ADC_DIF_LJUST (0 << 4) /*Left Justified, up to 24-bit data (default)*/
#define MUTEA (1 << 3)
#define MUTEB (1 << 2)
#define HPFDisableA (1 << 1)
#define HPFDisableB (1 << 0)

//Reg 7 (MODECTRL2)
#define PDN (1 << 0)	 /* Power Down Enable */
#define CPEN (1 << 1)	 /* Control Port Enable */
#define FREEZE (1 << 2)	 /* Freezes effects of register changes */
#define MUTECAB (1 << 3) /* Internal AND gate on AMUTEC and BMUTEC */
#define LOOP (1 << 4)	 /* Digital loopback (ADC->DAC) */

//Reg 8 (CHIPID) (Read-only)
#define PART_mask (0b11110000)
#define REV_mask (0b00001111)
