/*
 * codec.h - stm32f405_codec board codec interface routines
 *
 * Cut from stm32f4_discovery_audio_codec.c
 *
 */

#ifndef __codec__
#define __codec__

#include <stm32f4xx.h>

/* I2C clock speed configuration (in Hz)  */
#define I2C_SPEED                       10000

// Uncomment defines below to select standard for audio communication between Codec and I2S peripheral
//#define I2S_STANDARD_PHILLIPS
#define I2S_STANDARD_MSB
//#define I2S_STANDARD_LSB

//#define USE_DEFAULT_TIMEOUT_CALLBACK

/*-----------------------------------
Hardware Configuration defines parameters
-----------------------------------------*/                 
/* I2S peripheral configuration defines */

#define CODEC_I2S2                      SPI2
#define CODEC_I2S2_EXT                  I2S2ext
#define CODEC_I2S2_CLK                  RCC_APB1Periph_SPI2
#define CODEC_I2S2_ADDRESS              0x4000380C
#define CODEC_I2S2_EXT_ADDRESS          0x4000340C
#define CODEC_I2S2_GPIO_AF              GPIO_AF_SPI2
#define CODEC_I2S2ext_GPIO_AF			GPIO_AF_SPI2
#define CODEC_I2S2_IRQ                  SPI2_IRQn
#define CODEC_I2S2_EXT_IRQ              SPI2_IRQn
#define CODEC_I2S2_GPIO_CLOCK           (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB)


#define CODEC_I2S2_MCK_GPIO             GPIOC
#define CODEC_I2S2_MCK_PIN              GPIO_Pin_6
#define CODEC_I2S2_MCK_PINSRC           GPIO_PinSource6

#define CODEC_I2S2_GPIO                 GPIOB
#define CODEC_I2S2_WS_PIN               GPIO_Pin_12
#define CODEC_I2S2_WS_PINSRC            GPIO_PinSource12

#define CODEC_I2S2_GPIO_SD              GPIOB
#define CODEC_I2S2_SDO_PIN              GPIO_Pin_15
#define CODEC_I2S2_SDO_PINSRC           GPIO_PinSource15

#define CODEC_I2S2_GPIO_CK              GPIOB
#define CODEC_I2S2_SCK_PIN              GPIO_Pin_13
#define CODEC_I2S2_SCK_PINSRC           GPIO_PinSource13

#define CODEC_I2S2_GPIO_SDI             GPIOB
#define CODEC_I2S2_SDI_PIN              GPIO_Pin_14
#define CODEC_I2S2_SDI_PINSRC           GPIO_PinSource14

#define AUDIO_I2S2_IRQHandler           SPI2_IRQHandler
#define AUDIO_I2S2_EXT_IRQHandler       SPI2_IRQHandler

#define AUDIO_I2S2_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S2_DMA_STREAM           DMA1_Stream4
#define AUDIO_I2S2_DMA_DREG             CODEC_I2S2_ADDRESS
#define AUDIO_I2S2_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S2_DMA_IRQ              DMA1_Stream4_IRQn
#define AUDIO_I2S2_DMA_FLAG_TC          DMA_FLAG_TCIF0
#define AUDIO_I2S2_DMA_FLAG_HT          DMA_FLAG_HTIF0
#define AUDIO_I2S2_DMA_FLAG_FE          DMA_FLAG_FEIF0
#define AUDIO_I2S2_DMA_FLAG_TE          DMA_FLAG_TEIF0
#define AUDIO_I2S2_DMA_FLAG_DME         DMA_FLAG_DMEIF0

#define AUDIO_I2S2_EXT_DMA_STREAM       DMA1_Stream3
#define AUDIO_I2S2_EXT_DMA_DREG         CODEC_I2S2_EXT_ADDRESS
#define AUDIO_I2S2_EXT_DMA_CHANNEL      DMA_Channel_3
#define AUDIO_I2S2_EXT_DMA_IRQ          DMA1_Stream3_IRQn
#define AUDIO_I2S2_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF3
#define AUDIO_I2S2_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF3
#define AUDIO_I2S2_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF3
#define AUDIO_I2S2_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF3
#define AUDIO_I2S2_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF3



