
#include "RS485.h"
#include "Board.h"
#include "string.h"
#include "IRQ_LEVEL.h"
#include "Timer.h"
#include "GpIO.h"
#include "UsbQ.h"
#include "ECom.h"



AT91PS_USART COM1;

RS485_FLAGS  RS485_FLAGbits;


static char Buff_InterruptRx[(RS485buff_length+RS485buff_length)];
static char Buff_Rx[(RS485buff_length)];
static int m_nReceivedLength = 0;
static char m_bOverflowError = 0;
static char MI_adress=0;



unsigned int first =0;

extern void ExternUARTISREntry( void);



//////////////////////////////////////////////////////////////////////////////
// Function     : Usart_c_irq_handler()
// Purpose      : C handler interrupt function called by the interrupts
//                assembling routine
// Parameters   :
// Return value :
// Created      : 2006-01-10 by TAK
// Updated      : 2006-06-29 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
__arm void Usart_c_irq_handler(void)
{
   unsigned int status;

   // get Usart status register and active interrupt
   status = COM1->US_CSR ;
   status &= COM1->US_IMR;
   RS485_FLAGbits.com_error=0;

   if ( status & AT91C_US_RXBUFF)  //IRQ if RX buffer is full
   {
         int nOffset = 0;
         int i = 0;

         for(nOffset=0; nOffset < RS485buff_length; nOffset++)           //Fjerner alt til startbyte
         {
            if(Buff_InterruptRx[nOffset]==START_BYTE)
            {
               break;
             }
         }
         
         if (nOffset < RS485buff_length) // received msg with right startbyte
         {
           if (m_nReceivedLength > 0) // !!! ERROR, OVERFLOW
           {
             m_bOverflowError = 1;
           }
           else
           {
              m_nReceivedLength = 0;
             
              for (i = 0; i < RS485buff_length && Buff_InterruptRx[nOffset] != 255; i++) // copy to the receiving buffer
              {
                Buff_Rx[i] = Buff_InterruptRx[nOffset];
                nOffset ++;
                m_nReceivedLength ++;
              }

           }
         }
         else // !!! ERROR, RECEIVED BUFFER WITHOUT START BYTE
         {
           memset(Buff_InterruptRx, 255, (RS485buff_length + RS485buff_length)); // just for use of break point
         }
         
         memset(Buff_InterruptRx, 255, (RS485buff_length + RS485buff_length));
         
 	 COM1->US_RPR = (unsigned int) Buff_InterruptRx;
	 COM1->US_RCR = 48;
   }
	
   // Reset the satus bit for error
   COM1->US_CR = AT91C_US_RSTSTA;

   //Clear AIC to complete ISR processing
   AT91C_BASE_AIC->AIC_EOICR = 0;
}

//////////////////////////////////////////////////////////////////////////////
// Function     : AT91F_US_Put()
// Purpose      : This function is used to send a string through the US channel
// Parameters   :
// Return value :
// Created      : 2006-01-10 by TAK
// Updated      : 2006-01-10 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void AT91F_US_Put( char *buffer) // \arg pointer to a string ending by \0
{
	while(*buffer != '\0') {

                while (!AT91F_US_TxReady(COM1));
		AT91F_US_PutChar(COM1, *buffer++);
	}
}


//////////////////////////////////////////////////////////////////////////////
// Function     : Usart_init()
// Purpose      : USART initialization
// Parameters   :
// Return value :
// Created      : 2006-01-10 by TAK
// Updated      : 2006-01-10 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void Usart_init ( void )
{
    gpioClearBit(RS485_RSEL_BIT);  //RX = on

    COM1= AT91C_BASE_US1;
    //* Define RXD and TXD as peripheral
    // Configure PIO controllers to periph mode
    AT91F_PIO_CfgPeriph(
	 AT91C_BASE_PIOA, // PIO controller base address
           ((unsigned int) AT91C_PA21_RXD1    ) |
	   ((unsigned int) AT91C_PA22_TXD1    ) , // Peripheral A
	 0 ); // Peripheral B

    //* First, enable the clock of the PIOB
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<<AT91C_ID_US1 ) ;

    //* Usart Configure
    AT91F_US_Configure (COM1, MCK,AT91C_US_ASYNC_MODE,USART_BAUD_RATE , 0);

    //* Enable usart
    COM1->US_CR = AT91C_US_RXEN | AT91C_US_TXEN;

    //* open Usart interrupt
    AT91F_AIC_ConfigureIt (AT91C_BASE_AIC, AT91C_ID_US1, USART_INTERRUPT_LEVEL,
                          // AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE
                           AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED, ExternUARTISREntry);
    AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_US1);

    AT91F_PDC_Open (AT91C_BASE_PDC_US1);
    COM1->US_RPR = (unsigned int) Buff_InterruptRx;
    COM1->US_RCR = RS485buff_length;
    first = 0;
    RS485_FLAGbits.recived_msg=0;
    COM1->US_RTOR = 100;
    //* Enable USART IT error and AT91C_US_ENDRX
    AT91F_US_EnableIt(COM1,AT91C_US_RXBUFF /*| AT91C_US_TIMEOUT */);

    memset(Buff_InterruptRx, 255, (RS485buff_length+RS485buff_length));
    memset(Buff_Rx, 255, (RS485buff_length));
    
    timerSet(INX_RS485_MI_TX_TIMER, TIME_10_MS);
