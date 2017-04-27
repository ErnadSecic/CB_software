
//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : main.c
//* Object              : main application written in C
//* Creation            : JPP   16/Jun/2004
//*----------------------------------------------------------------------------


// Include Standard files
#include "Board.h"
#include "GpIO.h"
#include "Ext_Irq.h"
#include "Pwm.h"
#include "Adc.h"
#include "Timer.h"
#include "Motor.h"

/* Global variables */
#define SPEED 		(MCKKHz/10)
unsigned int LedSpeed = SPEED *50 ;
const int led_mask[8]= {LED1, LED2, LED3, LED4};

void change_speed ( void );
void wait ( void );

//*--------------------------------------------------------------------------------------
//* Function Name       : change_speed
//* Object              : Adjust "LedSpeed" value depending on SW1 and SW2 are pressed or not
//* Input Parameters    : none
//* Output Parameters   : Update of LedSpeed value.
//*--------------------------------------------------------------------------------------
void change_speed ( void )
{//* Begin
    if ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & SW1_MASK) == 0 )
    {
        if ( LedSpeed > SPEED ) LedSpeed -=SPEED ;
    }
    if ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & SW3_MASK) == 0 )
    {
        if ( LedSpeed < MCK ) LedSpeed +=SPEED ;
    }
}//* End

//*--------------------------------------------------------------------------------------
//* Function Name       : wait
//* Object              : Software waiting loop
//* Input Parameters    : none. Waiting time is defined by the global variable LedSpeed.
//* Output Parameters   : none
//*--------------------------------------------------------------------------------------
void wait ( void )
{//* Begin
    unsigned int waiting_time ;
    change_speed () ;
    for(waiting_time = 0; waiting_time < LedSpeed; waiting_time++) ;
}//* End

void TestHW ( void)
{
  int iTotalCnt = 0;
  int iDummy = 0;;
  int iDirection;
  int iStop;
  int iDutyCycle;
  int iSteps;
  int iPulseLimit;

  unsigned long uiAdcValue0 = 0;
  unsigned long uiAdcValue1 = 0;
  unsigned long uiAdcValue2 = 0;
  unsigned long uiAdcValue3 = 0;
  unsigned long uiAdcValue4 = 0;
  unsigned long uiAdcValue5 = 0;

  gpioClear(PWM_MASK);
  gpioSet(PWM_MASK);
  gpioClear(PWM_MASK);


  // Test Motor direction pins MOT_1 .. MOT_16:
   gpioClear(MOT_1);
   gpioSet(MOT_1);


  // Test motor pulse selection pins EJSSCT1 .. EJSSCT4:
   gpioClear(EJSSCT1);
   gpioSet(EJSSCT1);

   // Init and disable all PWM outlets !!!!
   pwmInit(MCOM_HI_A_CHAN,0);
   pwmStop( MCOM_HI_A_CHAN);
   pwmInit(MCOM_LO_A_CHAN,0);
   pwmStop( MCOM_LO_A_CHAN);

   pwmInit(MCOM_HI_B_CHAN,0);
   pwmStop( MCOM_HI_B_CHAN);
   pwmInit(MCOM_LO_B_CHAN,0);
   pwmStop( MCOM_LO_B_CHAN);


/*
   gpioClear(BRIDGE_B_MASK);
   motorSetDirection( 1, 1);
   pwmInit(MCOM_LO_A_CHAN,25);
   pwmStart( MCOM_LO_A_CHAN);


   pwmUpdate( MCOM_LO_A_CHAN, 50);
   pwmUpdate( MCOM_LO_A_CHAN, 75);
   pwmUpdate( MCOM_LO_A_CHAN, 25);
   pwmStop( MCOM_LO_A_CHAN);

while(1)
{
      // Test ADC ADC_CH0 .. ADC_CH4:
      uiAdcValue0 = adcRead(ADC_CH4);
}
***/

  iDirection = 0;
  iStop = 0;
  iDutyCycle = 100;
  iSteps = 300;
  iPulseLimit = 40;

      // Test Motor:
      motorSetStepCounter( 1 , iSteps);
      motorStart( 1 , iDirection, iDutyCycle);       // #define MOTOR_FORWARD   0,   #define REVERSE   1

while(1)
{

    if ((motorFinished( 1, &iTotalCnt, &iDirection)))
    {
         if (iDirection == 1)
           iDirection = 0;
         else
           iDirection = 1;
         motorSetStepCounter( 1 , iSteps);
         motorStart( 1 , iDirection, iDutyCycle);
     }
}

while(1)
{
   if ((motorFinished( 1, &iTotalCnt, &iDirection)))
   {
      iDummy++;
       motorStop( 1);
   }
   else
   {
      iDummy++;
   }
}


/*****************
   motorStop(1);

   if ((motorFinished( 1, &iTotalCnt, &iDirection)))
   {
      iDummy++;
   }
   else
   {
      iDummy++;
   }
********************/
}





//*--------------------------------------------------------------------------------------
//* Function Name       : Main
//* Object              : Software entry point
//* Input Parameters    : none.
//* Output Parameters   : none.
//*--------------------------------------------------------------------------------------
   AT91S_PMC     *p_PMC;
   AT91S_PIO     *p_PIOA;
   AT91S_PWMC    *p_PWMC;
   AT91S_ADC     *p_ADC;

int main()
{
    int ii;


   p_PMC = AT91C_BASE_PMC;
   p_PIOA = AT91C_BASE_PIOA;
   p_PWMC =  AT91C_BASE_PWMC;
   p_ADC  =  AT91C_BASE_ADC;


	/* When using the JTAG debugger the hardware is not always initialised to
	the correct default state.  This line just ensures that this does not
	cause all interrupts to be masked at the start. */
    AT91C_BASE_AIC->AIC_EOICR = 0;
	/* Most setup is performed by the low level init function called from the
	startup asm file. */


   ii = 0;

   gpioInit();
   adcInit();
   timer1_init();
   //init_INT_PMOV();

   //pwmInit(MCOM_LO_A_CHAN,50);
   //pwmStart( MCOM_LO_A_CHAN);
  // pwmStop( MCOM_LO_A_CHAN);

    // Loop forever
    for (;;)
    {

      TestHW();

/****
       motorSetDirection( 16, 0);

      // Once a Shot on each led
	   for ( ii=0 ; ii < NB_LEB ; ii++ )
       {
	      wait();
       }// End for

       motorSetDirection( 16, 1);

        // Once a Shot on each led
       for ( ii=(NB_LEB-1) ; ii >= 0 ; ii-- )
       {
	       wait();
       }
*****************************/

    }

}
