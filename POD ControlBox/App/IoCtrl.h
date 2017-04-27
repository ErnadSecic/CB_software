
#include "FreeRTOS.h"

#ifndef IO_CTRL_H
#define IO_CTRL_H

#define  SW_TYPE_SENSOR   0
#define  SW_TYPE_EJECTOR  1




void  vIoCtrl( void *pvParameters );
void Enable_Ejswitch(int aCommand,int aEqNo,unsigned char *aStatus);
portBASE_TYPE xAreIoCtrlTaskStillRunning( void );


#endif

