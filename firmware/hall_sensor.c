/*
 * EBike Smart Controller
 *
 * Copyright (C) Jorge Pinto aka Casainho, 2012, 2013.
 *
 *   casainho [at] gmail [dot] com
 *     www.casainho.net
 *
 * Released under the GPL License, Version 3
 */

/*
 * PA6  (TIM3_CH1)      -- Hall sensor 1
 * PA7  (TIM3_CH2)      -- Hall sensor 2
 * PB0  (TIM3_CH3)      -- Hall sensor 3
 */

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "bldc_hall.h"

unsigned int hall_sensors_time = 0;

void TIM3_IRQHandler(void)
{
  /* "Read" all sensors sequence and execute the BLDC coils commutation */
  commutate ();

  /* Save current time between each hall sensor signal change */
  hall_sensors_time = TIM_GetCapture1(TIM3);

  // clear interrupt flag
  TIM_ClearITPendingBit (TIM3, TIM_IT_Trigger);
}

unsigned int get_hall_sensors_us (void)
{
  return hall_sensors_time * 10; // multiply by 10 to get in us
}

void hall_sensor_init (void)
{
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* GPIOA and GPIOB clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
  /* TIM3 channel 1 and 2, pin (PA.06 and PA.07) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* TIM3 channel 3, pin (PB.00) configuration */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOB, &GPIO_InitStructure);


  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  // timer base configuration
  // 84 => 655ms till overflow ; 100kHz (10us) TimerClock [24MHz/Prescaler]
  TIM_TimeBaseStructure.TIM_Prescaler = 240;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  // enable hall sensor
  // T1F_ED will be connected to  HallSensors Inputs
  // TIM3_CH1, TIM3_CH2, TIM3_CH3
  TIM_SelectHallSensor(TIM3, ENABLE);

  // HallSensor event is delivered with signal TI1F_ED
  // (this is XOR of the three hall sensor lines)
  // Signal TI1F_ED: falling and rising edge of the inputs is used
  TIM_SelectInputTrigger(TIM3, TIM_TS_TI1F_ED);

  // On every TI1F_ED event the counter is resetted and update is tiggered
  TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);

  // Channel 1 in input capture mode
  // on every TCR edge (build from TI1F_ED which is a HallSensor edge)
  // the timervalue is copied into ccr register and a CCR1 Interrupt
  // TIM_IT_CC1 is fired
  TIM_ICInitTypeDef TIM_ICInitStructure;
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  // listen to T1, the  HallSensorEvent
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_TRC;
  // Div:1, every edge
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInit(TIM3, &TIM_ICInitStructure);

  /* Enable the TIM3 Trigger Interrupt Request */
  TIM_ITConfig(TIM3, TIM_IT_Trigger, ENABLE);

  /* TIM enable counter */
  TIM_Cmd(TIM3, ENABLE);
}
