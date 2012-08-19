/*
 * EBike Smart Controller
 *
 * Copyright (C) Jorge Pinto aka Casainho, 2012.
 *
 *   casainho [at] gmail [dot] com
 *     www.casainho.net
 *
 * Released under the GPL Licence, Version 3
 */

#include "lpc210x.h"
#include "pwm.h"
#include "ios.h"

BYTE bSector =1;             /* sector of rotor position, 1~6 is possible value */
BOOL fDir = FALSE;           /* motor direction variable---CCW direction is default */
BYTE baStartUpTimeTbl[12]= {180,150,130,100,80,60
                            ,50,40,30,20,10,5};

BYTE bFreeRunTimePointer = 0; /* pointer of startup time table */

//functions to control each of 6 PWM signals

void phase_u_h_pwm_on (void)
{
  /* LPC2103 P0.2 --> CPU4 */
  /* set to output */
  IODIR |= (1 << 2);
  IOSET = (1 << 2);
}

void phase_u_h_pwm_off (void)
{
  /* LPC2103 P0.2 --> CPU4 */
  /* set to output */
  IODIR |= (1 << 2);
  IOCLR = (1 << 2);
}

void phase_u_l_pwm_on (void)
{
  /* LPC2103 P0.19 (PWM; MAT1.2) --> CPU4 */
//  PINSEL1 |= (1 << 6);
  /* LPC2103 P0.19 (PWM; MAT1.2) --> CPU4 */
  PINSEL1 &= ~(1 << 6);
  /* set to output */
  IODIR |= (1 << 19);
  IOCLR = (1 << 19); /* inverted logic */
}

void phase_u_l_pwm_off (void)
{
  /* LPC2103 P0.19 (PWM; MAT1.2) --> CPU4 */
  PINSEL1 &= ~(1 << 6);
  /* set to output */
  IODIR |= (1 << 19);
  IOSET = (1 << 19); /* inverted logic */
}

void phase_v_h_pwm_on (void)
{
  /* LPC2103 P0.1 --> CPU3 */
  /* set to output */
  IODIR |= (1 << 1);
  IOSET = (1 << 1);
}

void phase_v_h_pwm_off (void)
{
  /* LPC2103 P0.1 --> CPU3 */
  /* set to output */
  IODIR |= (1 << 1);
  IOCLR = (1 << 1);
}

void phase_v_l_pwm_on (void)
{
  /* LPC2103 P0.13 (PWM; MAT1.1) --> CPU2 */
//  PINSEL0 |= (1 << 26);
  /* set to output */
  IODIR |= (1 << 13);
  IOCLR = (1 << 13); /* inverted logic */
}

void phase_v_l_pwm_off (void)
{
  /* LPC2103 P0.13 (PWM; MAT1.1) --> CPU2 */
  PINSEL0 &= ~(1 << 26);
  /* set to output */
  IODIR |= (1 << 13);
  IOSET = (1 << 13); /* inverted logic */
}

void phase_w_h_pwm_on (void)
{
  /* LPC2103 P0.0 --> CPU1 */
  /* set to output */
  IODIR |= (1 << 0);
  IOSET = (1 << 0);
}

void phase_w_h_pwm_off (void)
{
  /* LPC2103 P0.0 --> CPU1 */
  /* set to output */
  IODIR |= (1 << 0);
  IOCLR = (1 << 0);
}

void phase_w_l_pwm_on (void)
{
  /* LPC2103 P0.12 (PWM; MAT1.0) --> CPU44 */
//  PINSEL0 |= (1 << 24);
  /* set to output */
  IODIR |= (1 << 12);
  IOCLR= (1 << 12); /* inverted logic */
}

void phase_w_l_pwm_off (void)
{
  /* LPC2103 P0.12 (PWM; MAT1.0) --> CPU44 */
  PINSEL0 &= ~(1 << 24);
  /* set to output */
  IODIR |= (1 << 12);
  IOSET = (1 << 12); /* inverted logic */
}

void commutation_sector_1 (void)
{
  phase_u_l_pwm_off ();
  phase_u_h_pwm_on ();

  phase_v_h_pwm_off ();
  phase_v_l_pwm_on ();

  phase_w_h_pwm_off ();
  phase_w_l_pwm_off ();
}

void commutation_sector_2 (void)
{
  phase_u_l_pwm_off ();
  phase_u_h_pwm_on ();

  phase_v_h_pwm_off ();
  phase_v_l_pwm_off ();

  phase_w_h_pwm_off ();
  phase_w_l_pwm_on ();
}

void commutation_sector_3 (void)
{
  phase_u_h_pwm_off ();
  phase_u_l_pwm_off ();

  phase_v_l_pwm_off ();
  phase_v_h_pwm_on ();

  phase_w_h_pwm_off ();
  phase_w_l_pwm_on ();
}

void commutation_sector_4 (void)
{
  phase_u_h_pwm_off ();
  phase_u_l_pwm_on ();

  phase_v_l_pwm_off ();
  phase_v_h_pwm_on ();

  phase_w_h_pwm_off ();
  phase_w_l_pwm_off ();
}

void commutation_sector_5 (void)
{
  phase_u_h_pwm_off ();
  phase_u_l_pwm_on ();

  phase_v_h_pwm_off ();
  phase_v_l_pwm_off ();

  phase_w_l_pwm_off ();
  phase_w_h_pwm_on ();
}

void commutation_sector_6 (void)
{
  phase_u_h_pwm_off ();
  phase_u_l_pwm_off ();

  phase_v_h_pwm_off ();
  phase_v_l_pwm_on ();

  phase_w_l_pwm_off ();
  phase_w_h_pwm_on ();
}

void commutation_disable (void)
{
  phase_u_h_pwm_off ();
  phase_u_l_pwm_off ();

  phase_v_h_pwm_off ();
  phase_v_l_pwm_off ();

  phase_w_h_pwm_off ();
  phase_w_l_pwm_off ();
}

void commutate (BYTE sector)
{
  switch (sector)
  {
    case 1:
    commutation_sector_1 ();
    break;

    case 6:
    commutation_sector_6 ();
    break;

    case 5:
    commutation_sector_5 ();
    break;

    case 4:
    commutation_sector_4 ();
    break;

    case 3:
    commutation_sector_3 ();
    break;

    case 2:
    commutation_sector_2 ();
    break;

    default:
    commutation_disable ();
    break;
  }
}


void commutation(void)
{
  commutate(bSector);

  /* ClockWise rotation */
  if(fDir)
  {
    bSector--;
    if(bSector<1)
    {
      bSector = 6;
    }
  }

  else
  {
    bSector++;
    if(bSector>6)
    {
      bSector =1;
    }
  }
}

void CheckZeroCrossing()
{//TODO
}

#if 0
//running without any sensor feedback
void FreeRun(void)
{
    WORD wTimeCur;

    /* Switch channel */
    Commutation();

    wTimeCur = Timer1_wReadTimer();

    /* loop for next freerun commutate */
    while((Timer1_wReadTimer()-wTimeCur)< 100*(WORD)baStartUpTimeTbl[bFreeRunTimePointer])
        {
        // check for bemf zero crossing here
        CheckZeroCrossing();
    }

    /* Add speed bit by bit */
    bFreeRunTimePointer++;
    if(bFreeRunTimePointer >11 ) /* Keep constant speeed */
    {
        bFreeRunTimePointer = 11;
    }

}
#endif