//* End
}


//////////////////////////////////////////////////////////////////////////////
// Function     : getRS485_msg()
// Purpose      : Get Value string from recived RS485
// Parameters   :
// Return value :
// Created      : 2006-01-13 by TAK
// Updated      : 2006-01-13 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
/*
unsigned char getRS485_msg( char *srRS485data)
{
     unsigned char sr_i;
     if ( first == 0 )
     {
        memcpy(srRS485data,Buff_rx2,RS485buff_length);
        memset(Buff_rx2, 255, (RS485buff_length+RS485buff_length));
     }
     else
     {
        memcpy(srRS485data,Buff_rx1,RS485buff_length);
        memset(Buff_rx1, 255, (RS485buff_length+RS485buff_length));
     }
     for ( sr_i=0; sr_i<RS485buff_length; sr_i++ )
     {
       if(srRS485data[sr_i]==255)
       {
         sr_i--;
         break;
       }
     }
     return sr_i;
}*/

//////////////////////////////////////////////////////////////////////////////
// Function     : isRS485_Interrupted()
// Purpose      : To
// Parameters   :
// Return value : None
// Created      : 2006-01-13 by TAK
// Updated      : 2006-01-13 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
/*char isRS485_Interrupted(void)
{
   char chRet = RS485_FLAGbits.recived_msg;
   RS485_FLAGbits.recived_msg = 0;

   return chRet;
}*/



//////////////////////////////////////////////////////////////////////////////
// Function     : RS485_TX_MI()
// Purpose      : Polling of Rs485, and reciver of MI message
// Parameters   :
// Return value : None
// Created      : 2007-02-04 by ES
// Updated      : 
// Comments     :
//////////////////////////////////////////////////////////////////////////////

