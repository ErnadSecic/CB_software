/*---------------------------------------------------------------------------
* File Name           : Fallsensor.h
* Object              : Control box file.
*
* Creation            : TAK   13/02/2006
* Modif               :
*----------------------------------------------------------------------------
*/

#ifndef __Fallsensor_h
#define __Fallsensor_h

#define FALL_TRIGG_LIMIT_HIGH                      100
#define FALL_TRIGG_LIMIT_LOW                       15
#define FALL_TRIGG_LIMIT_GROUP_HIGH                6
#define FALL_TRIGG_LIMIT_GROUP_LOW                 3

// Ernad
#define NO_OF_CYCLE_FOR_CALCUTATE_AVERAGE        400
#define MAX_AVERAGE                              900


void Switch_IR_LED_IRQ (void);
void CAP_IR_IRQ(void);
void TX_FALL_TEST(void);
int isFall3_Interrupted(void);
unsigned char getFall_msg( char *sFALLdata,char LEDNO);
void Start_Fall_3(void);
void Stop_Fall_3(void);
void Fall3_Continue(void);
char Status_FallSensor_3(void);
char isFall3_Event_Enable(void);
void Fall_3_Event_Enable_Disable(char value);
void ClearSensor3Flags(char bClear);
int GetSensor3blocked(void);
char isFall3_Enabled(void);
void GetStatusMessage(char* chResult);
#endif /*Fallsensor.h*/
