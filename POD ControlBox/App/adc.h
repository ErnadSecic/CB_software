#ifndef ADC_H
#define ADC_H

#define  ADC0_PA_OUT  (1<<17)
#define  ADC1_PA_OUT  (1<<18)
#define  ADC2_PA_OUT  (1<<19)
#define  ADC3_PA_OUT  (1<<20)

#define ADC_CH0      0
#define ADC_CH1      1
#define ADC_CH2      2
#define ADC_CH3      3
#define ADC_CH4      4
#define ADC_CH5      5
#define ADC_CH6      6
#define ADC_CH7      7

#define ADC_MAX_PULSE_CHAN       4

#define ADC_PHRIPH_CLOCK (1<<4)   // To e checked out !!!!!

/////////////////////////////////////////////////////////////////////////////////////////////////
// A D C  S e l e c t i o n :

// Address AD0, motors(1,3,5,7)
#define PULSE_AD0  AT91C_PIO_PA17
// Address AD1, motors(9,11,13,15)
#define PULSE_AD1  AT91C_PIO_PA18
// Address AD2, motors(2,4,6,8)
#define PULSE_AD2  AT91C_PIO_PA19
// Address AD3, motors(10,12,14,16)
#define PULSE_AD3  AT91C_PIO_PA20

void adcInit(void);
unsigned long int adcRead(int ADC_Channel);

#endif
