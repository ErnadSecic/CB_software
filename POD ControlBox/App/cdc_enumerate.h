//*----------------------------------------------------------------------------
//*      ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : cdc_enumerate.h
//* Object              : Handle CDC enumeration
//*
//* 1.0 Apr 20 200 	: ODi Creation
//*----------------------------------------------------------------------------
#ifndef CDC_ENUMERATE_H
#define CDC_ENUMERATE_H

#include "Board.h"
//#include "AT91SAM7S128.h"
//#include "AT91SAM7S64.h"
//#define __inline inline
//#include "lib_AT91SAM7S128.h"
//#include "lib_AT91SAM7S64.h"

#define AT91C_EP_OUT 1
#define AT91C_EP_OUT_SIZE 0x40
#define AT91C_EP_IN  2


typedef struct _AT91S_CDC
{
	// Private members
	AT91PS_UDP pUdp;
	unsigned char currentConfiguration;
	unsigned char currentConnection;
	unsigned int  currentRcvBank;
	// Public Methods:
	unsigned char (*IsConfigured)(struct _AT91S_CDC *pCdc);
       //unsigned int  (*Write) (struct _AT91S_CDC *pCdc, const char *pData, unsigned int length);
	unsigned int  (*Write) (struct _AT91S_CDC *pCdc,  char *pData, unsigned int length);
	unsigned int  (*Read)  (struct _AT91S_CDC *pCdc, char *pData, unsigned int length);
} AT91S_CDC, *AT91PS_CDC;

//* external function description
__ramfunc  unsigned int AT91F_UDP_Read(AT91PS_CDC pCdc, char *pData, unsigned int length);
__ramfunc  unsigned int AT91F_UDP_Write(AT91PS_CDC pCdc, char *pData, unsigned int length);

AT91PS_CDC AT91F_CDC_Open(AT91PS_CDC pCdc, AT91PS_UDP pUdp);

#endif // CDC_ENUMERATE_H

