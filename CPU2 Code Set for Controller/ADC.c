/*
 * ADC.c
 *
 *  Created on: Mar 15, 2020
 *      Author: Paiscaso
 */

#include "ADC.h"

//
// ConfigureADC - Write ADC configurations and power up the ADC for both
//                ADC A and ADC B
//
void ConfigureADC(void)
{
    EALLOW;

    //
    //write configurations
    //
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

    //
    //Set pulse positions to late
    //
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    //
    //power up the ADC
    //
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;

    //
    //delay for 1ms to allow ADC time to power up
    //
    DELAY_US(1000);

    EDIS;
}

//
// SetupADCContinuous - setup the ADC to continuously convert on one channel
//
void SetupADCContinuous(void)
{
    Uint16 acqps;

    //
    // Determine minimum acquisition window (in SYSCLKS) based on resolution
    //
    if(ADC_RESOLUTION_12BIT == AdcaRegs.ADCCTL2.bit.RESOLUTION)
    {
        acqps = 14; //75ns
    }
    else //resolution is 16-bit
    {
        acqps = 63; //320ns
    }

    EALLOW;
    AdcaRegs.ADCSOC0CTL.bit.CHSEL  = 0;  //SOC will convert on channel
    AdcaRegs.ADCSOC1CTL.bit.CHSEL  = 1;  //SOC will convert on channel
    AdcaRegs.ADCSOC2CTL.bit.CHSEL  = 2;  //SOC will convert on channel
    AdcaRegs.ADCSOC3CTL.bit.CHSEL  = 3;  //SOC will convert on channel
    AdcaRegs.ADCSOC4CTL.bit.CHSEL  = 4;  //SOC will convert on channel
    AdcaRegs.ADCSOC5CTL.bit.CHSEL  = 5;  //SOC will convert on channel

    AdcaRegs.ADCSOC0CTL.bit.ACQPS  = acqps;    //sample window is acqps +
                                               //1 SYSCLK cycles
    AdcaRegs.ADCSOC1CTL.bit.ACQPS  = acqps;    //sample window is acqps +
                                                   //1 SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.ACQPS  = acqps;    //sample window is acqps +
                                                       //1 SYSCLK cycles
    AdcaRegs.ADCSOC3CTL.bit.ACQPS  = acqps;    //sample window is acqps +
                                               //1 SYSCLK cycles
    AdcaRegs.ADCSOC4CTL.bit.ACQPS  = acqps;    //sample window is acqps +
                                                   //1 SYSCLK cycles
    AdcaRegs.ADCSOC5CTL.bit.ACQPS  = acqps;    //sample window is acqps +
                                                       //1 SYSCLK cycles

    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 0; //disable INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT2E = 0; //disable INT2 flag
    AdcaRegs.ADCINTSEL3N4.bit.INT3E = 0; //disable INT3 flag
    AdcaRegs.ADCINTSEL3N4.bit.INT4E = 0; //disable INT4 flag

    AdcaRegs.ADCINTSEL1N2.bit.INT1CONT = 0;
    AdcaRegs.ADCINTSEL1N2.bit.INT2CONT = 0;
    AdcaRegs.ADCINTSEL3N4.bit.INT3CONT = 0;
    AdcaRegs.ADCINTSEL3N4.bit.INT4CONT = 0;

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 5;  //end of SOC6 will set INT1 flag

    //
    //ADCINT2 will trigger first 8 SOCs
    //
    AdcaRegs.ADCINTSOCSEL1.bit.SOC0 = 1;

    EDIS;
}
