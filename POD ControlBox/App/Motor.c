//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.04
//////////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"

#include "GpIo.h"
#include "PWM.h"
#include "Adc.h"
#include "Timer.h"
#include "Motor.h"
#include "Kogg.h"
#include "SpeedRegulator.h"
#include "ECom.h"
#include "Button.h"


 int m_nMainMotorTotalCount;
 int m_nAuxilaryMotorStopReason;
 int m_TwinMode;

char iActiveEJSSCT= 0;


#define   PERMITED_DIFFERENCE_TIGHT_TEST_PUSHER       8 // kan settes også 6 eller 7
#define   PERMITED_DIFFERENCE_TIGHT_TIGHTEN_PUSHER    12// var 16
#define   PERMITED_DIFFERENCE_REVERSE                 18

static long permitedDifferenceTighting;


#define SUM_COUNT              24  //75 // 375
#define COUNT                 100  //200 // 900

#define  LAST_SAMPLE_COUNT      3  // 3

static int bOverloadTimerStateA = -1;
static int bOverloadTimerStateB = -1;

int nLastIndexA = 0;
int nLastIndexB = 0;

unsigned int iAdcTight_A[COUNT]; 
unsigned int iAdcTight_B[COUNT]; 

unsigned int iAdcTightInd_A = 0;
unsigned int iAdcTightInd_B = 0;

unsigned int  iAdcLast_A[LAST_SAMPLE_COUNT];
unsigned int  iAdcLast_B[LAST_SAMPLE_COUNT];

static char bMotorConnected[MAX_MOTORS];

int iEJSSCT_Count[MAX_EJSSCT];
static int   iBridgeActive[2];         // Status to flag active bridge (A - B) in us

MOTOR_DATA MotorData[MAX_MOTORS];
int iLastMotorDirection[MAX_MOTORS];

static int iCurrentOverload = 0;

__arm int motorOverLoad(int iMotorNo, int * bOverloadTimerState, unsigned int iAdcTight[], unsigned int * iAdcTightInd, unsigned int iAdcLast[], int * nLastIndex, unsigned char IND_BRIDGE_TIMER, unsigned char ADC_CH);
int motorStop2( int iMotorNo, int iReason);

///////////////////////////////////
// PI REGULATOR
//int m_SpeedCoeff = 1;
////////////////////////////////////7

////////////////////////////////////////////////////////////////////////////
// Function     : motorChangeSpeed()
// Purpose      : Used by speed regulator to change speed
// Parameters   : iMotorIndex, iSpeedDifference
// Return value : NONE
// Created      : 2005-10-13 by ES
// Updated      : 2005-10-13 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorChangeSpeed(int iMotorIndex, int iSpeedDifference)
{
  motorSetSpeed(iMotorIndex, (MotorData[iMotorIndex].iSpeed + iSpeedDifference));
}


////////////////////////////////////////////////////////////////////////////
// Function     : motorSetSpeed()
// Purpose      : Used by speed regulator to set speed
// Parameters   : iMotorIndex, iSpeedDifference
// Return value : NONE
// Created      : 2005-10-13 by ES
// Updated      : 2005-10-13 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorSetSpeed(int iMotorIndex, int iNewSpeed)
{
  MotorData[iMotorIndex].iSpeed = iNewSpeed;

  if (MotorData[iMotorIndex].iSpeed > 99)
  {
      MotorData[iMotorIndex].iSpeed = 99;
  }
  else if(MotorData[iMotorIndex].iSpeed < 1)
  {
      MotorData[iMotorIndex].iSpeed = 1;
  }

  if (iMotorIndex > -1)  // motor 1 - 8
  {
    if (iMotorIndex < 8)
    {
      // Bridge A selected
      if (MotorData[iMotorIndex].iDirection == FORWARD)
      {
          pwmUpdate(PWM_CH1, MotorData[iMotorIndex].iSpeed);
      }
      else
      {
          pwmUpdate(PWM_CH0, MotorData[iMotorIndex].iSpeed);
      }
    }
   }

  if (iMotorIndex > 7)    // motor 9 - 16
   {
     if (iMotorIndex < 16)
     {
      // Bridge B selected
      if (MotorData[iMotorIndex].iDirection == FORWARD)
      {
          pwmUpdate(PWM_CH3, MotorData[iMotorIndex].iSpeed);
      }
      else
      {
          pwmUpdate(PWM_CH2, MotorData[iMotorIndex].iSpeed);
      }
     }
   }
}

