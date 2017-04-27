
//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.01
//////////////////////////////////////////////////////////////////////////////////////


// Include Standard LIB  files
#include "Board.h"
#include "Gpio.h"
#include "Jupiter.h"
#include "ext_irq.h"
#include "Sensors.h"
#include "Timer.h"
#include "IRQ_LEVEL.h"

extern void vExternISREntry( void);

extern  SENSOR_FLAGS  SENSOR_FLAGbits;


//-----------------------------------------------------------------------------
// Function     : vExt_ISR(void)
// Purpose      : To detect interrupts on PRODREL and SHUTEMP pin
// Created      : 2005-10-28 by ES
// Updated      : 2006-01-05 by TAK
// Comments     :
//-----------------------------------------------------------------------------
__arm void vExt_ISR(void)
{
    int interruptRegister;
    interruptRegister = AT91C_BASE_PIOA->PIO_ISR;


    if (interruptRegister & FALL2_INPUT_BIT) // Fall2
    {
       OnFALL2_Interrupted_IRQ();
    }

//PS..    if (interruptRegister & FALL1_INPUT_BIT)  //Fall1
//PS..    {
//PS..       OnFALL1_Interrupted_IRQ();
//PS..    }

    if (interruptRegister & TILTS_BIT)
    {
       OnTILT_Interrupted_IRQ();
    }

   // Get the signal that this interrupt has been finished.
   AT91C_BASE_AIC->AIC_EOICR = 0;
}


void Init_InetrruptsForSensorUnit(void)
{
   AT91PS_AIC     pAic;
   pAic = AT91C_BASE_AIC;

//PS..   AT91F_AIC_ConfigureIt ( pAic, AT91C_ID_PIOA, PIO_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, vExternISREntry);
//PS..   AT91F_PIO_InterruptEnable(AT91C_BASE_PIOA, (/*FALL1_INPUT_BIT |*/ FALL2_INPUT_BIT | TILTS_BIT));
//PS..   AT91F_PIO_InterruptDisable(AT91C_BASE_PIOA, ~(/*FALL1_INPUT_BIT |*/ FALL2_INPUT_BIT | TILTS_BIT));
	//* set the interrupt by software
//PS..   AT91F_AIC_EnableIt (pAic, AT91C_ID_PIOA);
}
