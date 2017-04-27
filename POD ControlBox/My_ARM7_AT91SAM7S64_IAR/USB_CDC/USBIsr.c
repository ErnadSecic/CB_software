//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.01
//////////////////////////////////////////////////////////////////////////////////////

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo application includes. */
#include "board.h"
#include "usb.h"
#include "USB-CDC.h"

#define usbINT_CLEAR_MASK	(AT91C_UDP_TXCOMP | AT91C_UDP_STALLSENT | AT91C_UDP_RXSETUP | AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RX_DATA_BK1 )
/*-----------------------------------------------------------*/

/* Messages and queue used to communicate between the ISR and the USB task. */
static xISRStatus xISRMessages[ usbQUEUE_LENGTH + 1 ];
extern xQueueHandle xUSBInterruptQueue;
/*-----------------------------------------------------------*/

/* The ISR can cause a context switch so is declared naked. */
__arm  void vUSB_ISR( void ); // ENO __attribute__ ((naked));

/*-----------------------------------------------------------*/


//__arm void vUSB_ISR( void ) // eno
__ramfunc void vUSB_ISR( void ) // eno
{
	/* This ISR can cause a context switch.  Therefore a call to the
	portENTER_SWITCHING_ISR() macro is made.  This must come BEFORE any
	stack variable declarations. */
	// ENO portENTER_SWITCHING_ISR();

	/* Now variables can be declared. */
	portCHAR cTaskWokenByPost = pdFALSE;
	static volatile unsigned portLONG ulNextMessage = 0;
	xISRStatus *pxMessage;
	unsigned portLONG ulRxBytes;
	unsigned portCHAR ucFifoIndex;

    /* Use the next message from the array. */
	pxMessage = &( xISRMessages[ ( ulNextMessage & usbQUEUE_LENGTH ) ] );
	ulNextMessage++;

    /* Save UDP ISR state for task-level processing. */
	pxMessage->ulISR = AT91C_BASE_UDP->UDP_ISR;
	pxMessage->ulCSR0 = AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_0 ];

    /* Clear interrupts from ICR. */
	AT91C_BASE_UDP->UDP_ICR = AT91C_BASE_UDP->UDP_IMR | AT91C_UDP_ENDBUSRES;
	

	/* Process incoming FIFO data.  Must set DIR (if needed) and clear RXSETUP
	before exit. */

    /* Read CSR and get incoming byte count. */
	ulRxBytes = ( pxMessage->ulCSR0 >> 16 ) & usbRX_COUNT_MASK;
	
	/* Receive control transfers on endpoint 0. */
	if( pxMessage->ulCSR0 & ( AT91C_UDP_RXSETUP | AT91C_UDP_RX_DATA_BK0 ) )
	{
		if(ulRxBytes>8)
                  ulRxBytes=8;

          /* Save FIFO data buffer for either a SETUP or DATA stage */
		for( ucFifoIndex = 0; ucFifoIndex < ulRxBytes; ucFifoIndex++ )
		{
			pxMessage->ucFifoData[ ucFifoIndex ] = AT91C_BASE_UDP->UDP_FDR[ usbEND_POINT_0 ];
		}

		/* Set direction for data stage.  Must be done before RXSETUP is
		cleared. */
		if( ( AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_0 ] & AT91C_UDP_RXSETUP ) )
		{
			if( ulRxBytes && ( pxMessage->ucFifoData[ usbREQUEST_TYPE_INDEX ] & 0x80 ) )
			{
				AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_0 ] |= AT91C_UDP_DIR;

				/* Might not be wise in an ISR! */
				while( !(AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_0 ] & AT91C_UDP_DIR) );
			}

			/* Clear RXSETUP */
			AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_0 ] &= ~AT91C_UDP_RXSETUP;

			/* Might not be wise in an ISR! */
			while ( AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_0 ] & AT91C_UDP_RXSETUP );
		}
		else
		{
		   /* Clear RX_DATA_BK0 */
		   AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_0 ] &= ~AT91C_UDP_RX_DATA_BK0;

		   /* Might not be wise in an ISR! */
		   while ( AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_0 ] & AT91C_UDP_RX_DATA_BK0 );
		}
	}
	
	/* If we received data on endpoint 1, disable its interrupts until it is
	processed in the main loop */
	if( AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_1 ] & ( AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RX_DATA_BK1 ) )
	{
		AT91C_BASE_UDP->UDP_IDR = AT91C_UDP_EPINT1;
	}
	
	AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_0 ] &= ~( AT91C_UDP_TXCOMP | AT91C_UDP_STALLSENT );

	/* Clear interrupts for the other endpoints, retain data flags for endpoint
	1. */
	AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_1 ] &= ~( AT91C_UDP_TXCOMP | AT91C_UDP_STALLSENT | AT91C_UDP_RXSETUP );
	AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_2 ] &= ~usbINT_CLEAR_MASK;
	AT91C_BASE_UDP->UDP_CSR[ usbEND_POINT_3 ] &= ~usbINT_CLEAR_MASK;

	/* Post ISR data to queue for task-level processing */
	cTaskWokenByPost = xQueueSendFromISR( xUSBInterruptQueue, &pxMessage, cTaskWokenByPost );

	/* Clear AIC to complete ISR processing */
	AT91C_BASE_AIC->AIC_EOICR = 0;

	/* Do a task switch if needed */
	// ENO portEXIT_SWITCHING_ISR( cTaskWokenByPost );
}

