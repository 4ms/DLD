/*
 * codec_CS4271.c - Setup for the Cirrus Logic CS4271 codec
 *
 * Author: Dan Green (danngreen1@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * See http://creativecommons.org/licenses/MIT/ for more information.
 *
 * -----------------------------------------------------------------------------
 */
#ifndef __codec__
#define __codec__

#include <stm32f4xx.h>

/* I2C clock speed configuration (in Hz)  */
#define CODEC_I2C_SPEED 50000

#define USE_I2S_STANDARD_PHILLIPS
//#define USE_I2S_STANDARD_MSB
//#define USE_I2S_STANDARD_LSB

//#define USE_DEFAULT_TIMEOUT_CALLBACK

// #define CODECA_TX_IRQHandler DMA2_Stream1_IRQHandler
// #define CODECA_TX_DMA_STREAM DMA2_Stream1
// #define CODECA_RX_IRQHandler DMA2_Stream5_IRQHandler
// #define CODECA_RX_DMA_IRQ DMA2_Stream5_IRQn
// #define CODECA_RX_DMA_STREAM DMA2_Stream5

// #define CODECB_TX_IRQHandler DMA2_Stream4_IRQHandler
// #define CODECB_TX_DMA_STREAM DMA2_Stream4
// #define CODECB_RX_IRQHandler DMA2_Stream6_IRQHandler
// #define CODECB_RX_DMA_IRQ DMA2_Stream6_IRQn
// #define CODECB_RX_DMA_STREAM DMA2_Stream6

// CODEC B (DLD side B)
// CODECB is SAI2, synced to SAI1 [on sch: I2S2 synced to I2S3]
#define CODECB_RESET_RCC_ENABLE __HAL_RCC_GPIOG_CLK_ENABLE
#define CODECB_RESET_pin GPIO_PIN_3
#define CODECB_RESET_GPIO GPIOG
#define CODECB_RESET_HIGH CODECB_RESET_GPIO->BSRR = CODECB_RESET_pin
#define CODECB_RESET_LOW CODECB_RESET_GPIO->BSRR = (CODECB_RESET_pin << 16)

// SAI2_SD_A: Channel 2 TX (DAC)
// #define CODECB_SAI_GPIO_SDO GPIOD
// #define CODECB_SAI_SDO_PIN GPIO_Pin_11
// #define CODECB_SAI_SDO_AF GPIO_AF10_SAI2

// #define CODECB_TX_DMA_CLOCK __HAL_RCC_DMA2_CLK_ENABLE
// #define CODECB_TX_DMA_STREAM DMA2_Stream4
// #define CODECB_TX_DMA_CHANNEL DMA_CHANNEL_3
// #define CODECB_TX_DMA_IRQ DMA2_Stream4_IRQn
// #define CODECB_TX_DMA_FLAG_TC DMA_FLAG_TCIF3_7
// #define CODECB_TX_DMA_FLAG_HT DMA_FLAG_HTIF3_7
// #define CODECB_TX_DMA_FLAG_FE DMA_FLAG_FEIF3_7
// #define CODECB_TX_DMA_FLAG_TE DMA_FLAG_TEIF3_7
// #define CODECB_TX_DMA_FLAG_DME DMA_FLAG_DMEIF3_7

// // SAI2_SD_B: Channel 2 RX (ADC)
// #define CODECB_SAI_GPIO_SDI GPIOG
// #define CODECB_SAI_SDI_PIN GPIO_Pin_10
// #define CODECB_SAI_SDI_AF GPIO_AF10_SAI2

// #define CODECB_RX_DMA_STREAM DMA2_Stream6
// #define CODECB_RX_DMA_CHANNEL DMA_CHANNEL_3
// #define CODECB_RX_DMA_IRQ DMA2_Stream6_IRQn
// #define CODECB_RX_DMA_FLAG_TC DMA_FLAG_TCIF3_7
// #define CODECB_RX_DMA_FLAG_HT DMA_FLAG_HTIF3_7
// #define CODECB_RX_DMA_FLAG_FE DMA_FLAG_FEIF3_7
// #define CODECB_RX_DMA_FLAG_TE DMA_FLAG_TEIF3_7
// #define CODECB_RX_DMA_FLAG_DME DMA_FLAG_DMEIF3_7

///////////////
// DLD Side A:
#define CODECA_RESET_RCC_ENABLE __HAL_RCC_GPIOD_CLK_ENABLE
#define CODECA_RESET_pin GPIO_PIN_2
#define CODECA_RESET_GPIO GPIOD
#define CODECA_RESET_HIGH CODECA_RESET_GPIO->BSRR = CODECA_RESET_pin
#define CODECA_RESET_LOW CODECA_RESET_GPIO->BSRR = (CODECA_RESET_pin << 16)

// SAI1_SD_A: DLD Left Side TX (DAC)
// #define CODECA_SAI_GPIO_WS GPIOE
// #define CODECA_SAI_WS_PIN GPIO_PIN_4
// #define CODECA_SAI_WS_AF GPIO_AF6_SAI1

