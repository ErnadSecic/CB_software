//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.04
//////////////////////////////////////////////////////////////////////////////////////

// Include Standard LIB  files
#include "Board.h"

#include "Jupiter.h"
#include "Motor.h"
#include "ECom.h"
#include "Timer.h"
#include "Adc.h"
#include "Pwm.h"
#include "Jupiter.h"
#include "Sensors.h"
#include "Gpio.h"
#include "IRQ_LEVEL.h"
#include "IoCtrl.h"
#include "Fallsensor.h"

int sswType;     //  #define SW_TYPE_POD   0,   #define SW_TYPE_EJECTOR  1
extern void vTimer0ISREntry( void );
extern void vTimer1ISREntry( void );

unsigned long int uiCurrMax = 0;
unsigned long int uiCurrValue;


//////////////////////////////////////////////////////////////////////////////
// Internal variables for the Timers
//////////////////////////////////////////////////////////////////////////////


// Make room for extra timers:
static unsigned short SystemTimers[MAX_TIMERS];

// Each timer has a flag indicating timeout:
static unsigned char TimersFlags[MAX_TIMERS];

// Update every ms only by adjusting TIMERS_UPDATE_RATE
static unsigned char ucUpdateRate1 = TIMERS_UPDATE_RATE1;
//static unsigned char ucUpdateRate0 = TIMERS_UPDATE_RATE0;




//*------------------------- Internal Function --------------------------------
//*----------------------------------------------------------------------------
//* Function Name       : AT91F_TC_Open
//* Object              : Initialize Timer Counter Channel and enable its clock
//* Input Parameters    : <tc_pt> = TC Channel Descriptor Pointer
//*                       <mode> = Timer Counter Mode
//*                     : <TimerId> = Timer peripheral ID definitions
//* Output Parameters   : None
//*----------------------------------------------------------------------------
void AT91F_TC_Open ( AT91PS_TC TC_pt, unsigned int Mode, unsigned int TimerId,unsigned int RC)
{
   //* First, enable the clock of the TIMER
   AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< TimerId ) ;

    //* Disable the clock and the interrupts
	TC_pt->TC_CCR = AT91C_TC_CLKDIS ;
	TC_pt->TC_IDR = 0xFFFFFFFF ;

     TC_pt->TC_RC = RC;

   // Set the Mode of the Timer Counter
	TC_pt->TC_CMR = Mode ;

   // Enable the clock
	TC_pt->TC_CCR = AT91C_TC_CLKEN ;

    return;
}


//*------------------------- Interrupt Function -------------------------------

__arm void vTim0ISR(void);

//*----------------------------------------------------------------------------
//* Function Name       : vTimer0ISREntry
//* Object              : C handler interrupt function called by the interrupts
//*                       assembling routine
//* Output Parameters   : increment count_timer0_interrupt
//*----------------------------------------------------------------------------
__arm void vTim0ISR(void)
{	
   //* Acknowledge interrupt status
   AT91C_BASE_TC0->TC_SR;
   if (isFall3_Enabled() == 1)
   {
      CAP_IR_IRQ();
   }

   Switch_IR_LED_IRQ();

   AT91C_BASE_AIC->AIC_EOICR = 0;
}


__arm void vTim1ISR(void);

//*----------------------------------------------------------------------------
//* Function Name       : vTimer1ISREntry
//* Object              : C handler interrupt function called by the interrupts
//*                       assembling routine for Ejector Control
//* Output Parameters   : increment count_timer1_interrupt
//*----------------------------------------------------------------------------

__arm void vTim1ISR(void)
{
    //* Acknowledge interrupt status
   AT91C_BASE_TC1->TC_SR;

   if (sswType == SW_TYPE_EJECTOR)
   {
      motorOnTimerInterrupted_IRQ();
   }

   //Universial timer controll
   timerCtrl_IRQ();

   //Hartbeat LED
   ToggleBeam_IRQ();


   // End the interrupt in the AIC.
   AT91C_BASE_AIC->AIC_EOICR = 0;
}


//*-------------------------- External Function -------------------------------

//*----------------------------------------------------------------------------
//* Function Name       : timer0_init
//* Object              : Init timer0 counter
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------

void timer0_init ( void )
{
   unsigned int RC=255; //timer interval 0 170us

   AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC0, TIMER0_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, vTimer0ISREntry);

   // Open timer0

   AT91F_TC_Open(AT91C_BASE_TC0,(TC_CLKS_MCK32|WAVE_ENA|WAVESEL),AT91C_ID_TC0,RC);
   AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;  //  IRQ enable CPC
   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC0);

   //* Start timer0
  AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG ;

   return;
}

