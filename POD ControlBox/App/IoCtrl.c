

/* Scheduler include files. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "Kogg.h"
#include "Button.h"
#include "RS485.h"
#include "Fallsensor.h"



/* VERSION HISTORY
3.15 -> 3.16:
Functionality has not been changed.
bootloader files inkluds board.h (to simplify chages from ATSAM7S64 to  ATSAM7S128)
3.16-> 3.17
I2C init if error occurred.
3.17->3.18
Endring pga bootloader problemer. (Endret file: cdc_enumerate.c)

Don't use 3.19 (used only for booting test)

3.18->3.20
Tilt sensor fix (avoid multiple event)

3.20->3.21
Reset delay  after booting.


3.21->3.22
Changed 485. Older version can get problem with activation of product.

3.23->3.24
This version is switching off lampe as early as posible to avoid noice problems on I2C bus.
Older versions can get start-up problems on I2C (temperature or lamps on Boxio).
!!! 3.23 and newer version must use fallsensor with LM75 IC. If older fallsensor with DS- IC
is used with this version, temperature reading error will occurre (low temperature: -102 [C])

3.24->3.25
Introduced the new fallsensor calibration message.

3.24->3.25
I2C improvement. Older version read wrong temperature some times and disconnected lamps on Boxio will not be
detected each time.

3.25->3.26
1. Removed false fall after delivering. If user try to activate 2 product in the short time, second product will not be
delevered caused by false fall after previous delivering.
2.LED lamp detection improvement.
3.bootloader: longer delay after booting. !!! DO NOT USE THIS VERSION. BOOTING ERROR

3.26->3.27
Introduced Watchdog
Booting improvement (Flash Microsecond Cycle Number changed)

3.27->3.28
Watchdog: flag for debuging
Fallsensor: changed stop-start logic

3.28->3.29
Kogg

3.29->3.30
Fall sensor, event if sum changed

3.30->3.31
Fall sensor, false evnets after enabling of fallsensor.

3.31->3.32
Fallsensor: if lamp is not connected, stop of fallsensor can occur

3.32->3.33
Fallsensor: error in 3.32 with autoadjust of ADC-referance

3.33->3.34
Temp sensor - Bug fix: wrong format when temperature is 10 deg.
Motor - Fix: Reset Bridge A & B active bit when door closing. The channel might
             hang if a user push two or more ejector switch at the same time.
Ejector_sw - Disable/enable: When door is closed, disable read/polling of
             ejector_sw.

3.34->3.35
Boxxio LED - Flashing boxxio led.

3.35->3.36
I2C_interface.c
Error geting temperature if Boxio LED is not connected. 
Solution: decreased I2C frequency and ACK for last received byte 
in void "I2C_Receive" has been changed 

3.36->3.37
Error i motorOverLoadB fixed and ADC limit changed from 6 to 7

3.37->3.38
motorOverloaB and motortOverloadA replaced with motorOverload. Overload algorithm changed.

3.38 > 3.39
Overload algorithm changed. Improved test tighting and tighting.

3.39 > 3.40
Adjusting of overload limit and small changes in algorithm.
3.40 > 3.41
PERMITED_DIFFERENCE_TIGHT_TIGHTEN_PUSHER changed from 16 to 12.

3.41 > 3.42
overload function changed (reverse limit and twin limit)

3.42 > 3.43
FWS_2FWS implemented for new microcontroller.

3.43 > 3.44
Relocated USB function into RAM, and optimized for Speed (Project options). This to run USB code faster.

3.44 > 3.45
Added Hatch switch. Read and send on USB when Hatch is open and close. PS! When debugging, remeber to 
set Download -> Use Loader in Debug Options...


*/
/////////////////////////////////////////////////////////////////////////////
//               SOFTWARE VERSION
unsigned char pVersion[5] = "3.44";
/////////////////////////////////////////////////////////////////////////////

/* Demo program include files. */
#include "serial.h"
#include "IoCtrl.h"
#include "partest.h"


// Include Vensafe Interfaces
#include "GpIO.h"
#include "Pwm.h"
#include "Adc.h"
#include "Timer.h"
#include "Motor.h"
#include "UsbQ.h"
#include "ECom.h"
#include "Sensors.h"
#include "ext_irq.h"
#include "I2C_interface.h"
#include "I2C_TemperatureSensor.h"
#include "DIP_switch.h"


#define comSTACK_SIZE				configMINIMAL_STACK_SIZE
#define comTX_LED_OFFSET			( 0 )
#define comRX_LED_OFFSET			( 1 )
#define comTOTAL_PERMISSIBLE_ERRORS ( 2 )

/* The Tx task will transmit the sequence of characters at a pseudo random
interval.  This is the maximum and minimum block time between sends. */
#define comTX_MAX_BLOCK_TIME		( ( portTickType ) 0x96 )
#define comTX_MIN_BLOCK_TIME		( ( portTickType ) 0x32 )
#define comOFFSET_TIME				( ( portTickType ) 3 )

/* We should find that each character can be queued for Tx immediately and we
don't have to block to send. */
#define comNO_BLOCK					( ( portTickType ) 0 )

/* The Rx task will block on the Rx queue for a long period. */
#define comRX_BLOCK_TIME			( ( portTickType ) 0xffff )

/* The sequence transmitted is from comFIRST_BYTE to and including comLAST_BYTE. */
#define comFIRST_BYTE				( 'A' )
#define comLAST_BYTE				( 'X' )

#define comBUFFER_LEN				( ( unsigned portBASE_TYPE ) ( comLAST_BYTE - comFIRST_BYTE ) + ( unsigned portBASE_TYPE ) 1 )
#define comINITIAL_RX_COUNT_VALUE	( 0 )



 unsigned char m_nUnitID;

 unsigned char bWaitingOnHigh = 1;
 unsigned char bFall3count = 0;

 char m_I2C_interType  = 0;
 char m_I2C_externType = 0;

 // motor.c
 extern int m_nMainMotorTotalCount;
 extern int m_nAuxilaryMotorStopReason;
 extern int m_TwinMode;
 
 //Boxxio flash leds
 unsigned char bBoxxLedFlash_state = 0;
 unsigned char bBoxxLedFlash_lednr = 0;
 unsigned char bBoxxLedFlash_nValue = 0;


static volatile unsigned portBASE_TYPE uxRxLoops = comINITIAL_RX_COUNT_VALUE;


//unsigned char ucRequest[USB_QUEUE_ITEM_LENGTH];
//unsigned char ucResponse[USB_QUEUE_ITEM_LENGTH];


#define NB_LEB			4
/* Global variables */
#define SPEED 		(MCKKHz/10)
unsigned int LedSpeed = SPEED *50 ;

void IdentifyTemperatureSensorsType(void);
void EjectorControlTaskRun ( void);
void ErrorControlTaskRun(void);
void SensorControlTaskRun ( void);
void SoftwareReset (void);
int isLedOpen(int iMotorNo);

void ioCtrlStartMotor(int iMotorNo, unsigned char* ucReq, unsigned char* ucResp, int iCommand, int iStepCnt, int nDirection, int iSpeed, int iRegulateSpeed, int iTwinMotorNo, int iIsMainMotor);
void ioCtrlOnCommandSetLED(unsigned char* ucReq, unsigned char* ucResp, int nLedNo);
void ioCtrlOnCommandFlashLED(unsigned char* ucReq, unsigned char* ucResp, int nLedNo, unsigned char nValue);
void ioCtrlBoxxFlashLED(void);

void ExecuteAdressErrorCommand(unsigned char* ucReq);
/////////////////////////////////////////////////////
// USED ONLY IN EJECTOR CONFIGURATION
/////////////////////////////////////////////////////
void ExecuteSensorCommand(unsigned char* ucReq);
void sensorOnCommandGetTemperature(unsigned char* ucReq, unsigned char* ucResp, int nEquipNo, int nLengOfVal);
void SendEventIfSensorsInterrupted(void);
void EnableSensorInterruptAfterDelayTime(void);

/////////////////////////////////////////////////////
// USED ONLY IN EJECTOR CONFIGURATION
/////////////////////////////////////////////////////
void ExecuteEjectorCommand(unsigned char* ucReq);

void OnMotorFinished(int iMotorNo, int iDirection, int iTotalCnt);
void ButtonPushedTest_MotorNotMoving(int iMotorNo);



static char lastDoorOpenState = 0;
static char lastHatchOpenState = 0; //V 3.45 (PS)
//static char lastTiltState = 0;

