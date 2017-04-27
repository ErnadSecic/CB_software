//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.02
//////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#include "string.h"
#include "Motor.h"
#include "ECom.h"
#include "DIP_switch.h"

// Ernad
void Get2_CharsFromInt(int nInput, unsigned char * );
void Get4_CharsFromInt(int nInput, unsigned char * );

//////////////////////////////////////////////////////////////////////////////
// Function     : ecomDisassembleReceivedCommand()
// Purpose      : To check and return parameters in command received from PC
// Return value : none
// Created      : 2005-10-25 by ES
// Updated      : 2005-10-25 by ES
// Comments     : USED BY CONTROL BOX
//////////////////////////////////////////////////////////////////////////////
void ecomDisassembleReceivedCommand(unsigned char *pRequest, int *pEquipmentType, int *pEquipmentNo, int *pRequestCommand, int *pLengthOfValue, int *pValue)
{
   unsigned char   szField[5];
   unsigned char   szvaluelength;

   memset(szField, 0, 5);
   // Pick up EQUIPMENT TYPE
   strncpy(szField, &pRequest[MSG_EQ_TYPE_IDX], 4);
   *pEquipmentType = atoi(szField);
   memset(szField, 0, 5);

   // Pick up EQUIPMENT NUMBER
   strncpy(szField, &pRequest[MSG_EQ_NO_IDX], 4);
   if(szField[0]=='*'&& szField[1]=='*' && szField[2]=='*'&& szField[3]=='*')
     *pEquipmentNo=WILDCARD;
   else
     *pEquipmentNo = atoi(szField);
   memset(szField, 0, 5);

   // Pick up REQUEST COMMAND
   strncpy(szField, &pRequest[MSG_COMMAND_IDX], 2);
   *pRequestCommand = atoi(szField);
   memset(szField, 0, 5);

   if (*pRequestCommand == SET_OUTPUT)
   {
      // Pick up LENGTH OF VALUE
      strncpy(szField, &pRequest[MSG_VAL_LEN_IDX], 2);
      *pLengthOfValue = atoi(szField);
      memset(szField, 0, 5);

      // Pick up VALUE
      if(*pLengthOfValue > 4)
        szvaluelength=4;
      else
        szvaluelength=*pLengthOfValue;
      strncpy(szField, &pRequest[MSG_DATA_IDX], szvaluelength);
      *pValue = atoi(szField);
      memset(szField, 0, 5);
   }
   else
   {
     *pLengthOfValue  = -1;
     *pValue          = -1;
   }
}

extern unsigned char m_nUnitID;
//////////////////////////////////////////////////////////////////////////////
// Function     : ecomBuildEventMessage()
// Purpose      : To build Event
// Parameters   :
// Return value : None
// Created      : 2005-10-25 by ES
// Updated      : 2005-10-25 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ecomBuildEventMessage(unsigned char *pResponse, int pnEquipmentType, int pnEquipmentNo, int pnEventCommand, int pnLengthOdValue, char *pValue, char *pEqStatus)
{
   unsigned char pResult[4];
   Get4_CharsFromInt(m_nUnitID, pResult);
   memcpy(&pResponse[MSG_ID_IDX], pResult, 4);

   Get4_CharsFromInt(pnEquipmentType, pResult);
   memcpy(&pResponse[MSG_EQ_TYPE_IDX], pResult, 4);

   Get4_CharsFromInt(pnEquipmentNo, pResult);
   memcpy(&pResponse[MSG_EQ_NO_IDX], pResult, 4);

   Get2_CharsFromInt(pnEventCommand, pResult);
   memcpy(&pResponse[MSG_COMMAND_IDX], pResult, 2);

   // Value
   Get2_CharsFromInt(pnLengthOdValue, pResult);
   memcpy(&pResponse[MSG_VAL_LEN_IDX], pResult, 2);
   // Becouse of "Ticket reader" (value can be up to 30 chars), her is used char-pointer and not int.
   memcpy(&pResponse[MSG_DATA_IDX], pValue, pnLengthOdValue);

   memcpy(&pResponse[MSG_DATA_IDX + pnLengthOdValue], pEqStatus, 2);

   pResponse[MSG_DATA_IDX + pnLengthOdValue + 2] = 0;   // NULL terminate message, to allow USB task to detect length

}

