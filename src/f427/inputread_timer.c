#include "buttons_jacks.h"
#include "dig_pins.h"
#include "stm32f4xx.h"

void init_inputread_timer(void) {
  TIM_TimeBaseInitTypeDef tim;

  NVIC_InitTypeDef nvic;
  RCC_APB1PeriphClockCmd(INFREVBUTTONJACK_PINGBUT_TIM_RCC, ENABLE);

  nvic.NVIC_IRQChannel = INFREVBUTTONJACK_PINGBUT_TIM_IRQn;
  nvic.NVIC_IRQChannelPreemptionPriority = 3;
  nvic.NVIC_IRQChannelSubPriority = 0;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);

  TIM_TimeBaseStructInit(&tim);
  // 3000 --> 28kHz
  tim.TIM_Period = 3000;
  tim.TIM_Prescaler = 0;
  tim.TIM_ClockDivision = 0;
  tim.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(INFREVBUTTONJACK_PINGBUT_TIM, &tim);

  TIM_ITConfig(INFREVBUTTONJACK_PINGBUT_TIM, TIM_IT_Update, ENABLE);

  TIM_Cmd(INFREVBUTTONJACK_PINGBUT_TIM, ENABLE);

  // Ping jack timer
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);

  nvic.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
  nvic.NVIC_IRQChannelPreemptionPriority = 0;
  nvic.NVIC_IRQChannelSubPriority = 1;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);

  TIM_TimeBaseStructInit(&tim);
  tim.TIM_Period = 5000; // every 30uS
  tim.TIM_Prescaler = 0;
  tim.TIM_ClockDivision = 0;
  tim.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM10, &tim);

  TIM_ITConfig(TIM10, TIM_IT_Update, ENABLE);

  TIM_Cmd(TIM10, ENABLE);
}

// Defined in button_jacks.c
void ping_jack_read(void);
void buttons_jacks_read(void);

void TIM1_UP_TIM10_IRQHandler(void) {
  if (TIM_GetITStatus(TIM10, TIM_IT_Update) != RESET) {
    ping_jack_read();
    TIM_ClearITPendingBit(TIM10, TIM_IT_Update);
  }
}

void TIM4_IRQHandler(void) {
  // Clear TIM update interrupt
  TIM_ClearITPendingBit(INFREVBUTTONJACK_PINGBUT_TIM, TIM_IT_Update);
  buttons_jacks_read();
}
