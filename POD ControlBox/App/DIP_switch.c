#include "adc.h"
#include "DIP_switch.h"




//////////////////////////////////////////////////////////////////////////////
// Function     : CalculateUnitID_fromADC()
// Purpose      : Find dip adress
// Parameters   :
// Return value : Dip Adress
// Created      : 2005-11-09 by ES
// Updated      : 2005-12-14 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////

unsigned char CalculateUnitID_fromADC( void )
{
  unsigned long ulDipSwitch=0;

  for (int i = 0; i < 100; i++)   //Må måle mange ganger for å få riktig måling
    ulDipSwitch = adcRead(ADC_CH7);

  ulDipSwitch = ulDipSwitch >> 5;


  if(ulDipSwitch == 0 || ulDipSwitch > 18)
    return ERROR_ID;
  else
    return ulDipSwitch;
}

