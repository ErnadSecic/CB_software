//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.04
//////////////////////////////////////////////////////////////////////////////////////
#include <board.h>
#include "FreeRTOS.h"
#include "task.h"
#include "adc.h"
#include "Gpio.h"
#include "Motor.h"



//////////////////////////////////////////////////////////////////////////////
// Function     : adcInit()
// Purpose      : To initialize the ADC
// Parameters   : None
// Return value : None
// Created      : 2005-09-17 by
// Updated      : 2005-09-17 by
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void adcInit(void)
{
   AT91F_ADC_CfgModeReg(AT91C_BASE_ADC,0x08000200);

}


//////////////////////////////////////////////////////////////////////////////
// Function     : adcRead()
// Purpose      : To read ADC
// Parameters   : ADC Channel
// Return value : Read value
// Created      : 2005-09-17 by
// Updated      : 2005-09-17 by
// Comments     :
//////////////////////////////////////////////////////////////////////////////
 //temp C=(((adc inn/1025)*3300)-480)/15,6

unsigned long int adcRead(int ADC_Channel)
{
   unsigned int answer=0;
   AT91PS_ADC padc = AT91C_BASE_ADC;

   switch(ADC_Channel)
   {
      case ADC_CH0:
         AT91F_ADC_EnableChannel(AT91C_BASE_ADC,AT91C_ADC_CH0);      // Enable this channel only
         answer=AT91F_ADC_GetConvertedDataCH0 (AT91C_BASE_ADC);      // Clean up by reading old value
         answer=0;
         AT91F_ADC_StartConversion(AT91C_BASE_ADC);                  // Start conversion

         while((padc->ADC_SR & AT91C_ADC_EOC0)==0)                   // Wait on "End Of Conversion Flag"

         {
         }
         answer=AT91F_ADC_GetConvertedDataCH0 (AT91C_BASE_ADC);      // Pick up converted ADC value
         AT91F_ADC_DisableChannel(AT91C_BASE_ADC,AT91C_ADC_CH0);     // Disable this channel
      break;

      case ADC_CH1:
         AT91F_ADC_EnableChannel(AT91C_BASE_ADC,AT91C_ADC_CH1);      // Enable this channel only
         answer=AT91F_ADC_GetConvertedDataCH1 (AT91C_BASE_ADC);
         answer=0;
         AT91F_ADC_StartConversion(AT91C_BASE_ADC);
         while((padc->ADC_SR & AT91C_ADC_EOC1)==0)
         {
         }
         answer=AT91F_ADC_GetConvertedDataCH1 (AT91C_BASE_ADC);
         AT91F_ADC_DisableChannel(AT91C_BASE_ADC,AT91C_ADC_CH1);     // Disable this channel
      break;
      case ADC_CH2:
         AT91F_ADC_EnableChannel(AT91C_BASE_ADC,AT91C_ADC_CH2);      // Enable this channel only
         answer=AT91F_ADC_GetConvertedDataCH2 (AT91C_BASE_ADC);
         answer=0;
         AT91F_ADC_StartConversion(AT91C_BASE_ADC);
         while((padc->ADC_SR & AT91C_ADC_EOC2)==0)
         {
         }
         answer=AT91F_ADC_GetConvertedDataCH2 (AT91C_BASE_ADC);
         AT91F_ADC_DisableChannel(AT91C_BASE_ADC,AT91C_ADC_CH2);     // Disable this channel
      break;
      case ADC_CH3:
         AT91F_ADC_EnableChannel(AT91C_BASE_ADC,AT91C_ADC_CH3);      // Enable this channel only
         answer=AT91F_ADC_GetConvertedDataCH3 (AT91C_BASE_ADC);
         answer=0;
         AT91F_ADC_StartConversion(AT91C_BASE_ADC);
         while((padc->ADC_SR & AT91C_ADC_EOC3)==0)
         {
         }
         answer=AT91F_ADC_GetConvertedDataCH3 (AT91C_BASE_ADC);
         AT91F_ADC_DisableChannel(AT91C_BASE_ADC,AT91C_ADC_CH3);     // Disable this channel
      break;
      case ADC_CH4:
         AT91F_ADC_EnableChannel(AT91C_BASE_ADC,AT91C_ADC_CH4);      // Enable this channel only
         answer=AT91F_ADC_GetConvertedDataCH4 (AT91C_BASE_ADC);
         answer=0;
         AT91F_ADC_StartConversion(AT91C_BASE_ADC);
         while((padc->ADC_SR & AT91C_ADC_EOC4)==0)
         {
         }
         answer=AT91F_ADC_GetConvertedDataCH4 (AT91C_BASE_ADC);
         AT91F_ADC_DisableChannel(AT91C_BASE_ADC,AT91C_ADC_CH4);     // Disable this channel
      break;
      case ADC_CH5:
         AT91F_ADC_EnableChannel(AT91C_BASE_ADC,AT91C_ADC_CH5);      // Enable this channel only
         answer=AT91F_ADC_GetConvertedDataCH5 (AT91C_BASE_ADC);
         answer=0;
         AT91F_ADC_StartConversion(AT91C_BASE_ADC);
         while((padc->ADC_SR & AT91C_ADC_EOC5)==0)
         {
         }
         answer=AT91F_ADC_GetConvertedDataCH5 (AT91C_BASE_ADC);
         AT91F_ADC_DisableChannel(AT91C_BASE_ADC,AT91C_ADC_CH5);     // Disable this channel
      break;
      case ADC_CH6:
         AT91F_ADC_EnableChannel(AT91C_BASE_ADC,AT91C_ADC_CH6);      // Enable this channel only
         answer=AT91F_ADC_GetConvertedDataCH6 (AT91C_BASE_ADC);
         answer=0;
         AT91F_ADC_StartConversion(AT91C_BASE_ADC);
         while((padc->ADC_SR & AT91C_ADC_EOC6)==0)
         {
         }
         answer=AT91F_ADC_GetConvertedDataCH6 (AT91C_BASE_ADC);
         AT91F_ADC_DisableChannel(AT91C_BASE_ADC,AT91C_ADC_CH6);     // Disable this channel
      break;
      case ADC_CH7:
         AT91F_ADC_EnableChannel(AT91C_BASE_ADC,AT91C_ADC_CH7);      // Enable this channel only
         answer=AT91F_ADC_GetConvertedDataCH7 (AT91C_BASE_ADC);
         answer=0;
         AT91F_ADC_StartConversion(AT91C_BASE_ADC);
         while((padc->ADC_SR & AT91C_ADC_EOC7)==0)
         {
         }
         answer=AT91F_ADC_GetConvertedDataCH7 (AT91C_BASE_ADC);
         AT91F_ADC_DisableChannel(AT91C_BASE_ADC,AT91C_ADC_CH7);     // Disable this channel
      break;

        default:
        break;
   }
   return answer;
}



