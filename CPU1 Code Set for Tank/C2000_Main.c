#include <F28x_Project.h>
#include "Common/diskio.h"
#include "Common/DSP.h"
#include "Common/ff.h"
#include "Common/lawOfCos.h"
#include "F2837xD_Ipc_drivers.h"


uint16_t c1_r_array[256];   // mapped to GS0 of shared RAM owned by CPU02
uint16_t c1_r_w_array[256]; // mapped to GS1 of shared RAM owned by CPU01
#pragma DATA_SECTION(c1_r_array,"SHARERAMGS0");
#pragma DATA_SECTION(c1_r_w_array,"SHARERAMGS1");

int16_t Angle, Distance = 0;
Uint16 MQ_135, MQ_9, MQ_2, Temperature, Covid, ACK;

uint16_t error;
uint16_t RAM_FLAG;

void Shared_Ram_dataRead_c1(void);
void Shared_Ram_dataWrite_c1(void);

#define TinyDelay() for(volatile long  i = 0; i < 100; i++)
#define sdBufferSize 1024
#define CPU_STEP (1.0f/200000000.0f)*16.0f

#define HIGH_TEMP_THRESH 85
#define GAS_THRESH 20
#define CO_THRESH 30
#define SMOKE_THRESH 20


#define CLAP_DETECTED "0:clap.txt"
#define HIGH_TEMP_DETECTED "0:temp.txt"
#define GAS_DETECTED "0:gas.txt"
#define CO_DETECTED "0:co.txt"
#define SMOKE_DETECTED "0:smoke.txt"
#define COVID_DETECTED "0:covid.txt"

static FATFS g_sFatFs;
static FIL g_sFileObject;


BYTE sdbuf1[sdBufferSize];
BYTE sdbuf2[sdBufferSize];
#pragma DATA_SECTION(sdbuf1, "ramgs2")
#pragma DATA_SECTION(sdbuf2, "ramgs2")

Uint16 testBuf[2] = {
              0x3FF,0x00
};

FRESULT fresult;
Uint16 file_ready =0;
Uint16 file_playing = 0;
float ans[2];

volatile Uint16 current_count = 44;
volatile Uint16 count = 0;
volatile int16 temp_0;
volatile int16 temp_1;
volatile Uint16 data;
volatile float data2;
volatile Uint16 refill_sdbuf1 = 0;
volatile Uint16 refill_sdbuf2 = 0;
volatile Uint16 sdbuf1_flag = 0;
volatile Uint16 sdbuf2_flag = 0;
volatile Uint16 first_mic = 1;
volatile Uint16 second_mic = 0;
volatile Uint16 done = 0;
volatile Uint16 order[3] ={
                           0,0,0
};


void DAC10Bit(Uint16 data){
    Uint16 new = (data >> 4) | 0xF000;
    //int16 new = 0xFFFF;
    SpibTransmit(new);
}

