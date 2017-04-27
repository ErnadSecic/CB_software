#include "Motor.h"
#include "SpeedRegulator.h"
#include "Kogg.h"


int nPulsePeriodOldA = -1;
int nPulsePeriodOldB = -1;
  
//int nDivagationA = 0;
//int nDivagationB = 0;

/*
int minT[200];
int cntr2 = 0;
*/
int cntrA = -1;
int cntrB = -1;


extern MOTOR_DATA MotorData[MAX_MOTORS];


void speedRegulatorSpeedControl(int iMotorIndex, int nPulsePeriod)
{
  int nPulsePeriodDifference;
  int nSpeedDifference;
  int nPositionDiff;
  int nDiv;

       nPositionDiff = MotorData[iMotorIndex].iRequestedCnt - MotorData[iMotorIndex].iPulseCount;

       if(nPulsePeriod > 0)
       {

         // cntr = 0;
          
           // if it's not the firste pulse
           if (MotorData[iMotorIndex].iPulseCount > 1)
           {
             if (iMotorIndex < 8)
             {
               cntrA = -1;
             // nPulsePeriodDifference = (nPulsePeriod - nPulsePeriodOldA);
              nPulsePeriodDifference =(nPulsePeriodOldA - (((FASTER_SPEED_LENGTH_OFF_HIGH + FASTER_SPEED_LENGTH_OFF_LOW) * 100 ) / MotorData[iMotorIndex].iWantedSpeed) );
             }
             else
             {
               cntrB = -1;
               nPulsePeriodDifference =(nPulsePeriodOldB - (((FASTER_SPEED_LENGTH_OFF_HIGH + FASTER_SPEED_LENGTH_OFF_LOW) * 100 ) / MotorData[iMotorIndex].iWantedSpeed) );
            //  nPulsePeriodDifference = (nPulsePeriod - nPulsePeriodOldB);
             }
           }
           else
           {
              nPulsePeriodDifference = 0;
           }

           if (iMotorIndex < 8)
           {
              nPulsePeriodOldA = nPulsePeriod;
            //  nDiv = nDivagationA;
           }
           else
           {
              nPulsePeriodOldB = nPulsePeriod;
             // nDiv = nDivagationB;
           }
           
           if (MotorData[iMotorIndex].iPulseCount == 1)
           {
             nPulsePeriodDifference = 0;
            // return;
           }
           

           nDiv =  (nPulsePeriod - (((FASTER_SPEED_LENGTH_OFF_HIGH + FASTER_SPEED_LENGTH_OFF_LOW) * 100 ) / MotorData[iMotorIndex].iWantedSpeed) );
      
          // nSpeedDifference = (nDiv  * 2 + (nPulsePeriodDifference *7))/ 50;
           
           nSpeedDifference = ((nDiv- nPulsePeriodDifference /4) )/ 5;
           
          if (nSpeedDifference > 5)
           {
              nSpeedDifference = 5;
           }
           else if(nSpeedDifference < -5)
           {
              nSpeedDifference = -5;
           }

          /* if (nPositionDiff < 2)
           {
             motorSetSpeed(iMotorIndex, 2);
           }
           else */if(nPositionDiff <5)
           {
             if (MotorData[iMotorIndex].iSpeed > 50)
             {
              motorSetSpeed(iMotorIndex, 5);
             }
             else
             {
               motorSetSpeed(iMotorIndex, 20);
             }
           }
           else
           {
             motorChangeSpeed(iMotorIndex, nSpeedDifference);
           }

        }
}

