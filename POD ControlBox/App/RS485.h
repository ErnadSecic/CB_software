/*---------------------------------------------------------------------------
* File Name           : RS485.h
* Object              : Control box file.
*
* Creation            : TAK   09/01/2006
* Modif               :
*----------------------------------------------------------------------------
*/

#ifndef __RS485_h
#define __RS485_h

#define USART_BAUD_RATE 		9600
#define RS485buff_length                49
#define MAX_MI_ADRESS                   31
#define START_BYTE                  0x81

typedef struct RS485_struct {
   unsigned char recived_msg;
   unsigned char com_error;
}RS485_FLAGS;

__arm void Usart_c_irq_handler(void);

void Usart_init ( void );
void AT91F_US_Put( char *buffer); // \arg pointer to a string ending by \0
//unsigned char getRS485_msg( char *srRS485data);
//char isRS485_Interrupted(void);
void RS485_TX_MI (void);
void AT91F_US_TX_no_of_byte( char *buffer,char no_of_byte);
char check_MIchecksum ( char *buffer, int *no_of_byte);

#endif /*RS485.h*/
