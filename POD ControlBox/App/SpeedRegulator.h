#ifndef SPEEDREGULATOR_H
#define SPEEDREGULATOR_H

void speedRegulatorSpeedControl(int iMotorIndex, int nPulsePeriod);
void speedRegulatorAvoidMotorStop(int iMotorIndex, int iPulseResult);
void speedRegulatorResetCounterA(void);
void speedRegulatorResetCounterB(void);
#endif
