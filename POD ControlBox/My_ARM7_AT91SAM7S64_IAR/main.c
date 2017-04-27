//////////////////////////////////////////////////////////////////////////////////////
//  VERSION 1.01
//////////////////////////////////////////////////////////////////////////////////////

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo application includes. */
#include "integer.h"
#include "PollQ.h"
#include "BlockQ.h"
#include "semtest.h"
#include "dynamic.h"
#include "comtest2.h"

#include "UsbQ.h"
#include "IoCtrl.h"
#include "Timer.h"
#include "Gpio.h"
#include "Motor.h"
#include "Adc.h"
#include "Ecom.h"


#include "USB_CDC/USB-CDC.h"
#include "DIP_switch.h"


///////////////////////////////////////////////////
// USED BY BOOTLOADER
///////////////////////////////////////////////////
#include "AT91SAM7SFlash.h"
#include "cdc_enumerate.h"


#define SIZEBUFMAX 	64
struct _AT91S_CDC 	pCDC;

void MainLoop(int nFileLength);
void AT91F_USB_Open(void);
void BootloaderStart(int nFileLength);
///////////////////////////////


// Priorities for the demo application tasks.

/* Constants required by the 'Check' task. */
#define mainNO_ERROR_FLASH_PERIOD	( ( portTickType ) 3000 / portTICK_RATE_MS  )
#define mainERROR_FLASH_PERIOD		( ( portTickType ) 500 / portTICK_RATE_MS  )
#define mainCHECK_TASK_LED			( 3 )

/* Constants for the ComTest tasks. */
#define mainCOM_TEST_BAUD_RATE		( ( unsigned portLONG ) 115200 )
#define mainCOM_TEST_LED			( 4 ) /* Off the board. */

/*
 * The task that executes at the highest priority and calls
 * prvCheckOtherTasksAreStillRunning().  See the description at the top
 * of the file.
 */
//static void vErrorChecks( void *pvParameters );

/*
 * Configure the processor for use with the Atmel demo board.  Setup is minimal
 * as the low level init function (called from the startup asm file) takes care
 * of most things.
 */
static void prvSetupHardware( void );


///////////////////////////////////////////////////
// USED BY BOOTLOADER
///////////////////////////////////////////////////
/*
 * Part of bootloader. Open new USB (and run it in the RAM) without enumeration.
 */
 void AT91F_USB_Open(void)
 {
// Set the PLL USB Divider
    AT91C_BASE_CKGR->CKGR_PLLR |= AT91C_CKGR_USBDIV_1 ;

    // Specific Chip USB Initialisation
    // Enables the 48MHz USB clock UDPCK and System Peripheral USB Clock
    AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_UDP;
    AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_UDP);

    // Enable UDP PullUp (USB_DP_PUP) : enable & Clear of the corresponding PIO
    // Set in PIO mode and Configure in Output
  /*  AT91F_PIO_CfgOutput(AT91C_BASE_PIOA,AT91C_PIO_PA16);

    AT91F_PIO_SetOutput(AT91C_BASE_PIOA,AT91C_PIO_PA16);
    for(i= 0; i <5000; i++)
     __asm("nop");
    // Clear for set the Pul up resistor
    AT91F_PIO_ClearOutput(AT91C_BASE_PIOA,AT91C_PIO_PA16);
*/
    // CDC Open by structure initialization
    AT91F_CDC_Open(&pCDC, AT91C_BASE_UDP);
}


/*
 * Checks that all the demo application tasks are still executing without error
 * - as described at the top of the file.
 */
//static portLONG prvCheckOtherTasksAreStillRunning( void );


/*-----------------------------------------------------------*/


// int iSwType;  // this value is changed after reading of ADC in "void main( void )"

// unsigned char m_nUnitID = ERROR_ID;

/*
 * Starts all the other tasks, then starts the scheduler.
 */
