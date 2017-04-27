//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.03
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// File         : Sensor.c
// Purpose      : To read POD Control sensors
// Created      : 2005-09-17 by
// Updated      : 2005-09-17 by
// Comments     :
//////////////////////////////////////////////////////////////////////////////



#include "Board.h"
#include "Sensors.h"
#include "Jupiter.h"
#include "ext_irq.h"
#include "GpIo.h"
#include "pwm.h"
#include "Adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Timer.h"
#include "Fallsensor.h"

SENSOR_FLAGS  SENSOR_FLAGbits;
int bTimerHasNotBeenSet = 1;


//////////////////////////////////////////////////////////////////////////////
// Function     : HatchSensor()
// Purpose      : 
// Parameters   :
// Return value : true or false
// Created      : 2016-11-22 by PS
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char HatchSensor(void)
{
   char answer;
      
   if(Sensor_In(HATCH_INPUT_BIT) == 1)
     answer=1;  //open
   else
     answer=0;  //closed
   return (answer);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : DoorSensor()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2005-06-17 by TAK
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char DoorSensor(void)
{
   char answer;
   if(Sensor_In(DOOROPN_BIT)==1)
     answer=1;  //open
   else
     answer=0;  //closed
   return (answer);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : FallSensor()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2005-11-22 by TAK
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char FallSensor(int EqNo)  //Active low
{
   char answer;
//DEBUG..PS   if(EqNo==1&&Sensor_In(FALL1_INPUT_BIT)==1)
//DEBUG..PS     answer=0;  //open
   /*else */if (EqNo==2&&Sensor_In(FALL2_INPUT_BIT)==1)
     answer=0;  //open
   else
     answer=1;  //closed
   return (answer);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Sensor_In()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2005-06-17 by TAK
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char Sensor_In(unsigned int sensor)
{
   if ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & sensor ) !=0 )
     return (1);
   else
     return (0);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Set_Lamp()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2005-11-22 by TAK
// Updated      : 2006-08-03 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////

char Set_Lamp(unsigned char value,int EqNo)
{
  char answer;
  
  if(value ==1)
  {
     timerStop(INX_LAMP_FALL_DELAY);
     Stop_Fall_3();
     vTaskDelay( 2 );
     if( EqNo==1)
       gpioClearBit(LAMP1_BIT);
     else
       gpioClearBit(LAMP2_BIT);
     timerSet(INX_LAMP_TIMER, TIME_1000_MS);
     vTaskDelay( 30 );
     answer=Get_Lamp(EqNo);
  }
  else
  {
    answer=Get_Lamp(EqNo);
    timerStop(INX_LAMP_TIMER);
    if( EqNo==1)
      gpioSetBit(LAMP1_BIT);
    else
      gpioSetBit(LAMP2_BIT);
  ///  if(answer!=2)
  //  {
      if (isFall3_Event_Enable() == 1)
      {
         timerSet(INX_LAMP_FALL_DELAY, TIME_500_MS);
      }
    //}
  }
  return (answer);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Get_Lamp()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2005-11-22 by TAK
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char Get_Lamp(int EqNo)
{
   char answer;
   int gAdcValue=0;
   int nHighCount = 0;
  
   
   for (int i = 0; i < 100; i++)
   {
      gAdcValue = adcRead(ADC_CH4);
      if (gAdcValue >0)
      {
        nHighCount ++;
      }
   }
   
   if(((Sensor_In(LAMP1_BIT)==0) && EqNo == 1) ||
      ((Sensor_In(LAMP2_BIT)==0) && EqNo == 2))
   {
     answer=1;  //open
   }
   else
   {
     answer=2;  //closed
   }
   //if(gAdcValue>20)
   if (nHighCount > 40)
     answer=answer+2;
   return (answer);
}


//////////////////////////////////////////////////////////////////////////////
// Function     : OnFALL1_Interrupted()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2005-10-27 by ES
// Updated      : 2005-11-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void OnFALL1_Interrupted_IRQ(void)
{
  if(SENSOR_FLAGbits.fall_1_on == 1)
    SENSOR_FLAGbits.fall1 = 1;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : OnTILT_Interrupted()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2006-02-22 by TAK
// Updated      : 2006-02-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void OnTILT_Interrupted_IRQ(void)
{
  if(SENSOR_FLAGbits.tilt_on != 1) // if disabled
  {
    return;
  }
  
  if (bTimerHasNotBeenSet == 1 || timerExpired(INX_TILT_TIMER))
  {
      SENSOR_FLAGbits.tilt = (Sensor_In(TILTS_BIT))+1;
      timerSet(INX_TILT_TIMER, TIME_5000_MS);
      bTimerHasNotBeenSet = 0;
  }
}

//////////////////////////////////////////////////////////////////////////////
// Function     : OnFALL2_Interrupted()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2005-10-27 by ES
// Updated      : 2005-11-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void OnFALL2_Interrupted_IRQ(void)
{
  if(SENSOR_FLAGbits.fall_2_on == 1)
    SENSOR_FLAGbits.fall2 = 1;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Start_Tilt()
// Purpose      :
// Parameters   :
// Return value :
// Created      : 2006-02-22 by TAK
// Updated      : 2006-02-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Start_Tilt(void)
{
   SENSOR_FLAGbits.tilt_on = 1;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Stop_Tilt()
// Purpose      :
// Parameters   :
// Return value :
// Created      : 2006-02-22 by TAK
// Updated      : 2006-02-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Stop_Tilt(void)
{
   SENSOR_FLAGbits.tilt_on = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Start_Fall_1()
// Purpose      :
// Parameters   :
// Return value :
// Created      : 2006-02-22 by TAK
// Updated      : 2006-02-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Start_Fall_1(void)
{
   SENSOR_FLAGbits.fall_1_on = 1;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Stop_Fall_1()
// Purpose      :
// Parameters   :
// Return value :
// Created      : 2006-02-22 by TAK
// Updated      : 2006-02-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Stop_Fall_1(void)
{
   SENSOR_FLAGbits.fall_1_on = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Start_Fall_2()
// Purpose      :
// Parameters   :
// Return value :
// Created      : 2006-02-22 by TAK
// Updated      : 2006-02-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Start_Fall_2(void)
{
   SENSOR_FLAGbits.fall_2_on = 1;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : Stop_Fall_2()
// Purpose      :
// Parameters   :
// Return value :
// Created      : 2006-02-22 by TAK
// Updated      : 2006-02-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Stop_Fall_2(void)
{
   SENSOR_FLAGbits.fall_2_on = 0;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : isTILT_Interrupted()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2006-02-22 by TAK
// Updated      : 2006-02-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char isTILT_Interrupted(void)
{
   char chRet = SENSOR_FLAGbits.tilt;
   SENSOR_FLAGbits.tilt = 0;

   return chRet;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : isPRODREL_Interrupted()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2005-10-27 by ES
// Updated      : 2005-11-21 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char isFALL1_Interrupted(void)
{
   char chRet = SENSOR_FLAGbits.fall1;
   SENSOR_FLAGbits.fall1 = 0;

   return chRet;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : isSHUTEMP_Interrupted()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2005-10-27 by ES
// Updated      : 2005-11-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char isFALL2_Interrupted(void)
{
  char chRet = SENSOR_FLAGbits.fall2;
  SENSOR_FLAGbits.fall2 = 0;

  return chRet;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Trace_Toggel_LED()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2006-02-13 by TAK
// Updated      : 2006-02-13 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Trace_Toggel_LED (unsigned int Led)
{
    if ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & Led ) == Led )
    {
        AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, Led );
    }
    else
    {
        AT91F_PIO_SetOutput( AT91C_BASE_PIOA, Led );
    }
}

//////////////////////////////////////////////////////////////////////////////
// Function     : TiltSensor()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2006-02-23 by TAK
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char TiltSensor(void)
{
   char answer;
   if(Sensor_In(TILTS_BIT)==1)
     answer=1;  //open
   else
     answer=0;  //closed
   return (answer);
}
