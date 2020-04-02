/* Include File */
#include "F28x_Project.h"
#include "Motor_Driver.h"
#include "UART.h"
#include <math.h>
#include "F2837xD_Ipc_drivers.h"

#define A 0.002382193948
#define B 0.00001776039252
#define C 0.000001040313455

uint16_t c2_r_w_array[256];   // mapped to GS1 of shared RAM owned by CPU02
uint16_t c2_r_array[256];     // mapped to GS0 of shared RAM owned by CPU01
#pragma DATA_SECTION(c2_r_array,"SHARERAMGS1");
#pragma DATA_SECTION(c2_r_w_array,"SHARERAMGS0");

/* Prototypes */
interrupt void scibRxFifoIsr(void);
void ADC_START(void);
__interrupt void cpu_timer0_isr(void);
void Shared_Ram_dataWrite_c2(void);

/* Global Variables */
volatile char UART_RX_DATA = 0;
volatile char XBee_Flag, ADC_Flag = 0;
volatile char Handshake_Ack = 0;
volatile Uint16 MQ_135, MQ_9, MQ_2, Temperature, T_FAR_Int, Covid, Covid_temp,
        ACK = 0;
volatile Uint16 MQ_135_ADC, MQ_9_ADC, MQ_2_ADC, Temperature_ADC = 0;
float Vout, Res, T_KEL, T_FAR;
volatile int16_t Angle, Distance, Angle_L, Angle_H = 0;
void main(void)
{
    //
    // Copy time critical code and Flash setup code to RAM
    // This includes InitFlash(), Flash API functions and any functions that are
    // assigned to ramfuncs section.
    // The  RamfuncsLoadStart, RamfuncsLoadEnd, and RamfuncsRunStart
    // symbols are created by the linker. Refer to the device .cmd file.
    //
#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t) &RamfuncsLoadSize);
#endif

    //
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the F2837xD_SysCtrl.c file.
    //
    InitSysCtrl();

    //
    // Call Flash Initialization to setup flash waitstates
    // This function must reside in RAM
    //
#ifdef _FLASH
    InitFlash();
#endif

//
// Step 2. Initialize GPIO:
// This example function is found in the F2837xD_Gpio.c file and
// illustrates how to set the GPIO to it's default state.
//
    //   InitGpio();

//
// For this example, only init the pins for the SCI-A port.
//  GPIO_SetupPinMux() - Sets the GPxMUX1/2 and GPyMUX1/2 register bits
//  GPIO_SetupPinOptions() - Sets the direction and configuration of the GPIOS
// These functions are found in the F2837xD_Gpio.c file.
//
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;

//
// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
//
    DINT;

//
// Initialize PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the F2837xD_PieCtrl.c file.
//
    InitPieCtrl();

//
// Disable CPU interrupts and clear all CPU interrupt flags:
//
    IER = 0x0000;
    IFR = 0x0000;

//
// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in F2837xD_DefaultIsr.c.
// This function is found in F2837xD_PieVect.c.
//
    InitPieVectTable();
    //
    // Wait until Shared RAM is available.
    //
    while (!(MemCfgRegs.GSxMSEL.bit.MSEL_GS0 & MemCfgRegs.GSxMSEL.bit.MSEL_GS14))
    {
    }
//
// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
//
    EALLOW;
    // This is needed to write to EALLOW protected registers
    PieVectTable.SCIB_RX_INT = &scibRxFifoIsr;
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    EDIS;

    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
    CpuSysRegs.PCLKCR7.bit.SCI_B = 1;
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
    // This is needed to disable write to EALLOW protected registers

    ADC_START();
    EDIS;
//
// Step 4. Initialize the Device Peripherals:
//
    scib_fifo_init();  // Init SCI-A
    //InitEPwm1Gpio();
    //InitEPwm2Gpio();
    EPWM1_INIT();
    EPWM2_INIT();

    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, 60, 1000000);
    CpuTimer0Regs.TCR.all = 0x4001;

// Step 5. User specific code, enable interrupts:
//
// Init send data.  After each transmission this data
// will be updated for the next transmission
//

//
// Enable interrupts required for this example
//
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx3 = 1;   // PIE Group 9, INT3
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    IER = 0x100;                         // Enable CPU INT
    IER |= M_INT1;
    EINT;
    XBee_Flag = 0;
    while (1)
    {
        if ((IPCRtoLFlagBusy(IPC_FLAG10) == 1)) //&& ACK)
        {
            //
            // Read c2_r_array and modify c2_r_w_array
            //
            Shared_Ram_dataWrite_c2();

            IPCRtoLFlagAcknowledge(IPC_FLAG10);
            //if(ACK){
            Covid = 0;
            //}
        }
        if (ADC_Flag)
        {
            MQ_135 = ((double) MQ_135_ADC / 0xFFF) * 100;
            MQ_9 = ((double) MQ_9_ADC / 0xFFF) * 100;
            MQ_2 = ((double) MQ_2_ADC / 0xFFF) * 100;
            Vout = ((double) Temperature_ADC / 0xF51) * 3.69;
            Res = ((3.16 - Vout) * 9730) / Vout;
            T_KEL = 1
                    / (A + B * log(Res) + C * (log(Res) * log(Res) * log(Res)));
            T_FAR = ((T_KEL - 273.15) * (1.8)) + 32;
            T_FAR_Int = T_FAR;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = 0xFF;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = MQ_135;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = 0xFE;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = MQ_9;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = 0xFD;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = MQ_2;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = 0xFC;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = T_FAR_Int;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = 0xFB;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = Angle_L;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = 0xFA;
            while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            {
            }
            ScibRegs.SCITXBUF.all = Angle_H;
            //while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            //{
            //}
            //ScibRegs.SCITXBUF.all = 0xF9;
            //while (ScibRegs.SCIFFTX.bit.TXFFST != 0)
            //{
            //}
            //.SCITXBUF.all = Distance;
            ADC_Flag = 0;
        }
    }

}