//static   int LengOfTempVal_1=0;
//static   int LengOfTempVal_2=0;
//static   char OldTemp1=0;
//static   char OldTemp2=0;
//static   unsigned portCHAR Temperature_1[7];
//static   unsigned portCHAR Temperature_2[7];




               // Sensor main loop
//////////////////////////////////////////////////////////////////////////////
// Function     : SensorControlTaskRun()
// Purpose      : To run the Sensor Control SW
// Parameters   : None
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2006-08-03 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void SensorControlTaskRun(void)
{
   unsigned char ucRequest[USB_QUEUE_ITEM_LENGTH];
  // char TempToggle = 0;


   timerSet(INX_RS485_MI_TX_TIMER, TIME_3_MS); // start MI_TX();
  // GetTemperature(Temperature_1, &LengOfTempVal_1, SENSOR_1_I2C_EXT_FALL_ADRESS);
   //GetTemperature(Temperature_2, &LengOfTempVal_2, SENSOR_1_I2C_INT_CB_ADRESS);
   // timerSet(INX_TEMP_READ_TIMER, TIME_200_MS);
   for( ;; )
   {
      AT91C_BASE_WDTC->WDTC_WDCR = 0xA5000001; // watchdog reset timer
      // Check for USB messages from PC:
      if ((usbqReadUsbMsg(ucRequest)))
      {
          // usb msg received, execute it.
          ExecuteSensorCommand(ucRequest);
      }

  //    EnableSensorInterruptAfterDelayTime();
      SendEventIfSensorsInterrupted();

 // Poll Multi interface
      RS485_TX_MI();

  //If lam is active in more then 1 sec, turn it off
  if (timerExpired( INX_LAMP_TIMER))
  {
     Set_Lamp(0,1);
     Set_Lamp(0,2);
  }
  if (timerExpired( INX_LAMP_FALL_DELAY))
  {
     Fall3_Continue();
  }

  //Boxxio led flash timer  
  if (timerExpired( INX_BOXXLED_TIMER))
  {
     ioCtrlBoxxFlashLED();
  }


/*
  if (timerExpired( INX_TEMP_READ_TIMER))
  {
     if(TempToggle == 0)
     {
        TempToggle++;
        if(GetTemperature(Temperature_1, &LengOfTempVal_1, SENSOR_1_I2C_EXT_FALL_ADRESS)==0)
        {
           if(OldTemp1<10)
             OldTemp1++;
        }
        else
          OldTemp1=0;
     }
     else
     {
        TempToggle=0;
        if(GetTemperature(Temperature_2, &LengOfTempVal_2, SENSOR_1_I2C_INT_CB_ADRESS)==0)
        {
           if(OldTemp2<10)
             OldTemp2++;
        }
        else
          OldTemp2=0;
     }
     timerSet(INX_TEMP_READ_TIMER, TIME_1000_MS);
  }
  if (timerExpired( INX_TEMP_INIT_TIMER))
  {
    // I2C_Start_after_Reset();
  }
*/
  } // End foreverloop
}
                       // Ejector main loop
//////////////////////////////////////////////////////////////////////////////
// Function     : EjectorControlTaskRun()
// Purpose      : To run the Ejector Control SW
// Parameters   : None
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void EjectorControlTaskRun(void)
{
  unsigned char ucRequest[USB_QUEUE_ITEM_LENGTH];
  static int    iTotalCnt;
  int    iMotorNo;
  int iDirection;
   koggInit();


   for( ;; )
   {
      AT91C_BASE_WDTC->WDTC_WDCR = 0xA5000001; // watchdog reset timer

      // Check for USB messages from PC:
      if ((usbqReadUsbMsg(ucRequest)))
      {
        // usb msg received, execute it.
        ExecuteEjectorCommand(ucRequest);
      }

      // Walk through all motor data structs. On completion, send response to PC
      for (iMotorNo=1; iMotorNo<=16; iMotorNo++)
      {
         if ((motorFinished( iMotorNo, &iDirection, &iTotalCnt)))
         {
            OnMotorFinished(iMotorNo, iDirection, iTotalCnt);
         //  motorInit();
         }

         //////////////////////////////////////////////////////////////////////////
         // Button pushed while motor is not moving TEST
         //////////////////////////////////////////////////////////////////////////
         else if ((motorNotInUse( iMotorNo))) // && (motorLedSwitchAllowed(iMotorNo)))
         {
            ButtonPushedTest_MotorNotMoving(iMotorNo);
         }
      }
   } // End foreverloop

}

//////////////////////////////////////////////////////////////////////////////
// Function     : ErrorControlTaskRun()
// Purpose      : To run the Error dip adress event
// Parameters   : None
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ErrorControlTaskRun(void)
{
  unsigned char ucRequest[USB_QUEUE_ITEM_LENGTH];

   for( ;; )
   {
      AT91C_BASE_WDTC->WDTC_WDCR = 0xA5000001; // watchdog reset timer

      // Check for USB messages from PC:
      if ((usbqReadUsbMsg(ucRequest)))
      {
        // usb msg received, execute it.
        ExecuteAdressErrorCommand(ucRequest);
      }
   } // End foreverloop
}



