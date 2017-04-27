//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.01
//////////////////////////////////////////////////////////////////////////////////////


#include "board.h"
#include "GpIO.h"
#include "I2C_interface.h"
#include "Timer.h"
#include "pwm.h"
#include "Sensors.h"

static  int iTestOn = -1;

void ToggleBeam_IRQ(void)
{
   if (iTestOn == -1) // the firste time
   {
     iTestOn = 0;
     timerSet(INX_TOGLE_BEAM_TIMER, TIME_1000_MS);
   }

   if (iTestOn == 0)
   {
      if (timerExpired(INX_TOGLE_BEAM_TIMER))
      {
        gpioSet(BEAM_BIT);
        iTestOn = 1;
        timerSet(INX_TOGLE_BEAM_TIMER, TIME_1000_MS);
      }
   }
   else if (iTestOn == 1)
   {
      if (timerExpired(INX_TOGLE_BEAM_TIMER))
      {
          gpioClear(BEAM_BIT);
          iTestOn = 0;
          timerSet(INX_TOGLE_BEAM_TIMER, TIME_1000_MS);
      }
   }

   return;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//              SENSOR AND EJECTOR CONTROLOR
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//////////////////////////////////////////////////////////////////////////////
// Function     : Init_GPIO_Common()
// Purpose      :
// Parameters   : None
// Return value : None
// Created      : 2005-10-28 by ES
// Updated      : 2005-10-28 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Init_GPIO_Common(void)
{
  ///////////////////////////////////////////////////////
  // SWITCH OFF PULL-UP ON THE ALL PINS (DEFULT IS ON)
  // PUDR is  Pull-up Disable Register
  ///////////////////////////////////////////////////////
  AT91C_BASE_PIOA->PIO_PPUDR = 0xFFFFFFFF;

  /////////////////////////////////////////////////////////////////////////////
  // Enable the peripheral clock for Parallel IO Controller. Needs for
  // INTERRUPTS ON IO-pins
  /////////////////////////////////////////////////////////////////////////////
  AT91F_PMC_EnablePeriphClock( AT91C_BASE_PMC, 1 << AT91C_ID_PIOA );
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Init_GPIO_for_SensorUnit()
// Purpose      : Initialising for Sensor Control
// Parameters   : None
// Return value : None
// Created      : 2005-10-28 by ES
// Updated      : 2006-03-20 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Init_GPIO_for_SensorUnit(void)
{
   AT91C_BASE_PIOA->PIO_PER =   ( HATCH_INPUT_BIT | /*FALL1_INPUT_BIT |*/ BEAM_BIT | FALL2_INPUT_BIT | DOOROPN_BIT| RS485_RSEL_BIT |
                            //     RS485_DENA_BIT | LAMP_MASK | FALL_OUT_MASK | TILTS_BIT | FALL1_INPUT_AD1 | NC_SENS_MASK); // Set in PIO mode
                                 RS485_DENA_BIT | LAMP_MASK | FALL_OUT_MASK | TILTS_BIT | NC_SENS_MASK);

   AT91C_BASE_PIOA->PIO_ODR =   ( HATCH_INPUT_BIT | /*FALL1_INPUT_BIT |*/ FALL2_INPUT_BIT | DOOROPN_BIT | TILTS_BIT | FALL1_INPUT_AD1 ); // input

   AT91C_BASE_PIOA->PIO_OER =   (BEAM_BIT | LAMP_MASK | FALL_OUT_MASK |
                                 RS485_RSEL_BIT | RS485_DENA_BIT | NC_SENS_MASK); // output

   gpioClearBit                 ( BEAM_BIT | RS485_RSEL_BIT | RS485_DENA_BIT | FALL_OUT_MASK | NC_SENS_MASK);
   gpioSetBit                   ( LAMP_MASK );

}

//////////////////////////////////////////////////////////////////////////////
// Function     : Card_ID_check
// Purpose      : Check type of board
// Parameters   : None
// Return value : None
// Created      : 2006-03-20 by TAK
// Updated      : 2006-03-20 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char Card_ID_check(void)
{
   char id;
   int i;
   AT91C_BASE_PIOA->PIO_PER =   (ID_BIT);
   AT91C_BASE_PIOA->PIO_ODR =   (ID_BIT);
   for(i= 0; i <5000; i++)
     __asm("nop");
   id=Sensor_In(ID_BIT);
   return(id);  //0 = motor board , 1 = sensor board
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Init_GPIO_for_EjectorUnit()
// Purpose      : Initialising for Ejector Control
// Parameters   : None
// Return value : None
// Created      : 2005-10-28 by PV
// Updated      : 2006-03-20 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Init_GPIO_for_EjectorUnit( void )
{
   AT91PS_PIO pPio = AT91C_BASE_PIOA;             // pointer to PIOA controller
	

   /////////////////////////////////////////////////////////////////////////////
   // Select witch pins will be used as PIO and wich as PERIPERIAL
   /////////////////////////////////////////////////////////////////////////////
   pPio->PIO_PER = (MOTOR_MASK | EJSSCT_MASK | BEAM_BIT | HATCH_INPUT_BIT |/*FALL1_INPUT_BIT |*/ FALL2_INPUT_BIT | DOOROPN_BIT | ADC_MASK | NC_MOTO_MASK); // Set in PIO mode
   pPio->PIO_PDR = ((~MOTOR_MASK) & (~EJSSCT_MASK) & (~BEAM_BIT) & (~HATCH_INPUT_BIT) & /*(~FALL1_INPUT_BIT) &*/ (~FALL2_INPUT_BIT) & (~DOOROPN_BIT) & (~ADC_MASK) & (~NC_MOTO_MASK)); // Set other as periphial

   /////////////////////////////////////////////////////////////////////////////
   // DIRECTION
   /////////////////////////////////////////////////////////////////////////////
   pPio->PIO_OER = (MOTOR_MASK | EJSSCT_MASK | BEAM_MASK | NC_MOTO_MASK);              // Output Enable Register     Configure as Output
   pPio->PIO_ODR = ((~MOTOR_MASK) & (~EJSSCT_MASK) & (~BEAM_MASK) & (~NC_MOTO_MASK));     // Output Disable Register     Configure as Input
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   gpioClearBit                 (NC_SENS_MASK);

   return;
}



int gpioSet(unsigned int uiSetBit)
{
   if (uiSetBit & (MOTOR_MASK | EJSSCT_MASK | BEAM_MASK))
   {
      AT91F_PIO_SetOutput( AT91C_BASE_PIOA, uiSetBit);
      return(1);
   }
   else
   {
      return(0);
   }
}


int gpioClear(unsigned int uiClearBit)
{
   if (uiClearBit & (MOTOR_MASK | EJSSCT_MASK | BEAM_MASK))
   {
      AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, uiClearBit);
      return(1);
   }
   else
   {
      return(0);
   }

}

void gpioSetBit(unsigned int uiSetBit)
{
   AT91F_PIO_SetOutput( AT91C_BASE_PIOA, uiSetBit);
}


void gpioClearBit(unsigned int uiClearBit)
{
   AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, uiClearBit);
}