//
// sciaRxFifoIsr - SCIA Receive FIFO ISR
//
interrupt void scibRxFifoIsr(void)
{

    UART_RX_DATA = ScibRegs.SCIRXBUF.all;
    //if((Handshake_Ack == 0) && (UART_RX_DATA == 'H')){
    //      Handshake_Ack = 1;
    //  }
    //else if (Handshake_Ack){
    switch (UART_RX_DATA)
    {
    case 0x4C:
        if (XBee_Flag & 0x01)
        {
            break;
        }
        XBee_Flag |= 0x01;
        break;
    case 0x52:
        if (XBee_Flag & 0x02)
        {
            break;
        }
        XBee_Flag |= 0x02;
        break;
    case 0x81:
        if (XBee_Flag & 0x04)
        {
            break;
        }
        XBee_Flag |= 0x04;
        break;
    default:
        XBee_Flag |= 0x80;
    }
    if (XBee_Flag & 0x80)
    {
        if (XBee_Flag & 0x01)
        {
            Left_Motor(UART_RX_DATA);
            XBee_Flag &= 0x7E;
        }
        if (XBee_Flag & 0x02)
        {
            Right_Motor(UART_RX_DATA);
            XBee_Flag &= 0x7D;
        }
        if (XBee_Flag & 0x04)
        {
            Covid = UART_RX_DATA;
            XBee_Flag &= 0x7B;
        }
    }
    ScibRegs.SCIFFRX.bit.RXFFOVRCLR = 1;   // Clear Overflow flag
    ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all |= 0x100;       // Issue PIE ack

}

void ADC_START(void)
{
    Uint16 acqps;

    //
    // Determine minimum acquisition window (in SYSCLKS) based on resolution
    //
    if (ADC_RESOLUTION_12BIT == AdcaRegs.ADCCTL2.bit.RESOLUTION)
    {
        acqps = 14; //75ns
    }
    else //resolution is 16-bit
    {
        acqps = 63; //320ns
    }
    EALLOW;
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  //SOC will convert on channel
    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;  //SOC will convert on channel
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;  //SOC will convert on channel
    AdcaRegs.ADCSOC3CTL.bit.CHSEL = 3;  //SOC will convert on channel
    AdcaRegs.ADCSOC4CTL.bit.CHSEL = 4;  //SOC will convert on channel
    AdcaRegs.ADCSOC5CTL.bit.CHSEL = 5;  //SOC will convert on channel

    AdcaRegs.ADCSOC0CTL.bit.ACQPS = acqps;    //sample window is acqps +
                                              //1 SYSCLK cycles
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = acqps;    //sample window is acqps +
                                              //1 SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = acqps;    //sample window is acqps +
                                              //1 SYSCLK cycles
    AdcaRegs.ADCSOC3CTL.bit.ACQPS = acqps;    //sample window is acqps +
                                              //1 SYSCLK cycles
    AdcaRegs.ADCSOC4CTL.bit.ACQPS = acqps;    //sample window is acqps +
                                              //1 SYSCLK cycles
    AdcaRegs.ADCSOC5CTL.bit.ACQPS = acqps;    //sample window is acqps +
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

__interrupt void cpu_timer0_isr(void)
{
    CpuTimer0.InterruptCount++;

    //enable ADCINT flags
    //
    EALLOW;
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;
    AdcaRegs.ADCINTFLGCLR.all = 0x000F;
    EDIS;

    //
    //software force start SOC0 to SOC7
    //
    AdcaRegs.ADCSOCFRC1.all = 0x00FF;
    //
    //wait for first set of 8 conversions to complete
    //
    while (0 == AdcaRegs.ADCINTFLG.bit.ADCINT1)
        ;

    //
    //clear both INT flags generated by first 8 conversions
    //
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    //
    //save results for first 8 conversions
    //
    //note that during this time, the second 8 conversions have
    //already been triggered by EOC6->ADCIN1 and will be actively
    //converting while first 8 results are being saved
    //
    MQ_135_ADC = AdcaResultRegs.ADCRESULT0;  // Smoke Sensor
    MQ_9_ADC = AdcaResultRegs.ADCRESULT1;    // CO, Methane
    Temperature_ADC = AdcaResultRegs.ADCRESULT2;    // Temperature
    MQ_2_ADC = AdcaResultRegs.ADCRESULT4;    // Combustable Gas

    ADC_Flag = 0x01;
    //
    // Acknowledge this __interrupt to receive more __interrupts from group 1
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

void Shared_Ram_dataWrite_c2(void)
{

    c2_r_w_array[0] = MQ_135;
    c2_r_w_array[1] = MQ_9;
    c2_r_w_array[2] = MQ_2;
    c2_r_w_array[3] = T_FAR_Int;
    c2_r_w_array[4] = Covid;

    Angle = c2_r_array[0];
    //Distance = c2_r_array[1];

    Angle_L = 0x00FF & Angle;
    Angle_H = (0xFF00 & Angle) >> 8;
    if(Angle_H == 0xFF){
        Angle_H = 1;
    }
    else{
        Angle_H = 0x00;
    }
    //ACK = c2_r_array[3];

}

//
// End of file
//