//////////////////////////////////////////////////////////////////////////////
// Function     : ButtonPushedTest_MotorNotMoving
// Purpose      : test about ejector button has been pushed (motor is not moving)
// Parameters   : iMotorNo
// Return value : None
// Created      : 2005-11-09 by ES
// Updated      : 2005-11-09 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ButtonPushedTest_MotorNotMoving(int iMotorNo)
{
   unsigned char ucResp[USB_QUEUE_ITEM_LENGTH];
   int nResult;
  unsigned char  ucOneOrMoreMotorsMoving = 0;

  // test about some of motors is running
  for (int iMotNo=1; iMotNo<=16; iMotNo++)
  {
     if (motorNotInUse(iMotNo) == 0)
     {
        ucOneOrMoreMotorsMoving = 1;
        break;
     }
  }

  if (ucOneOrMoreMotorsMoving > 0)
  {
    return; // ONE MOTOR IS RUNNING. IT'S NOT ALLOWED TO USE ADC0- ADC4 (sharing of resourses)
  }

  nResult = buttonPusherSwitch(iMotorNo);
  // Test if a user are oprating the pusher while motor not running
  if (nResult == BUTTON_PUSHED_DETECTED)
  {
    // Report Switch event to PC
    ecomBuildEventMessage(ucResp, EQ_TYPE_EJECTOR_SWITCH, iMotorNo, POD_EVENT,  1, "1", EQ_STATUS_CMD_OK);
    //ecomBuildSwitchEventMsg(ucResp, EQ_STATUS_CMD_OK, "1");
    // Send switch event
    usbqWriteUsbMsg(ucResp);
  }
  else if (nResult == BUTTON_RELEASED_DETECTED)
  {
    // Report Switch event to PC
    ecomBuildEventMessage(ucResp, EQ_TYPE_EJECTOR_SWITCH, iMotorNo, POD_EVENT,  1, "0", EQ_STATUS_CMD_OK);
    //ecomBuildSwitchEventMsg(ucResp, EQ_STATUS_CMD_OK, "0");
    // Send switch event
    usbqWriteUsbMsg(ucResp);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Function     : isLedOpen
// Purpose      :
// Parameters   : iMotorNo
// Return value : None
// Created      : 2005-14-09 by ES
// Updated      : 2005-14-09 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int isLedOpen(int iMotorNo)
{
  // test about some of motors is running
  for (int iMotNo=1; iMotNo<=16; iMotNo++)
  {
     if (motorNotInUse(iMotNo) == 0)
     {
        return 2; // ONE MOTOR IS RUNNING. IT'S NOT ALLOWED TO USE ADC0- ADC4 (sharing of resourses)
     }
  }

  // Test if a user are oprating the pusher while motor not running
  return buttonPusherSwitchOnceMeasuring(iMotorNo);

}

//////////////////////////////////////////////////////////////////////////////
// Function     : OnMotorFinished
// Purpose      : after motor has been stoped, this funksjon has to fine
// "led open" position and send position to PC.
// Parameters   : iMotorNo, iDirection, iTotalCnt
// Return value : None
// Created      : 2005-11-09 by ES
// Updated      : 2005-11-09 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void OnMotorFinished(int iMotorNo, int iDirection, int iTotalCnt)
{
  unsigned char ucResponse[USB_QUEUE_ITEM_LENGTH];
    // Send response
  unsigned char* szEqStatus;
  int nStopReason;
  int nTwinMotor;
  int nReturnMotor = iMotorNo;
  int nIsMainMotor = 0;

  nStopReason = motorGetReasonOfStop(iMotorNo -1);
  if (m_TwinMode)
  {
     nTwinMotor = motorGetTwinMotorNo(iMotorNo);
  }

  nIsMainMotor = motorIsMainMotor(iMotorNo);

  motorReset(iMotorNo -1);
 // sprintf(szParameter, "%02d", nStopReason);

  if(nStopReason == STOP_LED_OPEN_SCAN)
  {
    return; // don't send USB msg.
  }



  if (m_TwinMode)
  {
     if (motorNotInUse(nTwinMotor)) // second motor has stopped
     {
       if (nIsMainMotor == 1) // *** Main motor
       {
         nStopReason = m_nAuxilaryMotorStopReason;
       }
       else                         // *** Not Main motor (Auxiliary)
       {
         nReturnMotor = nTwinMotor;
         iTotalCnt = m_nMainMotorTotalCount;
       }
     }
     else  // second motor has not stopped yet
     {
      // motorStop( nTwinMotor, STOP_ON_COMMAND);

       if (nIsMainMotor == 1) // *** Main motor
       {
          m_nMainMotorTotalCount = iTotalCnt;
       }
       else                         // *** Not Main motor (Auxiliary)
       {
          m_nAuxilaryMotorStopReason = nStopReason;
       }
       return; // DO NOT SEND USB MSG, OTHER MOTOR HAS NOT STOPPED YET.
     }
  }

  switch (nStopReason)
  {
  case  STOP_FINISHED:
      szEqStatus = EQ_STATUS_CMD_OK;
      break;
  case STOP_ON_COMMAND:
      szEqStatus = EQ_STATUS_CMD_OK;
    break;
  case STOP_OVERLOAD:
      szEqStatus = EQ_EJECTOR_OVERLOAD;
      break;
  case STOP_NO_PULSES_LOW_LEVEL:
      szEqStatus = EQ_CAN_NOT_DETECT_PULSES_LOW_LEVEL;
      break;
  case STOP_NO_PULSES_HIGH_LEVEL:
      szEqStatus = EQ_CAN_NOT_DETECT_PULSES_HIGH_LEVEL;
     break;
  case STOP_UNSTABLE_PULSES:
    szEqStatus = EQ_CAN_NOT_DETECT_PULSES_HIGH_LEVEL;
    break;
  default:
    szEqStatus = EQ_CAN_NOT_DETECT_PULSES_HIGH_LEVEL;
    break;
  }
  ecomBuildOperationCompleteMsg(m_nUnitID, nReturnMotor, iDirection, iTotalCnt, szEqStatus , ucResponse);
  usbqWriteUsbMsg(ucResponse);

  motorOverLoadClear();
  motorResetTwinInfo();
  motorReset(iMotorNo -1);
  if (m_TwinMode)
  {
     motorReset(nTwinMotor -1);
  }
  //motorInit(); // !!!! DETTE MÅ SJEKKES
}

//////////////////////////////////////////////////////////////////////////////
// Function     : EnableSensorInterruptAfterDelayTime
// Purpose      : Enable again interrupts after disabling in ext_irq.c
// Parameters   :
// Return value : None
// Created      : 2005-11-09 by ES
// Updated      : 2005-11-22 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void EnableSensorInterruptAfterDelayTime(void)
{
  // if PRODREL interrupt occured and disabling time of this interrupt is out, enable it again.
  if (timerExpired(INX_TILT_FALL1_TIMER))
  {
    // first clear possible old interrupts
    AT91C_BASE_PIOA->PIO_ISR;
    // enable again interrupt
    AT91F_PIO_InterruptEnable(AT91C_BASE_PIOA, FALL2_INPUT_BIT);
  }

  // if SHUTEMP interrupt occured and disabling time of this interrupt is out, enable it again.
  if (timerExpired(INX_TILT_FALL2_TIMER))
  {
    // first clear possible old interrupts
    AT91C_BASE_PIOA->PIO_ISR;
    // enable again interrupt
    //DEBUG..PS  AT91F_PIO_InterruptEnable(AT91C_BASE_PIOA, FALL1_INPUT_BIT);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Function     : SendEventIfSensorsInterrupted
// Purpose      : If some of sensors has been interrupted, this function sends msg to PC
// Parameters   :
// Return value : None
// Created      : 2005-11-09 by ES
// Updated      : 2006-04-11 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void SendEventIfSensorsInterrupted(void)
{
  unsigned char ucResp[USB_QUEUE_ITEM_LENGTH];
  unsigned char    nAnswer[2];
  char sUSBvalue_msg [RS485buff_length];
  /*int sUSBvalue_length;
  char MI_com_status;
  char IRQ485 = 0;*/


/*  if (isFALL1_Interrupted() == 1)
  {
    nAnswer[0]=FallSensor(1)+0x30;
    ecomBuildEventMessage(ucResp, EQ_TYPE_FALL_SENSOR, 1, POD_EVENT,  1, nAnswer, EQ_STATUS_CMD_OK);
    // Send Event
    usbqWriteUsbMsg(ucResp);
  }

  if (isFALL2_Interrupted() == 1)
  {
     nAnswer[0]=FallSensor(2)+0x30;
     ecomBuildEventMessage(ucResp, EQ_TYPE_FALL_SENSOR, 2, POD_EVENT,  1, nAnswer, EQ_STATUS_CMD_OK);
     // Send Event
     usbqWriteUsbMsg(ucResp);
  } */

  sUSBvalue_msg[0]=isTILT_Interrupted();
  if(sUSBvalue_msg[0] == 1|| sUSBvalue_msg[0] == 2)
  {
    sUSBvalue_msg[0]=sUSBvalue_msg[0]+0x30-1;
   /*   if(lastTiltState!=sUSBvalue_msg[0])
     {
        lastTiltState=sUSBvalue_msg[0];
        sUSBvalue_msg[1]=0;*/
        ecomBuildEventMessage(ucResp, EQ_TYPE_TILT_SENSOR, 1, POD_EVENT,  1, sUSBvalue_msg, EQ_STATUS_CMD_OK);
        // Send Event
        usbqWriteUsbMsg(ucResp);
    // }
  }
/*
  IRQ485=isRS485_Interrupted();
  if( IRQ485 == 1)
  {
     sUSBvalue_length=getRS485_msg(sUSBvalue_msg);
     MI_com_status= check_MIchecksum (sUSBvalue_msg, &sUSBvalue_length);
     if( MI_com_status==1)
     {
       ecomBuildEventMessage(ucResp, EQ_TYPE_RS485, 0, POD_EVENT,  sUSBvalue_length, sUSBvalue_msg, EQ_STATUS_CMD_OK);
       if(sUSBvalue_length==22)
         timerSet(INX_RS485_MI_TX_TIMER, TIME_4000_MS);
     }
     else if( MI_com_status==0)
       ecomBuildEventMessage(ucResp, EQ_TYPE_RS485, 0, POD_EVENT,  sUSBvalue_length, sUSBvalue_msg, EQ_STATUS_RS485_CHCKSUM_ERROR);
     else
       ecomBuildEventMessage(ucResp, EQ_TYPE_RS485, 0, POD_EVENT,  sUSBvalue_length, sUSBvalue_msg, EQ_STATUS_RS485_LINGTH_OF_DATA_0);
     // Send Event
     usbqWriteUsbMsg(ucResp);
  }
  else if( IRQ485 == 2)
  {
     ecomBuildEventMessage(ucResp, EQ_TYPE_RS485, 0, POD_EVENT,  0, "", EQ_UNSTABILE_RS485);
      usbqWriteUsbMsg(ucResp);
  }*/

  if (bWaitingOnHigh == 1)
  {
    if (isFall3_Interrupted() > 0)
    {
      ClearSensor3Flags(1);
      bWaitingOnHigh = 0;

      ecomBuildEventMessage(ucResp, EQ_TYPE_FALL_SENSOR, 3, POD_EVENT,  1, "1", EQ_STATUS_CMD_OK);
      usbqWriteUsbMsg(ucResp);
    }
  }
  else
  {
    if (isFall3_Interrupted() == 0)
    {
      if (bFall3count >= 254)
      {
        bWaitingOnHigh = 1;

        ecomBuildEventMessage(ucResp, EQ_TYPE_FALL_SENSOR, 3, POD_EVENT,  1, "0", EQ_STATUS_CMD_OK);
        usbqWriteUsbMsg(ucResp);
        bFall3count = 0;
        ClearSensor3Flags(0);
      }
      else
      {
        bFall3count ++;
      }
    }
  }

  
  if (DoorSensor() == 1)
  {
     if (lastDoorOpenState == 0)
     {
         vTaskDelay( 100 );//Prell delay
         if (DoorSensor() == 1)
         {
            ecomBuildEventMessage(ucResp, EQ_TYPE_CABINET_DOR, 1, POD_EVENT, 1, "1", EQ_STATUS_CMD_OK);
         // Send Event
            usbqWriteUsbMsg(ucResp);
            lastDoorOpenState = 1;
         }
     }
  }
  else
  {
     if (lastDoorOpenState == 1)
     {
         vTaskDelay( 100 );//Prell delay
         if (DoorSensor() == 0)
         {
            ecomBuildEventMessage(ucResp, EQ_TYPE_CABINET_DOR, 1, POD_EVENT, 1, "0", EQ_STATUS_CMD_OK);
            usbqWriteUsbMsg(ucResp);
            lastDoorOpenState = 0;
         }
     }
  }
  
  
  //Check Hatch sensor -> V3.45 (PS)
  if (HatchSensor() == 1)
  {
     //Hatch Open
     if (lastHatchOpenState == 0)
     {
         vTaskDelay( 100 );//Prell delay
         if (HatchSensor() == 1)
         {
            ecomBuildEventMessage(ucResp, EQ_TYPE_HATCH_SENSOR, 1, POD_EVENT, 1, "1", EQ_STATUS_CMD_OK);
         // Send Event
            usbqWriteUsbMsg(ucResp);
            lastHatchOpenState = 1;
         }
     }
  }
  else
  {
     //Hatch Closed
     if (lastHatchOpenState == 1)
     {
         vTaskDelay( 100 );//Prell delay
         if (HatchSensor() == 0)
         {
            ecomBuildEventMessage(ucResp, EQ_TYPE_HATCH_SENSOR, 1, POD_EVENT, 1, "0", EQ_STATUS_CMD_OK);
            usbqWriteUsbMsg(ucResp);
            lastHatchOpenState = 0;
         }
     }
  }
  
  
  
  
}

// USED BY BOOTLOADER
extern void BootloaderStart(int nFileLength);

//////////////////////////////////////////////////////////////////////////////
// Function     : ExecuteSensorCommand()
// Purpose      : To execute commands received from PC (used only for Sensor Configuration)
// Parameters   : None
// Return value : None
// Created      : 2005-11-09 by ES
// Updated      : 2006-03-10 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ExecuteSensorCommand(unsigned char* ucReq)
{
  unsigned char ucResp[USB_QUEUE_ITEM_LENGTH];
  int    nEquipmentType, nEquipmentNo, nRequestCommand, nLengthOfValue, nValue;
  unsigned char    nAnswer[2]={0,0};
  unsigned char Buff485_Tx[RS485buff_length];
  char USB_Do_Not_Send=0;
  unsigned char ucFallResp[34]; // 32 + status

  int nFileLength;

  ecomDisassembleReceivedCommand(ucReq, &nEquipmentType, &nEquipmentNo, &nRequestCommand, &nLengthOfValue, &nValue);

  switch(nRequestCommand)
  {
    case GET_INPUT:
      /////////////////////////////////////////////////////////
      // Get temperature
      /////////////////////////////////////////////////////////
      if (nEquipmentType == EQ_TYPE_CABINET_TEMPERATURE)
      {
        sensorOnCommandGetTemperature(ucReq, ucResp, nEquipmentNo, nLengthOfValue);
        /*
         if (nEquipmentNo == 1)
         {
            if(LengOfTempVal_1 != 0)
            {
               if(OldTemp1>=10)
                 ecomBuildResponse(ucReq, ucResp, LengOfTempVal_1, Temperature_1, EQ_STATUS_OLDVALUE);
               else
                  ecomBuildResponse(ucReq, ucResp, LengOfTempVal_1, Temperature_1, EQ_STATUS_CMD_OK);
            }
            else
               ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_NOT_SUCCESSED );
         }
         else if (nEquipmentNo == 2)
         {
            if(LengOfTempVal_1 != 0)
            {
              if(OldTemp2>=10)
                ecomBuildResponse(ucReq, ucResp, LengOfTempVal_2, Temperature_2, EQ_STATUS_OLDVALUE);
              else
                ecomBuildResponse(ucReq, ucResp, LengOfTempVal_2, Temperature_2, EQ_STATUS_CMD_OK);
            }
            else
               ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_NOT_SUCCESSED );
         }
         else
         {
            ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_INVALID_COMMAND);
         }
        */
      }
      else if (nEquipmentType ==EQ_TYPE_FALL_SENSOR)
      {
         if(nEquipmentNo==1 || nEquipmentNo==2)
         {
            nAnswer[0]=FallSensor(nEquipmentNo)+0x30;
            ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_CMD_OK);
         }
         else if(nEquipmentNo==3)
         {
            nAnswer[0]=Status_FallSensor_3()+0x30;
            if(nAnswer[0]=='0' || nAnswer[0]=='1')
              ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_CMD_OK);
            else if(nAnswer[0]=='3')
              ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_FALL_NOT_FINISHED_INIT);
            else if(nAnswer[0]=='4')
              ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_FALL_OFF);
            else
              ecomBuildResponse(ucReq, ucResp, 0,"",EQ_STATUS_FALL_BROKEN);
         }
         else
         {
            ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
         }
      }
      else if (nEquipmentType ==EQ_TYPE_FALL_SENSOR_CALIB)
      {

        GetStatusMessage(ucFallResp);
        ecomBuildResponse(ucReq, ucResp, 32, ucFallResp, &ucFallResp[32]);
      }
      else if (nEquipmentType ==EQ_TYPE_CABINET_DOR)
      {
         if(nEquipmentNo==1)
         {
            nAnswer[0]=DoorSensor()+0x30;
            ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_CMD_OK);
         }
         else
         {
            ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
         }
      }
      else if (nEquipmentType ==EQ_TYPE_TILT_SENSOR)
      {
         if(nEquipmentNo==1)
         {
            nAnswer[0]=TiltSensor()+0x30;
            ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_CMD_OK);
         }
         else
         {
            ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
         }
      }
      else  if (nEquipmentType == EQ_TYPE_CAB_LIGHT_OR_BUZZOR)
      {
          if(nEquipmentNo==2 || nEquipmentNo== 1)
          {
             nAnswer[0]=Get_Lamp(nEquipmentNo)+0x30;
             if(nAnswer[0]>'1')
             {
                nAnswer[0]=nAnswer[0]-2;
                ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_CMD_OK);
             }
             else
             {
                ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_LAMP_BROKEN);
             }
          }
          else
          {
             ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
          }
      }
      //Hatch Sensor -> 3.45 (PS)
      else if (nEquipmentType == EQ_TYPE_HATCH_SENSOR)
      {
         if(nEquipmentNo==1)
         {
            nAnswer[0] = HatchSensor()+0x30;
            ecomBuildResponse(ucReq, ucResp, 1, nAnswer, EQ_STATUS_CMD_OK);
         }
         else
         {
            ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
         }
      }
      else if (nEquipmentType == EQ_TYPE_CONTROL_BOX_SW_VER)                //Software version
         ecomBuildResponse(ucReq, ucResp, 4, pVersion, EQ_STATUS_CMD_OK);
      /////////////////////////////////////////////////////////
      // ERROR, not expected command received
      /////////////////////////////////////////////////////////
      else
      {
        ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_INVALID_COMMAND);
      }
      break;
    case SET_OUTPUT:
       if (nEquipmentType == EQ_TYPE_CAB_LIGHT_OR_BUZZOR)
       {
          if(nEquipmentNo==2 || nEquipmentNo==1 )
          {
             if(nLengthOfValue==1&& nValue <=1)
             {
                nAnswer[0]=Set_Lamp(nValue,nEquipmentNo)+0x30;
                if(nAnswer[0]>'1')
                {
                   nAnswer[0]=Get_Lamp(nEquipmentNo)+0x30;
                   if(nAnswer[0]>'1')
                   {
                      nAnswer[0]=nAnswer[0]-2;
                   }
                   ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_CMD_OK);
                }
                else
                {
                   nAnswer[0]=Get_Lamp(nEquipmentNo)+0x30;
                   if(nAnswer[0]>'1')
                   {
                      nAnswer[0]=nAnswer[0]-2;
                   }
                   ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_LAMP_BROKEN);
                }
             }
             else
             {
                ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_INVALID_VALUES);
             }
          }
          else if (nEquipmentNo >= 4 && nEquipmentNo <= 7)
          {
             if(nValue != 0) ioCtrlOnCommandFlashLED(ucReq, ucResp, nEquipmentNo-3,nValue);
             else ioCtrlOnCommandSetLED (ucReq, ucResp, nEquipmentNo - 3);  
          }
          else
          {
             ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
          }
      }
      else if (nEquipmentType == EQ_TYPE_FALL_SENSOR)
      {
         if(nEquipmentNo==3 || nEquipmentNo==2 || nEquipmentNo==1)
         {
            if(nLengthOfValue==1&& nValue ==1)
            {
               if(nEquipmentNo==3)
               {
                  Fall_3_Event_Enable_Disable(1);
                  Start_Fall_3();
               }
               else if(nEquipmentNo==2)
                  Start_Fall_2();
               else if(nEquipmentNo==1)
                  Start_Fall_1();
               nAnswer[0]='1';
               ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_CMD_OK);
            }
            else if(nLengthOfValue==1&& nValue ==0)
            {
               if(nEquipmentNo==3)
               {
                  Stop_Fall_3();
                  Fall_3_Event_Enable_Disable(0);
               }
               else if(nEquipmentNo==2)
                  Stop_Fall_2();
               else if(nEquipmentNo==1)
                  Stop_Fall_1();
               nAnswer[0]='0';
               ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_CMD_OK);
            }
            else
            {
               ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_INVALID_VALUES);
            }
         }
         else
         {
            ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
         }
      }
      else if (nEquipmentType == EQ_TYPE_TILT_SENSOR)
      {
         if(nEquipmentNo==1)
         {
            if(nLengthOfValue==1&& nValue ==1)
            {
               Start_Tilt();
               nAnswer[0]='1';
               ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_CMD_OK);
            }
            else if(nLengthOfValue==1&& nValue ==0)
            {
               Stop_Tilt();
               nAnswer[0]='0';
               ecomBuildResponse(ucReq, ucResp, 1, nAnswer,EQ_STATUS_CMD_OK);
            }
            else
            {
               ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_INVALID_VALUES);
            }
         }
         else
         {
            ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
         }
      }
      else if (nEquipmentType == EQ_TYPE_RS485)
      {
       // while (testBeforSend() != 1)
       // {}
         memset(Buff485_Tx, 0, RS485buff_length);
         ecomGetValueString(ucReq,Buff485_Tx);
         AT91F_US_TX_no_of_byte(Buff485_Tx,nLengthOfValue);
         USB_Do_Not_Send=1;
      }
      else if (nEquipmentType == EQ_TYPE_CONTROL_BOX_SW_LOAD)// && nValue > 1000) // File lengt is min. 1000 bytes
      {
        nFileLength =  (ucReq[16] << 8) + ucReq[17];
        BootloaderStart(nFileLength);
      }
      else
        ecomBuildResponse(ucReq, ucResp, 1, "1", EQ_STATUS_INVALID_COMMAND);

      break;
    /////////////////////////////////////////////////////////
    // Software Reset Request
    /////////////////////////////////////////////////////////
    case RESTART_SOFTWARE:
      if (nEquipmentType == EQ_TYPE_CONTROL_BOX)
         SoftwareReset();
      else
        ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
      break;

    default:
      // Build Response "Invalid command in request"
      ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_INVALID_COMMAND);
      break;
    }

    // Send Response
    if(USB_Do_Not_Send ==0)
      usbqWriteUsbMsg(ucResp);
}



