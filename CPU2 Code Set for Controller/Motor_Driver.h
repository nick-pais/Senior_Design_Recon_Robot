/*
 * Motor_Driver.h
 *
 *  Created on: Feb 1, 2020
 *      Author: Paiscaso
 */
#include "F28x_Project.h"

#define EPWM1_TIMER_TBPRD  25000  // Period register
#define EPWM2_TIMER_TBPRD  25000  // Period register


#ifndef MOTOR_DRIVER_H_
#define MOTOR_DRIVER_H_


void EPWM1_INIT();
void EPWM2_INIT();

void Left_Motor(int16 Percent_Left);
void Right_Motor(int16 Percent_Right);
#endif /* MOTOR_DRIVER_H_ */
