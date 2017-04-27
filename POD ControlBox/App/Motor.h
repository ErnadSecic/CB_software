
// Direction logical levels (Genearal IO, output):
#define NO_DIRECTION    0
#define FORWARD         1              // Select MCOMHI
#define REVERSE         2              // Select MCOMLO


// Steps to obtain pusher to READ or TIGTHEN position
#define  STEPS_BEYOND_MAX     2000

#define REGULATE_SPEED      1
#define NOT_REGULATE_SPEED  0

#define  MAX_PULSE_ADC      4
#define  MAX_MOTORS        16
#define  MAX_EJSSCT         4

#define  MAX_MOTOR_LOAD_HIGH     700//700 // max=920 (550)
#define  MAX_MOTOR_LOAD_MIDLE    220 // max=920 (550)
#define  MAX_MOTOR_LOAD_TWIN     560//560
#define  MOTOR_STEPS_BEFORE_OVERLOAD   5
#define  MOTOR_MAX_STEPS_TO_OPEN_LED   3

#define MOTOR_1_ADC_CHAN      ADC_CH0
#define MOTOR_3_ADC_CHAN      ADC_CH0
#define MOTOR_5_ADC_CHAN      ADC_CH0
#define MOTOR_7_ADC_CHAN      ADC_CH0

#define MOTOR_9_ADC_CHAN      ADC_CH1
#define MOTOR_11_ADC_CHAN     ADC_CH1
#define MOTOR_13_ADC_CHAN     ADC_CH1
#define MOTOR_15_ADC_CHAN     ADC_CH1

#define MOTOR_2_ADC_CHAN      ADC_CH2
#define MOTOR_4_ADC_CHAN      ADC_CH2
#define MOTOR_6_ADC_CHAN      ADC_CH2
#define MOTOR_8_ADC_CHAN      ADC_CH2

#define MOTOR_10_ADC_CHAN     ADC_CH3
#define MOTOR_12_ADC_CHAN     ADC_CH3
#define MOTOR_14_ADC_CHAN     ADC_CH3
#define MOTOR_16_ADC_CHAN     ADC_CH3

///////////////////////////////////////////////////////////////////////////////
// Twin functionality
///////////////////////////////////////////////////////////////////////////////
 int motorGetTwinMotorNo(int iMotorNo);
 int motorIsMainMotor(int iMotorNo);
 void motorResetTwinInfo(void);

//*****************************************************************************
//              Structure to hold individual motor data and progress
// *****************************************************************************
typedef struct xMotor {
    int  iCommand;
    int  iDirection;
    int	 iAdcIndex; 	// Point out ADC to measure pulses from
    int	 iPulseCount; 	// No of pulses to be detected before stop, decremented to zero from interrupt routine.
    int	 iRequestedCnt;
    int  iRearposition;
    int  iSpeed;
    int  iWantedSpeed;
    int  iReasonOfStop;
    int  iRegulateSpeed;
    int  iMotorState;
    int  iTwinMotorNo;
    int  iIsMainMotor;
} MOTOR_DATA;

typedef enum
{
	MOTOR_NOT_IN_USE,
	MOTOR_MOVING,
	MOTOR_WAITING_ON_ROTATION_END,
	MOTOR_FINISHED
} eMotorState;

typedef enum
{
	MOTOR_NOT_CONNECTED,
	MOTOR_CONNECTED
} eMotorConnected;

typedef enum
{
        CANT_STOP_NOT_IN_USE,
        CANT_STOP_WAITING,
	MOTOR_STOPED
} eMotorStop;

//static int   iBridgeActive[2];         // Status to flag active bridge (A - B) in us


#define  STOP_FINISHED                0
#define  STOP_ON_COMMAND              1
#define  STOP_OVERLOAD                2
#define  STOP_NO_PULSES_LOW_LEVEL     3
#define  STOP_NO_PULSES_HIGH_LEVEL    4
#define  STOP_LED_OPEN_SCAN           5
#define  STOP_UNSTABLE_PULSES         6

//#define   PROMILLE_CURRENT_DIFFERENCE_TIGHT_TIGHTEN_PUSHER    950 //test
//#define   PROMILLE_CURRENT_DIFFERENCE_TIGHT_TEST_PUSHER       47 //Test ejector (italian motor)


//#define   PROMILLE_CURRENT_DIFFERENCE_TIGHT_TIGHTEN_PUSHER    40 //Normal (Japan motor)
//#define   PROMILLE_CURRENT_DIFFERENCE_TIGHT_TEST_PUSHER       37 //Test ejector (Japan motor)



// Prototypes:
void motorChangeSpeed(int iMotorIndex, int iSpeedDifference);
void motorSetSpeed(int iMotorIndex, int iNewSpeed);
int motorNotInUse( int iMotorNo);
void motorSetConnected(int iMotorNo, int iConnected);
int motorIsConnected(int iMotorNo);
void motorReset( int iMotorIndex);
void motorInit( void);
int motorGetBridgeIndex( int iMotorNo);
int motorGetAdcIndex( int iMotorIndex);
void motorSetDirection( int iMotorNo, int iDirection);
void motorSetStepCounter( int iMotorNo, int iSteps);
int motorStartKoggMeasure( int iMotor, int iPulses, int iRegulateSpeed);
__interwork void motorStopKoggMeasure( int iMotorNo);
int  motorValid( int iMotorNo);
int  motorFinished( int iMotorNo, int *pDirection, int *pTotalCnt);
int motorEjssctOn(int iEJSSCT_ID);
void motorEjssctOff(int iEJSSCT_ID);
void  motorPWM( int iMotorNo, int iEjssctNo, int iDirection, int iSpeed);
//int motorStart( int iMotorNo, int iDirection, int iSpeed);
int motorStart( int iMotorNo, int iDirection, int iSpeed, int iTwinMotorNo, int iIsMainMotor);
__interwork int motorStop( int iMotorNo, int iReason);
int motorGetReasonOfStop(int iMotorIndex);
void motorSaveCommand(int iMotorNo, int iCommand);
void motorGetCommand(int iMotorNo, int *pCommand);
int motorGetEjssctNo(int iMotorNo);
int motorGetDirection( int iMotorNo);
int motorRearposition( int iMotorNo);
void motorSetRearposition( int iMotorNo);
void motorClearRearposition( int iMotorNo);
void Set_Move_Type(int Mov_Type);
int motorOverLoadA( void);
int motorOverLoadB( void);
int motorOverLoadDetected( void);
void motorOverLoadClear( void);
void motorOnTimerInterrupted_IRQ(void);
void motorResetChActive(void);