//////////////////////////////////////////////////////////////////////////////
// Function     : ioCtrlOnCommandSetLED()
// Purpose      : To set led ON/ OFF
// Parameters   : None
// Return value : None
// Created      : 2005-05-18 by ES
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ioCtrlOnCommandSetLED(unsigned char* ucReq, unsigned char* ucResp, int nLedNo)
{
  char chTest = 0;

  timerStop(INX_BOXXLED_TIMER);
  
  if (nLedNo == 1)
  {
    chTest = 0xFE;
  }
  else if (nLedNo == 2)
  {
    chTest = 0xFD;
  }
  else if (nLedNo == 3)
  {
    chTest = 0xFB;
  }
  else if (nLedNo == 4)
  {
    chTest = 0xF7;
  }

  if (I2C_Write(LED_I2C_INT_CB_ADRESS, 0, 0, &chTest, 1) == 1)
  {
     ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_CMD_OK);
  }
  else
  {
     ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_NOT_SUCCESSED);
     return;
  }
}


//////////////////////////////////////////////////////////////////////////////
// Function     : ioCtrlOnCommandFlashLED()
// Purpose      : To flash led ON/ OFF
// Parameters   : None
// Return value : None
// Created      : 2008-05-16 by PS
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ioCtrlOnCommandFlashLED(unsigned char* ucReq, unsigned char* ucResp, int nLedNo, unsigned char nValue)
{
  char chTest = 0;
  
  bBoxxLedFlash_state = 0;
  bBoxxLedFlash_lednr = nLedNo;         //Led 1
  bBoxxLedFlash_nValue = nValue;        //Led 2
  
  timerStop(INX_BOXXLED_TIMER);
  
  if (nLedNo == 1)
  {
    chTest = 0xFE;
  }
  else if (nLedNo == 2)
  {
    chTest = 0xFD;
  }
  else if (nLedNo == 3)
  {
    chTest = 0xFB;
  }
  else if (nLedNo == 4)
  {
    chTest = 0xF7;
  }

  if (I2C_Write(LED_I2C_INT_CB_ADRESS, 0, 0, &chTest, 1) == 1)
  {
     ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_CMD_OK);
  }
  else
  {
     ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_NOT_SUCCESSED);
     return;
  }
  timerSet(INX_BOXXLED_TIMER, 500);     //500ms timer
}



