

#ifndef PWM_H
#define PWM_H


/*-------------------------*/
/* PWM  MCOM                 */
/*-------------------------*/

#define CPRE            0x0000   // 0b1010
#define CPD             (0<<10)
#define CPOL            (0<<9)
#define CALG            (0<<8)

#define PWM_MODE        (CALG|CPOL|CPD|CPRE)

#define PWM_PERIOD       1200  // 2,5ms, Resolution=20us      18Mhz Crystal  //test

#define MAX_PWM_LOAD     PWM_PERIOD

#define PWM_CH0              0
#define PWM_CH1              1
#define PWM_CH2              2
#define PWM_CH3              3

#define MCOM_LoA         0
#define MCOM_HiA         1
#define MCOM_LoB         2
#define MCOM_HiB         3

#define PWM_ID_CH0           (1<<0)    // MCOMLOA
#define PWM_ID_CH1           (1<<1)    // MCOMHIA
#define PWM_ID_CH2           (1<<2)    // MCOMLOB
#define PWM_ID_CH3           (1<<3)    // MCOMHIB

#define PWM_PHRIPH_CLOCK (1<<10)       // MCK documentation section

#define PWM2_PA13_OUT          AT91C_PIO_PA13	//
#define PWM3_PA14_OUT          AT91C_PIO_PA14	//
#define PWM0_PA23_OUT          AT91C_PIO_PA23	//
#define PWM1_PA24_OUT          AT91C_PIO_PA24	//


/////////////////////////////////////////////////////////////////////////////////////////////////
// B r i d g e  A :

// PWM Channels:
#define MCOM_LO_A_CHAN     PWM_CH0
#define MCOM_HI_A_CHAN     PWM_CH1


// PWM ID's:
#define MCOM_LO_A_ID  PWM_ID_CH0         // PWM output when direction level="1" is selected
#define MCOM_HI_A_ID  PWM_ID_CH1         // PWM output when direction level="0" is selected


/////////////////////////////////////////////////////////////////////////////////////////////////
// B r i d g e  B :

// PWM Channels:
#define MCOM_LO_B_CHAN     PWM_CH2
#define MCOM_HI_B_CHAN     PWM_CH3

// PWM ID's:
#define MCOM_LO_B_ID  PWM_ID_CH2         // PWM output when direction level="0"  is selected
#define MCOM_HI_B_ID  PWM_ID_CH3         // PWM output when direction level="1"  is selected

//////////////////////////////////////////////////////////////////////////////////////////////////

void pwmUpdate( int iPWM_Channel, int iPercentLoad);
void pwmInit(int iPWM_Channel, int iPercentLoad);
void pwmStart( int iPWM_Channel);
__interwork void pwmStop( int iPWM_Channel);


#endif