// #define CODECA_SAI_GPIO_SCK GPIOE
// #define CODECA_SAI_SCK_PIN GPIO_PIN_5
// #define CODECA_SAI_SCK_AF GPIO_AF6_SAI1

// #define CODECA_SAI_GPIO_SDO GPIOE
// #define CODECA_SAI_SDO_PIN GPIO_PIN_6
// #define CODECA_SAI_SDO_AF GPIO_AF6_SAI1

// #define CODECA_SAI_MCK_GPIO GPIOE
// #define CODECA_SAI_MCK_PIN GPIO_PIN_2
// #define CODECA_SAI_MCK_AF GPIO_AF6_SAI1

// #define AUDIO_I2S3_DMA_CLOCK __HAL_RCC_DMA2_CLK_ENABLE
// #define AUDIO_I2S3_DMA_STREAM DMA2_Stream1
// #define AUDIO_I2S3_DMA_CHANNEL DMA_CHANNEL_0
// #define AUDIO_I2S3_DMA_IRQ DMA2_Stream1_IRQn
// #define AUDIO_I2S3_DMA_FLAG_TC DMA_FLAG_TCIF0_4
// #define AUDIO_I2S3_DMA_FLAG_HT DMA_FLAG_HTIF0_4
// #define AUDIO_I2S3_DMA_FLAG_FE DMA_FLAG_FEIF0_4
// #define AUDIO_I2S3_DMA_FLAG_TE DMA_FLAG_TEIF0_4
// #define AUDIO_I2S3_DMA_FLAG_DME DMA_FLAG_DMEIF0_4

// // SAI1_SD_B: DLD Left Side RX (ADC)
// #define CODECA_SAI_GPIO_SDI GPIOE
// #define CODECA_SAI_SDI_PIN GPIO_PIN_3
// #define CODECA_SAI_SDI_AF GPIO_AF6_SAI1

// #define AUDIO_I2S3_EXT_DMA_STREAM DMA1_Stream2
// #define AUDIO_I2S3_EXT_DMA_DREG CODECA_I2S_EXT_ADDRESS
// #define AUDIO_I2S3_EXT_DMA_CHANNEL DMA_Channel_2
// #define AUDIO_I2S3_EXT_DMA_IRQ DMA1_Stream2_IRQn
// #define AUDIO_I2S3_EXT_DMA_FLAG_TC DMA_FLAG_TCIF2
// #define AUDIO_I2S3_EXT_DMA_FLAG_HT DMA_FLAG_HTIF2
// #define AUDIO_I2S3_EXT_DMA_FLAG_FE DMA_FLAG_FEIF2
// #define AUDIO_I2S3_EXT_DMA_FLAG_TE DMA_FLAG_TEIF2
// #define AUDIO_I2S3_EXT_DMA_FLAG_DME DMA_FLAG_DMEIF2

/* I2S DMA Stream definitions */

/* I2C peripheral configuration defines (control interface of the audio codec) */
#define CODECB_I2C I2C2
#define CODECB_I2C_CLK __HAL_RCC_I2C2_CLK_ENABLE
#define CODECB_I2C_GPIO_CLOCK __HAL_RCC_GPIOB_CLK_ENABLE
#define CODECB_I2C_GPIO_AF GPIO_AF4_I2C2
#define CODECB_I2C_GPIO GPIOB
#define CODECB_I2C_SCL_PIN GPIO_PIN_10
#define CODECB_I2C_SDA_PIN GPIO_PIN_11
// #define CODECB_I2C_SCL_PINSRC GPIO_PinSource10
// #define CODECB_I2C_SDA_PINSRC GPIO_PinSource11

#define CODECA_I2C I2C1
#define CODECA_I2C_CLK __HAL_RCC_I2C1_CLK_ENABLE
#define CODECA_I2C_GPIO_CLOCK RCC_AHB1Periph_GPIOB
#define CODECA_I2C_GPIO_AF GPIO_AF4_I2C1
#define CODECA_I2C_GPIO GPIOB
#define CODECA_I2C_SCL_PIN GPIO_PIN_8
#define CODECA_I2C_SDA_PIN GPIO_PIN_9
// #define CODECA_I2C_SCL_PINSRC GPIO_PinSource8
// #define CODECA_I2C_SDA_PINSRC GPIO_PinSource9

/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will 
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define CODEC_FLAG_TIMEOUT ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT ((uint32_t)(300 * CODEC_FLAG_TIMEOUT))

void Codec_A_CtrlInterface_Init(void);
void Codec_B_CtrlInterface_Init(void);

void Codec_A_AudioInterface_Init(uint32_t AudioFreq);
void Codec_B_AudioInterface_Init(uint32_t AudioFreq);

uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue, I2C_TypeDef *CODEC);

void Codecs_Deinit(void);

void Codec_GPIO_Init(void);

void init_i2s_clkin(void);

#endif