void ioCtrlBoxxFlashLED(void)
{
  char chTest = 0;
  
  //LED OFF OR SWITCH LED
  if(bBoxxLedFlash_state==0)
  {
    if(bBoxxLedFlash_lednr!=bBoxxLedFlash_nValue)
    {
        if (bBoxxLedFlash_nValue == 1)
        {
          chTest = 0xFE;
        }
        else if (bBoxxLedFlash_nValue == 2)
        {
          chTest = 0xFD;
        }
        else if (bBoxxLedFlash_nValue == 3)
        {
          chTest = 0xFB;
        }
        else if (bBoxxLedFlash_nValue == 4)
        {
          chTest = 0xF7;
        }
    }
    else if(bBoxxLedFlash_lednr==bBoxxLedFlash_nValue)
    {
        chTest = 0xFF;
    }
    bBoxxLedFlash_state = 1;
  }
  
  //START LED ON
  else if(bBoxxLedFlash_state==1)
  {
      if (bBoxxLedFlash_lednr == 1)
      {
        chTest = 0xFE;
      }
      else if (bBoxxLedFlash_lednr == 2)
      {
        chTest = 0xFD;
      }
      else if (bBoxxLedFlash_lednr == 3)
      {
        chTest = 0xFB;
      }
      else if (bBoxxLedFlash_lednr == 4)
      {
        chTest = 0xF7;
      }
      bBoxxLedFlash_state = 0;
  }

  I2C_Write(LED_I2C_INT_CB_ADRESS, 0, 0, &chTest, 1);
  timerSet(INX_BOXXLED_TIMER, 500);     //500ms timer  
}





//////////////////////////////////////////////////////////////////////////////
// Function     : sensorOnCommandGetTemperature()
// Purpose      : To execute command Get Temperature
// Parameters   : None
// Return value : None
// Created      : 2005-11-09 by ES
// Updated      : 2005-11-24 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void sensorOnCommandGetTemperature(unsigned char* ucReq, unsigned char* ucResp, int nEquipNo, int nLengOfVal)
{
  unsigned portCHAR pTemperature_1[7];
  int nSensorAddress;
  char cSensorType;

  if (nEquipNo == 2)
  {
     nSensorAddress = SENSOR_1_I2C_INT_CB_ADRESS;
     cSensorType = m_I2C_interType;
  }
  else if (nEquipNo == 1)
  {
    nSensorAddress = SENSOR_1_I2C_EXT_FALL_ADRESS;
    cSensorType = m_I2C_externType;
  }
  else
  {
      ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_INVALID_COMMAND);
      return;
  }

  if (GetTemperature(pTemperature_1, &nLengOfVal, nSensorAddress, cSensorType) == 1)
  {
      ecomBuildResponse(ucReq, ucResp, nLengOfVal, pTemperature_1, EQ_STATUS_CMD_OK);
  }
  else
  {
      ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_NOT_SUCCESSED );
  }
}

