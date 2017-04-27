
#ifndef BUTTON_H
#define BUTTON_H



#define BUTTON_NOT_CONNECTED_LEVEL  900

// return values
typedef enum
{
   BUTTON_NOT_PUSHED,
   BUTTON_PUSHED_DETECTED,
   BUTTON_RELEASED_DETECTED,
   BUTTON_NOT_CONNECTED
} eButtonState;



typedef enum
{
  BUTTON_NOT_ACTIVATED      = 0,
  BUTTON_ACTIVATED          = 1
} eButtonActiv;

void ResetButtons(void);
void ActivateButton(int iMotorNo);

int buttonPusherSwitchOnceMeasuring(int iMotorNo);
int buttonPusherSwitch(int iMotorNo);
void ResetButton(int iMotorNo);
unsigned char GET_Button(int iMotorNo);


#endif
