

/*----------------------------------------------------------------------------
* File Name           : InitHW.h
* Object              : Jupiter Controller Board Features Definition File.
*
* Creation            : PVA   17/Sep/2005
*----------------------------------------------------------------------------
*/
#ifndef GP_IO_H
#define GP_IO_H


/////////////////////////////////////////////////////////////////////////////////////////////////
//              SENSOR AND EJECTOR CONTROLOR
/////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////
//                     ADC
/////////////////////////////////////////////////////


#define BEAM_BIT        AT91C_PIO_PA12       // OUTPUT
#define ID_BIT          AT91C_PIO_PA26


/////////////////////////////////////////////////////////////////////////////////////////////////
//              SENSOR CONTROLOR
/////////////////////////////////////////////////////////////////////////////////////////////////

//Not connected
#define NC_SENS_PA17  AT91C_PIO_PA17
#define NC_SENS_PA27  AT91C_PIO_PA27


#define HATCH_INPUT_BIT     AT91C_PIO_PA16  // INPUT -> 3.45
//#define FALL1_INPUT_BIT     AT91C_PIO_PA16  // INPUT
#define FALL2_INPUT_BIT     AT91C_PIO_PA15  // INPUT
#define DOOROPN_BIT         AT91C_PIO_PA20  // INPUT
#define LAMP1_BIT           AT91C_PIO_PA13  // OUTPUT
#define LAMP2_BIT           AT91C_PIO_PA14  // OUTPUT
#define TILTS_BIT           AT91C_PIO_PA19  // INPUT

#define I2C_SCL_BIT          AT91C_PIO_PA4  // OUTPUT

#define FALL1_INPUT_AD1     AT91C_PIO_PA18   //INPUT     ADC

#define FALL_OUT_O0_BIT     AT91C_PIO_PA0   // OUTPUT
#define FALL_OUT_O1_BIT     AT91C_PIO_PA1   // OUTPUT
#define FALL_OUT_O2_BIT     AT91C_PIO_PA2   // OUTPUT
#define FALL_OUT_O3_BIT     AT91C_PIO_PA23  // OUTPUT
#define FALL_OUT_O4_BIT     AT91C_PIO_PA24  // OUTPUT
#define FALL_OUT_O5_BIT     AT91C_PIO_PA5   // OUTPUT
#define FALL_OUT_O6_BIT     AT91C_PIO_PA6   // OUTPUT
#define FALL_OUT_O7_BIT     AT91C_PIO_PA7   // OUTPUT
#define FALL_OUT_O8_BIT     AT91C_PIO_PA8   // OUTPUT
#define FALL_OUT_O9_BIT     AT91C_PIO_PA9   // OUTPUT
#define FALL_OUT_10_BIT     AT91C_PIO_PA10  // OUTPUT
#define FALL_OUT_11_BIT     AT91C_PIO_PA11  // OUTPUT
#define FALL_OUT_12_BIT     AT91C_PIO_PA28  // OUTPUT
#define FALL_OUT_13_BIT     AT91C_PIO_PA29  // OUTPUT
#define FALL_OUT_14_BIT     AT91C_PIO_PA30  // OUTPUT
#define FALL_OUT_15_BIT     AT91C_PIO_PA31  // OUTPUT

#define RS485_RSEL_BIT    AT91C_PIO_PA25    // OUTPUT
#define RS485_DENA_BIT    AT91C_PIO_PA26    // OUTPUT  1= TX , 0=RX

#define FALL_OUT_MASK (FALL_OUT_O0_BIT|FALL_OUT_O1_BIT|FALL_OUT_O2_BIT|FALL_OUT_O3_BIT|FALL_OUT_O4_BIT|FALL_OUT_O5_BIT|FALL_OUT_O6_BIT|FALL_OUT_O7_BIT|FALL_OUT_O8_BIT|FALL_OUT_O9_BIT|FALL_OUT_10_BIT|FALL_OUT_11_BIT|FALL_OUT_12_BIT|FALL_OUT_13_BIT|FALL_OUT_14_BIT|FALL_OUT_15_BIT)

#define LAMP_MASK     (LAMP1_BIT|LAMP2_BIT)
#define NC_SENS_MASK  (NC_SENS_PA27|NC_SENS_PA17)