void IdentifyTemperatureSensorsType(void)
{
  int i;
  //int nResult;

  unsigned portCHAR pTemperature_1[7];
  int nLengOfVal;

  m_I2C_interType = I2C_TYPE_LM75;
  m_I2C_externType = I2C_TYPE_LM75;
  /*
  for (i = 0; i < 3; i++)
  {
    m_I2C_interType = TestSensorType(SENSOR_1_I2C_INT_CB_ADRESS);
    if (m_I2C_interType > 0)
    {
      if (m_I2C_interType == I2C_TYPE_DS1621)
      {
        GetTemperature(pTemperature_1, &nLengOfVal, SENSOR_1_I2C_INT_CB_ADRESS, m_I2C_interType);
      }
      break;
    }
  }

  for (i = 0; i < 3; i++)
  {
    m_I2C_externType = TestSensorType(SENSOR_1_I2C_EXT_FALL_ADRESS);
    if (m_I2C_externType > 0)
    {
      if (m_I2C_interType == I2C_TYPE_DS1621)
      {
        GetTemperature(pTemperature_1, &nLengOfVal, SENSOR_1_I2C_EXT_FALL_ADRESS, m_I2C_externType);
      }
      break;
    }
  }*/
}

void ioCtrlStartMotor(int iMotorNo, unsigned char* ucReq, unsigned char* ucResp, int iCommand, int iStepCnt, int nDirection, int iSpeed, int iRegulateSpeed, int iTwinMotorNo, int iIsMainMotor)
{
  if (motorNotInUse( iMotorNo))
        {
          // Motor has not been started, allocate HW and trig step measuring:
          if ((motorStartKoggMeasure( iMotorNo, iStepCnt, iRegulateSpeed)))
          {
            // Save given move command in motor struct
            motorSaveCommand(iMotorNo, iCommand);

            /*
            if (iCommand == TIGHTEN_PUSHER)
            {
               if ((iMotorNo >= 1) && (iMotorNo <= 8))
              {
                pwmUpdate(PWM_CH1,100);
              }
              else if ((iMotorNo >= 9) && (iMotorNo <= 16))
              {
                pwmUpdate(PWM_CH3,100);
              }
            }*/
            // Start of measuring succeeded, so ADC and Bridge are available, start motor:
            motorStart( iMotorNo , nDirection, iSpeed, iTwinMotorNo, iIsMainMotor);

            // Build Response
            // ecomBuildAckMsg(ucReq, ucResp, EQ_STATUS_CMD_OK);
           }
           else
           {
            // Build Response "HW Allocation Error"
            ecomBuildAckMsg(ucReq, ucResp, EQ_STATUS_NOT_SUCCESSED);
            usbqWriteUsbMsg(ucResp);
           }
          }
          else
          {
            // Build Response "Motor allready running"
            ecomBuildAckMsg(ucReq, ucResp, EQ_STATUS_MOTOR_BUSY);
            usbqWriteUsbMsg(ucResp);
          }
}


