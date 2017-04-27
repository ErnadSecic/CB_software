


#define TIMER0_INTERRUPT_LEVEL		1
#define TIMER1_INTERRUPT_LEVEL		1

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


void AT91F_TC_Open ( AT91PS_TC TC_pt, unsigned int Mode, unsigned int TimerId);
void Reset (void);
void timer0_c_irq_handler(void);
void timer1_c_irq_handler(void);
void timer0_init ( void );
void timer1_init ( void );


