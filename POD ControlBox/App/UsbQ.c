
//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.01
//////////////////////////////////////////////////////////////////////////////////////

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"



#include "UsbQ.h"


static xQueueHandle xUsbRxQueue;
static xQueueHandle xUsbTxQueue;


/*
// Hold incoming messages for manipulation from IO
static unsigned char ucRxData[USB_QUEUE_ITEM_LENGTH];


// Hold outgoing messages for manipulation from USB
static unsigned char ucTxData[USB_QUEUE_ITEM_LENGTH];
*/

//////////////////////////////////////////////////////////////////////////////
// Function     : usbqInit()
// Purpose      : To init USB queues
// Parameters   : None
// Return value :
// Created      : 2005-09-26 by PVA
// Updated      : 2005-09-26 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int usbqInit( void )
{
   xUsbRxQueue = xQueueCreate( USB_QUEUE_ITEMS, ( unsigned portBASE_TYPE ) USB_QUEUE_ITEM_LENGTH );
   if (xUsbRxQueue != 0)
   {
      xUsbTxQueue = xQueueCreate( USB_QUEUE_ITEMS, ( unsigned portBASE_TYPE ) USB_QUEUE_ITEM_LENGTH );
      if (xUsbTxQueue != 0)
      {
        return(1);
      }
   }
   return(0);
}


//////////////////////////////////////////////////////////////////////////////
// Function     : usbqReadUsbMsg()
// Purpose      : To receive message from USB task
// Parameters   : Reurn pointer to where the message will copied
// Return value : TRUE on success
// Created      : 2005-09-26 by PVA
// Updated      : 2005-09-26 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int usbqReadUsbMsg(unsigned char *pMsg)
{ 
   if( xQueueReceive( xUsbRxQueue, (void *)pMsg, (portTickType)0  ) == pdTRUE )
   {
      return (1);
   }
   else
   {
      return (0);
   }
}



//////////////////////////////////////////////////////////////////////////////
// Function     : usbqWriteUsbMsg()
// Purpose      : To send a message to the USB task
// Parameters   : Pointer to messge
// Return value : TRUE on success
// Created      : 2005-09-26 by PVA
// Updated      : 2005-09-26 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int usbqWriteUsbMsg(unsigned char *pMsg)
{
   if( xQueueSend( xUsbTxQueue, pMsg, (portTickType)10  ) == pdTRUE )
   {
      return (1);
   }
   else
   {
      return (0);
   }
}



//////////////////////////////////////////////////////////////////////////////
// Function     : usbqReadIoMsg()
// Purpose      : To receive a message from the IO-application within the USB task
// Parameters   : Return pointer to where the message will copied
// Return value : TRUE on success
// Created      : 2005-09-26 by PVA
// Updated      : 2005-09-26 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int usbqReadIoMsg(unsigned char *pMsg)
{
   if( xQueueReceive( xUsbTxQueue, (void *)pMsg, (portTickType)0  ) == pdTRUE )
   {
      return (1);
   }
   else
   {
      return (0);
   }
}



//////////////////////////////////////////////////////////////////////////////
// Function     : usbqWriteIoMsg()
// Purpose      : To  send a message to the IO-application task from the USB task
// Parameters   : None
// Return value :
// Created      : 2005-09-26 by PVA
// Updated      : 2005-09-26 by PVA
// Comments     :
//////////////////////////////////////////////////////////////////////////////
int usbqWriteIoMsg(unsigned char *pMsg)
{
   if( xQueueSend( xUsbRxQueue, pMsg, (portTickType)0  ) == pdTRUE )
   {
      return (1);
   }
   else
   {
      return (0);
   }
}




