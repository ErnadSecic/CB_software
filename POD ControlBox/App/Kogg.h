#ifndef KOGG_H
#define KOGG_H

#define  KOGG_PULSE_DEBONCE_LIMIT  3
#define  KOGG_PULSE_MIN_STABLE_LOW  10

#define  KOGG_PULSE_LEVEL      633       // new values 633 Variation: 414 - 853

//////////////////////////////////////////////////////
// return value of pulse detection:
//////////////////////////////////////////////////////
#define  KOGG_NO_PULSE                   0
#define  KOGG_PULSE                      1
#define  KOGG_USER_SWITCH                2
#define  KOGG_LOW_TO_LONG                3
#define  KOGG_HIGH_TO_LONG               4
#define  KOGG_MISSING_PULSES_LOW_LEV     5
#define  KOGG_MISSING_PULSES_HIGH_LEV    6
#define  KOGG_UNSTABLE_TO_LONG           7

//////////////////////////////////////////////////////
// KOGG- pulse state:
//////////////////////////////////////////////////////
#define KOGG_PULSE_IDLE           0
#define KOGG_PULSE_STARTING       1
#define KOGG_PULSE_STABLE_HIGH    2
#define KOGG_PULSE_ENDING         3


#define FASTER_SPEED_LENGTH_OFF_HIGH     7  // Max speed -  Kogg is high in 5 times  (5 * 200us = 1ms)
#define FASTER_SPEED_LENGTH_OFF_LOW     28  // Max speed -  Kogg is low  in 10 times (25 * 200us = 5ms)

__interwork int koggPulseDetected(int iChannel, int * pnPulsePeriod, int speedCoeff);
__arm  void koggCountPulses( int iMotorNo, int iAdcChannel);
void koggClearOldPulsesCounter(int iAdcIndex);
void koggInit(void);
#endif
