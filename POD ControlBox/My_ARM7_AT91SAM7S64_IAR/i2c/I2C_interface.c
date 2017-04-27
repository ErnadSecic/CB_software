//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.01
//////////////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>

#include "FreeRTOS.h"

#include "I2C_interface.h"
#include "I2C_TemperatureSensor.h"
#include "GpIO.h"
#include "task.h"
#include "Timer.h"

//////////////////////////////////////////////////////////////////
// Number of trial waiting on Status Register change
#define TRIAL_COUNTER             20000

#define MASTER_FREQ           48000000
#define I2C_FREQ              4800  // !!! 400.000 is max
                                    //50000  //100000
//static
void OnI2C_Error(void);
//static void OnI2C_ErrorDS1621(void);

char WaitOnRxRDY(void);
char WaitOnTxRDY(void);
char WaitOnTxCOMPL(void);
///////////////////////////////////////////////////////////////////
// Function: Init_I2C
//      Initialising of master I2C
////////////////////////////////////////////////////////////////////
void Init_I2C( void )
{
 // char cOnecMeass[2];
  int sclock;
  
  //cOnecMeass[0] = 0xAC;
  //cOnecMeass[1] = 0x01;
  /////////////////////////////
  AT91PS_TWI twiPtr = (AT91PS_TWI)AT91C_BASE_TWI;
  AT91PS_PIO pPio = (AT91PS_PIO)AT91C_BASE_PIOA;
  AT91PS_PMC pPMC = (AT91PS_PMC)AT91C_BASE_PMC;



  ///////////////////////////////////////////////////////////////////
  // PERIPHERAL CONTROL MODE ON TWD AND TWCK (disable PIO)
  ///////////////////////////////////////////////////////////////////
  pPio->PIO_PDR =
		((unsigned)AT91C_PA3_TWD) | ((unsigned)AT91C_PA4_TWCK);


  ///////////////////////////////////////////////////////////////////
  // SELECT TWD and TWCK pin like  PERIPHERAL (A-REGISTER).
  ///////////////////////////////////////////////////////////////////
  pPio->PIO_ASR =
  ((unsigned)AT91C_PA3_TWD) | ((unsigned)AT91C_PA4_TWCK);

  ///////////////////////////////////////////////////////////////////
  // Open Drain (must be used for both pins)
  ///////////////////////////////////////////////////////////////////
  pPio->PIO_MDER = (AT91C_PA3_TWD | AT91C_PA4_TWCK);


  ///////////////////////////////////////////////////////////////////
  // ENABLE PERIPHERAL CLOCK ON TWI
  ///////////////////////////////////////////////////////////////////
  pPMC->PMC_PCER = ((unsigned) 1 << AT91C_ID_TWI);


  ///////////////////////////////////////////////////////////////////
  // DISABLE INTERRUPT
  ///////////////////////////////////////////////////////////////////
  twiPtr->TWI_IDR = (unsigned) -1;


  ///////////////////////////////////////////////////////////////////
  // RESET OF TW
  ///////////////////////////////////////////////////////////////////
  twiPtr->TWI_CR = AT91C_TWI_SWRST;


  ///////////////////////////////////////////////////////////////////
  // SELECT MASTER AND DISABLE SLAVE
  ///////////////////////////////////////////////////////////////////
  //twiPtr->TWI_CR = AT91C_TWI_MSEN | AT91C_TWI_SVDIS;
  twiPtr->TWI_CR = AT91C_TWI_MSEN;  //


  ///////////////////////////////////////////////////////////////////
  // SELECT CLOCK PERIOD
  //
  // Calculation of TWI_CWGR to get required I2C_FREQ.
  ///////////////////////////////////////////////////////////////////
  sclock = (10 * MASTER_FREQ / I2C_FREQ);
  if (sclock % 10 >= 5)
    sclock = (sclock /10) - 5;
  else
    sclock = (sclock /10)- 6;
  sclock = (sclock + (4 - sclock %4)) >> 2;	// div 4

  twiPtr->TWI_CWGR =  0x00010000 | sclock | (sclock << 8);
  
  vTaskDelay(50);
  
  //I2C_Write(SENSOR_1_I2C_INT_CB_ADRESS, 0, 0, cOnecMeass, 2);
}


///////////////////////////////////////////////////////////////////
// Function: I2C_Write
//      Send bytes to slave
// returns:
//      successed: 1
//      error:     0
////////////////////////////////////////////////////////////////////
int I2C_Write(char cSlaveAddress, char cInternalAddress, int nHasInternalAddress, char* pcToWrite, unsigned int nLengthToWrite)
{
  // unsigned int i;
   unsigned int nTemp;
   char chTemp;
// Init_I2C( );

  nTemp= AT91C_BASE_TWI->TWI_SR; // read old register values before start
  
  ///////////////////////////////////////////////////////////////////
  // Set slave address, internal address size and READ bit
  ////////////////////////////////////////////////////////////////////
  if (nHasInternalAddress == 1)
  {
     AT91C_BASE_TWI->TWI_MMR =  ((0x7F) & cSlaveAddress) << 16 | AT91C_TWI_IADRSZ_1_BYTE ;
     AT91C_BASE_TWI->TWI_IADR = cInternalAddress;
  }
  else
  {
    AT91C_BASE_TWI->TWI_MMR =  ((0x7F) & cSlaveAddress) << 16 | AT91C_TWI_IADRSZ_NO; //AT91C_TWI_IADRSZ_1_BYTE ;
  }

  AT91C_BASE_TWI->TWI_THR = *pcToWrite++;
  nLengthToWrite --;
    
  ///////////////////////////////////////////////////////////////////
  // Start sending
  ////////////////////////////////////////////////////////////////////
   AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START;
   
   if (nLengthToWrite == 0)
   {
     if (WaitOnTxRDY() == 0)
    {
      OnI2C_Error();
      return 0; 
    }
   }
   else
   {
      while (nLengthToWrite > 0)
      {
        if (WaitOnTxRDY() == 0)
        {
          OnI2C_Error();
          return 0; 
        }
        ///////////////////////////////////////////////////////////////////
        // Set the next byte to send
        ////////////////////////////////////////////////////////////////////
        chTemp = *pcToWrite++;
        AT91C_BASE_TWI->TWI_THR = chTemp;
        nLengthToWrite --;
      }
   }

int nTemp1= AT91C_BASE_TWI->TWI_SR;

    //////////////////////////////////////////////////////////
    // All bytes has been send. Send STOP and wait on TXCOMP
    //////////////////////////////////////////////////////////
    AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;

    // Wait until transfer is finished
    if (WaitOnTxCOMPL() == 1)
    {
      return 1;
    }
    OnI2C_Error();
    return 0;

}

