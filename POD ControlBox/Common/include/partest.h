

#ifndef PARTEST_H
#define PARTEST_H

#define partstDEFAULT_PORT_ADDRESS		( ( unsigned portSHORT ) 0x378 )

void vParTestInitialise( void );
void vParTestSetLED( unsigned portBASE_TYPE uxLED, signed portBASE_TYPE xValue );
void vParTestToggleLED( unsigned portBASE_TYPE uxLED );

#endif

