//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.04
//////////////////////////////////////////////////////////////////////////////////////
#include <board.h>
#include "FreeRTOS.h"
#include "task.h"
#include "adc.h"
#include "Motor.h"
#include "Kogg.h"
#include "SpeedRegulator.h"


//char cFirstePulsePeriod = 1;
char cFirstePulsePeriod[ADC_MAX_PULSE_CHAN];
//int nPulsePeriodCounter = 0;
int nPulsePeriodCounter[ADC_MAX_PULSE_CHAN];

//int iIdleCounter = 0;
int iIdleCounter [ADC_MAX_PULSE_CHAN];

//int uiLedHighCount[ADC_MAX_PULSE_CHAN];

static unsigned int uiLevACount[ADC_MAX_PULSE_CHAN];    // Pulse level counters A-Level   (LED Light Blocked)
static unsigned int uiLevBCount[ADC_MAX_PULSE_CHAN];    // Pulse level counters B-Level   (LED Light Opened)
static int uiPulseState[ADC_MAX_PULSE_CHAN];            // Pulse counter states

//* Global variable
//int testVerdi[100];
//int testCount = 0;

extern MOTOR_DATA MotorData[MAX_MOTORS];


void koggInit(void)
{

   uiPulseState[0] = KOGG_PULSE_IDLE;
   uiPulseState[1] = KOGG_PULSE_IDLE;
   uiPulseState[2] = KOGG_PULSE_IDLE;
   uiPulseState[3] = KOGG_PULSE_IDLE;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : koggCountPulses()
// Purpose      : To check Kogg pulses state and
// Parameters   : iMotorNo, iAdcChannel
// Return value :
// Created      : 2005-09-17 by PVA
// Updated      : 2005-11-14 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
__interwork void koggCountPulses( int iMotorNo, int iAdcChannel)
{
  int iMotorIndex = iMotorNo-1;
  int iPulseResult;
  int nPulsePeriod;

   if (MotorData[iMotorIndex].iMotorState ==  MOTOR_MOVING || MotorData[iMotorIndex].iMotorState == MOTOR_WAITING_ON_ROTATION_END)
   {
      iPulseResult = koggPulseDetected(iAdcChannel, &nPulsePeriod, MotorData[iMotorIndex].iWantedSpeed);

    if (iPulseResult == KOGG_MISSING_PULSES_LOW_LEV)
    {
      if (MotorData[iMotorIndex].iMotorState == MOTOR_MOVING)
      {
        motorStop(iMotorNo, STOP_NO_PULSES_LOW_LEVEL);
      }
      else //
      {
        MotorData[iMotorIndex].iMotorState = MOTOR_FINISHED;
      }
    }
    else if (iPulseResult == KOGG_MISSING_PULSES_HIGH_LEV)
    {
       if (MotorData[iMotorIndex].iMotorState == MOTOR_MOVING)
      {
        motorStop(iMotorNo, STOP_NO_PULSES_HIGH_LEVEL);
      }
      else //
      {
        MotorData[iMotorIndex].iMotorState = MOTOR_FINISHED;
      }
    }
    else if (iPulseResult == KOGG_UNSTABLE_TO_LONG)
    {
      if (MotorData[iMotorIndex].iMotorState == MOTOR_MOVING)
      {
        motorStop(iMotorNo, STOP_UNSTABLE_PULSES);
      }
      else //
      {
        MotorData[iMotorIndex].iMotorState = MOTOR_FINISHED;
      }
    }
    else if ((iPulseResult == KOGG_PULSE))// && (MotorData[iMotorIndex].iPulseCount > 0))
    {
       // Count down, motor will be stopped from motorStop() when zero is reached
       MotorData[iMotorIndex].iPulseCount++;

       if ((MotorData[iMotorIndex].iPulseCount) == (MotorData[iMotorIndex].iRequestedCnt))
       {
           // Stop motor
           motorStop( iMotorNo, STOP_FINISHED);
       }
       else
       {
         if (MotorData[iMotorIndex].iRegulateSpeed == REGULATE_SPEED)
         {
          speedRegulatorSpeedControl(iMotorIndex, nPulsePeriod);
         }
       }
     }
     else if (iPulseResult == KOGG_HIGH_TO_LONG)
     {
       if (MotorData[iMotorIndex].iRegulateSpeed == REGULATE_SPEED)
       {
           speedRegulatorAvoidMotorStop(iMotorIndex, iPulseResult);
       }
     }
     else if (iPulseResult == KOGG_LOW_TO_LONG)
     {
       if (MotorData[iMotorIndex].iRegulateSpeed == REGULATE_SPEED)
       {
           speedRegulatorAvoidMotorStop(iMotorIndex, iPulseResult);
       }
     }
   }

   return;
}



//////////////////////////////////////////////////////////////////////////////
// Function     : koggPulseDetected()
// Purpose      : To detect pulses from motor
// Parameters   : ADC channel to detect pulse on
// Return value : 0==NO PULSE, 1==PULSE, 2==USER_SWITCH
// Created      : 2005-09-17 by PVA
// Updated      : 2005-11-14 by ESE
// Comments     : This function starts from timer interrupt each 200us
//////////////////////////////////////////////////////////////////////////////
__interwork int koggPulseDetected(int iChannel, int * pnPulsePeriod, int speedCoeff)
{
   unsigned long int uiAdcSample;

   // Read ADC:
   uiAdcSample = adcRead(iChannel);
   nPulsePeriodCounter[iChannel] ++;
   *pnPulsePeriod = -1;

   if (speedCoeff < 1)
   {
     speedCoeff = 1;
   }
   
   if ((uiPulseState[iChannel] == KOGG_PULSE_IDLE) && (uiAdcSample > KOGG_PULSE_LEVEL))
   {
      // Start pulse seeking at high level (when LED ligth is blocked)
      uiPulseState[iChannel] = KOGG_PULSE_STARTING;
      uiLevACount[iChannel] = 1;
      uiLevBCount[iChannel] = 0;
   }
   else if (uiPulseState[iChannel] == KOGG_PULSE_IDLE)
   {
     iIdleCounter[iChannel] ++;

     if (iIdleCounter[iChannel] > ((FASTER_SPEED_LENGTH_OFF_LOW * 100 * 20) /(speedCoeff)))
     {
       return (KOGG_MISSING_PULSES_LOW_LEV);
     }
     // ************************* ENO 
     if (iIdleCounter[iChannel] > ((FASTER_SPEED_LENGTH_OFF_LOW *100*5/(speedCoeff*3))))// 100 * 3) /(speedCoeff * 2)))
     {
       return(KOGG_LOW_TO_LONG);
     }
   }
   else if (uiPulseState[iChannel] == KOGG_PULSE_STARTING)
   {iIdleCounter[iChannel] = 0;
      // Count up equal samlples to accept level (Ligth is blocked)
      if (uiAdcSample > KOGG_PULSE_LEVEL)
      {
         uiLevACount[iChannel]++;
         if (uiLevACount[iChannel] >= KOGG_PULSE_DEBONCE_LIMIT)
         {
            // Level stabel, wait for level change
            uiPulseState[iChannel] = KOGG_PULSE_STABLE_HIGH;
         }
      }
      else
      {
         // Unstable line, start all over
         uiPulseState[iChannel] = KOGG_PULSE_IDLE;
      }
   }
   else if (uiPulseState[iChannel] == KOGG_PULSE_STABLE_HIGH)
   {
      if  (uiAdcSample <= KOGG_PULSE_LEVEL)
      {
         // Next level detected (Light Open), beging counting samples at this level
         uiPulseState[iChannel] = KOGG_PULSE_ENDING;
         uiLevBCount[iChannel] = 1;
         uiLevACount[iChannel] = 0;
      }

      else
      {
         // No change in level (Ligth is still blocked, so this can be a user switch event
         uiLevACount[iChannel]++;
         if (uiLevACount[iChannel] > ((FASTER_SPEED_LENGTH_OFF_LOW * 100 * 20) / (speedCoeff)))
         {
           return KOGG_MISSING_PULSES_HIGH_LEV;
         }
         // ******************************* eno
         else if (uiLevACount[iChannel] > ((FASTER_SPEED_LENGTH_OFF_HIGH *100*5/(speedCoeff*3))))//* 100 * 3) / (speedCoeff *2)))
         {
            // Ligth blocked, so this is a user switch event
            return(KOGG_HIGH_TO_LONG);
         }
      }
   }
   else if (uiPulseState[iChannel] == KOGG_PULSE_ENDING)
   {
      if (uiAdcSample <= KOGG_PULSE_LEVEL)
      {
         uiLevBCount[iChannel]++;
         if (uiLevBCount[iChannel] >= KOGG_PULSE_MIN_STABLE_LOW)
         {
            // Low level stabel, pulse detected
            uiPulseState[iChannel] = KOGG_PULSE_IDLE;
            if (cFirstePulsePeriod[iChannel] == 1)
            {
               *pnPulsePeriod = -1;
               cFirstePulsePeriod[iChannel] = 0;
            }
            else
            {
              *pnPulsePeriod = nPulsePeriodCounter[iChannel];
            }

            nPulsePeriodCounter[iChannel] = 0;

            return (KOGG_PULSE);
         }
      }
      else
      {
         // Unstable line, start all over
         uiPulseState[iChannel] = KOGG_PULSE_IDLE;
      }
   }
   else
   {
     // never comming here.
     uiPulseState[iChannel] = KOGG_PULSE_IDLE;
   }

   if (nPulsePeriodCounter[iChannel] > ((FASTER_SPEED_LENGTH_OFF_LOW * 100 * 20) /(speedCoeff)))
   {
     return (KOGG_UNSTABLE_TO_LONG);
   }

   return (KOGG_NO_PULSE);
}



void koggClearOldPulsesCounter(int iAdcIndex)
{
  uiLevACount[iAdcIndex] = 0;
  uiLevBCount[iAdcIndex] = 0;

  cFirstePulsePeriod[iAdcIndex] = 1;
  nPulsePeriodCounter[iAdcIndex] = 0;
  iIdleCounter[iAdcIndex] = 0;
  
  uiPulseState[iAdcIndex] = KOGG_PULSE_IDLE;
}
