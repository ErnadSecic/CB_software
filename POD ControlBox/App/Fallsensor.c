//#include <stdlib.h>
#include "string.h"

#include "Fallsensor.h"
#include "GpIO.h"
#include "board.h"
#include "Timer.h"
#include "Adc.h"
#include "FreeRTOS.h"
#include "task.h"


static char m_bFallTimeOutOver   = 0;
static char bFallON              = 0;
static char IR_LED_NO            = 0;
static int  bFallSensorReady     = 0;
static int  bClearFlags          = 0;
static int  m_nCycleCounter      = 0;
static char bFallDisable         = 0;

long nTempSum[16];
long nSum[16];
long nTotaltSum = 0;    //Ny, legger sammen verdien av siste 400 måling på alle dioder og bruker som referanse
long nLastSum = 0;      //Ny, legger sammen verdien av siste måling på alle dioder


int nLastMeas[16];


static int fallSensorHighFlags    = 0;
static int fallSensorBlockedFlags = 0; // skal ikke brukes ???
//static int fallSensorSumHighFlag    = 0;

static char m_nOneOfFiveCycleCounter       = 0;
static char bEventOccuredInLast_5_cycle    = 0;

//#define PERMITED_LOW_LIGHT_LEVEL  320000 // = 20%   lys  (1023*400 cycles = 409200;  320000/409200 = 0.80; => 100% -80% = 20%)
#define PERMITED_LOW_LIGHT_LEVEL  368280 // = 20%   lys  (1023*400 cycles = 409200;  320000/409200 = 0.80; => 100% -80% = 20%)

#define PERMITED_HIGH_LIGHT_LEVEL 300
//////////////////////////////////////////////////////////////////////////////
// Function     : Switch_IR_LED_IRQ()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2006-02-14 by TAK
// Updated      : 2006-02-14 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Switch_IR_LED_IRQ (void)
{
   gpioClearBit (FALL_OUT_MASK);

   IR_LED_NO++;
   if(IR_LED_NO==16)
     IR_LED_NO=0;
   switch(IR_LED_NO)
   {
      case 0  : gpioSetBit(FALL_OUT_O0_BIT);
                break;
      case 1  : gpioSetBit(FALL_OUT_O1_BIT);
                break;
      case 2  : gpioSetBit(FALL_OUT_O2_BIT);
                break;
      case 3  : gpioSetBit(FALL_OUT_O3_BIT);
                break;
      case 4  : gpioSetBit(FALL_OUT_O4_BIT);
                break;
      case 5  : gpioSetBit(FALL_OUT_O5_BIT);
                break;
      case 6  : gpioSetBit(FALL_OUT_O6_BIT);
                break;
      case 7  : gpioSetBit(FALL_OUT_O7_BIT);
                break;
      case 8  : gpioSetBit(FALL_OUT_O8_BIT);
                break;
      case 9  : gpioSetBit(FALL_OUT_O9_BIT);
                break;
      case 10 : gpioSetBit(FALL_OUT_10_BIT);
                break;
      case 11 : gpioSetBit(FALL_OUT_11_BIT);
                break;
      case 12 : gpioSetBit(FALL_OUT_12_BIT);
                break;
      case 13 : gpioSetBit(FALL_OUT_13_BIT);
                break;
      case 14 : gpioSetBit(FALL_OUT_14_BIT);
                break;
      case 15 : gpioSetBit(FALL_OUT_15_BIT);
                break;
      default : break;
   }
}