interrupt void xint1_isr(){
    //First to receive sound
    EALLOW;
    XintRegs.XINT1CR.bit.ENABLE = 0;
    if (first_mic){
        //Start timers
        EALLOW;
        CpuTimer1Regs.TIM.all = 0;
        CpuTimer2Regs.TIM.all = 0;
        CpuTimer1Regs.TCR.bit.TSS = 0;
        CpuTimer2Regs.TCR.bit.TSS = 0;
        EDIS;
        order[0] = 1;
        first_mic = 0;
        second_mic = 1;
    }
    //Second to receive sound
    else if (second_mic){
        //Collect 1st timer data
        CpuTimer1Regs.TCR.bit.TSS = 1;
        order[1] = 1;
        second_mic = 0;
    }
    //Last to receive sound
    else{
        order[2] = 1;
        //Collect 2nd timer data
        CpuTimer2Regs.TCR.bit.TSS = 1;

        done = 1;
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void xint2_isr(){
    //First to receive sound
    EALLOW;
    XintRegs.XINT2CR.bit.ENABLE = 0;
    if (first_mic){
        //Start timers
        EALLOW;
        CpuTimer1Regs.TIM.all = 0;
        CpuTimer2Regs.TIM.all = 0;
        CpuTimer1Regs.TCR.bit.TSS = 0;
        CpuTimer2Regs.TCR.bit.TSS = 0;
        EDIS;

        order[0] = 2;
        first_mic = 0;
        second_mic = 1;
    }
    //Second to receive sound
    else if (second_mic){
        //Collect 1st timer data
        CpuTimer1Regs.TCR.bit.TSS = 1;
        order[1] = 2;
        second_mic = 0;
    }
    //Last to receive sound
    else{
        //Collect 2nd timer data
        CpuTimer2Regs.TCR.bit.TSS = 1;
        order[2] = 2;
        done = 1;
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void xint3_isr(){
    //First to receive sound
    EALLOW;
    XintRegs.XINT3CR.bit.ENABLE = 0;
    if (first_mic){
        //Start timers
        EALLOW;
        CpuTimer1Regs.TIM.all = 0;
        CpuTimer2Regs.TIM.all = 0;
        CpuTimer1Regs.TCR.bit.TSS = 0;
        CpuTimer2Regs.TCR.bit.TSS = 0;
        EDIS;
        order[0] = 3;
        first_mic = 0;
        second_mic = 1;
    }
    //Second to receive sound
    else if (second_mic){
        //Collect 1st timer data
        CpuTimer1Regs.TCR.bit.TSS = 1;
        order[1] = 3;
        second_mic = 0;
    }
    //Last to receive sound
    else{
        //Collect 2nd timer data
        CpuTimer2Regs.TCR.bit.TSS = 1;
        order[2] = 3;
        done = 1;
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}



interrupt void cpu_timer0_isr(void){

    if (sdbuf1_flag){
        temp_0 = sdbuf1[current_count%sdBufferSize];
        current_count++;
        temp_1 = sdbuf1[current_count%sdBufferSize]<<8;
        temp_0 |= temp_1;
        data2 = (float)temp_0;
        data2 += 32767;
        data = (Uint16)data2;
        //data = (sdbuf1[current_count%sdBufferSize] << 8) | (sdbuf1[current_count+1%sdBufferSize]);
        DAC10Bit(data);
        current_count++;
    }
    else {
        temp_0 = sdbuf2[current_count%sdBufferSize];
        current_count++;
        temp_1 = sdbuf2[current_count%sdBufferSize]<<8;
        temp_0 |= temp_1;
        data2 = (float)temp_0;
        data2 += 32767;
        data = (Uint16)data2;
        //data = (sdbuf2[current_count%sdBufferSize] << 8) | (sdbuf1[current_count+1%sdBufferSize]);
        DAC10Bit(data);
        current_count++;
    }

    if (current_count%sdBufferSize == 0 && current_count != 0){
        if (sdbuf1_flag){
            sdbuf1_flag = 0;
            sdbuf2_flag = 1;
            refill_sdbuf1 = 1;
        }
        else {
            sdbuf1_flag = 1;
            sdbuf2_flag = 0;
            refill_sdbuf2 = 1;
        }
    }
    count++;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

void InitTimer0(){
    EALLOW;
    PieVectTable.TIMER0_INT = &cpu_timer0_isr; //function for ADCA interrupt 1
    initSpib();
    InitCpuTimers(); // For this example, only initialize the Cpu Timers
    ConfigCpuTimer(&CpuTimer0, 200, 125);
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    EDIS;
}

void StartTimer0(){
    EALLOW;
    CpuTimer0Regs.TCR.bit.TSS = 0;
    EDIS;
}

void StopTimer0(){
    EALLOW;
    CpuTimer0Regs.TCR.bit.TSS = 1;
    EDIS;
}

void EnablePinInts(){
    XintRegs.XINT1CR.bit.ENABLE = 1;
    XintRegs.XINT2CR.bit.ENABLE = 1;
    XintRegs.XINT3CR.bit.ENABLE = 1;

    first_mic = 1;
    second_mic = 0;
    done = 0;

}

void DisablePinInts(){
    XintRegs.XINT1CR.bit.ENABLE = 0;
    XintRegs.XINT2CR.bit.ENABLE = 0;
    XintRegs.XINT3CR.bit.ENABLE = 0;

}

void InitGpioInt(){
    EALLOW;
    PieVectTable.XINT1_INT = &xint1_isr;
    PieVectTable.XINT2_INT = &xint2_isr;
    PieVectTable.XINT3_INT = &xint3_isr;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieCtrlRegs.PIEIER1.bit.INTx4 = 1;
    PieCtrlRegs.PIEIER1.bit.INTx5 = 1;
    PieCtrlRegs.PIEIER12.bit.INTx1 = 1;

    GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 0;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO8 = 0;

    GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 0;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO8 = 0;

    GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 0;
    GpioCtrlRegs.GPAQSEL1.bit.GPIO8 = 0;

    GPIO_SetupXINT1Gpio(6);
    GPIO_SetupXINT2Gpio(7);
    GPIO_SetupXINT3Gpio(8);

    XintRegs.XINT1CR.bit.POLARITY = 0;
    XintRegs.XINT2CR.bit.POLARITY = 0;
    XintRegs.XINT3CR.bit.POLARITY = 0;

    XintRegs.XINT1CR.bit.ENABLE = 1;
    XintRegs.XINT2CR.bit.ENABLE = 1;
    XintRegs.XINT3CR.bit.ENABLE = 1;

    EDIS;
}


int main(){
#ifdef _FLASH
memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif
//
// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the F2837xD_SysCtrl.c file.
//
   InitSysCtrl();

#ifdef _STANDALONE
#ifdef _FLASH
//
// Send boot command to allow the CPU2 application to begin execution
//
IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH);
#else
//
// Send boot command to allow the CPU2 application to begin execution
//
IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_RAM);
#endif
#endif

//
// Call Flash Initialization to setup flash waitstates
// This function must reside in RAM
//
#ifdef _FLASH
   InitFlash();
#endif

    InitGpio();
    DINT;
    IER = 0x0000;
    IFR = 0x0000;
    InitPieCtrl();
    InitPieVectTable();



    while( !(MemCfgRegs.GSxMSEL.bit.MSEL_GS0 &
             MemCfgRegs.GSxMSEL.bit.MSEL_GS14))
    {
        EALLOW;
        MemCfgRegs.GSxMSEL.bit.MSEL_GS0 = 1;
        MemCfgRegs.GSxMSEL.bit.MSEL_GS14 = 1;
        EDIS;
    }

    InitEPwm1Gpio();
    InitEPwm2Gpio();

    GPIO_SetupPinMux(19, GPIO_MUX_CPU2, 2);
    GPIO_SetupPinOptions(19, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(18, GPIO_MUX_CPU2, 2);
    GPIO_SetupPinOptions(18, GPIO_OUTPUT, GPIO_ASYNC);
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;

    EALLOW;
    DevCfgRegs.CPUSEL0.bit.EPWM1 = 1;
    DevCfgRegs.CPUSEL0.bit.EPWM2 = 1;
    DevCfgRegs.CPUSEL11.bit.ADC_A = 1;
    DevCfgRegs.CPUSEL5.bit.SCI_B = 1;
    EDIS;



    InitTimer0();
    InitGpioInt();

    IER |= M_INT1 | M_INT12;
    EINT;

    Shared_Ram_dataWrite_c1();
    IPCLtoRFlagSet(IPC_FLAG10);

    WORD * bytes_read = 0;

    fresult = f_mount(0, &g_sFatFs);

/*
    fresult = f_mount(0, &g_sFatFs);
    fresult = f_open(&g_sFileObject, CLAP_DETECTED, FA_READ);
    fresult = f_read(&g_sFileObject, &sdbuf1[0], 1024, bytes_read);
    fresult = f_read(&g_sFileObject, &sdbuf2[0], 1024, bytes_read);

    //Start with buffer1
    sdbuf1_flag = 1;
    sdbuf2_flag = 0;

    StartTimer0();*/

    Uint32 start_count = 0xFFFFFFFF;
    Uint32 t1;
    Uint32 t2;
    float td1;
    float td2;
    float return_code;
    char* curr_file;
    ACK = 0;
    while(1){

        if (done){
            t1 = (start_count -  CpuTimer1Regs.TIM.all) >> 4;
            t2 = (start_count -  CpuTimer2Regs.TIM.all) >> 4;
            td1 = (float)t1*CPU_STEP;
            td2 = (float)t2*CPU_STEP;

            if (td1 < 1.0f && td2 < 1.0f && calcSoundRange(0.0f, td1, td2, ans)){
                Angle = calibrateResult(ans[0], ans[1], order);
                Distance = (int16)ans[1];


                file_ready = 1;
                file_playing = 0;
                curr_file = CLAP_DETECTED;

            }
            else {
                EnablePinInts();
            }
/*
            fresult = f_open(&g_sFileObject, CLAP_DETECTED, FA_READ);
            fresult = f_read(&g_sFileObject, &sdbuf1[0], 1024, bytes_read);
            fresult = f_read(&g_sFileObject, &sdbuf2[0], 1024, bytes_read);

            //Start with buffer1
            sdbuf1_flag = 1;
            sdbuf2_flag = 0;

            StartTimer0();
            //EnablePinInts();*/
            done = 0;
        }

        if (file_ready && !file_playing){
            DisablePinInts();

            fresult = f_open(&g_sFileObject, curr_file, FA_READ);
            fresult = f_read(&g_sFileObject, &sdbuf1[0], 1024, bytes_read);
            fresult = f_read(&g_sFileObject, &sdbuf2[0], 1024, bytes_read);

            //Start with buffer1
            sdbuf1_flag = 1;
            sdbuf2_flag = 0;

            StartTimer0();
            file_playing =1;
        }

        if (refill_sdbuf1){
            fresult = f_read(&g_sFileObject, &sdbuf1[0], 1024, bytes_read);
            refill_sdbuf1 = 0;
        }
        else if (refill_sdbuf2){
            fresult = f_read(&g_sFileObject, &sdbuf2[0], 1024, bytes_read);
            refill_sdbuf2 = 0;
        }

        if (g_sFileObject.fptr >= g_sFileObject.fsize && g_sFileObject.fsize > 0){
            //fresult = f_open(&g_sFileObject, CLAP_DETECTED, FA_READ);
            StopTimer0();
            g_sFileObject.fptr = 0;
            file_playing = 0;
            file_ready = 0;
            //if (curr_file == CLAP_DETECTED){
            EnablePinInts();
            //}
        }



        if (second_mic || (!first_mic && !second_mic)){
            t1 = (start_count -  CpuTimer1Regs.TIM.all) >> 4;
            t2 = (start_count -  CpuTimer2Regs.TIM.all) >> 4;
            td1 = (float)t1*CPU_STEP;
            td2 = (float)t2*CPU_STEP;
            if (td1 >1.0 || td2 >1.0){
                //Reset timeout occured
                EnablePinInts();
            }
        }

        if (Temperature > HIGH_TEMP_THRESH && !file_playing){
            curr_file = HIGH_TEMP_DETECTED;
            file_ready = 1;
        }
        if (MQ_135 > SMOKE_THRESH && !file_playing){
            curr_file = SMOKE_DETECTED;
            file_ready = 1;
        }
        if (MQ_9 > CO_THRESH && !file_playing){
            curr_file = CO_DETECTED;
            file_ready = 1;
        }
        if (MQ_2 > GAS_THRESH && !file_playing){
            curr_file = GAS_DETECTED;
            file_ready = 1;
        }
        if ((Covid == 1) && !file_playing){
            curr_file = COVID_DETECTED;
            file_ready = 1;
           // ACK = 1;
        }

        if(IPCLtoRFlagBusy(IPC_FLAG10) == 0)
            {
                Shared_Ram_dataRead_c1();


                //
                // Write an array to a memory location owned by CPU01
                //
                Shared_Ram_dataWrite_c1();

                //
                // Set a flag to notify CPU02 that data is available
                //
                IPCLtoRFlagSet(IPC_FLAG10);
            }

    }


}

//
// Shared_Ram_dataWrite_c1 - Write a pattern to an array in shared RAM
//

void Shared_Ram_dataWrite_c1(void)
{
    c1_r_w_array[0] = Angle;
    c1_r_w_array[2] = ACK;
    //ACK = 0;

}

//
// Shared_Ram_dataRead_c1 - Read and compare an array from shared RAM
//
void Shared_Ram_dataRead_c1(void)
{
    MQ_135 = c1_r_array[0];  // Smoke Sensor
    MQ_9 = c1_r_array[1];    // CO, Methane
    MQ_2 = c1_r_array[2];    // Combustable Gas
    Temperature = c1_r_array[3];  // Temperature
    Covid = c1_r_array[4];
}
