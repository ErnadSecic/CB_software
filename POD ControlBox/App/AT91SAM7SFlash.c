
/* Include Standard c Libraries to allow stand alone compiling and operation */
#include <stdio.h>

#include "Board.h"



#include "AT91SAM7SFlash.h"
extern void AT91F_LowLevelInit( void) ;
static unsigned char RAMSectorData[FLASH_PAGE_SIZE_BYTE];
//static int basseAddress ;
//static unsigned int NbByte;
static unsigned int page,page_prev;

unsigned char nRamAddress;


__ramfunc int AT91F_Flash_Write( unsigned int Flash_Address , unsigned int nPage, int size ,unsigned int * buff);
__ramfunc void AT91F_Flash_Ready (void);
__ramfunc void AT91F_Flash_Unlock(unsigned int Flash_Lock_Page);



//*----------------------------------------------------------------------------
//* \fn    AT91F_Flash_Ready
//* \brief Wait the flash ready
//*----------------------------------------------------------------------------
__ramfunc void AT91F_Flash_Ready (void)
{
    //* Wait the and of command
     while ((AT91C_BASE_MC->MC_FSR & AT91C_MC_FRDY) != AT91C_MC_FRDY ) {};
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_Flash_Unlock
//* \brief Clear the Non Volatile Memory Bits and set at 1 FSR bit=0
//* \input page number (0-1023) a same region have some page (16)
//* \output Region
//*----------------------------------------------------------------------------
__ramfunc void AT91F_Flash_Unlock(unsigned int Flash_Lock_Page)
{
    unsigned int nSector;
    AT91C_BASE_MC->MC_FMR = ((AT91C_MC_FMCN)&(48 <<16)) | AT91C_MC_FWS_2FWS ;

    nSector = Flash_Lock_Page / FLASH_PAGE_SIZE_LONG;

  AT91F_Flash_Ready();
  //* Write the Errase All command

  if ((AT91C_BASE_MC->MC_FSR & (AT91C_MC_LOCKS12 << nSector)) == (AT91C_MC_LOCKS12 << nSector))
{
        AT91C_BASE_MC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_UNLOCK | (AT91C_MC_PAGEN & (Flash_Lock_Page << 8) ) ;
}
    //* Wait the and of command
        AT91F_Flash_Ready();
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_Flash_Write
//* \brief Write in one Flash page located in AT91C_IFLASH,  size in 32 bits
//* \input Flash_Address: start at 0x0010 0000 size: in byte
//*----------------------------------------------------------------------------
__ramfunc int AT91F_Flash_Write( unsigned int Flash_Address , unsigned int nPage, int size ,unsigned int * buff)
{    //* set the Flasc controler base address
    AT91PS_MC ptMC = AT91C_BASE_MC;
    unsigned int i;
    unsigned int * Flash;
//    sprintf(message,"Download:add %x\n",Flash_Address);
//    FlMessageLog(message);

    //* init flash pointer
  // Flash = (unsigned int *) 0x00100000;
       AT91C_BASE_MC->MC_FMR = ((AT91C_MC_FMCN)&(72 <<16)) | AT91C_MC_FWS_2FWS ;
       
       Flash = (unsigned int *) Flash_Address;
    //* Get the Flasg page number
       // nPage = ((Flash_Address - (unsigned int)AT91C_IFLASH ) /FLASH_PAGE_SIZE_BYTE) & AT91C_PAGE_MASK;
   //* copy the new value
	for (i=0; (i < FLASH_PAGE_SIZE_BYTE) & (size > 0) ;i++, Flash++,buff++,size-=4 )
        {
	    //* copy the flash to the write buffer ensure that code generation
	    *Flash=*buff;
	}
        AT91F_Flash_Ready();
        
        //* Write the Errase_All command
        ptMC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_START_PROG | (AT91C_MC_PAGEN & (nPage <<8)) ;
        //* Wait the end of command
         AT91F_Flash_Ready();
        //* Check the result
        if ( (ptMC->MC_FSR & ( AT91C_MC_PROGE | AT91C_MC_LOCKE ))!=0) 
        {
          return false;
        }
  return true;
}
//*----------------------------------------------------------------------------
//* \fn    FlashWriteByte
//* \brief The bytes are buffered in sectorbuf. The sectorbuf is written to
//*        the flash when it overflows.
//* \input Flash_Address: start at 0x0010 0000 size: in byte
//*----------------------------------------------------------------------------

__ramfunc  void FlashWriteByte(unsigned long  addr, int byteToWrite)
{
  unsigned int nRest;



  page  =(addr ) /FLASH_PAGE_SIZE_BYTE;

  // IF  END of booting
  if (byteToWrite == -1)
  {
        AT91F_Flash_Unlock(page);
        
        
        nRest = addr - (addr / FLASH_PAGE_SIZE_BYTE) * FLASH_PAGE_SIZE_BYTE; // = addr % FLASH_PAGE_SIZE_BYTE

        //* Write page and get status
        if (!AT91F_Flash_Write( (page*FLASH_PAGE_SIZE_BYTE)+ FLASH_ADDRESS, page, nRest , (unsigned int *) &RAMSectorData))
        {
          //     AT91F_Flash_Ready();
        }
 	// Copy Rom page to RAM page
        /*for (NbByte = 0; NbByte < FLASH_PAGE_SIZE_BYTE; NbByte++)
        {
           RAMSectorData[NbByte] = ((unsigned char*) ((page*FLASH_PAGE_SIZE_BYTE)+ FLASH_ADDRESS )) [NbByte]; //basseAddress)) [NbByte];
        }*/
        return;
  } // end flush

  //* Check if page must write
  if (page != page_prev)
  {// New Page
         //  ENO   sprintf(message,"Download:page %d\n",page_prev);
         //  ENO FlMessageLog(message);
        //* Unlock current sector base address - curent address by sector size
        AT91F_Flash_Unlock(page_prev);
        // Write page and get status
        
        
        if (!AT91F_Flash_Write( (page_prev*FLASH_PAGE_SIZE_BYTE)+ FLASH_ADDRESS , page_prev, FLASH_PAGE_SIZE_BYTE, (unsigned int *) &RAMSectorData)) 
        {
          //  ENO   sprintf(message,"Flasher cannot write page: %d\n",page_prev);
          //  ENO   FlMessageBox(message);
      // Her skulle returnere false      FlErrorExit();
        }
 	// Copy Rom page to RAM page
    /*    for (NbByte = 0; NbByte < FLASH_PAGE_SIZE_BYTE; NbByte++)
        {
           RAMSectorData[NbByte] = ((unsigned char*) ((page_prev*FLASH_PAGE_SIZE_BYTE)+ FLASH_ADDRESS )) [NbByte]; //basseAddress)) [NbByte];
        }*/
        // set works page
        page_prev = page ;
   }


    RAMSectorData[nRamAddress ++] = (unsigned char)byteToWrite;

    if (nRamAddress >= FLASH_PAGE_SIZE_BYTE)
    {
      nRamAddress = 0;
    }
}

//*----------------------------------------------------------------------------
//* \fn    FlashDriverInitialize
//* \brief Init the Flash driver
//*----------------------------------------------------------------------------
void FlashDriverInitialize(void) //int argc, char const* argv[])
{    // check param

  //  basseAddress = 0;
  //  AT91F_LowLevelInit();
 //  ENO    sprintf(message,"Downloader Version 1.1\n");
 //  ENO    FlMessageLog(message);

    // get the Flash base address
/* ENO    if (argc >= 2){
        NbByte=sprintf(message,"%s",argv[1]);
        if (message[1]!='x'){
            FlMessageBox("Param ERROR ! must be 0xYYYYYY");

         FlErrorExit();
        }
        for (i=0 ; NbByte > 1 ; NbByte--, i++){
            val = message[NbByte-1] & 0x0f;
            if ( message[NbByte-1]  > 0x40)  val += 9;
            basseAddress +=  val << (4 *i) ;
        }
        status = true;
      //  ENO   sprintf(message,"Download: AT91SAM7Sx At:%s",argv[1]);
    }
    else
    {*/
     //   basseAddress = FLASH_ADDRESS;
     //  ENO    sprintf(message,"Download1: AT91SAM7Sx At: 0x%X\n",basseAddress);
     //   status = false;
   // }

     //* Get the Flash version
  //  ENO   FlMessageLog(message);
    //* Get the Flash version
  //  ENO   sprintf(message,"Download: AT91SAM7Sx Version: 0x%X\n",*AT91C_MC_FVR);
  //  ENO   FlMessageLog(message);
   // get page
  //  page  =  (((basseAddress )- (unsigned int) FLASH_ADDRESS ) /FLASH_PAGE_SIZE_BYTE)&AT91C_PAGE_MASK;
    page = 0; //100;
    page_prev = 0; //100;
     nRamAddress = 0;
    /*
    for (NbByte = 0; NbByte < FLASH_PAGE_SIZE_BYTE; NbByte++)
    {
        RAMSectorData[NbByte] = ((unsigned char*) ( (page_prev*FLASH_PAGE_SIZE_BYTE)+basseAddress)) [NbByte];
    }*/
    // if (status) basseAddress = 0;

    //* Set Flash Waite sate
    //  Single Cycle Access at Up to 30 MHz
    //  if MCK = 47923200 I have 50 Cycle for 1 useconde ( flied MC_FMR->FMCN)
    //  = A page erase is performed before programming.
      // AT91C_BASE_MC->MC_FMR = ((AT91C_MC_FMCN)&(48 <<16)) | AT91C_MC_FWS_1FWS ;
  AT91C_BASE_MC->MC_FMR = ((AT91C_MC_FMCN)&(72 <<16)) | AT91C_MC_FWS_2FWS ;
    // Register the flash write function.
    //FlRegisterWriteFunction(FlashWriteByte);
}