void main( void )
{
   char Board_type;
   /* Setup any hardware that has not already been configured by the low
   level init routines. */

   prvSetupHardware();
   Init_GPIO_Common();
   Board_type = Card_ID_check();
   
   if (Board_type == 0)
   {
      motorInit();      // NOTE: motorInit() MUST be called here to switch OFF PWM !!!!!!!!!!!
   }
   else
   {
     Init_GPIO_for_SensorUnit();
     gpioSetBit(LAMP2_BIT);
   }

   
   adcInit();

   // Create the USB queues:
   if (usbqInit() != 1)
   {
     // !!!! RESTART, SOMETHING IS WRONG
   }


   // Start USB tasks
   xTaskCreate( vUSBCDCTask, "USB-CDC", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL ); // ENO

   // Start IO task:
   xTaskCreate( vIoCtrl, ( const signed portCHAR * const ) "IoCtrl", configMINIMAL_STACK_SIZE+100, NULL, (tskIDLE_PRIORITY + 2), ( xTaskHandle * ) NULL );
//   xTaskCreate( vIoCtrl, ( const signed portCHAR * const ) "IoCtrl", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES, ( xTaskHandle * ) NULL );

	
   /* Start the scheduler. */
    vTaskStartScheduler();

   /* We should never get here as control is now taken by the scheduler. */
   return;
}
/*-----------------------------------------------------------*/


///////////////////////////////////////////////////
// USED BY BOOTLOADER
///////////////////////////////////////////////////
/*
 * Part of bootloader. Stop operativ system, switch interrupts off, initialise bootloader and start it.
 */
void BootloaderStart(int nFileLength)
{
  
   vTaskEndScheduler();
   AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF; // dissable alle interrupts

   FlashDriverInitialize();//argc, argv);

   AT91F_USB_Open();


   // !!! DON'T Wait enumeration
   // while(!pCDC.IsConfigured(&pCDC));

   MainLoop(nFileLength);
}

///////////////////////////////////////////////////
// USED BY BOOTLOADER
///////////////////////////////////////////////////

/*
 * Part of bootloader. Wait on data and write it to FLASH
 */
__ramfunc void MainLoop(int nFileLength)
{
  unsigned long curr_addr;
  char data[SIZEBUFMAX];
  int len, i, nRest;
  curr_addr = 0;

  data[0] = 31;
  data[1] = 31;
  data[2] = 31;
  data[3] = 31;
  data[4] = 31;

  AT91F_UDP_Write(&pCDC, data, 5);

  nRest = nFileLength;

  while (nRest > 0)
  {
    AT91C_BASE_WDTC->WDTC_WDCR = 0xA5000001; // watchdog reset timer
    
    len = AT91F_UDP_Read(&pCDC, data, SIZEBUFMAX);
    char* chTemp = data;

    nRest -= len;

    if (len)
    {
        for (i = 0; i < len; i++)
        {
          FlashWriteByte(curr_addr++, *chTemp); // Send byte to flash loader driver.
          chTemp++;
        }
        /*
        if (len < 64)
        {
          FlashWriteByte(curr_addr, -1); // Send END-byte to flash loader driver.
          // SOFTWARE RESET
          AT91PS_RSTC pReset = AT91C_BASE_RSTC;
          pReset->RSTC_RMR=0xa5000F00;
          pReset->RSTC_RCR=0xa500000D;
        }
        */
    }
  }

  FlashWriteByte(curr_addr, -1); // Send END-byte to flash loader driver.
  
 /* for(i= 0; i <5000; i++)
     __asm("nop");
  */
  for(i= 0; i <500; i++)
     __asm("nop");
  // SOFTWARE RESET
  AT91PS_RSTC pReset = AT91C_BASE_RSTC;
  pReset->RSTC_RMR=0xa5000900;
  for(i= 0; i <500; i++)
     __asm("nop");
  pReset->RSTC_RCR=0xa500000D;
}
///////////////////////////////////////////////////////////////////////////////


static void prvSetupHardware( void )
{
	/* When using the JTAG debugger the hardware is not always initialised to
	the correct default state.  This line just ensures that this does not
	cause all interrupts to be masked at the start. */
	AT91C_BASE_AIC->AIC_EOICR = 0;
	
	/* Most setup is performed by the low level init function called from the
	startup asm file. */

}