//*----------------------------------------------------------------------------
//* Function Name       : timer1_init
//* Object              : Init timer counter
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
void timer1_init (int swType)//void)
{
  unsigned int RC=300; //timer interval 0 200us
  sswType = swType;

   AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC1, TIMER1_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, vTimer1ISREntry);

   // Open timer1
   AT91F_TC_Open(AT91C_BASE_TC1,(TC_CLKS_MCK32|WAVE_ENA|WAVESEL),AT91C_ID_TC1,RC);

   AT91C_BASE_TC1->TC_IER  = AT91C_TC_CPCS;  //  IRQ enable CPC
   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC1);

   //* Start timer1
   AT91C_BASE_TC1->TC_CCR = AT91C_TC_SWTRG ;

   return;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : StartTimer2_init
// Purpose      : This function sets timer
// Parameters   : US to compare
// Return value :
// Created      : 2006-09-27 by TAK
// Updated      : 2006-09-27 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void StartTimer2_init (int us)
{
  unsigned int RC= (us *15)/10 ;

   AT91F_TC_Open(AT91C_BASE_TC2,(TC_CLKS_MCK32|WAVE_ENA|WAVESEL),AT91C_ID_TC2,RC);
   Timer2done();
   AT91C_BASE_TC2->TC_CCR = AT91C_TC_SWTRG ;
   return;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Timer2done
// Purpose      : This function is used to report if timer has compared
// Parameters   :
// Return value : done = true
// Created      : 2006-09-27 by TAK
// Updated      : 2006-09-27 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////

int Timer2done(void)
{
   if((AT91C_BASE_TC2->TC_SR & AT91C_TC_CPCS) !=0)
     return (1);
   else
     return(0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// X-TIMERS:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Function     : timerSet()
// Purpose      : To set and start a timer
// Parameters   : Timer id, count down value
// Return value : Result of operation, (TRUE/FALSE)
// Created      : 2005-10-20 by PVA
// Updated      : 2005-10-20 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
unsigned char timerSet(unsigned char ucTimerIdx, unsigned short uiValue)
{

  if (ucTimerIdx < MAX_TIMERS)
  {
      SystemTimers[ucTimerIdx] = uiValue;
      TimersFlags[ucTimerIdx] = 0;

      return(1);
  }
  else
  {
      return(0);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Function     : timerExpired()
// Purpose      : To test if a timer has expired
// Parameters   : Time ID
// Return value : Result of operation, (TRUE/FALSE)
// Created      : 2005-10-20 by PVA
// Updated      : 2005-10-20 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
unsigned char timerExpired(unsigned char ucTimerIdx)
{
  if( ucTimerIdx < MAX_TIMERS )
  {
     if (TimersFlags[ucTimerIdx])
     {
        TimersFlags[ucTimerIdx] = 0;
        return(1);
     }
  }
  return(0);
}


/////////////////////////////////////////////////////////////////////////////
// Function     : timerStop()
// Purpose      : To stop and deactivate a timer
// Parameters   : None
// Return value : Result of operation, (TRUE/FALSE)
// Created      : 2005-10-20 by PVA
// Updated      : 2005-10-20 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
unsigned char timerStop(unsigned char ucTimerIdx)
{
  if( ucTimerIdx < MAX_TIMERS )
  {
     // Reset Timer:
     SystemTimers[ucTimerIdx] = 0;

     // Clear Flag:
     TimersFlags[ucTimerIdx] = 0;

     return(1);
  }
  return(0);
}




/////////////////////////////////////////////////////////////////////////////
// Function     : timerRunning()
// Purpose      : To test if a timer is running
// Parameters   : Time ID
// Return value : Result of operation, (TRUE if timer is running)
// Created      : 2005-10-20 by PVA
// Updated      : 2005-10-20 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
unsigned char timerRunning(unsigned char ucTimerIdx)
{
  if( ucTimerIdx < MAX_TIMERS )
  {
     if ( SystemTimers[ucTimerIdx] > 0)
     {
        return(1);
     }
  }
  return(0);
}




/////////////////////////////////////////////////////////////////////////////
// Function     : timerCtrl()
// Purpose      : To update System Clock and Timers.
// Parameters   : None
// Return value : None
// Created      : 2005-10-20 by PVA
// Updated      : 2005-10-20 by PVA
// Comments     : Should be updated every 10 ms
//////////////////////////////////////////////////////////////////////////////
void timerCtrl_IRQ( void)
{
   unsigned char ucTimerIdx;

   ucUpdateRate1--;
   if (ucUpdateRate1 == 0)
   {
      // 1 ms elapsed:
 	
      // Update Timers:
      for ( ucTimerIdx = 0; ucTimerIdx < MAX_TIMERS; ucTimerIdx++)
      {
         if ( SystemTimers[ucTimerIdx] > 0)
         {
            SystemTimers[ucTimerIdx]--;
            if (SystemTimers[ucTimerIdx] == 0)
            {
               // Send TimeOut Event to actual process/Module:
			   TimersFlags[ucTimerIdx] = 1;
            }
         }
      }

      ucUpdateRate1 = TIMERS_UPDATE_RATE1;  // 10 ms before next update
   }
}