//////////////////////////////////////////////////////////////////////////////
// Function     : ExecuteEjectorCommand()
// Purpose      : To execute commands received from PC (used only for Ejector Configuration)
// Parameters   : None
// Return value : None
// Created      : 2005-11-09 by ES
// Updated      : 2006-09-21 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void  ExecuteEjectorCommand(unsigned char* ucReq)
{
  int i;
 // int    nEquipmentType;
  int nFileLength;
  unsigned char ucResp[USB_QUEUE_ITEM_LENGTH];
  static int    iCommand;
  static int    iEqNo;
  static int    iAuxilaryEqNo;
  static int    iEqType;
  static int    iEqValue;
  static int    iDirection;
  static int    iStepCnt;
  static int    iSpeed;
  static int    iStringLength;
  int iValidData;
  unsigned char    nAnswer[33];
  unsigned char    iStatus [3];
  unsigned char iValues[17];



  // Pick up command in message
  iCommand = ecomGetCommand(ucReq);
  iEqNo = ecomGetEquipmentNo(ucReq);
  iEqType = ecomGetEquipmentType(ucReq);
  iStringLength = ecomStringLength(ucReq);

  switch(iCommand)
  {
    /*case LOAD_SOFTWARE:
      if (iEqType == EQ_TYPE_CONTROL_BOX)// && nValue > 1000) // File lengt is min. 1000 bytes
      {
        nFileLength =  (ucReq[16] << 8) + ucReq[17];
        BootloaderStart(nFileLength);
      }
      else
      {
        ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_INVALID_COMMAND);
      }
    break;
    */
    case MOVE_PUSHER:
      if (iEqType == EQ_TYPE_EJECTOR)
      {
      // Find iMotorNo, iDirection, iStepCnt and iDutyCycle:
        iValidData = ecomValidMoveRequest(iCommand, ucReq, &iEqNo, &iDirection, &iStepCnt, &iSpeed, &iAuxilaryEqNo);
        if (iValidData)
        {
          if (iEqNo == 2)
          {
            iEqNo = 2;
          }
          if (motorIsConnected(iEqNo) == MOTOR_NOT_CONNECTED)
          {
            ecomBuildAckMsg(ucReq, ucResp, EQ_EJECTOR_NOT_CONNECTED);
            usbqWriteUsbMsg(ucResp);
            break;
          }
          if (iAuxilaryEqNo != 0)
          {
            if (motorIsConnected(iAuxilaryEqNo) == MOTOR_NOT_CONNECTED)
            {
              ecomBuildAckMsg(ucReq, ucResp, EQ_EJECTOR_NOT_CONNECTED);
              usbqWriteUsbMsg(ucResp);
              break;
            }
          }
          if(iDirection==FORWARD)
          {
            if(iSpeed < 5)
              iSpeed=5;
            ioCtrlStartMotor(iEqNo, ucReq, ucResp, iCommand, iStepCnt, iDirection, iSpeed, REGULATE_SPEED, iAuxilaryEqNo, 1);
            if (iAuxilaryEqNo != 0)
            {
              ioCtrlStartMotor(iAuxilaryEqNo, ucReq, ucResp, iCommand, iStepCnt, iDirection, iSpeed, REGULATE_SPEED,iEqNo , 0);
              m_TwinMode = 1;
            }
          }
          else
          {
            ioCtrlStartMotor(iEqNo, ucReq, ucResp, iCommand, iStepCnt, iDirection, 100, NOT_REGULATE_SPEED, iAuxilaryEqNo, 1);
            if (iAuxilaryEqNo != 0)
            {
              ioCtrlStartMotor(iAuxilaryEqNo, ucReq, ucResp, iCommand, iStepCnt, iDirection, 100, REGULATE_SPEED,iEqNo , 0);
              m_TwinMode = 1;
            }
          }
        }
        else
        {
          // Build Response "Invalid data in request"
          ecomBuildAckMsg(ucReq, ucResp, EQ_STATUS_INVALID_VALUES);
          usbqWriteUsbMsg(ucResp);
        }
      }
      else
      {
         ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
         usbqWriteUsbMsg(ucResp);
      }
      break;

    case TIGHTEN_PUSHER:
        // Find iMotorNo, iDirection, iStepCnt and iDutyCycle:
      if (iEqType == EQ_TYPE_EJECTOR)
      {
         iValidData = ecomValidMoveRequest(iCommand, ucReq, &iEqNo, &iDirection, &iStepCnt, &iSpeed, &iAuxilaryEqNo);
         if (iValidData)
         {
            if (motorIsConnected(iEqNo) == MOTOR_NOT_CONNECTED)
            {
               ecomBuildAckMsg(ucReq, ucResp, EQ_EJECTOR_NOT_CONNECTED);
               usbqWriteUsbMsg(ucResp);
               break;
            }
            if (iAuxilaryEqNo != 0)
            {
              if (motorIsConnected(iAuxilaryEqNo) == MOTOR_NOT_CONNECTED)
              {
                ecomBuildAckMsg(ucReq, ucResp, EQ_EJECTOR_NOT_CONNECTED);
                usbqWriteUsbMsg(ucResp);
                break;
              }
            }
            Set_Move_Type(TIGHTEN_PUSHER);

            ioCtrlStartMotor(iEqNo, ucReq, ucResp, iCommand, iStepCnt, iDirection, iSpeed, NOT_REGULATE_SPEED, iAuxilaryEqNo , 1);
            if (iAuxilaryEqNo != 0)
            {
              ioCtrlStartMotor(iAuxilaryEqNo, ucReq, ucResp, iCommand, iStepCnt, iDirection, iSpeed, NOT_REGULATE_SPEED, iEqNo , 0);
              m_TwinMode = 1;
            }
         }
         else
         {
            // Build Response "Invalid data in request"
            ecomBuildAckMsg(ucReq, ucResp, EQ_STATUS_INVALID_VALUES);
            usbqWriteUsbMsg(ucResp);
         }
      }
      else
      {
         ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
         usbqWriteUsbMsg(ucResp);
      }
      break;


     case TEST_PUSHER:
        // Find iMotorNo, iDirection, iStepCnt and iDutyCycle:
      if (iEqType == EQ_TYPE_EJECTOR)
      {
         iValidData = ecomValidMoveRequest(iCommand, ucReq, &iEqNo, &iDirection, &iStepCnt, &iSpeed, &iAuxilaryEqNo);
         if (iValidData)
         {
            if (motorIsConnected(iEqNo) == MOTOR_NOT_CONNECTED)
            {
               ecomBuildAckMsg(ucReq, ucResp, EQ_EJECTOR_NOT_CONNECTED);
               usbqWriteUsbMsg(ucResp);
               break;
            }
            if (iAuxilaryEqNo != 0)
            {
              if (motorIsConnected(iAuxilaryEqNo) == MOTOR_NOT_CONNECTED)
              {
                ecomBuildAckMsg(ucReq, ucResp, EQ_EJECTOR_NOT_CONNECTED);
                usbqWriteUsbMsg(ucResp);
                break;
              }
            }
            Set_Move_Type(TEST_PUSHER);

            ioCtrlStartMotor(iEqNo, ucReq, ucResp, iCommand, iStepCnt, iDirection, iSpeed, NOT_REGULATE_SPEED, iAuxilaryEqNo , 1);
            if (iAuxilaryEqNo != 0)
            {
              ioCtrlStartMotor(iAuxilaryEqNo, ucReq, ucResp, iCommand, iStepCnt, iDirection, iSpeed, NOT_REGULATE_SPEED, iEqNo , 0);
              m_TwinMode = 1;
            }
         }
         else
         {
            // Build Response "Invalid data in request"
            ecomBuildAckMsg(ucReq, ucResp, EQ_STATUS_INVALID_VALUES);
            usbqWriteUsbMsg(ucResp);
         }
      }
      else
      {
         ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
         usbqWriteUsbMsg(ucResp);
      }
      break;

     case SET_OUTPUT:
       if (iEqType == EQ_TYPE_CONTROL_BOX_SW_LOAD)// && nValue > 1000) // File lengt is min. 1000 bytes
       {
          nFileLength =  (ucReq[16] << 8) + ucReq[17];
          BootloaderStart(nFileLength);
       }
       else if (iEqType == EQ_TYPE_EJECTOR_SWITCH)
       {

          if (iEqNo >= 1 && iEqNo <= 16)
          {
             iValidData = ecomValidOpenLedRequest(ucReq, &iEqNo, &iEqValue, &iAuxilaryEqNo);
             if (motorIsConnected(iEqNo) == MOTOR_NOT_CONNECTED)
             {
                ecomBuildAckMsg(ucReq, ucResp, EQ_EJECTOR_NOT_CONNECTED);
                usbqWriteUsbMsg(ucResp);
                 break;
             }
             else if(iEqValue==0 && iValidData == 1)
             {
                ResetButton(iEqNo);
                ecomBuildResponse(ucReq, ucResp, 1, "0", EQ_STATUS_CMD_OK);
                usbqWriteUsbMsg(ucResp);
             }
             else if (iValidData == 1 && iEqValue==1)
             {
                Enable_Ejswitch(iCommand,iEqNo,iStatus);
                if (iStatus == EQ_STATUS_CMD_OK)
                {
                  if (iAuxilaryEqNo >= 1 && iAuxilaryEqNo <= 16)
                  {
                    Enable_Ejswitch(iCommand,iAuxilaryEqNo,iStatus);
                  }
                }
                ecomBuildResponse(ucReq, ucResp, 1, "1",iStatus);
                usbqWriteUsbMsg(ucResp);
             }
             else
             {
                // Build Response "Invalid data in request"
                ecomBuildResponse(ucReq, ucResp, 1, "0",EQ_STATUS_INVALID_VALUES);
                usbqWriteUsbMsg(ucResp);
             }
             break;
          }
          else if(iEqNo == WILDCARD)
          {
             if(iStringLength != 16)
             {
                ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_INVALID_VALUES);
                usbqWriteUsbMsg(ucResp);
                break;
             }
             else
             {
                ecomGetValueString(ucReq,iValues);
                for(i=0;i<16;i++)
                {
                   if(iValues[i] != '1' && iValues[i] != '0')
                     break;
                   iValues[i] = iValues[i] - 0x30;
                }
                if( i < 16)
                {
                   ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_INVALID_VALUES);
                   usbqWriteUsbMsg(ucResp);
                   break;
                }
                else
                {
                   for(i=0;i<16;i++)
                   {
                      if (motorIsConnected(i+1) == MOTOR_NOT_CONNECTED)
                      {
                         memcpy(iStatus,&EQ_EJECTOR_NOT_CONNECTED,2);
                         nAnswer[i*2]=iStatus[0];
                         nAnswer[(i*2)+1]=iStatus[1];
                      }
                      //DOOR CLOSING
                      else if(iValues[i] == 0)
                      {
                         ResetButton(i+1);
                         memcpy(iStatus,&EQ_STATUS_CMD_OK,2);
                         nAnswer[i*2]=iStatus[0];
                         nAnswer[(i*2)+1]=iStatus[1];
                         motorResetChActive();           //Reset Bridge A & B, active bit
                      }
                      //DOOR OPENING
                      else if (iValues[i] == 1)
                      {
                         Enable_Ejswitch(iCommand,i+1,iStatus);
                         nAnswer[i*2]=iStatus[0];
                         nAnswer[(i*2)+1]=iStatus[1];
                      }
                   }
                   ecomBuildResponse(ucReq, ucResp, 32, nAnswer, EQ_STATUS_CMD_OK);
                   usbqWriteUsbMsg(ucResp);
                   break;
                }
             }
          }
          else
          {
              ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
              usbqWriteUsbMsg(ucResp);
              break;
          }
       }
       else
       {
            ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
            usbqWriteUsbMsg(ucResp);
            break;
       }


        case STOP_PUSHER:

          if (iEqType == EQ_TYPE_EJECTOR)
          {
             if (motorIsConnected(iEqNo) == MOTOR_NOT_CONNECTED)
             {
                break;
             }

             // Stop addressed motor !!
             if ((iEqNo >= 1) && (iEqNo <= 16))
             {
                motorStop( iEqNo, STOP_ON_COMMAND);
             }
          }
          else
          {
             ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
             usbqWriteUsbMsg(ucResp);
          }
           break;

    /////////////////////////////////////////////////////////
    // Software Reset Request
    /////////////////////////////////////////////////////////
    case RESTART_SOFTWARE:
      if (iEqType == EQ_TYPE_CONTROL_BOX)
         SoftwareReset();
      else
      {
        ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
        usbqWriteUsbMsg(ucResp);
      }
      break;
         /////////////////////////////////////////////////////////
         // Ejector status connected
         /////////////////////////////////////////////////////////
      case GET_INPUT:
        if (iEqType == EQ_TYPE_EJECTOR)
        {
           if(iEqNo==WILDCARD)
           {
              for(i=0;i<16;i++)
              {
                 if(motorIsConnected(i+1) == MOTOR_NOT_CONNECTED)
                   nAnswer[i]='0';
                 else
                   nAnswer[i]='1';
              }
              ecomBuildResponse(ucReq, ucResp, 16, nAnswer, EQ_STATUS_NA);
           }
           else if ((iEqNo >= 1) && (iEqNo <= 16))
           {
              if(motorIsConnected(iEqNo) == MOTOR_NOT_CONNECTED)
              {
                 nAnswer[0]='0';
                 ecomBuildResponse(ucReq, ucResp, 1, nAnswer, EQ_EJECTOR_NOT_CONNECTED);
              }
              else
              {
                 nAnswer[0]='1';
                 ecomBuildResponse(ucReq, ucResp, 1, nAnswer, EQ_STATUS_CMD_OK);
              }
           }
           else
               // EquipmentNo out of range !!
              ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
        }
        else if (iEqType == EQ_TYPE_CONTROL_BOX_SW_VER)                //Software version
          ecomBuildResponse(ucReq, ucResp, 4, pVersion, EQ_STATUS_CMD_OK);
        else if (iEqType == EQ_TYPE_EJECTOR_SWITCH && iEqNo >= 1 && iEqNo  <= 16) //ejector switch
        {
           if (motorIsConnected(iEqNo) == MOTOR_NOT_CONNECTED)
           {
              nAnswer[0]='0';
              ecomBuildResponse(ucReq, ucResp, 1,  nAnswer, EQ_EJECTOR_NOT_CONNECTED);
            }
            else
            {
                nAnswer[0]=GET_Button(iEqNo);
                if(GET_Button(iEqNo)==0)
                {
                   nAnswer[0]='0';
                   ecomBuildResponse(ucReq, ucResp, 1, nAnswer, EQ_STATUS_EJ_SWITCH_DISABLE);
                }
                else if(buttonPusherSwitchOnceMeasuring(iEqNo)== BUTTON_NOT_PUSHED)
                {
                   nAnswer[0]='0';
                   ecomBuildResponse(ucReq, ucResp, 1, nAnswer, EQ_STATUS_CMD_OK);
                }
                else
                {
                   nAnswer[0]='1';
                   ecomBuildResponse(ucReq, ucResp, 1, nAnswer, EQ_STATUS_CMD_OK);
                }
            }
        }
        else
          ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
        usbqWriteUsbMsg(ucResp);
        break;

      default:
            // Build Response "Invalid command in request"
        ecomBuildAckMsg(ucReq, ucResp, EQ_STATUS_INVALID_COMMAND);
            // Send Response
        usbqWriteUsbMsg(ucResp);
        break;
    }  // End "switch"

}

