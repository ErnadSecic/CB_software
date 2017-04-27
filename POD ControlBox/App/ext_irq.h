/*---------------------------------------------------------------------------
* File Name           : ext_irq.h
* Object              : Control box file.
*
* Creation            : TAK   31/May/2005
* Modif               :
*----------------------------------------------------------------------------
*/
#ifndef __ext_irq_h
#define __ext_irq_h

void Init_InetrruptsForSensorUnit(void);
void FIQ_init_handler(void);
void at91_IRQ0_handler(void);
__arm void vExt_ISR(void);

#endif /*ext_irq.h.h*/
