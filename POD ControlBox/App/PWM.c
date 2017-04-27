//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.01
//////////////////////////////////////////////////////////////////////////////////////


#include "board.h"
#include "pwm.h"






//////////////////////////////////////////////////////////////////////////////
// Function     : pwmStart()
// Purpose      : To start a PWM channel
// Parameters   : PWM Channel
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void pwmStart( int iPWM_Channel)
{
   switch(iPWM_Channel)
   {
   case PWM_CH0:
      AT91F_PWMC_StartChannel(AT91C_BASE_PWMC,PWM_ID_CH0);    // Use this to start PWM
      break;

   case PWM_CH1:
      AT91F_PWMC_StartChannel(AT91C_BASE_PWMC,PWM_ID_CH1);    // Use this to start PWM
      break;

   case PWM_CH2:
      AT91F_PWMC_StartChannel(AT91C_BASE_PWMC,PWM_ID_CH2);    // Use this to start PWM
      break;

   case PWM_CH3:
      AT91F_PWMC_StartChannel(AT91C_BASE_PWMC,PWM_ID_CH3);    // Use this to start PWM
      break;

   default:
      break;
   }

   return;
}



//////////////////////////////////////////////////////////////////////////////
// Function     : pwmStop()
// Purpose      : To stop a PWM channel
// Parameters   : PWM Channel
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
__interwork void pwmStop( int iPWM_Channel)
{

   switch(iPWM_Channel)
   {
   case PWM_CH0:
      AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,PWM_ID_CH0);    // Use this to start PWM
      break;

   case PWM_CH1:
      AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,PWM_ID_CH1);    // Use this to start PWM
      break;

   case PWM_CH2:
      AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,PWM_ID_CH2);    // Use this to start PWM
      break;

   case PWM_CH3:
      AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,PWM_ID_CH3);    // Use this to start PWM
      break;

   default:
     break;
   }

   return;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : pwmUpdate()
// Purpose      : To update a PWM channel
// Parameters   : PWM Channel
// Return value : None
// Created      : 2005-09-17 by
// Updated      : 2005-09-17 by
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void pwmUpdate( int iPWM_Channel, int iPercentLoad)
{
  unsigned int uiNewPwmDuty;
//AT91C_BASE_PWMC->PWMC_CH[iPWM_Channel].PWMC_CMR = 0;
   uiNewPwmDuty = PWM_PERIOD - (PWM_PERIOD/100) * iPercentLoad;

   AT91F_PWMC_UpdateChannel( AT91C_BASE_PWMC, iPWM_Channel, uiNewPwmDuty);

   return;
}


//////////////////////////////////////////////////////////////////////////////
// Function     : pwmInit()
// Purpose      : To initialize a PWM channel
// Parameters   : PWM Channel
// Return value : None
// Created      : 2005-09-17 by
// Updated      : 2005-09-17 by
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void pwmInit(int iPWM_Channel, int iPercentLoad)
{
  unsigned int uiPwmDuty;

  uiPwmDuty =  PWM_PERIOD - (PWM_PERIOD/100) * iPercentLoad;

  //Config PWM CH0-CH3 5 hz for Motor PWM
   switch(iPWM_Channel)
   {
   case PWM_CH0:
      AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,PWM_ID_CH0);
      AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC, PWM_PHRIPH_CLOCK);
      AT91F_PIO_CfgPeriph(
		   AT91C_BASE_PIOA,         // PIO controller base address
		   0,                       // Peripheral A
		   (PWM0_PA23_OUT) );       // Peripheral B

      AT91F_PWMC_CfgChannel(AT91C_BASE_PWMC,PWM_CH0,PWM_MODE,PWM_PERIOD,uiPwmDuty);
   break;

   case PWM_CH1:
      AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,PWM_ID_CH1);
      AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC,PWM_PHRIPH_CLOCK);
      AT91F_PIO_CfgPeriph(
		   AT91C_BASE_PIOA,         // PIO controller base address
		   0,                       // Peripheral A
		   (PWM1_PA24_OUT) );       // Peripheral B

      AT91F_PWMC_CfgChannel(AT91C_BASE_PWMC,PWM_CH1,PWM_MODE,PWM_PERIOD,uiPwmDuty);
     break;
   case PWM_CH2:
      AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,PWM_ID_CH2);
      AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC,PWM_PHRIPH_CLOCK);
      AT91F_PIO_CfgPeriph(
		   AT91C_BASE_PIOA,         // PIO controller base address
		   0,                       // Peripheral A
		   (PWM2_PA13_OUT) );       // Peripheral B

      AT91F_PWMC_CfgChannel(AT91C_BASE_PWMC,PWM_CH2,PWM_MODE,PWM_PERIOD,uiPwmDuty);
     break;
   case PWM_CH3:
      AT91F_PWMC_StopChannel(AT91C_BASE_PWMC,PWM_ID_CH3);
      AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC,PWM_PHRIPH_CLOCK);
      AT91F_PIO_CfgPeriph(
		   AT91C_BASE_PIOA,         // PIO controller base address
		   0,                       // Peripheral A
		   (PWM3_PA14_OUT) );       // Peripheral B

      AT91F_PWMC_CfgChannel(AT91C_BASE_PWMC,PWM_CH3,PWM_MODE,PWM_PERIOD,uiPwmDuty);
     break;
   default:
     break;
   }
   return;
}