//////////////////////////////////////////////////////////////////////////////
// Function     : CAP_IR_IRQ()
// Purpose      : Used for detection of the fall.
// Parameters   :
// Return value : None
// Created      : 2006-05-14 by ESE
// Updated      : 2006-05-14 by ESE
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void CAP_IR_IRQ(void)
{
   int adc_cp=0;

   adc_cp=adcRead(ADC_CH1);
   int i = 0;

   if (m_bFallTimeOutOver == 1)
   {
      m_bFallTimeOutOver = 0;
      for (i = 0; i < 16; i++)
      {
        nTempSum[i] = 0;
      }
      m_nCycleCounter = 0;
      m_nOneOfFiveCycleCounter = 0;
      
      nLastSum = 0;
   }
   else
   {
      if (bFallSensorReady)
      {
        if (IR_LED_NO == 0)
        {
          m_nCycleCounter ++;
        }
      
        nLastMeas[IR_LED_NO] = adc_cp;
      
        nLastSum += adc_cp;
        
             
        // if "adc_cp" is over permited value
        //Sjekk om det er fall på en diode
        if (adc_cp > (nSum[IR_LED_NO] / NO_OF_CYCLE_FOR_CALCUTATE_AVERAGE + FALL_TRIGG_LIMIT_HIGH))
        {
          fallSensorHighFlags |= (1 << IR_LED_NO);
          bEventOccuredInLast_5_cycle = 1;
        }
        
        //Sjekk om fall på flere dioder, endring av sum
        if (IR_LED_NO == 15) //Ny, trigg om endring av sum
        {
          if (nLastSum > (nTotaltSum / NO_OF_CYCLE_FOR_CALCUTATE_AVERAGE + FALL_TRIGG_LIMIT_HIGH * 3))
          {
            //fallSensorSumHighFlag = 1;
            fallSensorHighFlags = 0xFFFF;
            bEventOccuredInLast_5_cycle = 1;
          }
        }
       
        // Vi skal ikke slette info om fall før IoCtrl task har oppdaget den. Etter at IoCtr task sender USB- melding om fall,
        // settes "bClearFlags" slik at disse flags kan slettes hvis lys-verdier har kommet tilbake til gamle verdier.
        if (bClearFlags == 1)  // !!! don't clear flags befor IoCtrl task has find out about fall and permited clearing.
        {
          if (adc_cp < (nSum[IR_LED_NO] / NO_OF_CYCLE_FOR_CALCUTATE_AVERAGE + FALL_TRIGG_LIMIT_LOW)  && adc_cp > (nSum[IR_LED_NO] / NO_OF_CYCLE_FOR_CALCUTATE_AVERAGE - FALL_TRIGG_LIMIT_LOW))
          {
            fallSensorHighFlags &= ~(1 << IR_LED_NO);  // !!!  "fallSensorHighFlags" = 0  => alle diode is in the normal condition
          }
          
         
        }
        
        if (IR_LED_NO == 15) //Ny
        {
          nLastSum = 0;   //Reset nLastSum
        }
        
        /////////////////////////////////////////////////////
        if (m_nCycleCounter >= NO_OF_CYCLE_FOR_CALCUTATE_AVERAGE && IR_LED_NO == 15)
        {
            m_nOneOfFiveCycleCounter ++;
            m_nCycleCounter = 0; // start calculate of new average
        }

        ////////////////////////////////////////////////////////
        // For calculating of average use cycle No. 3 but ONLY if
        // event not occurred in this 5 cycles.
        ////////////////////////////////////////////////////////
        if (m_nOneOfFiveCycleCounter == 3)
        {
          nTempSum[IR_LED_NO]  += adc_cp;
        }
        else if (m_nOneOfFiveCycleCounter >= 5)
        {
          if (bEventOccuredInLast_5_cycle == 0)  // OK, not event in the last 5 cycel. Use cycle No 3 for calculating of the new average
          {
            nTotaltSum = 0;
            
            for (i = 0; i < 16; i++)
            {
                nSum[i]= nTempSum[i];
                nTotaltSum += nSum[i];
            }
          }

          for (i = 0; i < 16; i++)
          {
              nTempSum[i] = 0;
          }

          m_nOneOfFiveCycleCounter  = 0;
          bEventOccuredInLast_5_cycle = 0;
        }
        /////////////////////////////////////////////////////////
      }
      else
      {
        nTempSum[IR_LED_NO]  += adc_cp; // for caclulating of start-average

        if (IR_LED_NO == 0)
        {
          m_nCycleCounter ++;
        }

        if (m_nCycleCounter >= NO_OF_CYCLE_FOR_CALCUTATE_AVERAGE && IR_LED_NO == 15)
        {
            m_nOneOfFiveCycleCounter ++;
            
            nTotaltSum = 0;
              
            for (i = 0; i < 16; i++)
            {
              nSum[i]= nTempSum[i];
              nTempSum[i] = 0;
              nTotaltSum += nSum[i];
            }

            bFallSensorReady = 1;
        }
      }
   }

}

// Ernad
void ClearSensor3Flags(char bClear)
{
  bClearFlags = bClear;
}