//////////////////////////////////////////////////////////////////////////////
// Function     : Get2_CharsFromInt()
// Purpose      : Convert to 2-ciphers string (f.eks. convert 1 to "01")
// Parameters   :
// Return value : None
// Created      : 2005-10-25 by ES
// Updated      : 2005-10-25 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Get2_CharsFromInt(int nInput, unsigned char * chResult)
{
  chResult[0] = (nInput / 10) + 0x30;

  chResult[1] = (nInput % 10) + 0x30;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : Get4_CharsFromInt()
// Purpose      : Convert to 4-ciphers string (f.eks. convert 1 to "0001")
// Parameters   :
// Return value : None
// Created      : 2005-10-25 by ES
// Updated      : 2005-10-25 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Get4_CharsFromInt(int nInput, unsigned char * chResult)
{
  chResult[0] = (nInput / 1000) + 0x30;

  nInput = nInput % 1000;

  chResult[1] = (nInput / 100) + 0x30;

  nInput = nInput % 100;

  chResult[2] = (nInput / 10) + 0x30;

  chResult[3] = nInput % 10 + 0x30;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : ecomBuildResponse()
// Purpose      : To build response mesages
// Return value : None
// Created      : 2005-10-25 by ES
// Updated      : 2005-10-25 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ecomBuildResponse(unsigned char *pRequest, unsigned char *pResponse, int nLengthOfValue, char * pValue, char *pEqStatus)
{
   unsigned char pResult[4];

   // Copy the first 14 bytes from request message (until MSG_VAL_LEN_IDX):
   memcpy(&pResponse[MSG_ID_IDX], &pRequest[MSG_ID_IDX], 14);

   // Put nLengthOfValue and pValue
   Get2_CharsFromInt(nLengthOfValue, pResult);
   memcpy(&pResponse[MSG_VAL_LEN_IDX], pResult, 2);     // Value length is 0
   memcpy(&pResponse[MSG_DATA_IDX], pValue, nLengthOfValue);      // Equipment status is the response return code

   // Put Equipment Status
   strncpy(&pResponse[MSG_DATA_IDX + nLengthOfValue], pEqStatus, 2);

   pResponse[MSG_DATA_IDX + nLengthOfValue + 2] = 0;   // NULL terminate message, to allow USB task to detect length
}

//////////////////////////////////////////////////////////////////////////////
// Function     : ecomGetValueString()
// Purpose      : Get Value string from USB
// Parameters   :
// Return value :
// Created      : 2006-01-10 by TAK
// Updated      : 2006-01-10 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void  ecomGetValueString(unsigned char *pRequest, unsigned char *nRS485data)
{
     char szParameter[3];
     char szValue_length;

     memcpy(szParameter,&pRequest[MSG_VAL_LEN_IDX],2);
     szParameter[2] = '\0';
     szValue_length = atoi(szParameter);
     if(szValue_length>48)
       szValue_length=48;
     memcpy(nRS485data,&pRequest[MSG_DATA_IDX],szValue_length);
     return;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : ecomGetEquipmentNo()
// Purpose      : To retrive the Equipment Number from a PC request message
// Parameters   : Pointer to request message buffer
// Return value : TRUE or FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int   ecomGetEquipmentNo(unsigned char *pRequest)
{
  int iEqNo;
   char szParameter[5];

   // Pick up command in message
   memcpy(szParameter,&pRequest[MSG_EQ_NO_IDX],4);
   szParameter[4] = '\0';
   if(szParameter[0]=='*'&& szParameter[1]=='*' && szParameter[2]=='*'&& szParameter[3]=='*')
     iEqNo = WILDCARD;
   else
     iEqNo = atoi(szParameter);

   return (iEqNo);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : ecomGetEquipmentType()
// Purpose      : To retrive the Equipment Type from a PC request message
// Parameters   : Pointer to request message buffer
// Return value : TRUE or FALSE
// Created      : 2005-12-05 by TAK
// Updated      : 2005-12-05 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int   ecomGetEquipmentType(unsigned char *pRequest)
{
   int iEqType;
   char szParameter[5];

   // Pick up command in message
   memcpy(szParameter,&pRequest[MSG_EQ_TYPE_IDX],4);
   szParameter[4] = '\0';
   iEqType = atoi(szParameter);
   return (iEqType);
}


//////////////////////////////////////////////////////////////////////////////
// Function     : ecomGetCommand()
// Purpose      : To retrive the command from a PC request message
// Parameters   : Pointer to request message buffer
// Return value : TRUE or FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int   ecomGetCommand(unsigned char *pRequest)
{
   int iCmd;
   char szParameter[3];

   // Pick up command in message
   memcpy(szParameter,&pRequest[MSG_COMMAND_IDX],2);
   szParameter[2] = '\0';

   iCmd = atoi(szParameter);

   return (iCmd);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : ecomStringLength()
// Purpose      : To retrive the stringLength from a PC request message
// Parameters   : Pointer to request message buffer
// Return value : TRUE or FALSE
// Created      : 2006-09-21 by TAK
// Updated      : 2006-09-21 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int   ecomStringLength(unsigned char *pRequest)
{
   int iCmd;
   char szParameter[3];

   memcpy(szParameter,&pRequest[MSG_VAL_LEN_IDX],2);
   szParameter[2] = '\0';

   iCmd = atoi(szParameter);

   return (iCmd);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : ecomBuildAckMsg()
// Purpose      : To build response mesages
// Parameters   : Request, Response, EqStatus
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ecomBuildAckMsg(unsigned char *pRequest, unsigned char *pResponse, char *pEqStatus)
{

   // Copy from request message:
   memcpy(&pResponse[MSG_ID_IDX], &pRequest[MSG_ID_IDX], 4);
   memcpy(&pResponse[MSG_EQ_TYPE_IDX], &pRequest[MSG_EQ_TYPE_IDX], 4);
   memcpy(&pResponse[MSG_EQ_NO_IDX], &pRequest[MSG_EQ_NO_IDX], 4);
   memcpy(&pResponse[MSG_COMMAND_IDX], &pRequest[MSG_COMMAND_IDX], 2);

   // Build rest of Acknowledge message
   memcpy(&pResponse[MSG_VAL_LEN_IDX], "00", 2);     // Value length is 0
   memcpy(&pResponse[MSG_DATA_IDX], pEqStatus, 2);      // Equipment status is the response return code

   pResponse[MSG_DATA_IDX+2] = 0;   // NULL terminate message, to allow USB task to detect length

   return;

}

//////////////////////////////////////////////////////////////////////////////
// Function     : ecomValidMoveRequest()
// Purpose      : To check and return parameters in Move Request from PC
// Parameters   : Command, Requestbuffer, &MotorNo, &Direction, &Steps, &Speed
// Return value : TRUE when success, otherwise FALSE
// Created      : 2005-09-17 by PVA
// Updated      : 2005-09-17 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int ecomValidMoveRequest(int iCommand, unsigned char *pRequest, int *pMotorNo, int *pDirection, int *pSteps, int *pSpeed, int *iAuxilaryEqNo)
{
  unsigned char   szField[5];

  // Pick up motor number from message
   memcpy(szField, &pRequest[MSG_EQ_NO_IDX], 4);
   szField[4] = 0;//'\0';
   *pMotorNo = atoi(szField);
   
   
   
   if ((motorValid( *pMotorNo)))
   {

     if (iCommand == TIGHTEN_PUSHER || iCommand == TEST_PUSHER)
      {
         *pDirection = FORWARD;
         *pSteps = STEPS_BEYOND_MAX;
         
         memcpy(szField, &pRequest[AUXILIARY_EJECTOR_IDX_TIGHTEN], 2);
         szField[2] = 0;
         *iAuxilaryEqNo = atoi(szField);
         
         if (motorValid(*iAuxilaryEqNo) != 1 && *iAuxilaryEqNo != 0)
         {
           return(0);   
         }
      }
      else
      {
         memcpy(szField, &pRequest[AUXILIARY_EJECTOR_IDX], 2);
         szField[2] = 0; //'\0';
         *iAuxilaryEqNo = atoi(szField);
   
         // Read Direction, Steps and Speed from message:
         if (pRequest[DIRECTION_IDX] == '+')
         {
            *pDirection = FORWARD;
         }
         else if (pRequest[DIRECTION_IDX] == '-')
         {
            *pDirection = REVERSE;
         }
         else
         {
            return(0);    // Invalid direction given
         }
         memcpy(szField,&pRequest[LENGTH_1_IDX],4);
         szField[4] =  0; //'\0';
         *pSteps = atoi(szField);
         if (*pSteps < 1)
         {
            return (0);    // Invalid number of steps given
         }
      }
   }
   else
   {
      return (0);    // Invalid motor number given
   }

   if (iCommand == TIGHTEN_PUSHER || iCommand == TEST_PUSHER)
   {
      *pSpeed = 100;  // 100% speed

      return(1);  // All data OK!!!
   }

   // Read Speed from message:
   memcpy(szField,&pRequest[SPEED_1_IDX],3);
   szField[3] =  0; //'\0';
   *pSpeed = atoi(szField);
   if ((*pSpeed >= 2) && (*pSpeed <= 100))      // Speed between 50 - 100%
   {
      return(1);     // All data OK, return OK!!!
   }
   else
   {
      return(0);     // Invalid Speed given
   }
}


//////////////////////////////////////////////////////////////////////////////
// Function     : ecomValidOpenLedRequest()
// Purpose      : To check and return parameters in Move Request from PC
// Parameters   : Command, Requestbuffer, &MotorNo, &Direction, &Steps, &Speed
// Return value : TRUE when success, otherwise FALSE
// Created      : 2005-11-19 by ES
// Updated      : 2005-11-19 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int ecomValidOpenLedRequest(unsigned char *pRequest, int *pMotorNo, int *pValue, int *iAuxilaryEqNo)
{
  unsigned char   szField[5];

  // Pick up motor number from message
   memcpy(szField, &pRequest[MSG_EQ_NO_IDX], 4);
   szField[4] = '\0';
   *pMotorNo = atoi(szField);
   memset(szField, 0, 5);

   memcpy(szField, &pRequest[MSG_DATA_IDX], 1);
   szField[1] = '\0';
   *pValue = atoi(szField);
   
   memcpy(szField, &pRequest[MSG_DATA_IDX + 2], 2);
   szField[3] = '\0';
   *iAuxilaryEqNo = atoi(szField);


   if ((motorValid( *pMotorNo)))
   {
      return(1);     // All data OK, return OK!!!
   }
   else
   {
      return(0);     // Invalid Speed given
   }
}

//////////////////////////////////////////////////////////////////////////////
// Function     : ecomBuildOperationComplete()
// Purpose      : To build "Operation Compled" mesages
// Parameters   : Request, Response, EqStatus
// Return value : None
// Created      : 2005-09-17 by PVA
// Updated      : 2005-11-10 by ES
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void ecomBuildOperationCompleteMsg(int nUnitID, int iMotorNo, int iDirection, int iTotalCnt, unsigned char * pEqStatus, unsigned char *pResponse)
{
   char   szParameter[6];
   char szEquipmentType[] = "0600";
   int    iCommand;

   memset(pResponse, 0 , 64);

   sprintf(szParameter, "%04d", nUnitID);
   strcat(pResponse,szParameter);

   memcpy(&pResponse[MSG_EQ_TYPE_IDX], szEquipmentType, 4);

   sprintf(szParameter, "%04d", iMotorNo);
   strcat(pResponse,szParameter);                                    // Equipment No stored in response

   motorGetCommand(iMotorNo, &iCommand);                             // Find last command used
   sprintf(szParameter, "%02d", iCommand);
   strcat(pResponse,szParameter);                                    // Response Command stored in response

   strcat(pResponse,"05");                                           // Length of Value Field stored in response

   if (iDirection == FORWARD)
   {
      strcat(pResponse,"+");                                         // Forward direction stored
   }
   else if (iDirection == REVERSE)
   {
      strcat(pResponse,"-");                                        // Reverse direction stored
   }
   else
   {
      strcat(pResponse," ");                                       // Direction not specified stored
   }

   sprintf(szParameter, "%04d", iTotalCnt);
   strcat(pResponse,szParameter);                                  // No of Steps from the start of the last move command stored

   strcat(pResponse, pEqStatus); //"00");                     // Status code stored

   return;
}
