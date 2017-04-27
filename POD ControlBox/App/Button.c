
#include <board.h>
#include "FreeRTOS.h"
#include "adc.h"
#include "Motor.h"
#include "Button.h"
#include "task.h"
#include "Kogg.h"

//*****************************************************************************
//              Structure to hold
// *****************************************************************************


typedef enum
{
	BUTTON_NOT_IN_USE,
	BUTTON_PUSHED,
	BUTTON_RELEASED
} eButtState;

typedef struct xButton {
    int  iActivated;
    int  iState;
} BUTTON_DATA;
extern int iEJSSCT_Count[MAX_EJSSCT];

BUTTON_DATA ButtonData[MAX_MOTORS];


//////////////////////////////////////////////////////////////////////////////
// Function     : ResetButtons()
// Purpose      :
// Parameters   :
// Return value :
// Created      : 2005-11-20 by ES
// Updated      : 2005-11-20 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ResetButtons(void)
{
  int i;

  for(i = 0; i < MAX_MOTORS; i++)
  {
    ButtonData[i].iActivated = BUTTON_NOT_ACTIVATED;
    ButtonData[i].iState = BUTTON_NOT_IN_USE;
  }
}

//////////////////////////////////////////////////////////////////////////////
// Function     : ResetButton(int iMotorNo)
// Purpose      :
// Parameters   : iMotorNo
// Return value :
// Created      : 2005-11-20 by ES
// Updated      : 2005-11-20 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ResetButton(int iMotorNo)
{
  ButtonData[iMotorNo - 1].iActivated = BUTTON_NOT_ACTIVATED;
  ButtonData[iMotorNo - 1].iState = BUTTON_NOT_IN_USE;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : ActivateButton(int iMotorNo)
// Purpose      :
// Parameters   : iMotorNo
// Return value :
// Created      : 2005-11-20 by ES
// Updated      : 2005-11-20 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ActivateButton(int iMotorNo)
{
  ResetButton(iMotorNo);
  ButtonData[iMotorNo - 1].iActivated = BUTTON_ACTIVATED;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : ActivateButton(int iMotorNo)
// Purpose      :
// Parameters   : iMotorNo
// Return value :
// Created      : 2005-12-01 by TAK
// Updated      : 2005-12-01 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
unsigned char GET_Button(int iMotorNo)
{
  if(ButtonData[iMotorNo - 1].iActivated == BUTTON_ACTIVATED)
     return (1);
  else
     return (0);
}


//////////////////////////////////////////////////////////////////////////////
// Function     : buttonPusherSwitch()
// Purpose      :
// Parameters   : iMotorNo
// Return value :
// Created      : 2005-11-09 by ES
// Updated      : 2005-11-14 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int buttonPusherSwitchOnceMeasuring(int iMotorNo)
{
   unsigned long int uiAdcSample1;
   unsigned long int uiAdcSample2;
   unsigned long int uiAdcSample3;
   
   int iAdcIndex;
   int iEjssctNo;
   int iRetval = 0;

   // Find out which adc channel this motor uses
   iAdcIndex = motorGetAdcIndex( iMotorNo);


      iEjssctNo = motorGetEjssctNo(iMotorNo);           // Find which EJSSCT
      if (motorEjssctOn(iEjssctNo) != 1) // Set/Ensure EJSSCT is set activate
      {
        return BUTTON_NOT_PUSHED; // EJSCC is busy (her skulle vi sende en annen return verdi!!!)
      }

      vTaskDelay(3);                                  // Wait for Op Amp throughput

      // Read ADC:
      uiAdcSample1 = adcRead(iAdcIndex);
      vTaskDelay(3); 
      uiAdcSample2 = adcRead(iAdcIndex);
      vTaskDelay(3); 
      uiAdcSample3 = adcRead(iAdcIndex);

      if ((uiAdcSample1 + uiAdcSample2 + uiAdcSample3) > (BUTTON_NOT_CONNECTED_LEVEL * 3))
      {
         iRetval = BUTTON_NOT_CONNECTED;
      }
      else if ((uiAdcSample1 + uiAdcSample2 + uiAdcSample3) > (KOGG_PULSE_LEVEL * 3))
      {
         iRetval = BUTTON_PUSHED_DETECTED;
      }
      else
      {
        iRetval = BUTTON_NOT_PUSHED;
      }


      motorEjssctOff(iEjssctNo);


   return (iRetval);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : buttonPusherSwitch()
// Purpose      : To detect Pusher Switch while pusher is in READ position
// Parameters   : ADC channel to detect pulse on
// Return value : 0==NO PULSE, 1==USER_SWITCH
// Created      : 2005-09-17 by PVA
// Updated      : 2005-12-01 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int buttonPusherSwitch(int iMotorNo)
{
  int nResult;

   nResult = buttonPusherSwitchOnceMeasuring(iMotorNo);

   if (nResult == BUTTON_NOT_CONNECTED)
   {
        // motor is disconnected!
        ResetButton(iMotorNo);
        motorSetConnected(iMotorNo, MOTOR_NOT_CONNECTED);
        return BUTTON_NOT_CONNECTED;
   }
   else
   {
      motorSetConnected(iMotorNo, MOTOR_CONNECTED);
      

      if (ButtonData[iMotorNo - 1].iActivated == BUTTON_ACTIVATED)
      {
        // Don't allowed detection  before the new motor start.

        if (nResult == BUTTON_PUSHED_DETECTED)
        {
          if (ButtonData[iMotorNo - 1].iState != BUTTON_PUSHED)
          {
              // Userswitch detected !
              ButtonData[iMotorNo - 1].iState = BUTTON_PUSHED;
              return BUTTON_PUSHED_DETECTED;
          }
        }
        else if (ButtonData[iMotorNo - 1].iState == BUTTON_PUSHED)
        {
            ButtonData[iMotorNo - 1].iState = BUTTON_RELEASED;
            return BUTTON_RELEASED_DETECTED;
        }
     }
   }
   return BUTTON_NOT_PUSHED;
}