void RS485_TX_MI (void)
{
   unsigned char ucResp[USB_QUEUE_ITEM_LENGTH];
   unsigned char MI485_Tx[4]={0x81,0,0x01};
   char checkSumStatus;
   
   if (Buff_InterruptRx[0] != 255 || Buff_InterruptRx[1] != 255 || Buff_InterruptRx[2] != 255)
   {
     int nOffset = 0;
     int i = 0;

     for(nOffset=0; nOffset < RS485buff_length; nOffset++)           //Fjerner alt til startbyte
     {
         if(Buff_InterruptRx[nOffset]==START_BYTE)
         {
            break;
         }
     }
    
     if (nOffset < RS485buff_length) // received msg with right startbyte
     {
          // If msg has been completed
          if( ((Buff_InterruptRx[nOffset+2]==0xc || Buff_InterruptRx[nOffset+2]==0xd || Buff_InterruptRx[nOffset+2]==0x13) && Buff_InterruptRx[nOffset+3]!=255) ||
             (Buff_InterruptRx[nOffset+22]!= 255))
          {
            
              m_nReceivedLength = 0;
             
              for (i = 0; i < RS485buff_length && Buff_InterruptRx[nOffset] != 255; i++) // copy to the receiving buffer
              {
                Buff_Rx[i] = Buff_InterruptRx[nOffset];
                nOffset ++;
                m_nReceivedLength ++;
              }
              memset(Buff_InterruptRx, 255, (RS485buff_length + RS485buff_length));
              COM1->US_RPR = (unsigned int) Buff_InterruptRx;
	      COM1->US_RCR = RS485buff_length;
                
              if (m_nReceivedLength > 0)
              {
                if (((Buff_Rx[2]==0xc || Buff_Rx[2]==0xd || Buff_Rx[2]==0x13) && Buff_Rx[3]!=255))
                {
                  timerSet(INX_RS485_MI_TX_TIMER, TIME_10_MS);
                  m_nReceivedLength = 4;
                }
                else
                {
                  m_nReceivedLength = 23;
                  timerSet(INX_RS485_MI_TX_TIMER, TIME_2000_MS);
                }
                  checkSumStatus = check_MIchecksum (Buff_Rx, &m_nReceivedLength);
     
                  if( checkSumStatus==1)
                  {
                    ecomBuildEventMessage(ucResp, EQ_TYPE_RS485, 0, POD_EVENT,  m_nReceivedLength, Buff_Rx, EQ_STATUS_CMD_OK);
                  }
                  else if( checkSumStatus==0)
                  {
                    ecomBuildEventMessage(ucResp, EQ_TYPE_RS485, 0, POD_EVENT,  m_nReceivedLength, Buff_Rx, EQ_STATUS_RS485_CHCKSUM_ERROR);
                  }
                  else
                  {
                    ecomBuildEventMessage(ucResp, EQ_TYPE_RS485, 0, POD_EVENT,  m_nReceivedLength, Buff_Rx, EQ_STATUS_RS485_LINGTH_OF_DATA_0);
                  }
                  usbqWriteUsbMsg(ucResp);
     
                  memset(Buff_Rx, 255, (RS485buff_length ));
                  m_nReceivedLength = 0;
                  
                  timerStop(INX_RS485_MI_ERROR_TIMER);
                  RS485_FLAGbits.com_error=0;
              }
          }
          else
          {
            if (RS485_FLAGbits.com_error==0)
            {
              timerSet(INX_RS485_MI_ERROR_TIMER,TIME_100_MS); // Starts timer to look after buffer errors
              timerSet(INX_RS485_MI_TX_TIMER, TIME_100_MS);
              RS485_FLAGbits.com_error=1;
            }
            else if (timerExpired( INX_RS485_MI_ERROR_TIMER))  // clears buffer if errors aqurs
            {
              memset(Buff_Rx, 255, (RS485buff_length));
              memset(Buff_InterruptRx, 255, (RS485buff_length + RS485buff_length)); 
              COM1->US_RPR = (unsigned int) Buff_InterruptRx;
	      COM1->US_RCR = RS485buff_length;
              timerSet(INX_RS485_MI_TX_TIMER, TIME_10_MS);
              RS485_FLAGbits.com_error=0;
              // !!! dette er feil med start byte  men vi har ikke en slik melding
              ecomBuildEventMessage(ucResp, EQ_TYPE_RS485, 0, POD_EVENT,  m_nReceivedLength, Buff_Rx, EQ_STATUS_RS485_CHCKSUM_ERROR);
            }
            
          }
        }
        else // !!! ERROR, RECEIVED BUFFER WITHOUT START BYTE
        {
          memset(Buff_Rx, 255, (RS485buff_length));
           memset(Buff_InterruptRx, 255, (RS485buff_length + RS485buff_length)); 
           COM1->US_RPR = (unsigned int) Buff_InterruptRx;
	      COM1->US_RCR = RS485buff_length;

              timerStop(INX_RS485_MI_ERROR_TIMER);
                  RS485_FLAGbits.com_error=0;
                  ecomBuildEventMessage(ucResp, EQ_TYPE_RS485, 0, POD_EVENT,  m_nReceivedLength, Buff_Rx, EQ_STATUS_RS485_CHCKSUM_ERROR);
        }
   }

   ////////////////////////////////////////////////////////////////////////////////
   // SEND TO POD IF MSG FROM MI HAS BEEN RECEIVED 
   ////////////////////////////////////////////////////////////////////////////////
   
 
   
    ////////////////////////////////////////////////////////////////////////////////
   // CHECK ABOUT THE NEXT MI HAS SOMETHING TO SEND
   ////////////////////////////////////////////////////////////////////////////////
   if (timerExpired( INX_RS485_MI_TX_TIMER)) //Every 10 ms
   {
      MI_adress++; //next MI
      if(MI_adress > MAX_MI_ADRESS)
      {
        MI_adress=1;
      }
      
      MI485_Tx[1]=MI_adress;
      AT91F_US_TX_no_of_byte(MI485_Tx,3);

      timerSet(INX_RS485_MI_TX_TIMER, TIME_10_MS);
    }
}




