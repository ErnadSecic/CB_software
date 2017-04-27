

/*-----------------*/
/* Clock Selection */
/*-----------------*/
#define TC_CLKS                  0x7
#define TC_CLKS_MCK2             0x0
#define TC_CLKS_MCK8             0x1
#define TC_CLKS_MCK32            0x2
#define TC_CLKS_MCK128           0x3
#define TC_CLKS_MCK1024          0x4

#define WAVE_ENA                 (1<<15)

#define WAVESEL                  (1<<14)  //13=0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Delay-TIMERS (timer 2) definitions:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIME_1_US                 1
#define TIME_3_US                 3
#define TIME_10_US               10
#define TIME_15_US               15
#define TIME_20_US               20
#define TIME_30_US               30
#define TIME_50_US               50
#define TIME_100_US             100
#define TIME_200_US             200
#define TIME_250_US             250
#define TIME_300_US             300
#define TIME_400_US             400
#define TIME_500_US             500
#define TIME_600_US             600
#define TIME_1000_US           1000

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// X-TIMERS definitions:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIMERS_UPDATE_RATE1    5   // Interrupt every 200us, thus 5 makes 1ms

#define TIME_1_MS                 1
#define TIME_3_MS                 3
#define TIME_10_MS               10
#define TIME_15_MS               15
#define TIME_20_MS               20
#define TIME_30_MS               30
#define TIME_50_MS               50
#define TIME_100_MS             100
#define TIME_200_MS             200
#define TIME_250_MS             250
#define TIME_300_MS             300
#define TIME_400_MS             400
#define TIME_500_MS             500
#define TIME_600_MS             600
#define TIME_1000_MS           1000
#define TIME_2000_MS           2000
#define TIME_3000_MS           3000
#define TIME_4000_MS           4000
#define TIME_5000_MS           5000
#define TIME_7000_MS           7000
#define TIME_10_SEC           10000
// Max-time is 65535 = 10,92 min:
#define MAX_MINUTES          10


// Indexes used to address timers or timeout flags.
#define INX_CTRL0_TIMER       0
#define INX_CTRL1_TIMER	      1
#define INX_CTRL2_TIMER       2
#define INX_CTRL3_TIMER       3
#define INX_CTRL4_TIMER       4
#define INX_CTRL5_TIMER       5
#define INX_CTRL6_TIMER       6
#define INX_CTRL7_TIMER       7
#define INX_CTRL8_TIMER       8
#define INX_CTRL9_TIMER       9
#define INX_CTRL10_TIMER      10
#define INX_CTRL11_TIMER      11
#define INX_CTRL12_TIMER      12

#define MAX_TIMERS            13


/////////////////////////////////////////////////////////
//  USED BY EJECTOR CONTROLLER
/////////////////////////////////////////////////////////
#define INX_BRIDGE_A_TIMER	INX_CTRL0_TIMER
#define INX_BRIDGE_B_TIMER	INX_CTRL1_TIMER
#define INX_TOGLE_BEAM_TIMER    INX_CTRL2_TIMER
#define INX_SPARE1_TIMER	INX_CTRL3_TIMER
#define INX_SPARE2_TIMER	INX_CTRL4_TIMER
#define INX_SPARE3_TIMER	INX_CTRL5_TIMER
#define INX_SPARE4_TIMER	INX_CTRL6_TIMER
#define INX_SPARE5_TIMER	INX_CTRL7_TIMER
#define INX_SPARE6_TIMER	INX_CTRL8_TIMER
#define INX_SPARE7_TIMER	INX_CTRL9_TIMER
#define INX_SPARE8_TIMER	INX_CTRL10_TIMER


/////////////////////////////////////////////////////////
//  USED BY SENSOR CONTROLLER
/////////////////////////////////////////////////////////
#define INX_TILT_FALL1_TIMER		INX_CTRL0_TIMER
#define INX_TILT_FALL2_TIMER		INX_CTRL1_TIMER
#define INX_TOGLE_BEAM_TIMER            INX_CTRL2_TIMER
#define INX_RS485_MI_TX_TIMER	        INX_CTRL3_TIMER
#define INX_FALL_IR_TIMER               INX_CTRL4_TIMER
#define INX_RS485_MI_ERROR_TIMER	INX_CTRL5_TIMER
#define INX_LAMP_TIMER                  INX_CTRL6_TIMER
#define INX_FALL_IRQ_DELAY_TIMER        INX_CTRL7_TIMER
#define INX_LAMP_FALL_DELAY             INX_CTRL8_TIMER
#define INX_TEMP_READ_TIMER             INX_CTRL9_TIMER
#define INX_TEMP_INIT_TIMER             INX_CTRL10_TIMER
#define INX_TILT_TIMER                  INX_CTRL11_TIMER
#define INX_BOXXLED_TIMER               INX_CTRL12_TIMER

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Prototypes:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char timerSet(unsigned char ucTimerIdx, unsigned short uiValue);
unsigned char timerExpired(unsigned char ucTimerIdx);
unsigned char timerStop(unsigned char ucTimerIdx);
unsigned char timerRunning(unsigned char ucTimerIdx);
void timerCtrl_IRQ(void);
void AT91F_TC_Open ( AT91PS_TC TC_pt, unsigned int Mode, unsigned int TimerId,unsigned int RC);
void timer1_init (int swType);
void timer0_init ( void );
int Timer2done(void);
void StartTimer2_init (int us);