void GetStatusMessage(char* chResult)
{
  int nTemp;
  int nLedNo;
  nLedNo = IR_LED_NO - 1;
  if (nLedNo < 0)
  {
    nLedNo = 0;
  }
  
  for (int i = 0; i < 16; i++)
  {
    nTemp = (nLastMeas[i] * 100)/ (1024);
    if (nTemp == 100)
    {
      nTemp = 99; // fra 0 til 99 fordi bare 2 bytes skal brukes.
    }
    
    nTemp = 99 -nTemp; // invertere
    chResult[i *2] = (nTemp / 10) + 0x30;
    chResult[i *2 + 1] = (nTemp - (nTemp / 10)*10) + 0x30;
  }
  
  chResult[32] = 0x30;
  chResult[33] = 0x30;
}

// Ernad
int GetSensor3blocked(void)
{
   return fallSensorBlockedFlags;
}


int isFall3_Interrupted(void)
{
 /* if (fallSensorSumHighFlag == 1)
  {
    return 1;
  }*/
  return fallSensorHighFlags;
}

void Start_Fall_3(void)
{
  int i;

  IR_LED_NO               = 0;
  bFallSensorReady        = 0;
  bClearFlags             = 0;
  m_nCycleCounter         = 0;
  fallSensorHighFlags     = 0;
  //fallSensorSumHighFlag   = 0;
  fallSensorBlockedFlags  = 0;
  bFallON                 = 1;
  m_nOneOfFiveCycleCounter       = 0;
  bEventOccuredInLast_5_cycle      = 0;

  for (i = 0; i < 16; i++)
  {
    nTempSum[i] = 0;
    nSum[i]     = 0;
  }
  
  nLastSum = 0;
  nTotaltSum = 0;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : Fall3_Continue()
// Purpose      : Used before switching on the lamp or when dissabling fall-sensor from POD-app
// Parameters   :
// Return value : None
// Created      : 2006-05-14 by ESE
// Updated      : 2006-10-20 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Stop_Fall_3(void)
{
   bFallON = 0;
   gpioClearBit (FALL_OUT_MASK);  // switch off all IR led
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Fall3_Continue()
// Purpose      : To continue meassuring after lamp is switched off
// Parameters   :
// Return value : None
// Created      : 2006-05-14 by ESE
// Updated      : 2006-05-14 by ESE
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Fall3_Continue(void)
{
  fallSensorHighFlags = 0x00;
  //fallSensorSumHighFlag = 0;
  m_bFallTimeOutOver = 1;
  IR_LED_NO=15;
 // gpioSetBit(FALL_OUT_15_BIT);

  bFallON = 1;
}

char isFall3_Enabled(void)
{
   return bFallON;
}



//////////////////////////////////////////////////////////////////////////////
// Function     : Fall_3_Event_Disable()
// Purpose      : Get Value string from recived RS485
// Parameters   :
// Return value :
// Created      : 2006-04-07 by TAK
// Updated      : 2006-04-11 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Fall_3_Event_Enable_Disable(char value)
{
     bFallDisable = value;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : isFall3_Enable()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2006-04-11 by TAK
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char isFall3_Event_Enable(void)
{
   char chRet = bFallDisable;

   return chRet;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : Status_FallSensor_3()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2006-02-15 by TAK
// Updated      : 2006-04-11 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char Status_FallSensor_3(void)  //Active low
{
   char answer;
   char i;
   if (bFallON == 0)
     return 4; //EQ_STATUS_FALL_OFF
   else if(bFallSensorReady == 0)
     return 3; //EQ_STATUS_FALL_NOT_FINISHED_INIT
   else if (fallSensorHighFlags == 0x00) //hvis ikke fall
   {
     answer = 0; //LOW
     for (i=0;i<16;i++)
     {
        if(nSum[i] < PERMITED_HIGH_LIGHT_LEVEL || nSum[i]> PERMITED_LOW_LIGHT_LEVEL)
        {
          return 2;
        }
     }
   }
   else
   {
     answer =1;  //High
     for (i=0;i<16;i++)
     {
        if(nSum[i] < PERMITED_HIGH_LIGHT_LEVEL || nSum[i] > PERMITED_LOW_LIGHT_LEVEL)
        {
          return 2;
        }
     }
   }
   
   /*if (fallSensorSumHighFlag == 1)
   {
     return 2;
   }*/
   return (answer);
}