//////////////////////////////////////////////////////////////////////////////
// Function     : AT91F_US_TX_no_of_byte()
// Purpose      : This function is used to send a x no of byte through the US channel with checksum
// Parameters   :
// Return value :
// Created      : 2006-02-01 by TAK
// Updated      : 2006-06-29 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
void AT91F_US_TX_no_of_byte( char *buffer,char no_of_byte) // \arg pointer to a string ending by \0
{
   char i;
   int checksum = 0;
   
   // Hvis den er ikke her, noen ganger POD melding blir ikke mottat på MI
   StartTimer2_init (TIME_600_US);        
   while(Timer2done()==0)
   {
   }
   
   timerSet(INX_RS485_MI_TX_TIMER,TIME_100_MS);
   while (!AT91F_US_TxReady(AT91C_BASE_US1))
   {
      if (timerExpired( INX_RS485_MI_TX_TIMER))
        break;
   }
   COM1->US_CR = AT91C_US_RXDIS; //DISABLE RECEIVER
   gpioSetBit(RS485_DENA_BIT); // TX=ON

   // Hvis den er ikke her, stor sjanse for å motta feil melding fra MI (ser ut som at
   // mottate bytes blir forskjivet og f.eks. i stedet av 0x30 blir mottatt 0x82
   StartTimer2_init (TIME_300_US);      
   while(Timer2done()==0)
   {
   }

   
   timerSet(INX_RS485_MI_TX_TIMER,TIME_100_MS);
   while( (AT91F_PIO_GetInput( AT91C_BASE_PIOA) & RS485_DENA_BIT ) != RS485_DENA_BIT)
   {
      if (timerExpired( INX_RS485_MI_TX_TIMER))
        break;
   }

   for(i=0;i<no_of_byte;i++)
   {
      timerSet(INX_RS485_MI_TX_TIMER,TIME_100_MS);
      while (!AT91F_US_TxReady(COM1))
      {
         if (timerExpired( INX_RS485_MI_TX_TIMER))
           break;
      }
      AT91F_US_PutChar(COM1, *buffer);
      checksum= *buffer + checksum;
      *buffer++;
   }

   checksum=checksum & 0x7F; //1111111 bin
   timerSet(INX_RS485_MI_TX_TIMER,TIME_100_MS);
   while (!AT91F_US_TxReady(COM1))
   {
      if (timerExpired( INX_RS485_MI_TX_TIMER))
        break;
   }
   AT91F_US_PutChar(COM1,checksum);


   timerSet(INX_RS485_MI_TX_TIMER,TIME_100_MS);
   while(1)
   {
      if((COM1->US_CSR & AT91C_US_TXEMPTY) == AT91C_US_TXEMPTY &&
         (COM1->US_CSR & AT91C_US_TXBUFE)  == AT91C_US_TXBUFE  &&
         (COM1->US_CSR & AT91C_US_ENDTX)   == AT91C_US_ENDTX      )
        break;
      if (timerExpired( INX_RS485_MI_TX_TIMER))
        break;
   }

   gpioClearBit(RS485_DENA_BIT);  //TX=off
 /*  StartTimer2_init (TIME_20_US);        //200us delay
   while(Timer2done()==0)
   {
   }*/
   gpioClearBit(RS485_RSEL_BIT);  //RX = on

   // hvis den er her, kan miste første byte i mottatte melding fra MI
   /*StartTimer2_init (TIME_200_US);        //200us delay
   while(Timer2done()==0)
   {
   }*/
   
   COM1->US_CR = AT91C_US_RXEN; //ENABLE RECEIVER
   
//timerSet(INX_RS485_MI_TX_TIMER, TIME_10_MS);
   if(no_of_byte==22)
     timerSet(INX_RS485_MI_TX_TIMER, TIME_400_MS); //(Kan kanskje settes til 300ms, avhengig av MI SW pollux)
   else
     timerSet(INX_RS485_MI_TX_TIMER, TIME_10_MS);
}

//////////////////////////////////////////////////////////////////////////////
// Function     : check_MIchecksum()
// Purpose      : This function is used to send a x no of byte through the US channel with checksum
// Parameters   :
// Return value :
// Created      : 2006-02-01 by TAK
// Updated      : 2006-06-28 by TAK
// Comments     :
//////////////////////////////////////////////////////////////////////////////
char check_MIchecksum ( char *buffer, int *no_of_byte)
{
   char i;
   unsigned int checksum=0;
   if(*no_of_byte==0)
     return (2);
   for(i=0;i<(*no_of_byte-1);i++)
   {
      checksum = buffer[0] + checksum;
      *buffer++;
   }
   checksum=checksum & 0x7F; //1111111 bin
   *no_of_byte=*no_of_byte-1;
   if(checksum!=buffer[0])
   {
      buffer[0]=0;
      return(0);
   }
   else
   {
      buffer[0]=0;
      return(1);
   }
}

