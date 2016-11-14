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
#define I2C_SPEED                       50000

// Uncomment defines below to select standard for audio communication between Codec and I2S peripheral
#define I2S_STANDARD_PHILLIPS
//#define I2S_STANDARD_MSB
//#define I2S_STANDARD_LSB

//#define USE_DEFAULT_TIMEOUT_CALLBACK


// For CS4271
#define CODECA_RESET_RCC RCC_AHB1Periph_GPIOB
#define CODECA_RESET_pin GPIO_Pin_4
#define CODECA_RESET_GPIO GPIOB
#define CODECA_RESET_HIGH CODECA_RESET_GPIO->BSRRL = CODECA_RESET_pin
#define CODECA_RESET_LOW CODECA_RESET_GPIO->BSRRH = CODECA_RESET_pin

#define CODECB_RESET_RCC RCC_AHB1Periph_GPIOD
#define CODECB_RESET_pin GPIO_Pin_12
#define CODECB_RESET_GPIO GPIOD
#define CODECB_RESET_HIGH CODECB_RESET_GPIO->BSRRL = CODECB_RESET_pin
#define CODECB_RESET_LOW CODECB_RESET_GPIO->BSRRH = CODECB_RESET_pin

/*-----------------------------------
Hardware Configuration defines parameters
-----------------------------------------*/                 
/* I2S peripheral configuration defines */

#define CODECB_I2S                      SPI2
#define CODECB_I2S_EXT                  I2S2ext
#define CODECB_I2S_CLK                  RCC_APB1Periph_SPI2
#define CODECB_I2S_ADDRESS              0x4000380C
#define CODECB_I2S_EXT_ADDRESS          0x4000340C
#define CODECB_I2S_GPIO_AF              GPIO_AF_SPI2
#define CODECB_I2Sext_GPIO_AF			GPIO_AF_SPI2
#define CODECB_I2S_IRQ                  SPI2_IRQn
#define CODECB_I2S_EXT_IRQ              SPI2_IRQn
#define CODECB_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB)


#define CODECB_I2S_MCK_GPIO             GPIOC
#define CODECB_I2S_MCK_PIN              GPIO_Pin_6
#define CODECB_I2S_MCK_PINSRC           GPIO_PinSource6

#define CODECB_I2S_GPIO_WS              GPIOB
#define CODECB_I2S_WS_PIN               GPIO_Pin_12
#define CODECB_I2S_WS_PINSRC            GPIO_PinSource12

#define CODECB_I2S_GPIO_SDO             GPIOB
#define CODECB_I2S_SDO_PIN              GPIO_Pin_15
#define CODECB_I2S_SDO_PINSRC           GPIO_PinSource15

#define CODECB_I2S_GPIO_SCK             GPIOB
#define CODECB_I2S_SCK_PIN              GPIO_Pin_13
#define CODECB_I2S_SCK_PINSRC           GPIO_PinSource13

#define CODECB_I2S_GPIO_SDI             GPIOB
#define CODECB_I2S_SDI_PIN              GPIO_Pin_14
#define CODECB_I2S_SDI_PINSRC           GPIO_PinSource14

#define AUDIO_I2S2_IRQHandler           SPI2_IRQHandler
#define AUDIO_I2S2_EXT_IRQHandler       SPI2_IRQHandler

//SPI2_TX: S4 C0
#define AUDIO_I2S2_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S2_DMA_STREAM           DMA1_Stream4
#define AUDIO_I2S2_DMA_DREG             CODECB_I2S_ADDRESS
#define AUDIO_I2S2_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S2_DMA_IRQ              DMA1_Stream4_IRQn
#define AUDIO_I2S2_DMA_FLAG_TC          DMA_FLAG_TCIF0
#define AUDIO_I2S2_DMA_FLAG_HT          DMA_FLAG_HTIF0
#define AUDIO_I2S2_DMA_FLAG_FE          DMA_FLAG_FEIF0
#define AUDIO_I2S2_DMA_FLAG_TE          DMA_FLAG_TEIF0
#define AUDIO_I2S2_DMA_FLAG_DME         DMA_FLAG_DMEIF0

//I2S2_EXT_RX: S3 C3
#define AUDIO_I2S2_EXT_DMA_STREAM       DMA1_Stream3
#define AUDIO_I2S2_EXT_DMA_DREG         CODECB_I2S_EXT_ADDRESS
#define AUDIO_I2S2_EXT_DMA_CHANNEL      DMA_Channel_3
#define AUDIO_I2S2_EXT_DMA_IRQ          DMA1_Stream3_IRQn
#define AUDIO_I2S2_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF3
#define AUDIO_I2S2_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF3
#define AUDIO_I2S2_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF3
#define AUDIO_I2S2_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF3
#define AUDIO_I2S2_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF3



#define CODECA_I2S                      SPI3
#define CODECA_I2S_EXT                  I2S3ext
#define CODECA_I2S_CLK                  RCC_APB1Periph_SPI3
#define CODECA_I2S_ADDRESS              0x40003C0C
#define CODECA_I2S_EXT_ADDRESS          0x4000400C
#define CODECA_I2S_GPIO_AF              GPIO_AF_SPI3