char WaitOnRxRDY(void)
{
  int nTemp;
  int i;
  
  for(i = 0; i < TRIAL_COUNTER; i++)
    {
      nTemp= AT91C_BASE_TWI->TWI_SR;

      if ( nTemp &  AT91C_TWI_RXRDY )
      {
       /* if ( (nTemp &  AT91C_TWI_NACK | AT91C_TWI_OVRE | AT91C_TWI_UNRE)) > 0 ) // 0x1C4 ) >
        {
          return 0;
        }*/
        
        return 1;
      }
    }
  
    return 0;
}

char WaitOnTxRDY(void)
{
  int nTemp;
  int i;
  
  for(i = 0; i < TRIAL_COUNTER; i++)
  {
      nTemp= AT91C_BASE_TWI->TWI_SR;

      if ( nTemp &  AT91C_TWI_TXRDY )
      {
        if ( (nTemp &  (AT91C_TWI_NACK | AT91C_TWI_OVRE | AT91C_TWI_UNRE)) > 0 ) // 0x1C4 ) >
        {
          return 0;
        }
       
        return 1;
      }
  }
  return 0;
}

char WaitOnTxCOMPL(void)
{
  int nTemp;
  int i;

  
  for(i = 0; i < TRIAL_COUNTER; i++)
    {
      nTemp= AT91C_BASE_TWI->TWI_SR;

      if ( nTemp &  AT91C_TWI_TXCOMP )
      {
        if ( (nTemp & (AT91C_TWI_NACK | AT91C_TWI_OVRE | AT91C_TWI_UNRE)) > 0 ) // 0x1C4 ) >
        {
          return 0;
        }
        
        return 1;
      }
    }
  
    return 0;
}

///////////////////////////////////////////////////////////////////
// Function: I2C_Receive
//      Read bytes from slave
// returns:
//      successed: 1
//      error:     0
////////////////////////////////////////////////////////////////////
  int I2C_Receive(char cSlaveAddress, int nUseInternalAddress, char cInternalAddress,  char* pcToRead, unsigned int nLengthToRead)
  {
    unsigned int nStatusRegister;
    unsigned int k;

    
    //  Init_I2C( );
    nStatusRegister = AT91C_BASE_TWI->TWI_SR;
    
    AT91C_BASE_TWI->TWI_RHR;

    if (nUseInternalAddress == 1)
    {
      ///////////////////////////////////////////////////////////////////
      // Set slave address,  MsgRead flag and one byte like internal address (it will be sent automaticly after address-byte)
      ////////////////////////////////////////////////////////////////////
      AT91C_BASE_TWI->TWI_MMR =  (((0x7F) & cSlaveAddress) << 16) | AT91C_TWI_MREAD | AT91C_TWI_IADRSZ_1_BYTE; //AT91C_TWI_IADRSZ_1_BYTE; //AT91C_TWI_IADRSZ_NO;
      ///////////////////////////////////////////////////////////////////
      // Set internal address witch has to be read from.
      ////////////////////////////////////////////////////////////////////
      AT91C_BASE_TWI->TWI_IADR = cInternalAddress;
    }
    else
    {
      ///////////////////////////////////////////////////////////////////
      // Set slave address and MsgRead flag
      ////////////////////////////////////////////////////////////////////
      AT91C_BASE_TWI->TWI_MMR =  (((0x7F) & cSlaveAddress) << 16) | AT91C_TWI_MREAD | AT91C_TWI_IADRSZ_NO;
    }
    
    ///////////////////////////////////////////////////////////////////
    // Send
    ////////////////////////////////////////////////////////////////////
    AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START;

    
    // vTaskDelay(5);
    ///////////////////////////////////////////////////////////////////
    // Wait until last but one byte has been received.
    ////////////////////////////////////////////////////////////////////
    for (k = 0; k < nLengthToRead; k++) //
    {
      if (WaitOnRxRDY() == 0)
      {
        OnI2C_Error();
        return 0;
      }
      *pcToRead++ = (portCHAR)AT91C_BASE_TWI->TWI_RHR;
    }

    ///////////////////////////////////////////////////////////////////
    // Stop before receiving of the last byte
    ////////////////////////////////////////////////////////////////////
    AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
    


    ///////////////////////////////////////////////////////////////////
    // Receive the last byte
    ////////////////////////////////////////////////////////////////////
    if (WaitOnTxCOMPL() == 1)
    {
       //*pcToRead = (portCHAR)AT91C_BASE_TWI->TWI_RHR;
        return 1;
    }
    else
    {
      OnI2C_Error();
      return 0;
    }
  }



///////////////////////////////////////////////////////////////////
// Function: OnI2C_Error
//
////////////////////////////////////////////////////////////////////
void OnI2C_Error()
{
  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
  Init_I2C( );
}


