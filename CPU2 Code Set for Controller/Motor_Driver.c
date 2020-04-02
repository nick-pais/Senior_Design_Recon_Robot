/*
 * Motor_Driver.c
 *
 *  Created on: Feb 1, 2020
 *      Author: Paiscaso
 */

#include "Motor_Driver.h"

void EPWM1_INIT()
{
    //
    // Setup TBCLK
    //
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    EPwm1Regs.TBPRD = EPWM1_TIMER_TBPRD;       // Set timer period
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;    // Disable phase loading
    EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;        // Phase is 0
    EPwm1Regs.TBCTR = 0x0000;                  // Clear counter
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV4;   // Clock ratio to SYSCLKOUT
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV4;

    //
    // Setup shadow register load on ZERO
    //
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    EPwm1Regs.CMPA.bit.CMPA = 0;     // Set compare A value
    EPwm1Regs.CMPB.bit.CMPB = 0;     // Set Compare B value

    //
    // Set actions
    //
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;            // Set PWM1A on Zero
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;          // Clear PWM1A on event A,
                                                  // up count

    EPwm1Regs.AQCTLB.bit.ZRO = AQ_SET;            // Set PWM1B on Zero
    EPwm1Regs.AQCTLB.bit.CBU = AQ_CLEAR;          // Clear PWM1B on event B,
                                                  // up count

    //
    // Interrupt where we will change the Compare Values
    //
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;     // Select INT on Zero event
    EPwm1Regs.ETSEL.bit.INTEN = 1;                // Enable INT
    EPwm1Regs.ETPS.bit.INTPRD = ET_3RD;           // Generate INT on 3rd event

}

void EPWM2_INIT()
{
    //
    // Setup TBCLK
    //
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up
    EPwm2Regs.TBPRD = EPWM2_TIMER_TBPRD;       // Set timer period
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;    // Disable phase loading
    EPwm2Regs.TBPHS.bit.TBPHS = 0x0000;        // Phase is 0
    EPwm2Regs.TBCTR = 0x0000;                  // Clear counter
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV2;   // Clock ratio to SYSCLKOUT
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV2;

    //
    // Setup shadow register load on ZERO
    //
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    //
    // Set Compare values
    //
    EPwm2Regs.CMPA.bit.CMPA = 0;     // Set compare A value
    EPwm2Regs.CMPB.bit.CMPB = 0;     // Set Compare B value

    //
    // Set actions
    //
    EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET;            // Set PWM1A on Zero
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;          // Clear PWM1A on event A,
                                                  // up count

    EPwm2Regs.AQCTLB.bit.ZRO = AQ_SET;            // Set PWM1B on Zero
    EPwm2Regs.AQCTLB.bit.CBU = AQ_CLEAR;          // Clear PWM1B on event B,
                                                  // up count

    //
    // Interrupt where we will change the Compare Values
    //
    EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;     // Select INT on Zero event
    EPwm2Regs.ETSEL.bit.INTEN = 1;                // Enable INT
    EPwm2Regs.ETPS.bit.INTPRD = ET_3RD;           // Generate INT on 3rd event

}


void Left_Motor(int16 Percent_Left)
{

    if ((Percent_Left) & 0x80)
    {
        Percent_Left |= 0xFF00;
        EPwm1Regs.CMPB.bit.CMPB = 0;     // Set Compare B value
        EPwm1Regs.CMPA.bit.CMPA = abs(20000 * (float) Percent_Left / 100); // Set compare A value
    }
    else
    {
        EPwm1Regs.CMPA.bit.CMPA = 0;     // Set compare A value
        EPwm1Regs.CMPB.bit.CMPB = abs(20000 * (float) Percent_Left / 100); // Set Compare B value
    }
}
void Right_Motor(int16 Percent_Right)
{
    if ((Percent_Right) & 0x80)
    {
        Percent_Right |= 0xFF00;
        EPwm2Regs.CMPB.bit.CMPB = 0;     // Set Compare B value
        EPwm2Regs.CMPA.bit.CMPA = abs(20000 * (float) Percent_Right / 100); // Set compare A value
    }
    else
    {
        EPwm2Regs.CMPA.bit.CMPA = 0;     // Set compare A value
        EPwm2Regs.CMPB.bit.CMPB = abs(20000 * (float) Percent_Right / 100); // Set Compare B value
    }
}