#define CODECA_I2Sext_GPIO_AF			((uint8_t)0x05) /* for PC11 */
//#define CODECA_I2Sext_GPIO_AF			GPIO_AF_I2S3ext /* for PB4 */

#define CODECA_I2S_IRQ                  SPI3_IRQn
#define CODECA_I2S_EXT_IRQ              SPI3_IRQn
#define CODECA_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC)

#define CODECA_I2S_GPIO_WS              GPIOA
#define CODECA_I2S_WS_PIN               GPIO_Pin_4
#define CODECA_I2S_WS_PINSRC            GPIO_PinSource4

#define CODECA_I2S_GPIO_SCK             GPIOC
#define CODECA_I2S_SCK_PIN              GPIO_Pin_10
#define CODECA_I2S_SCK_PINSRC           GPIO_PinSource10

#define CODECA_I2S_GPIO_SDO             GPIOC
#define CODECA_I2S_SDO_PIN              GPIO_Pin_12
#define CODECA_I2S_SDO_PINSRC           GPIO_PinSource12

#define CODECA_I2S_GPIO_SDI             GPIOC
#define CODECA_I2S_SDI_PIN              GPIO_Pin_11
#define CODECA_I2S_SDI_PINSRC           GPIO_PinSource11

#define CODECA_I2S_MCK_GPIO             GPIOC
#define CODECA_I2S_MCK_PIN              GPIO_Pin_7
#define CODECA_I2S_MCK_PINSRC           GPIO_PinSource7

#define AUDIO_I2S3_IRQHandler           SPI3_IRQHandler
#define AUDIO_I2S3_EXT_IRQHandler       SPI3_IRQHandler

//SPI3_TX: S5 C0 or S7 C0
#define AUDIO_I2S3_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S3_DMA_STREAM           DMA1_Stream5
#define AUDIO_I2S3_DMA_DREG             CODECA_I2S_ADDRESS
#define AUDIO_I2S3_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S3_DMA_IRQ              DMA1_Stream5_IRQn
#define AUDIO_I2S3_DMA_FLAG_TC          DMA_FLAG_TCIF0
#define AUDIO_I2S3_DMA_FLAG_HT          DMA_FLAG_HTIF0
#define AUDIO_I2S3_DMA_FLAG_FE          DMA_FLAG_FEIF0
#define AUDIO_I2S3_DMA_FLAG_TE          DMA_FLAG_TEIF0
#define AUDIO_I2S3_DMA_FLAG_DME         DMA_FLAG_DMEIF0

//I2S3_EXT_RX: S0 C3 or S2 C2
#define AUDIO_I2S3_EXT_DMA_STREAM       DMA1_Stream2
#define AUDIO_I2S3_EXT_DMA_DREG         CODECA_I2S_EXT_ADDRESS
#define AUDIO_I2S3_EXT_DMA_CHANNEL      DMA_Channel_2
#define AUDIO_I2S3_EXT_DMA_IRQ          DMA1_Stream2_IRQn
#define AUDIO_I2S3_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF2
#define AUDIO_I2S3_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF2
#define AUDIO_I2S3_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF2
#define AUDIO_I2S3_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF2
#define AUDIO_I2S3_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF2




#define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord
#define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord
#define DMA_MAX_SZE                    0xFFFF

/* I2S DMA Stream definitions */

/* I2C peripheral configuration defines (control interface of the audio codec) */
#define CODECB_I2C                      I2C2
#define CODECB_I2C_CLK                  RCC_APB1Periph_I2C2
#define CODECB_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODECB_I2C_GPIO_AF              GPIO_AF_I2C2
#define CODECB_I2C_GPIO                 GPIOB
#define CODECB_I2C_SCL_PIN              GPIO_Pin_10
#define CODECB_I2C_SDA_PIN              GPIO_Pin_11
#define CODECB_I2C_SCL_PINSRC           GPIO_PinSource10
#define CODECB_I2C_SDA_PINSRC           GPIO_PinSource11

#define CODECA_I2C                      I2C1
#define CODECA_I2C_CLK                  RCC_APB1Periph_I2C1
#define CODECA_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODECA_I2C_GPIO_AF              GPIO_AF_I2C1
#define CODECA_I2C_GPIO                 GPIOB
#define CODECA_I2C_SCL_PIN              GPIO_Pin_8
#define CODECA_I2C_SDA_PIN              GPIO_Pin_9
#define CODECA_I2C_SCL_PINSRC           GPIO_PinSource8
#define CODECA_I2C_SDA_PINSRC           GPIO_PinSource9


/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will 
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define CODEC_FLAG_TIMEOUT             ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_FLAG_TIMEOUT))


uint32_t Codec_Register_Setup(uint8_t enable_DCinput);

void Codec_CtrlInterface_Init(void);

void Codec_AudioInterface_Init(uint32_t AudioFreq);

uint32_t Codec_Reset(I2C_TypeDef *CODEC, uint8_t master_slave, uint8_t enable_DCinput);

uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue, I2C_TypeDef *CODEC);

void Codecs_Deinit(void);

void Codec_GPIO_Init(void);

void init_i2s_clkin(void);

#endif
