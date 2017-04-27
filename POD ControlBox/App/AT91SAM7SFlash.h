//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : AT91SAM7SFlash.h
//* Object              : Flash constan description
//* Creation            : JPP  18/Aug/2004
//* 07/Feb/05  JPP      : Set the page mask
//*----------------------------------------------------------------------------


#ifndef Flash_h
#define Flash_h


#define true	-1
#define false	0

#define AT91C_MC_CORRECT_KEY  ((unsigned int) 0x5A << 24) // (MC) Correct Protect Key

#define AT91C_PAGE_MASK       ((unsigned int) 0x0000FFFF ) //

/*-------------------------------*/
/* Flash Status Field Definition */
/*-------------------------------*/
#define AT91C_MC_FVR    		  ((AT91_REG *) 	0xFFFFFF6C) // (MC) MC Flash Version Register
#define AT91C_MC_FLASH_VERSION    (0x00000110) 					// (MC) MC Flash Version Register for SAM7S64 first

/*-------------------------------*/
/* Flash Status Field Definition */
/*-------------------------------*/

#define 	AT91C_MC_FSR_MVM 		((unsigned int) 0xFF << 8)		// (MC) Status Register GPNVMx: General-purpose NVM Bit Status
#define 	AT91C_MC_FSR_LOCK 		((unsigned int) 0xFFFF << 16)	// (MC) Status Register LOCKSx: Lock Region x Lock Status


#define	 ERASE_VALUE 		0xFFFFFFFF

/*-----------------------*/
/* Flash size Definition */
/*-----------------------*/
/* 64 Kbytes of Internal High-speed Flash, Organized in 512 Pages of 128 Bytes */

#define  FLASH_PAGE_SIZE_BYTE	256  // 128 for at91sam7s64
#define  FLASH_PAGE_SIZE_LONG	64  // 32 for at91sam7s64

#ifndef FLASH_ADDRESS //FLASH_BASE_ADDRESS
#define  FLASH_ADDRESS		0x00100000
#endif
//#define  FLASH_ADDRESS		        0x00100000


__ramfunc  void FlashWriteByte(unsigned long  addr, int byteToWrite);
void FlashDriverInitialize(void);

/*------------------------------*/
/* External function Definition */
/*------------------------------*/
//void AT91F_LowLevelInit2( void);
/* Flash function */
/*extern void AT91F_Flash_Init(void);
extern int AT91F_Flash_Check_Erase(unsigned int * start, unsigned int size);
extern int AT91F_Flash_Erase_All(void);
*/
//extern int AT91F_Flash_Write( unsigned int Flash_Address , unsigned int page, int size ,unsigned int * buff);
//extern int AT91F_Flash_Write_all( unsigned int Flash_Address ,int size ,unsigned int * buff);

/* Lock Bits functions */
//extern int AT91F_Flash_Lock_Status(void);
//extern int AT91F_Flash_Lock (unsigned int Flash_Lock);

 __ramfunc void AT91F_Flash_Unlock(unsigned int Flash_Lock);
//extern

/* NVM bits functions */
/*extern int AT91F_NVM_Status(void);
extern int AT91F_NVM_Set (unsigned char NVM_Number);
extern int AT91F_NVM_Clear(unsigned char NVM_Number);
*/
/* Security bit function */
/*extern int AT91F_SET_Security_Status (void);
extern int AT91F_SET_Security (void);
*/
#endif /* Flash_h */