////////////////////////////////////////////////////////////////////////////
// Function     : motorGetReasonOfStop()
// Purpose      :
// Parameters   : iMotorIndex
// Return value : Reason of the last stop
// Created      : 2005-10-13 by ES
// Updated      : 2005-10-13 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorGetReasonOfStop(int iMotorIndex)
{
  return MotorData[iMotorIndex].iReasonOfStop;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorOverLoadDetected()
// Purpose      : To check if an overload situation has occured
// Parameters   : None
// Return value : TRUE/FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorOverLoadDetected( void)
{
   return( iCurrentOverload);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorOverLoadClear()
// Purpose      : To restart overloadflag befor restarting motor
// Parameters   : None
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorOverLoadClear( void)
{
   iCurrentOverload = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorRearposition()
// Purpose      : To check wether a motor has stopped in rear position
// Parameters   : Motor no 1 - 16
// Return value : TRUE/FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorRearposition( int iMotorNo)
{
   return(MotorData[iMotorNo-1].iRearposition);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorSetRearposition()
// Purpose      : To registrate motor in rear position
// Parameters   : Motor no 1 - 16
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorSetRearposition( int iMotorNo)
{
   MotorData[iMotorNo-1].iRearposition = 1;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorClearRearposition()
// Purpose      : To clear rear position flag
// Parameters   : Motor no 1 - 16
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorClearRearposition( int iMotorNo)
{
   MotorData[iMotorNo-1].iRearposition = 0;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : motorGetBridgeIndex()
// Purpose      : To get index to Bridge status (A or B)
// Parameters   : Motor no 1 - 16
// Return value : Return index
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorGetBridgeIndex( int iMotorNo)
{
  if (iMotorNo < 9)
  {
      return (0);
  }
  else
  {
      return (1);
  }
}


//////////////////////////////////////////////////////////////////////////////
// Function     : motorGetAdcIndex()
// Purpose      : To get index to ADC for selected motor 1 - 16
// Parameters   : Motor no 1 - 16
// Return value : Return index
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorGetAdcIndex( int iMotorNo)
{
  int iAdcIndex = 0;

  switch (iMotorNo)
  {
  case 1:
  case 3:
  case 5:
  case 7:
    // Measure on ADC0:
      iAdcIndex = 0;
    break;

  case 9:
  case 11:
  case 13:
  case 15:
    // Measure on ADC1:
      iAdcIndex = 1;
    break;

  case 2:
  case 4:
  case 6:
  case 8:
    // Measure on ADC2:
      iAdcIndex = 2;
    break;

  case 10:
  case 12:
  case 14:
  case 16:
    // Measure on ADC3:
      iAdcIndex = 3;
    break;

  default:
    break;
  }

  return ( iAdcIndex);
}



//////////////////////////////////////////////////////////////////////////////
// Function     : motorReset()
// Purpose      : To initialize  one MotorData structure
// Parameters   : Motor Index
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorReset( int iMotorIndex)
{
   MotorData[iMotorIndex].iDirection = NO_DIRECTION;
   MotorData[iMotorIndex].iAdcIndex = -1;
   MotorData[iMotorIndex].iPulseCount = 0;
   MotorData[iMotorIndex].iRequestedCnt = 0;
   MotorData[iMotorIndex].iRearposition = 0;
   MotorData[iMotorIndex].iSpeed = 0;
   MotorData[iMotorIndex].iWantedSpeed = 0;
   MotorData[iMotorIndex].iReasonOfStop = 0;
   MotorData[iMotorIndex].iRegulateSpeed = 0;
   MotorData[iMotorIndex].iMotorState = MOTOR_NOT_IN_USE;
   MotorData[iMotorIndex].iTwinMotorNo = 0;
   MotorData[iMotorIndex].iIsMainMotor = 0;

   if (iMotorIndex < 8)
   {
      bOverloadTimerStateA = -1;
      speedRegulatorResetCounterA();
   }
   else
   {
      bOverloadTimerStateB=-1;
      speedRegulatorResetCounterB();
   }
   koggClearOldPulsesCounter(motorGetAdcIndex(iMotorIndex + 1));
}

void motorResetTwinInfo(void)
{
  m_nMainMotorTotalCount = 0;
  m_nAuxilaryMotorStopReason = 0;
  m_TwinMode = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorInit()
// Purpose      : To initialize  MotorData 1 - 16
// Parameters   : None
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorInit( void)
{
  int i;

   gpioClear(BRIDGE_A_MASK);
   gpioClear(BRIDGE_B_MASK);

   // Init and disable all PWM outlets !!!!
   pwmInit(MCOM_HI_A_CHAN,0);
   pwmStop( MCOM_HI_A_CHAN);
   pwmInit(MCOM_LO_A_CHAN,0);
   pwmStop( MCOM_LO_A_CHAN);

   pwmInit(MCOM_HI_B_CHAN,0);
   pwmStop( MCOM_HI_B_CHAN);
   pwmInit(MCOM_LO_B_CHAN,0);
   pwmStop( MCOM_LO_B_CHAN);

   for (i=0; i<MAX_MOTORS; i++)
   {
    //  bMotorConnected[i] = 1;
      motorReset(i);
      iLastMotorDirection[i] = NO_DIRECTION;
   }

   for (i=0; i<MAX_EJSSCT; i++)
   {
      iEJSSCT_Count[i] = 0;
   }


   iBridgeActive[0] = 0;
   iBridgeActive[1] = 0;

   return;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : motorResetChActive()
// Purpose      : To reset active bit for ch A & B
// Parameters   : None
// Return value : None
// Created      : 2007-11-13 by PS
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorResetChActive(void)
{
   iBridgeActive[0] = 0;
   iBridgeActive[1] = 0;

   return;
}




//////////////////////////////////////////////////////////////////////////////
// Function     : motorStartKoggMeasure()
// Purpose      : To allocate ADC and initiate pulse measure on selected motor
// Parameters   : MotorIndex, No of pulses to measure
// Return value : TRUE on success, otherwise FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorStartKoggMeasure( int iMotorNo, int iPulses, int iRegulateSpeed)
{
   int iBridgeIndex;
   int iAdcIndex;
   int iMotorIndex = iMotorNo-1;

   // Find out which Bridge this motor uses
   iBridgeIndex = motorGetBridgeIndex( iMotorNo);     // if iMotorNo < 9 ret = 0 else ret = 1

   // Find out which adc channel this motor uses (kogg measuring)
   iAdcIndex = motorGetAdcIndex( iMotorNo);           // 1,3,5,7 = 0   9,11,13,15 = 1   2,4,6,8 = 2   10,12,14,16 = 3

   // Check if this ADC is available:
   if (iBridgeActive[iBridgeIndex] == 0)
   {
      iBridgeActive[iBridgeIndex] = 1;                 // Reserve Bridge
      MotorData[iMotorIndex].iAdcIndex = iAdcIndex;    // Remember reserved ADC channel
      MotorData[iMotorIndex].iRequestedCnt = iPulses;
      MotorData[iMotorIndex].iPulseCount = 0;
      MotorData[iMotorIndex].iRegulateSpeed = iRegulateSpeed;
      return (1);
   }
   else
   {
     return (0);
   }
}


//////////////////////////////////////////////////////////////////////////////
// Function     : motorStopKoggMeasure()
// Purpose      : To terminate pulsemeasure on motor
// Parameters   : MotorNo, No of pulses measured
// Return value : TRUE on success, otherwise FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-11-16 by ESE
// Comments     :
//////////////////////////////////////////////////////////////////////////////
__interwork void motorStopKoggMeasure( int iMotorNo)
{
   int iEjssctNo;
   int iBridgeIndex;
   int iMotorIndex = iMotorNo-1;

   iEjssctNo = motorGetEjssctNo(iMotorNo);

   motorEjssctOff(iEjssctNo);

   iBridgeIndex = motorGetBridgeIndex( iMotorNo);

   iBridgeActive[iBridgeIndex] = 0;                // Release Bridge

   MotorData[iMotorIndex].iAdcIndex = -1;          // -1 indicates no ADC in use for this motor
              // Register motor not active

   return;    // Note : ALLWAYS use a simple return !!! Else stack problems occures
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Set_Move_Type
// Purpose      :
// Parameters   : None
// Return value : T
// Created      : 2006-04-05 by TAK
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////

void Set_Move_Type(int Mov_Type)
{
   if(Mov_Type == TIGHTEN_PUSHER)
     permitedDifferenceTighting = PERMITED_DIFFERENCE_TIGHT_TIGHTEN_PUSHER;
   else if(Mov_Type == TEST_PUSHER)
     permitedDifferenceTighting = PERMITED_DIFFERENCE_TIGHT_TEST_PUSHER;
}


__arm int motorOverLoadA( void)
{
   int iMotorNo = 1;
   // find motor number
   while((MotorData[iMotorNo-1].iMotorState != MOTOR_MOVING) && (iMotorNo <= 8))
   {
      iMotorNo++;
   }
   if (iMotorNo > 8)
   {
      timerStop(INX_BRIDGE_A_TIMER); // stop timer if its running
      return 0; // not active motor on the bridge A
   }
   
   return motorOverLoad(iMotorNo, &bOverloadTimerStateA, iAdcTight_A, &iAdcTightInd_A, iAdcLast_A, &nLastIndexA, INX_BRIDGE_A_TIMER, ADC_CH4);
}

__arm int motorOverLoadB( void)
{
   int iMotorNo = 9;
   
   // find motor number
   while((MotorData[iMotorNo-1].iMotorState != MOTOR_MOVING) && (iMotorNo <= 16))
   {
      iMotorNo++;
   }
   if (iMotorNo > 16)
   {
      timerStop(INX_BRIDGE_B_TIMER); // stop timer if its running
      return 0; // not active motor on the bridge B
   }
   
   return motorOverLoad(iMotorNo, &bOverloadTimerStateB, iAdcTight_B, &iAdcTightInd_B, iAdcLast_B, &nLastIndexB, INX_BRIDGE_B_TIMER, ADC_CH5);
}

__arm int motorOverLoad(int iMotorNo, int * bOverloadTimerState, unsigned int iAdcTight[], unsigned int * iAdcTightInd, unsigned int iAdcLast[], int * nLastIndex, unsigned char IND_BRIDGE_TIMER, unsigned char ADC_CH)
{
   int  iAdcValue;
   int  iMaxOverloadValue;
   long iTempValue=0;
   int  ni;

   if (*bOverloadTimerState == -1) // the firste time after motor start wait 100 ms
   {
     for(ni = 0; ni < COUNT; ni++)
            iAdcTight[ni]=0;

     *bOverloadTimerState = 0;
     *nLastIndex = 0;
     *iAdcTightInd = 0;
     timerSet(IND_BRIDGE_TIMER, TIME_20_MS);
     return 0;
   }
   
   iAdcValue = adcRead(ADC_CH);
   
   iAdcTight[*iAdcTightInd] = iAdcLast[*nLastIndex];
   
   iAdcLast[*nLastIndex] = iAdcValue;
   *nLastIndex  = *nLastIndex  + 1;
   
   if (*nLastIndex >= LAST_SAMPLE_COUNT)
   {
      *nLastIndex = 0;
   }
   
   *iAdcTightInd = *iAdcTightInd + 1;
   
   if (*iAdcTightInd >= COUNT)
   {
      *iAdcTightInd = 0;
   }
   
   if (timerExpired(IND_BRIDGE_TIMER)) // the second time after motor start wait 250 ms
   {
      if (*bOverloadTimerState == 0) 
      {
         *bOverloadTimerState = 1;
         timerSet(IND_BRIDGE_TIMER, TIME_250_MS);
      }
      else 
        *bOverloadTimerState = 2;
   }
   
   if (*bOverloadTimerState == 2) // after the firste 100 + 250ms
   {
        if(MotorData[iMotorNo-1].iCommand == TIGHTEN_PUSHER || MotorData[iMotorNo-1].iCommand == TEST_PUSHER || MotorData[iMotorNo-1].iDirection == REVERSE)
        {
          if (*iAdcTightInd < (COUNT - SUM_COUNT))
          {
            for (ni= *iAdcTightInd; ni < (*iAdcTightInd + SUM_COUNT); ni++)
              iTempValue = iAdcTight[ni] + iTempValue;
          }
          else
          {
            for (ni= *iAdcTightInd; ni < COUNT; ni++)
              iTempValue = iAdcTight[ni] + iTempValue;
            for (ni = 0; ni < (SUM_COUNT - COUNT + *iAdcTightInd) + 1; ni++)
              iTempValue = iAdcTight[ni] + iTempValue;
          }
          
          if (MotorData[iMotorNo-1].iDirection != REVERSE) // tighten or test
          {
            iMaxOverloadValue = iTempValue /  SUM_COUNT + permitedDifferenceTighting;
          }
          else
          {
              iMaxOverloadValue = iTempValue /  SUM_COUNT + PERMITED_DIFFERENCE_REVERSE;
          }
          
          if (iMaxOverloadValue > MAX_MOTOR_LOAD_MIDLE)
              iMaxOverloadValue = MAX_MOTOR_LOAD_MIDLE;
        } 
        else
        {
          iMaxOverloadValue= MAX_MOTOR_LOAD_HIGH;
        }
     
     iTempValue = 0;
     
     for(ni = 0; ni < LAST_SAMPLE_COUNT; ni++)
             iTempValue += iAdcLast[ni];

     if (iTempValue >= iMaxOverloadValue *LAST_SAMPLE_COUNT)
     {
          motorStop(iMotorNo, STOP_OVERLOAD);
          iTempValue = *iAdcTightInd;
          *bOverloadTimerState=-1;
          iCurrentOverload = 1;
          return 1;
     }
   }
   else
   {
       if(MotorData[iMotorNo-1].iDirection == REVERSE && (MotorData[iMotorNo-1].iTwinMotorNo > 0))
       {
          iMaxOverloadValue= MAX_MOTOR_LOAD_TWIN;
       }
       else
       {
          iMaxOverloadValue= MAX_MOTOR_LOAD_HIGH;
       }
       
       
       if (iAdcValue >= iMaxOverloadValue)
       {
            motorStop(iMotorNo, STOP_OVERLOAD);
            *bOverloadTimerState=-1;
            iCurrentOverload = 1;
            return 1;
       }
   }
   
   return(0);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorFinished()
// Purpose      : To detect if a motor has finnished to run
// Parameters   : MotorNo, &Direction, &pTotalCnt
// Return value : TRUE when finished, otherwise FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-12-14 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorFinished( int iMotorNo, int *pDirection, int *pTotalCnt)
{
   if (MotorData[iMotorNo-1].iMotorState == MOTOR_FINISHED)
   {

      *pDirection = MotorData[iMotorNo-1].iDirection;
      *pTotalCnt =  MotorData[iMotorNo-1].iPulseCount;
      if(*pTotalCnt > 0)
        *pTotalCnt--;

      motorStopKoggMeasure(iMotorNo);

      return(1);
   }
   else
   {
      return(0);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorNotInUse()
// Purpose      : To detect if a motor has finnished to run
// Parameters   : MotorNo, &Direction, &pTotalCnt
// Return value : TRUE when finished, otherwise FALSE
// Created      : 2005-11-09 by ES
// Updated      : 2005-11-09 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorNotInUse( int iMotorNo)
{
   if (MotorData[iMotorNo-1].iMotorState == MOTOR_NOT_IN_USE)
   {
      return(1);
   }
   else
   {
      return(0);
   }
}

int motorGetTwinMotorNo(int iMotorNo)
{
  return MotorData[iMotorNo - 1].iTwinMotorNo;
}

int motorIsMainMotor(int iMotorNo)
{
  return MotorData[iMotorNo - 1].iIsMainMotor;
}
//////////////////////////////////////////////////////////////////////////////
// Function     : motorSetConnected()
// Purpose      :
// Parameters   :
// Return value :
// Created      : 2005-11-20 by ES
// Updated      : 2005-11-20 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorSetConnected(int iMotorNo, int iConnected)
{
  bMotorConnected[iMotorNo - 1] = iConnected;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : motorIsConnected()
// Purpose      :
// Parameters   :
// Return value :
// Created      : 2005-11-20 by ES
// Updated      : 2005-11-20 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorIsConnected(int iMotorNo)
{
  if (bMotorConnected[iMotorNo - 1] == 1)
  {
    return 1;
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorGetDirection()
// Purpose      : To get direction of the motor
// Parameters   : MotorNo
// Return value : Mask with directionbit set
// Created      : 2005-11-19 by ES
// Updated      : 2005-11-19 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorGetDirection( int iMotorNo)
{
  return iLastMotorDirection[iMotorNo-1];
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorSetDirection()
// Purpose      : To set direction on selected motor
// Parameters   : MotorMask, Direction
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorSetDirection( int iMotorNo, int iDirection)
{
  unsigned int uiMotorMask;

  switch( iMotorNo)
  {
      case 1:
            uiMotorMask = MOT_1;
        break;
      case 2:
            uiMotorMask = MOT_2;
        break;
      case 3:
            uiMotorMask = MOT_3;
        break;
      case 4:
            uiMotorMask = MOT_4;
        break;
      case 5:
            uiMotorMask = MOT_5;
        break;
      case 6:
            uiMotorMask = MOT_6;
        break;
      case 7:
            uiMotorMask = MOT_7;
        break;
      case 8:
            uiMotorMask = MOT_8;
        break;
      case 9:
            uiMotorMask = MOT_9;
        break;
      case 10:
            uiMotorMask = MOT_10;
        break;
      case 11:
            uiMotorMask = MOT_11;
        break;
      case 12:
            uiMotorMask = MOT_12;
        break;
      case 13:
            uiMotorMask = MOT_13;
        break;
      case 14:
            uiMotorMask = MOT_14;
        break;
      case 15:
            uiMotorMask = MOT_15;
        break;
      case 16:
            uiMotorMask = MOT_16;
        break;
  default:
      break;
  }

  if (iDirection == FORWARD)
  {
      // Disable all motors for selected PWM at current Bridge !!
      if (iMotorNo < 9)
      {
         gpioSet(BRIDGE_A_MASK);
      }
      else
      {
         gpioSet(BRIDGE_B_MASK);
      }

      // Select on motor at current bridge for PWM only !!
      gpioClear(uiMotorMask);
  }
  else
  {
      // Disable all motors for selected PWM at current Bridge !!
      if (iMotorNo < 9)
      {
         gpioClear(BRIDGE_A_MASK);
      }
      else
      {
         gpioClear(BRIDGE_B_MASK);
      }

      // Select on motor at current bridge for PWM only !!
      gpioSet(uiMotorMask);
  }

  return;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : motorEjssctOn()
// Purpose      : To set on of EJSSCT outlet active
// Parameters   : EJSSCT outlet
// Return value : successed
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorEjssctOn(int iEJSSCT_ID)
{
  if (iActiveEJSSCT != 0)
  {
    if (iActiveEJSSCT != iEJSSCT_ID)
    {
      return 0;
    }
  }

  iActiveEJSSCT = iEJSSCT_ID;

  iEJSSCT_Count[iEJSSCT_ID - 1]++;

  if (iEJSSCT_Count[iEJSSCT_ID -1] > 1)
  {
    return 1; // this EJSSCT is
  }


  switch(iEJSSCT_ID)
  {
  case 1:
      gpioSet(EJSSCT1);
    break;
  case 2:
      gpioSet(EJSSCT2);
    break;
  case 3:
      gpioSet(EJSSCT3);
    break;
  case 4:
      gpioSet(EJSSCT4);
    break;
  default:
    break;
  }

  vTaskDelay( 3 );

  return 1;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : motorEjssctOff()
// Purpose      : To inactivate on of EJSSCT outlet
// Parameters   : EJSSCT outlet
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-11-21 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorEjssctOff(int iEJSSCT_ID)
{
  iEJSSCT_Count[iEJSSCT_ID-1]--;

  if (iEJSSCT_Count[iEJSSCT_ID-1] > 0)
  {
    return;
  }

  iEJSSCT_Count[iEJSSCT_ID-1] = 0;

  switch(iEJSSCT_ID)
  {
  case 1:
      gpioClear(EJSSCT1);
    break;
  case 2:
      gpioClear(EJSSCT2);
    break;
  case 3:
      gpioClear(EJSSCT3);
    break;
  case 4:
      gpioClear(EJSSCT4);
    break;
  default:
    break;

  }

  iActiveEJSSCT = 0;
  return;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : motorSaveCommand()
// Purpose      : To save PC command for the current session
// Parameters   : MotorNo, Command
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorSaveCommand(int iMotorNo, int iCommand)
{
   MotorData[iMotorNo-1].iCommand = iCommand;

   return;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : motorGetCommand()
// Purpose      : To get the PC command used in current session
// Parameters   : MotorNo, Command
// Return value : TRUE or FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void motorGetCommand(int iMotorNo, int *pCommand)
{
   *pCommand = MotorData[iMotorNo-1].iCommand;

   return;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorValid()
// Purpose      : To check if motor number is valid
// Parameters   : MotorNo
// Return value : TRUE or FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int  motorValid( int iMotorNo)
{
   if ((iMotorNo>=1) && (iMotorNo <= 16))
   {
      return( 1);
   }
   else
   {
      return(0);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorPWM()
// Purpose      : To PWM selected motor
// Parameters   : MotorNo, EJSSCT index, Direction, Speed
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-11-21 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void  motorPWM( int iMotorNo, int iEjssctNo, int iDirection, int iSpeed)
{

  if ((iMotorNo >= 1) && (iMotorNo <= 8))
      {
        // Switch OFF PWM MCOMLoA (PWM_CH0) !!!!!!!!!!!!!!!!!!
            pwmStop(PWM_CH0);
            // Switch OFF PWM MCOMHiA (PWM_CH1) !!!!!!!!!!!!!!!!!!
            pwmStop(PWM_CH1);
      }
  else if ((iMotorNo >= 9) && (iMotorNo <= 16))
  {
    // Switch OFF PWM MCOMLoB (PWM_CH2) !!!!!!!!!!!!!!!!!!
            pwmStop(PWM_CH2);
            // Switch OFF PWM MCOMHiB (PWM_CH3) !!!!!!!!!!!!!!!!!!
            pwmStop(PWM_CH3);
  }
  else
  {
      // invalid motor given, exit!!!
      return;
  }

  // Check if motor has been started:
  if (motorNotInUse(iMotorNo))
  {
      // Motor not started, set outlets, update MotorData[]

                                 // Count this motors use of EJSSCT outlet
// vTaskDelay(2000);
      motorSetDirection( iMotorNo, iDirection);                // Set direction, Note: Motor must stop to change direction!!!
      MotorData[iMotorNo-1].iDirection = iDirection;           // Remember direction!

      //MotorData[iMotorNo-1].iStarted = 1;                      // Register motor as started

      // Init and start PWM:
      if ((iMotorNo >= 1) && (iMotorNo <= 8))
      {
         // Bridge A selected
         if (iDirection == FORWARD)
         {

            // PWM MCOMHiA (PWM_CH1)
            pwmInit(PWM_CH1,iSpeed);
            pwmStart(PWM_CH1);
         }
         else
         {

            // PWM MCOMLoA (PWM_CH0)
            pwmInit(PWM_CH0,iSpeed);
            pwmStart(PWM_CH0);
         }
      }
      else if ((iMotorNo >= 9) && (iMotorNo <= 16))
      {
         // Bridge B selected
         if (iDirection == FORWARD)
         {


            // PWM MCOMHiB (PWM_CH3)
            pwmInit(PWM_CH3,iSpeed);
            pwmStart(PWM_CH3);
         }
         else
         {

            // PWM MCOMLoB (PWM_CH2)
            pwmInit(PWM_CH2,iSpeed);
            pwmStart(PWM_CH2);
         }
      }

      MotorData[iMotorNo-1].iMotorState = MOTOR_MOVING;                      // Register motor as started
  }
  else
  {
      // Motor is running and will be updated with new PWM duty cycle

      if ((iMotorNo >= 1) && (iMotorNo <= 8))
      {
         // Bridge A selected
         if (iDirection == FORWARD)
         {
            // Switch OFF PWM MCOMLoA (PWM_CH0) !!!!!!!!!!!!!!!!!!
            pwmStop(PWM_CH0);

            // PWM on MCOMHiA (PWM_CH1)
            pwmUpdate( PWM_CH1, iSpeed);
         }
         else
         {
            // Switch OFF PWM MCOMHiA (PWM_CH1) !!!!!!!!!!!!!!!!!!
            pwmStop(PWM_CH1);

            // PWM on MCOMLoA (PWM_CH0)
            pwmUpdate( PWM_CH0, iSpeed);
         }
      }
      else if ((iMotorNo >= 9) && (iMotorNo <= 16))
      {
         // Bridge B selected
         if (iDirection == FORWARD)
         {
            // Switch OFF PWM MCOMLoB (PWM_CH2) !!!!!!!!!!!!!!!!!!
            pwmStop(PWM_CH2);

            // PWM on MCOMHiB (PWM_CH3)
            pwmUpdate( PWM_CH3, iSpeed);
         }
         else
         {
            // Switch OFF PWM MCOMHiB (PWM_CH3) !!!!!!!!!!!!!!!!!!
            pwmStop(PWM_CH3);

            // PWM on MCOMLoB  (PWM_CH2)
            pwmUpdate( PWM_CH2, iSpeed);
         }
      }
  }

  return;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : motorStop()
// Purpose      : To switch off PWM for selected motor
// Parameters   : MotorNo
// Return value : TRUE/FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-11-28 by ESE
// Comments     :
//////////////////////////////////////////////////////////////////////////////
__interwork int motorStop( int iMotorNo, int iReason)
{
  int nResult;

  if (MotorData[iMotorNo-1].iMotorState == MOTOR_NOT_IN_USE)
  {
     return CANT_STOP_NOT_IN_USE;
  }
  else if (MotorData[iMotorNo-1].iMotorState == MOTOR_WAITING_ON_ROTATION_END)
  {
     return CANT_STOP_WAITING;
  }

  if (MotorData[iMotorNo-1].iTwinMotorNo > 0)
  {

  }

  nResult = motorStop2(  iMotorNo,  iReason);

  if (MotorData[iMotorNo-1].iTwinMotorNo > 0)
  {
    if (MotorData[iMotorNo-1].iCommand == TIGHTEN_PUSHER)
    {
      motorStop2(MotorData[iMotorNo-1].iTwinMotorNo,  iReason);
    }
    else if (iReason != STOP_FINISHED)
    {
      if (MotorData[iMotorNo-1].iDirection == REVERSE)
      {
        if (iReason == STOP_NO_PULSES_LOW_LEVEL || iReason == STOP_NO_PULSES_HIGH_LEVEL)
        {
          motorStop2(MotorData[iMotorNo-1].iTwinMotorNo,  iReason);
        }
      }
      else
      {
        motorStop2(MotorData[iMotorNo-1].iTwinMotorNo,  iReason);
      }
    }
  }

  return nResult;
}

int motorStop2( int iMotorNo, int iReason)
{
  if ((iMotorNo >= 1) && (iMotorNo <= 8))
  {
      MotorData[iMotorNo-1].iReasonOfStop = iReason;

     // Bridge A selected
     if ( MotorData[iMotorNo-1].iDirection == FORWARD)
     {
        // PWM on MCOMHiA (PWM_CH1)
        pwmStop(PWM_CH1);
     }
     else
     {
        // PWM on MCOMLoA (PWM_CH0)
        pwmStop(PWM_CH0);
     }

     gpioClear(BRIDGE_A_MASK);
     //motorStopMeasure(iMotorNo);
  }
  else if ((iMotorNo >= 9) && (iMotorNo <= 16))
  {
     MotorData[iMotorNo-1].iReasonOfStop = iReason;

     // Bridge B selected
     if (MotorData[iMotorNo-1].iDirection == FORWARD)
     {
        // PWM pn MCOMHiB (PWM_CH3)
        pwmStop(PWM_CH3);
     }
     else
     {
        // PWM on MCOMLoB (PWM_CH2)
        pwmStop(PWM_CH2);
     }

     gpioClear(BRIDGE_B_MASK);
     //motorStopMeasure(iMotorNo);
  }

  MotorData[iMotorNo-1].iMotorState = MOTOR_WAITING_ON_ROTATION_END;



  return MOTOR_STOPED;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : motorON()
// Purpose      : To switch selected motor ON
// Parameters   : MotorNo, Direction
// Return value : TRUE on success, otherwise FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-12-01 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorStart( int iMotorNo, int iDirection, int iSpeed, int iTwinMotorNo, int iIsMainMotor)
{
  int iEjssctNo;
  int iRetval = 1;

  if ((iMotorNo < 1) || (iMotorNo > 16))
  {
    return 0;
  }

  ResetButton(iMotorNo);


  switch( iMotorNo)
  {
  case 1:
  case 2:
  case 9:
  case 10:
      iEjssctNo = 1;
    break;

  case 3:
  case 4:
  case 11:
  case 12:
      iEjssctNo = 2;
    break;
  case 5:
  case 6:
  case 13:
  case 14:
      iEjssctNo = 3;
    break;

  case 7:
  case 8:
  case 15:
  case 16:
      iEjssctNo = 4;
    break;

  default:
      iRetval = 0;
    break;
  }

  if (iSpeed > 99)
  {
      iSpeed = 99;
  }
  else if(iSpeed < 1)
  {
      iSpeed = 1;
  }

  if (motorEjssctOn(iEjssctNo) != 1)                             // Set/Ensure EJSSCT set activate
  {
    return 0;
  }

  if ((iMotorNo >= 1) && (iMotorNo <= 8))
  {
    if (iSpeed < 40)
    {
      pwmUpdate(PWM_CH1,40);
      pwmUpdate(PWM_CH0,40);
    }
    else if (iSpeed < 60)
    {
      pwmUpdate(PWM_CH1,60);
      pwmUpdate(PWM_CH0,60);
    }
    else
    {
      pwmUpdate(PWM_CH1,iSpeed);
      pwmUpdate(PWM_CH0,iSpeed);
    }
  }
  else
  {
    if (iSpeed < 40)
    {
      pwmUpdate(PWM_CH3,40);
      pwmUpdate(PWM_CH2,40);
    }
    else if (iSpeed < 60)
    {
      pwmUpdate(PWM_CH3,60);
      pwmUpdate(PWM_CH2,60);
    }
    else
    {
      pwmUpdate(PWM_CH3,iSpeed);
      pwmUpdate(PWM_CH2,iSpeed);
    }
  }


  MotorData[iMotorNo -1].iSpeed = iSpeed;
  MotorData[iMotorNo -1].iWantedSpeed = iSpeed;
  if (iSpeed < 40)
  {
    MotorData[iMotorNo -1].iSpeed = 40;
    motorPWM( iMotorNo, iEjssctNo, iDirection, 40);
  }
  else
  {
    motorPWM( iMotorNo, iEjssctNo, iDirection, iSpeed);
  }

  iLastMotorDirection[iMotorNo-1] = iDirection;
  MotorData[iMotorNo -1].iTwinMotorNo = iTwinMotorNo;
  MotorData[iMotorNo -1].iIsMainMotor = iIsMainMotor;

  return(iRetval);
}



//////////////////////////////////////////////////////////////////////////////
// Function     : motorGetEjssctNo()
// Purpose      : To find which Ejssct outlet is used for a pusher
// Parameters   : MotorNo
// Return value : Ejssct outlet number 1..4, or 0
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int motorGetEjssctNo(int iMotorNo)
{
  int iEjssctNo;

  switch( iMotorNo)
  {
  case 1:
  case 2:
  case 9:
  case 10:
      iEjssctNo = 1;
    break;

  case 3:
  case 4:
  case 11:
  case 12:
      iEjssctNo = 2;
    break;
  case 5:
  case 6:
  case 13:
  case 14:
      iEjssctNo = 3;
    break;

  case 7:
  case 8:
  case 15:
  case 16:
      iEjssctNo = 4;
    break;

  default:
      iEjssctNo = 0;
    break;
  }


  return(iEjssctNo);
}

void motorOnTimerInterrupted_IRQ(void)
{
  int iMotorNo;

      for (iMotorNo=1; iMotorNo<8; iMotorNo+=2)    // Motors 1,3 5 and 7
      {
         koggCountPulses( iMotorNo, ADC_CH0);
      }


      // Count pulses on active motors connected with AD1
      for (iMotorNo=9; iMotorNo<16; iMotorNo+=2)    // Motors 9,11 13 and 15
      {
         koggCountPulses( iMotorNo, ADC_CH1);
      }

      // Count pulses on active motors connected with AD3
      for (iMotorNo=2; iMotorNo<9; iMotorNo+=2)    // Motors 2,4 6 and 8
      {
         koggCountPulses( iMotorNo, ADC_CH2);
      }

      // Count pulses on active motors connected with AD4
      for (iMotorNo=10; iMotorNo<17; iMotorNo+=2)    // Motor 10,12 14 and 16
      {
         koggCountPulses( iMotorNo, ADC_CH3);
      }


  motorOverLoadA();
  motorOverLoadB();
}
