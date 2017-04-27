/*---------------------------------------------------------------------------
* File Name           : Sensors.h
* Object              : Control box file.
*
* Creation            : TAK   31/May/2005
* Modif               :
*----------------------------------------------------------------------------
*/
#ifndef __Sensors_h
#define __Sensors_h

typedef struct FLAGS {
   unsigned char fall1;
   unsigned char fall2;
   unsigned char tilt;
   unsigned char fall_1_on;
   unsigned char fall_2_on;
   unsigned char tilt_on;
}SENSOR_FLAGS;

void OnFALL1_Interrupted_IRQ(void);
void OnFALL2_Interrupted_IRQ(void);
void OnTILT_Interrupted_IRQ(void);
char isFALL1_Interrupted(void);
char isFALL2_Interrupted(void);
char isTILT_Interrupted(void);
void Stop_Fall_1(void);
void Start_Fall_1(void);
void Stop_Fall_2(void);
void Start_Fall_2(void);
void Stop_Tilt(void);
void Start_Tilt(void);

char DoorSensor(void);
char Sensor_In(unsigned int sensor);
char FallSensor(int EqNo);
char Set_Lamp(unsigned char value,int EqNo);
char Get_Lamp(int EqNo);
void Trace_Toggel_LED (unsigned int Led);
char TiltSensor(void);

#endif /*Sensor.h*/