void speedRegulatorAvoidMotorStop(int iMotorIndex, int iPulseResult)
{
  int cntr;
  int nCritic;
  
  if (iMotorIndex < 8)
  {
    cntrA ++;
    cntr = cntrA;
  }
  else
  {
    cntrB ++;
    cntr = cntrB;
  }
  
  if (iPulseResult == KOGG_HIGH_TO_LONG)
  {
    nCritic = (7*150) / MotorData[iMotorIndex].iWantedSpeed;//iSpeed;
  }
  else
  {
    nCritic = (28*150) / MotorData[iMotorIndex].iWantedSpeed;//iSpeed;
  }

if ((cntr % nCritic) == 0)  //100 501  
    {
      if (MotorData[iMotorIndex].iSpeed < 40)
      {
         motorSetSpeed(iMotorIndex, 43);
      }
      else if (MotorData[iMotorIndex].iSpeed < 43)
      {
         motorSetSpeed(iMotorIndex, 46);
      }
      else if (MotorData[iMotorIndex].iSpeed < 46)
      {
        motorSetSpeed(iMotorIndex, 50);
      }
      else if (MotorData[iMotorIndex].iSpeed < 50)
      {
        motorSetSpeed(iMotorIndex, 53);
      }
      else if (MotorData[iMotorIndex].iSpeed < 53)
      {
        motorSetSpeed(iMotorIndex, 56);
      }
      else if (MotorData[iMotorIndex].iSpeed < 56)
      {
        motorSetSpeed(iMotorIndex, 60);
      }
      else if (MotorData[iMotorIndex].iSpeed < 60)
      {
        motorSetSpeed(iMotorIndex, 64);
      }
      else if (MotorData[iMotorIndex].iSpeed < 64)
      {
        motorSetSpeed(iMotorIndex, 68);
      }
      else if (MotorData[iMotorIndex].iSpeed < 68)
      {
        motorSetSpeed(iMotorIndex, 72);
      }
      else if (MotorData[iMotorIndex].iSpeed < 72)
      {
        motorSetSpeed(iMotorIndex, 76);
      }
      else if (MotorData[iMotorIndex].iSpeed < 76)
      {
        motorSetSpeed(iMotorIndex, 80);
      }
      else if (MotorData[iMotorIndex].iSpeed < 80)
      {
        motorSetSpeed(iMotorIndex, 84);
      }
      
      else if (MotorData[iMotorIndex].iSpeed < 84)
      {
         motorSetSpeed(iMotorIndex, 88);
      }
      else if (MotorData[iMotorIndex].iSpeed < 88)
      {
        motorSetSpeed(iMotorIndex, 90);
      }
      else if (MotorData[iMotorIndex].iSpeed < 90)
      {
        motorSetSpeed(iMotorIndex, 92);
      }
      
    }
  
  else if (((cntr+ 1) % (nCritic*16)) == 0)  //100 501
    { 
        motorSetSpeed(iMotorIndex, 100);
    }
      /*
      else if (MotorData[iMotorIndex].iSpeed < 55)
      {
        motorSetSpeed(iMotorIndex, 60);
      }
      else if (MotorData[iMotorIndex].iSpeed < 60)
      {
        motorSetSpeed(iMotorIndex, 65);
      }
      else  if (MotorData[iMotorIndex].iSpeed < 65)
      {
        motorSetSpeed(iMotorIndex, 70);
      }
      else if (MotorData[iMotorIndex].iSpeed < 70)
      {
        motorSetSpeed(iMotorIndex, 75);
      }
      else  if (MotorData[iMotorIndex].iSpeed < 75)
      {
        motorSetSpeed(iMotorIndex, 80);
      }
      else if (MotorData[iMotorIndex].iSpeed < 80)
      {
        motorSetSpeed(iMotorIndex, 85);
      }
      else  if (MotorData[iMotorIndex].iSpeed < 85)
      {
        motorSetSpeed(iMotorIndex, 90);
      }
      else if (MotorData[iMotorIndex].iSpeed < 90)
      {
        motorSetSpeed(iMotorIndex, 95);
      }*/
    
}

void speedRegulatorResetCounterA(void)
{
  cntrA = -1;
  nPulsePeriodOldA = -1;
 // nDivagationA = 0;
}

void speedRegulatorResetCounterB(void)
{
  cntrB = -1;
  nPulsePeriodOldB = -1;
 // nDivagationB = 0;
}