#define CODEC_I2S3                      SPI3
#define CODEC_I2S3_EXT                  I2S3ext
#define CODEC_I2S3_CLK                  RCC_APB1Periph_SPI3
#define CODEC_I2S3_ADDRESS              0x40003C0C
#define CODEC_I2S3_EXT_ADDRESS          0x4000400C
#define CODEC_I2S3_GPIO_AF              GPIO_AF_SPI3
#define CODEC_I2S3ext_GPIO_AF			GPIO_AF_I2S3ext
#define CODEC_I2S3_IRQ                  SPI3_IRQn
#define CODEC_I2S3_EXT_IRQ              SPI3_IRQn
#define CODEC_I2S3_GPIO_CLOCK           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC)

#define CODEC_I2S3_GPIO                 GPIOA
#define CODEC_I2S3_WS_PIN               GPIO_Pin_4
#define CODEC_I2S3_WS_PINSRC            GPIO_PinSource4

#define CODEC_I2S3_GPIO_CK              GPIOC
#define CODEC_I2S3_SCK_PIN              GPIO_Pin_10
#define CODEC_I2S3_SCK_PINSRC           GPIO_PinSource10

#define CODEC_I2S3_GPIO_SD              GPIOC
#define CODEC_I2S3_SDO_PINSRC           GPIO_PinSource12
#define CODEC_I2S3_SDO_PIN              GPIO_Pin_12

#define CODEC_I2S3_GPIO_SDI             GPIOC
#define CODEC_I2S3_SDI_PIN              GPIO_Pin_11
#define CODEC_I2S3_SDI_PINSRC           GPIO_PinSource11

#define CODEC_I2S3_MCK_GPIO             GPIOC
#define CODEC_I2S3_MCK_PIN              GPIO_Pin_7
#define CODEC_I2S3_MCK_PINSRC           GPIO_PinSource7

#define AUDIO_I2S3_IRQHandler           SPI3_IRQHandler
#define AUDIO_I2S3_EXT_IRQHandler       SPI3_IRQHandler

#define AUDIO_I2S3_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S3_DMA_STREAM           DMA1_Stream5
#define AUDIO_I2S3_DMA_DREG             CODEC_I2S3_ADDRESS
#define AUDIO_I2S3_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S3_DMA_IRQ              DMA1_Stream5_IRQn
#define AUDIO_I2S3_DMA_FLAG_TC          DMA_FLAG_TCIF0
#define AUDIO_I2S3_DMA_FLAG_HT          DMA_FLAG_HTIF0
#define AUDIO_I2S3_DMA_FLAG_FE          DMA_FLAG_FEIF0
#define AUDIO_I2S3_DMA_FLAG_TE          DMA_FLAG_TEIF0
#define AUDIO_I2S3_DMA_FLAG_DME         DMA_FLAG_DMEIF0

#define AUDIO_I2S3_EXT_DMA_STREAM       DMA1_Stream2
#define AUDIO_I2S3_EXT_DMA_DREG         CODEC_I2S3_EXT_ADDRESS
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
#define CODEC_I2C2                      I2C2
#define CODEC_I2C2_CLK                  RCC_APB1Periph_I2C2
#define CODEC_I2C2_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C2_GPIO_AF              GPIO_AF_I2C2
#define CODEC_I2C2_GPIO                 GPIOB
#define CODEC_I2C2_SCL_PIN              GPIO_Pin_10
#define CODEC_I2C2_SDA_PIN              GPIO_Pin_11
#define CODEC_I2C2_SCL_PINSRC           GPIO_PinSource10
#define CODEC_I2C2_SDA_PINSRC           GPIO_PinSource11

#define CODEC_I2C1                      I2C1
#define CODEC_I2C1_CLK                  RCC_APB1Periph_I2C1
#define CODEC_I2C1_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C1_GPIO_AF              GPIO_AF_I2C1
#define CODEC_I2C1_GPIO                 GPIOB
#define CODEC_I2C1_SCL_PIN              GPIO_Pin_8
#define CODEC_I2C1_SDA_PIN              GPIO_Pin_9
#define CODEC_I2C1_SCL_PINSRC           GPIO_PinSource8
#define CODEC_I2C1_SDA_PINSRC           GPIO_PinSource9


/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will 
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define CODEC_FLAG_TIMEOUT             ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_FLAG_TIMEOUT))


uint32_t Codec_Init(uint32_t AudioFreq);

void Codec_CtrlInterface_Init(void);

void Codec_AudioInterface_Init(uint32_t AudioFreq);

uint32_t Codec_Reset(I2C_TypeDef *CODEC, uint8_t master_slave);

uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue, I2C_TypeDef *CODEC);

void Codec_GPIO_Init(void);
void init_i2s_clkin(void);

#endif