//////////////////////////////////////////////////////////////////////////////
// Function     : Enable_Ejswitch()
// Purpose      :
// Parameters   : None
// Return value : None
// Created      : 2006-09-22 by TAK
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////

void Enable_Ejswitch(int aCommand,int aEqNo,unsigned char *aStatus)
{
   int aOpenLedDirection;
   int aLedOpen;
   int i;

   aOpenLedDirection = motorGetDirection(aEqNo);
   if (aOpenLedDirection == FORWARD) // opposite direction
   {
      aOpenLedDirection = REVERSE;
   }
   else
   {
      aOpenLedDirection = FORWARD;
   }
   for ( i = 0; i < 10; i++)
   {
      if (i == 4)  //not succesed, change direction
      {
         if (aOpenLedDirection == FORWARD)
         {
            aOpenLedDirection = REVERSE;
         }
         else
         {
            aOpenLedDirection = FORWARD;
         }
      }
      aLedOpen = isLedOpen(aEqNo);
      if (aLedOpen == BUTTON_NOT_PUSHED)  // finished
      {
          memcpy(aStatus,&EQ_STATUS_CMD_OK,2);
          ActivateButton(aEqNo);
          break;
      }
      else if (aLedOpen == 2)  // one motor is running, can't check Led opning
      {
          memcpy(aStatus,&EQ_STATUS_MOTOR_BUSY,2);
          break;
      }
      if (motorNotInUse( aEqNo)) // Activate given move command for motors not running
      {
         if ((motorStartKoggMeasure( aEqNo, 10, NOT_REGULATE_SPEED))) // Motor has not been started, allocate HW and trig step measuring:
         {
            motorSaveCommand(aEqNo, aCommand); // Save given move command in motor struct

            // Start of measuring succeeded, so ADC and Bridge are available, start motor:
            motorStart( aEqNo , aOpenLedDirection, 100, 0, 0);
            vTaskDelay(4);
            motorStop(aEqNo, STOP_LED_OPEN_SCAN);
            motorStopKoggMeasure( aEqNo);
            motorReset(aEqNo - 1);
            vTaskDelay(30);
         }
         else if (i >= 10)
         {
            // Build Response "HW Allocation Error"
            memcpy(aStatus,&EQ_STATUS_EJ_SWITCH_FAILED_HW_IN_USE,2);
            break;
         }
         else
         {
            vTaskDelay(34);
         }
      }
      else
      {
         // Build Response "Motor allready running"
         memcpy(aStatus,&EQ_STATUS_MOTOR_BUSY,2);
         break;
      }
   }
   if (i >= 10)
   {
      for(i = 0; i < 20 ; i++)    //wait extra 760 ms , total 1000ms to check button
      {
         vTaskDelay(38);
         aLedOpen = isLedOpen(aEqNo);
         if (aLedOpen == BUTTON_NOT_PUSHED)  // finished
         {
            memcpy(aStatus,&EQ_STATUS_CMD_OK,2);
            ActivateButton(aEqNo);
            break;
         }
      }
      if (i >= 20)
      {
         memcpy(aStatus,&EQ_STATUS_EJ_SWITCH_FAILED_LIGHT_SWITCH,2);
      }
   }
}


//////////////////////////////////////////////////////////////////////////////
// Function     : ExecuteAdressErrorCommand()
// Purpose      : To execute commands received from PC (used only for Adress Error Configuration)
// Parameters   : None
// Return value : None
// Created      : 2005-12-14 by TAK
// Updated      :
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ExecuteAdressErrorCommand(unsigned char* ucReq)
{
  unsigned char ucResp[USB_QUEUE_ITEM_LENGTH];
  static int    iCommand;
  static int    iEqType;

  iCommand = ecomGetCommand(ucReq);
  iEqType = ecomGetEquipmentType(ucReq);



  switch(iCommand)
  {
    /////////////////////////////////////////////////////////
    // Software Reset Request
    /////////////////////////////////////////////////////////
    case RESTART_SOFTWARE:
      if (iEqType == EQ_TYPE_CONTROL_BOX)
         SoftwareReset();
      else
      {
        ecomBuildResponse(ucReq, ucResp, 0, "", EQ_STATUS_EQUIPMENT_NO_ERROR);
        usbqWriteUsbMsg(ucResp);
      }
      break;


      default:
            // Build Response "Invalid command in request"
        ecomBuildAckMsg(ucReq, ucResp, EQ_STATUS_INVALID_COMMAND);
            // Send Response
        usbqWriteUsbMsg(ucResp);
        break;
    }  // End "switch"

}


void vIoCtrl( void *pvParameters )
{
   char Board_type;
   /* Just to stop compiler warnings. */
  ( void ) pvParameters;

   // Init common GPIO pins for Sensor and Ejector
   Init_GPIO_Common();

   m_nUnitID = CalculateUnitID_fromADC();

   Board_type=Card_ID_check();
   // get unit ID


   // Check about this unit is Ejector or Sensor

   if ((m_nUnitID != 1 && m_nUnitID != 7 && m_nUnitID != 13 && m_nUnitID != ERROR_ID) && Board_type==0 )  // EJECTOR
   {
      timer1_init(SW_TYPE_EJECTOR);
      Init_GPIO_for_EjectorUnit();
      ResetButtons();
      EjectorControlTaskRun();
   }
   else if ((m_nUnitID == 1 || m_nUnitID == 7 || m_nUnitID == 13 ) && Board_type==1 ) // SENSOR
   {
      timer1_init(SW_TYPE_SENSOR);
      Stop_Fall_1();
      Stop_Fall_2();
      Start_Fall_3();
      Stop_Tilt();
      Init_I2C( );
      IdentifyTemperatureSensorsType();
      timer0_init(); //Start timer for fallsensor
      Init_GPIO_for_SensorUnit();
      Init_InetrruptsForSensorUnit();   // Init extern interrupt related to sensors
     // I2C_Start_after_Reset();
      Usart_init (  );                  //INIT 485
      SensorControlTaskRun();
   }
   else
     ErrorControlTaskRun();
}


portBASE_TYPE xAreIoCtrlTaskStillRunning( void )
{
portBASE_TYPE xReturn;

	/* If the count of successful reception loops has not changed than at
	some time an error occurred (i.e. a character was received out of sequence)
	and we will return false. */
	if( uxRxLoops == comINITIAL_RX_COUNT_VALUE )
	{
		xReturn = pdFALSE;
	}
	else
	{
		xReturn = pdTRUE;
	}

	/* Reset the count of successful Rx loops.  When this function is called
	again we expect this to have been incremented. */
	uxRxLoops = comINITIAL_RX_COUNT_VALUE;

	return xReturn;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : SoftwareReset()
// Purpose      : Reset of software.
// Parameters   : None
// Return value : None
// Created      : 2005-11-09 by ES
// Updated      : 2005-11-09 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void SoftwareReset (void)
{
    AT91PS_RSTC pReset = AT91C_BASE_RSTC;
    pReset->RSTC_RMR=0xa5000F00;
    pReset->RSTC_RCR=0xa500000D;

    return;
}