/////////////////////////////////////////////////////////////////////////////////////////////////
//              EJECTOR CONTROLOR
/////////////////////////////////////////////////////////////////////////////////////////////////
//Not connected
#define NC_MOTO_PA15  AT91C_PIO_PA15
#define NC_MOTO_PA16  AT91C_PIO_PA16
#define NC_MOTO_PA27  AT91C_PIO_PA27


/////////////////////////////////////////////////////////////////

#define ADC_MASK           (INPUT_AD0|INPUT_AD1|INPUT_AD2|INPUT_AD3)

#define INPUT_AD0 AT91C_PIO_PA17
#define INPUT_AD1 AT91C_PIO_PA18
#define INPUT_AD2 AT91C_PIO_PA19
#define INPUT_AD3 AT91C_PIO_PA20

// Selects individual motors direction by setting 0 or 1 (output):
/////////////////////////////////////////////////////////////////
// B r i d g e  A :
#define MOT_1  AT91C_PIO_PA31
#define MOT_2  AT91C_PIO_PA29
#define MOT_3  AT91C_PIO_PA11
#define MOT_4  AT91C_PIO_PA9
#define MOT_5  AT91C_PIO_PA7
#define MOT_6  AT91C_PIO_PA5
#define MOT_7  AT91C_PIO_PA3
#define MOT_8  AT91C_PIO_PA1

/////////////////////////////////////////////////////////////////
// B r i d g e  B :
#define MOT_9   AT91C_PIO_PA30
#define MOT_10  AT91C_PIO_PA28
#define MOT_11  AT91C_PIO_PA10
#define MOT_12  AT91C_PIO_PA8
#define MOT_13  AT91C_PIO_PA6
#define MOT_14  AT91C_PIO_PA4
#define MOT_15  AT91C_PIO_PA2
#define MOT_16  AT91C_PIO_PA0




// Address motorgroups to measure pulses from ADC inputs, AD0 -AD3:
/////////////////////////////////////////////////////////////////////////////////////////////////
// P u l s e m e a s u r e m e n t  S e l e c t i o n (Genearal IO, output):

// Address GroupW, motors(1,9,2,10)       -> 1A,1B,2A,2B
#define  EJSSCT1  AT91C_PIO_PA21
// Address GroupX, motors(3,11,4,12)      -> 3A,3B,4A,4B
#define EJSSCT2  AT91C_PIO_PA22
// Address GroupY, motors(5,13,6,14)      -> 5A,5B,6A,6B
#define EJSSCT3  AT91C_PIO_PA25
// Address GroupZ, motors(7,15,8,16)      -> 7A,7B,8A,8B
#define EJSSCT4  AT91C_PIO_PA26



#define PWM_MASK AT91C_PIO_PA23

#define MOTOR_MASK         (MOT_1|MOT_2|MOT_3|MOT_4|MOT_5|MOT_6|MOT_7|MOT_8|MOT_9|MOT_10|MOT_11|MOT_12|MOT_13|MOT_14|MOT_15|MOT_16)
#define EJSSCT_MASK        (EJSSCT1|EJSSCT2|EJSSCT3|EJSSCT4)
#define BEAM_MASK          BEAM_BIT
#define NC_MOTO_MASK       (NC_MOTO_PA15|NC_MOTO_PA16|NC_MOTO_PA27)

#define BRIDGE_A_MASK     (MOT_1|MOT_2|MOT_3|MOT_4|MOT_5|MOT_6|MOT_7|MOT_8)
#define BRIDGE_B_MASK     (MOT_9|MOT_10|MOT_11|MOT_12|MOT_13|MOT_14|MOT_15|MOT_16)


void ToggleBeam_IRQ(void);
void Init_GPIO_Common(void);
void Init_GPIO_for_SensorUnit(void);
void Init_GPIO_for_EjectorUnit( void );
int gpioSet(unsigned int iSetBit);
int gpioClear(unsigned int iSetBit);
void gpioSetBit(unsigned int uiClearBit);
void gpioClearBit(unsigned int uiClearBit);
char Card_ID_check(void);


#endif /* GP_IO_H */
