#ifndef I2C_INTERFACE_H
#define I2C_INTERFACE_H

#define  LED_I2C_INT_CB_ADRESS        0x20

///////////////////////////////////////////////////////////
// Must be called once before any calls to i2cMessage.
void Init_I2C( void );
//int I2C_ReceiveDS1621(char cSlaveAddress, char cInternalAddress, char* pcToRead, unsigned int nLengthToRead);
int I2C_Write(char cSlaveAddress, char cInternalAddress, int nHasInternalAddress, char* pcToWrite, unsigned int nLengthToWrite);
/*void I2C_TempstartDS1621(char cSlaveAddress);
void I2C_StartConvDS1621(char cSlaveAddress);
int I2C_ReceiveDS1621First(char cSlaveAddress, char cInternalAddress, char* pcToRead, unsigned int nLengthToRead);*/
int I2C_Receive(char cSlaveAddress, int nUseInternalAddress, char cInternalAddress,  char* pcToRead, unsigned int nLengthToRead);
//void I2C_Start_after_Reset(void);
#endif

