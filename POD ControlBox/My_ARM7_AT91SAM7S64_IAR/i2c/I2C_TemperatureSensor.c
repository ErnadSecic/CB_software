//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.01
//////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "I2C_TemperatureSensor.h"
#include "I2C_interface.h"

#include "FreeRTOS.h"
#include "task.h"


//////////////////////////////////////////////////////////////////////////////
// Function     : GetTemperature()
// Purpose      : To get temperatrure from I2C sensor
// Parameters   : pTemperature, pTemperatureLength,  nI2C_SensorID
// Return value : 1 on success, otherwise 0
// Created      : 2005-11-01 by ES
// Updated      : 2005-11-01 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int GetTemperature(unsigned char* pTemperature, int * pTemperatureLength,  int nI2C_SensorID,  char cSensorType)
{
  unsigned char pTempH;
  unsigned char pTempL;
  unsigned portCHAR pTemperature_temp[7];
  //unsigned char cTemp;
  //int i;
  int cPos;
  int nTemp;
  cPos = 0;

  //char cAC = 0xAC;
  char cStartConvert = 0xEE;

  //char cOnecMeass = 0x01;
  char cAA = 0xAA;
  char c00 = 0x00;



    if (cSensorType == I2C_TYPE_DS1621)
    {
      vTaskDelay(10);
      nTemp = 1;
      nTemp = I2C_Write(nI2C_SensorID, 0, 0, &cStartConvert, 1);
      if (nTemp == 1)
      {


       // for (i = 0; i < 5; i++)
       // {
          vTaskDelay(10);

        //  if (nTemp == 1)
        //  {
         //   if (cTemp & 0x40)
         //   {
              I2C_Write(nI2C_SensorID, 0, 0, &cAA, 1);
              nTemp = I2C_Receive(nI2C_SensorID, 0, 0 , pTemperature_temp, 2);
          /*  break;
            }
            else
            {
              nTemp = 0;
            }
          }
         else
         {
           return 0;
          }
        }*/
      }
      else
      {
        return 0;
      }
    }
    else if (cSensorType == I2C_TYPE_LM75)
    {
     // nTemp = I2C_Write(nI2C_SensorID, 0, 0, &c00, 1);
     // if (nTemp == 1)
     // {
        //vTaskDelay(10);
        nTemp = I2C_Receive(nI2C_SensorID , 0, 0, pTemperature_temp, 2);
     /* }
      else
      {
        return 0;
      }*/
    }
    else
    {
      return 0;
    }


    if (nTemp == 1)
    {
      pTempH = pTemperature_temp[0];
      pTempL = pTemperature_temp[1];

      // skal det vekk ??????
      if(pTempH==0xc4)
        return 0;

      // IF TEMPERATURE < 0
      if (pTempH & 0x80)
      {
        pTemperature[cPos] = '-'; // negative temperature
        pTempH = ~pTempH;   // 1- complement
        pTempH = (unsigned char)(pTempH +1); // 2- complement
      }
      else
      {
        pTemperature[cPos] = '+'; // positive temperature
      }

      cPos ++;





      if (pTempH >= 100)
      {
        pTemperature[cPos] = pTempH / 100 + 0x30;
        pTempH = pTempH % 100;

        cPos++;
      }

      if (pTempH >= 10)
      {
        pTemperature[cPos] = pTempH / 10 + 0x30;
        pTempH = pTempH % 10;
        cPos ++;
      }
      else if(cPos == 2)
      {
        pTemperature[cPos] = 0x30;        // If temp > 100 C
        cPos ++;
      }

      pTemperature[cPos] = pTempH + 0x30;

      cPos ++;
      pTemperature[cPos] = '.';

      cPos ++;
      if (pTempL & 0x80)
      {
        pTemperature[cPos] = '5';
      }
      else
      {
         pTemperature[cPos] = '0';
      }
      cPos ++;
      *pTemperatureLength = cPos;

      return 1;
  }
  return 0;
}
/*
char TestSensorType(int nI2Caddress)
{
  char cInit[2];
  char c00 = 0xAC;
  char cResult;
  int nTemp;

  cInit[0] = 0xAC;
  cInit[1] = 0x89;

  nTemp = I2C_Write(nI2Caddress, 0, 0, &cInit, 2);
  if (nTemp == 0)
  {
    return 0;
  }

  vTaskDelay(10);

  nTemp = I2C_Write(nI2Caddress, 0, 0, &c00, 1);
  if (nTemp == 0)
  {
    return 0;
  }

  nTemp = I2C_Receive(nI2Caddress, 0, 0, &cResult, 1);
  if (nTemp == 0)
  {
    return 0;
  }

  if (cResult == 0x89)
  {
    return I2C_TYPE_DS1621;
  }
  else
  {
    return I2C_TYPE_LM75;
  }
}
*/
///////////////////////////////////////////////////////////////////////////
// Don't remove this function. This function is used for test of I2C
// Created      : 2005-11-01 by ES

/* unsigned portCHAR pTemperature_2[7];
  unsigned portCHAR ucByteToWrite[2] = {0X44, 0};
  unsigned portCHAR ucByteTest[2];


   if (I2C_Write(73, 2, ucByteToWrite, 2) == 0)
   {
      ucByteTest[0] = '1'; //brukes bare for å kunne sette breakpoint gjennom testing
   }
*/

