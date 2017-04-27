//#include "UsbQ.h"


// Max number of items to be post in queue:
#define  USB_QUEUE_ITEMS            5

// Fixed length of queue item
#define  USB_QUEUE_ITEM_LENGTH      64

// Used from main:
int usbqInit( void );

// Used from IO:
int usbqReadUsbMsg(unsigned char *pMsg);
int usbqWriteUsbMsg(unsigned char *pMsg);

// Used from USB:
int usbqReadIoMsg(unsigned char *pMsg);
int usbqWriteIoMsg(unsigned char *pMsg);

