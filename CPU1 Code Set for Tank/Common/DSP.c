#include <F28x_Project.h>
#include "OnetoOneI2CDriver.h"
#include "AIC23.h"
#include <stdint.h>
#include "DSP.h"

/***************** Defines ***************/
#define SmallDelay() for(volatile long  i = 0; i < 500000; i++)
/***************** Defines ***************/
#define CodecSPI_CLK_PULS {EALLOW; GpioDataRegs.GPASET.bit.GPIO18 = 1; GpioDataRegs.GPACLEAR.bit.GPIO18 = 1;}
#define CodecSPI_CS_LOW {EALLOW; GpioDataRegs.GPACLEAR.bit.GPIO19 = 1;}
#define CodecSPI_CS_HIGH {EALLOW; GpioDataRegs.GPASET.bit.GPIO19 = 1;}

void writeLCDCommand(Uint16 cmd){
    //Initialize array
    Uint16 data_arr[4];
    //Command must be broken into nibbles
    //We need to send
    //Upper nibble enable high 0xC
    Uint16 nibble = cmd & 0xF0;
    nibble = nibble | 0x0C;
    data_arr[0] = nibble;

    //Upper nibble enable low  0x8
    nibble = nibble & 0xF0;
    nibble = nibble | 0x08;
    data_arr[1] = nibble;

    //Lower nibble enable high 0xC
    nibble = cmd << 4;
    nibble = nibble & 0xF0;
    nibble = nibble | 0x0C;
    data_arr[2] = nibble;

    //Lower nibble enable low  0x8
    nibble = nibble & 0xF0;
    nibble = nibble | 0x08;
    data_arr[3] = nibble;

    I2C_O2O_SendBytes(data_arr, 4);
}

void writeLCDData(Uint16 data){
    //Initialize array
    Uint16 data_arr[4];
    //Command must be broken into nibbles
    //We need to send
    //Upper nibble enable high 0xC
    Uint16 nibble = data & 0xF0;
    nibble = nibble | 0x0D;
    data_arr[0] = nibble;

    //Upper nibble enable low  0x8
    nibble = nibble & 0xF0;
    nibble = nibble | 0x09;
    data_arr[1] = nibble;

    //Lower nibble enable high 0xC
    nibble = data << 4;
    nibble = nibble & 0xF0;
    nibble = nibble | 0x0D;
    data_arr[2] = nibble;

    //Lower nibble enable low  0x8
    nibble = nibble & 0xF0;
    nibble = nibble | 0x09;
    data_arr[3] = nibble;

    I2C_O2O_SendBytes(data_arr, 4);
}

void initializeLCD(void){
    //Write commands for 4bit mode
    writeLCDCommand(0x33);
    writeLCDCommand(0x32);
    writeLCDCommand(0x28);
    writeLCDCommand(0x0F);
    writeLCDCommand(0x01);
}

void writeString(char* str){
    Uint16 i = 0;
    while(str[i] != '\0'){
        writeLCDData(str[i]);
        i++;
    }
}

void InitMcBSPb()
{
    /* Init McBSPb GPIO Pins */

    //modify the GPxMUX, GPxGMUX, GPxQSEL
    //all pins should be set to asynch qualification

    /*
     * MDXB -> GPIO24
     * MDRB -> GPIO25
     * MCLKRB -> GPIO60
     * MCLKXB -> GPIO26
     * MFSRB -> GPIO61
     * MFSXB -> GPIO27
     */
    EALLOW;

    // MDXB -> GPIO24 (GPIOA)

    GpioCtrlRegs.GPAGMUX2.bit.GPIO24 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 3;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO24 = 3;

    // MDRB -> GPIO25 (GPIOA)

    GpioCtrlRegs.GPAGMUX2.bit.GPIO25 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 3;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO25 = 3;

    // MFSRB -> GPIO61 (GPIOB)

    GpioCtrlRegs.GPBGMUX2.bit.GPIO61 = 0;
    GpioCtrlRegs.GPBMUX2.bit.GPIO61 = 1;
    GpioCtrlRegs.GPBQSEL2.bit.GPIO61 = 3;

    // MFSXB -> GPIO27 (GPIOA)

    GpioCtrlRegs.GPAGMUX2.bit.GPIO27 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 3;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO27 = 3;

    // MCLKRB -> GPIO60 (GPIOB)

    GpioCtrlRegs.GPBGMUX2.bit.GPIO60 = 0;
    GpioCtrlRegs.GPBMUX2.bit.GPIO60 = 1;
    GpioCtrlRegs.GPBQSEL2.bit.GPIO60 = 3;

    // MCLKXB -> GPIO26 (GPIOA)

    GpioCtrlRegs.GPAGMUX2.bit.GPIO26 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 3;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO26 = 3;
    EDIS;

    /* Init McBSPb for I2S mode */
    EALLOW;
    McbspbRegs.SPCR2.all = 0; // Reset FS generator, sample rate generator & transmitter
    McbspbRegs.SPCR1.all = 0; // Reset Receiver, Right justify word
    McbspbRegs.SPCR1.bit.RJUST = 2; // left-justify word in DRR and zero-fill LSBs
    McbspbRegs.MFFINT.all=0x0; // Disable all interrupts
    McbspbRegs.SPCR1.bit.RINTM = 0; // McBSP interrupt flag - RRDY
    McbspbRegs.SPCR2.bit.XINTM = 0; // McBSP interrupt flag - XRDY
    // Clear Receive Control Registers
    McbspbRegs.RCR2.all = 0x0;
    McbspbRegs.RCR1.all = 0x0;
    // Clear Transmit Control Registers
    McbspbRegs.XCR2.all = 0x0;
    McbspbRegs.XCR1.all = 0x0;
    // Set Receive/Transmit to 32-bit operation
    McbspbRegs.RCR2.bit.RWDLEN2 = 5;
    McbspbRegs.RCR1.bit.RWDLEN1 = 5;
    McbspbRegs.XCR2.bit.XWDLEN2 = 5;
    McbspbRegs.XCR1.bit.XWDLEN1 = 5;
    McbspbRegs.RCR2.bit.RPHASE = 1; // Dual-phase frame for receive
    McbspbRegs.RCR1.bit.RFRLEN1 = 0; // Receive frame length = 1 word in phase 1
    McbspbRegs.RCR2.bit.RFRLEN2 = 0; // Receive frame length = 1 word in phase 2
    McbspbRegs.XCR2.bit.XPHASE = 1; // Dual-phase frame for transmit
    McbspbRegs.XCR1.bit.XFRLEN1 = 0; // Transmit frame length = 1 word in phase 1
    McbspbRegs.XCR2.bit.XFRLEN2 = 0; // Transmit frame length = 1 word in phase 2
    // I2S mode: R/XDATDLY = 1 always
    McbspbRegs.RCR2.bit.RDATDLY = 1;
    McbspbRegs.XCR2.bit.XDATDLY = 1;
    // Frame Width = 1 CLKG period, CLKGDV must be 1 as slave
    McbspbRegs.SRGR1.all = 0x0001;
    McbspbRegs.PCR.all=0x0000;
    // Transmit frame synchronization is supplied by an external source via the FSX pin
    McbspbRegs.PCR.bit.FSXM = 0;
    // Receive frame synchronization is supplied by an external source via the FSR pin
    McbspbRegs.PCR.bit.FSRM = 0;
    // Select sample rate generator to be signal on MCLKR pin
    McbspbRegs.PCR.bit.SCLKME = 1;
    McbspbRegs.SRGR2.bit.CLKSM = 0;
    // Receive frame-synchronization pulses are active low - (L-channel first)
    McbspbRegs.PCR.bit.FSRP = 1;
    // Transmit frame-synchronization pulses are active low - (L-channel first)
    McbspbRegs.PCR.bit.FSXP = 1;
    // Receive data is sampled on the rising edge of MCLKR
    McbspbRegs.PCR.bit.CLKRP = 1;



    // Transmit data is sampled on the rising edge of CLKX
    McbspbRegs.PCR.bit.CLKXP = 1;



    // The transmitter gets its clock signal from MCLKX
    McbspbRegs.PCR.bit.CLKXM = 0;
    // The receiver gets its clock signal from MCLKR
    McbspbRegs.PCR.bit.CLKRM = 0;
    // Enable Receive Interrupt
    McbspbRegs.MFFINT.bit.RINT = 1;
    // Ignore unexpected frame sync
    //McbspbRegs.XCR2.bit.XFIG = 1;
    McbspbRegs.SPCR2.all |=0x00C0; // Frame sync & sample rate generators pulled out of reset
    delay_loop();
    McbspbRegs.SPCR2.bit.XRST=1; // Enable Transmitter
    McbspbRegs.SPCR1.bit.RRST=1; // Enable Receiver
    EDIS;
}

void InitBigBangedCodecSPI(){
    /*
     * GPIO19 - CS
     * GPIO18 - CLK
     * GPIO58 - SPISIMOA
     */

    EALLOW;

    //enable pullups
    GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO58 = 0;

    GpioCtrlRegs.GPAGMUX2.bit.GPIO19 = 0;
    GpioCtrlRegs.GPAGMUX2.bit.GPIO18 = 0;
    GpioCtrlRegs.GPBGMUX2.bit.GPIO58 = 0;

    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 0;
    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 0;

    GpioCtrlRegs.GPADIR.bit.GPIO19 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO18 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO58 = 1;

    GpioDataRegs.GPASET.bit.GPIO19 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO18 = 1;



    EDIS;
}

void BitBangedCodecSpiTransmit(Uint16 data){
    CodecSPI_CS_LOW;
    /* Transmit 16 bit data */
    //send data out MSB first
    for(Uint16 i = 16; i > 0; i--){
        GpioDataRegs.GPBDAT.bit.GPIO58 = (data >> (i-1)) & 1;
        CodecSPI_CLK_PULS;
    }

    CodecSPI_CS_HIGH;
}

void InitAIC23()
{
    SmallDelay();
    uint16_t command;
    command = reset();
    BitBangedCodecSpiTransmit (command);
    SmallDelay();
    command = softpowerdown();       // Power down everything except device and clocks
    BitBangedCodecSpiTransmit (command);
    SmallDelay();
    command = linput_volctl(LIV);    // Unmute left line input and maintain default volume
    BitBangedCodecSpiTransmit (command);
    SmallDelay();
    command = rinput_volctl(RIV);    // Unmute right line input and maintain default volume
    BitBangedCodecSpiTransmit (command);
    SmallDelay();
    command = lhp_volctl(LHV);       // Left headphone volume control
    BitBangedCodecSpiTransmit (command);
    SmallDelay();
    command = rhp_volctl(RHV);       // Right headphone volume control
    BitBangedCodecSpiTransmit (command);
    SmallDelay();
    command = nomicaaudpath();      // Turn on DAC, mute mic
    BitBangedCodecSpiTransmit (command);
    SmallDelay();
    command = digaudiopath();       // Disable DAC mute, add de-emph
    BitBangedCodecSpiTransmit (command);
    SmallDelay();

    // I2S
    command = I2Sdigaudinterface(); // AIC23 master mode, I2S mode,32-bit data, LRP=1 to match with XDATADLY=1
    BitBangedCodecSpiTransmit (command);
    SmallDelay();
    //command = CLKsampleratecontrol (SR48);
    command = CLKsampleratecontrol(0x0);
    BitBangedCodecSpiTransmit (command);
    SmallDelay();

    command = digact();             // Activate digital interface
    BitBangedCodecSpiTransmit (command);
    SmallDelay();
    command = nomicpowerup();      // Turn everything on except Mic.
    BitBangedCodecSpiTransmit (command);

}


void InitSPIA()
{
    /* Init GPIO pins for SPIA */

    //enable pullups for each pin
    //set to asynch qualification
    //configure each mux

    //SPISTEA -> GPIO19
    //SPISIMOA -> GPIO58
    //SPICLKA -> GPIO18

    EALLOW;

    //enable pullups
    GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO58 = 0;

    GpioCtrlRegs.GPAGMUX2.bit.GPIO19 = 0;
    GpioCtrlRegs.GPAGMUX2.bit.GPIO18 = 0;
    GpioCtrlRegs.GPBGMUX2.bit.GPIO58 = 3;

    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 1;
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 1;
    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 3;

    //asynch qual
    GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 3;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3;
    GpioCtrlRegs.GPBQSEL2.bit.GPIO58 = 3;

    EDIS;

    /* Init SPI peripheral */
    SpiaRegs.SPICCR.all = 0x5F; //CLKPOL = 0, SOMI = SIMO (loopback), 16 bit characters
    SpiaRegs.SPICTL.all = 0x06; //master mode, enable transmissions
    SpiaRegs.SPIBRR.all = 50; //gives baud rate of approx 850 kHz

    SpiaRegs.SPICCR.bit.SPISWRESET = 1;
    SpiaRegs.SPIPRI.bit.FREE = 1;

}
void SpiTransmit(uint16_t data)
{
    /* Transmit 16 bit data */
    SpiaRegs.SPIDAT = data; //send data to SPI register
    while(SpiaRegs.SPISTS.bit.INT_FLAG == 0); //wait until the data has been sent
    Uint16 dummyLoad = SpiaRegs.SPIRXBUF; //reset flag
}

void initSpib(){
    EALLOW;

    /**************Configure GPIO for SPI********************/
    GpioCtrlRegs.GPBGMUX2.bit.GPIO63 = 0x3;
    GpioCtrlRegs.GPCGMUX1.bit.GPIO64 = 0x3;
    GpioCtrlRegs.GPCGMUX1.bit.GPIO65 = 0x3;
    //GpioCtrlRegs.GPCGMUX1.bit.GPIO66 = 0x3;

    GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 0x3;
    GpioCtrlRegs.GPCMUX1.bit.GPIO64 = 0x3;
    GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 0x3;
    //GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 0x3;

    //chip select 0 and 1 are outputs
    GpioCtrlRegs.GPCDIR.bit.GPIO66 = 1;
    //GpioCtrlRegs.GPCDIR.bit.GPIO67 = 1;

    //Set chip selects high
    GpioDataRegs.GPCDAT.bit.GPIO66 = 1;
    //GpioDataRegs.GPCDAT.bit.GPIO67 = 1;
    /**************END CONFIGURE GPIO********************/


    //Peripheral clock prescaling
    EALLOW;
    //ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 0;

    //Clear SPISWRESET
    SpibRegs.SPICCR.bit.SPISWRESET = 0;

    //Master and prescale SPICLK for 200MHz/5
    SpibRegs.SPICTL.bit.MASTER_SLAVE = 1;
    SpibRegs.SPICTL.bit.TALK = 1;
    SpibRegs.SPIBRR.all = 0x1F;

    //Match CLKPOLARITY and CLKPHASE
    SpibRegs.SPICCR.all = 0x2F;
    SpibRegs.SPICCR.bit.CLKPOLARITY = 0;
    SpibRegs.SPICTL.bit.CLK_PHASE = 1;

    //Software reset and set free but high
    SpibRegs.SPICCR.bit.SPISWRESET = 1;
    SpibRegs.SPIPRI.bit.FREE = 1;
}

Uint16 SpibTransmit(Uint16 transmitdata){
    //bring chip select low
    GpioDataRegs.GPCDAT.bit.GPIO66 = 0;

    SpibRegs.SPIDAT = transmitdata;

    while(!SpibRegs.SPISTS.bit.INT_FLAG){
        //wait until sent
    }

    Uint16 returndata = SpibRegs.SPIRXBUF;

    //Bring chip select high
    GpioDataRegs.GPCDAT.bit.GPIO66 = 1;

    return returndata;
}

void WriteSRAM0(Uint32 address, Uint16 data){
    //40 bits in total 8 instruction bits, 24 address bits, and 16 data bits
    Uint16 temp;
    //Start
    GpioDataRegs.GPCDAT.bit.GPIO66 = 0;

    SpibTransmit(0x02);
    temp = address >> 16;
    SpibTransmit(temp);
    temp = address >> 8;
    SpibTransmit(temp);
    temp = address;
    SpibTransmit(temp);
    SpibTransmit(data >> 8);
    SpibTransmit(data);

    GpioDataRegs.GPCDAT.bit.GPIO66 = 1;
}

void WriteSRAM1(Uint32 address, Uint16 data){
    //40 bits in total 8 instruction bits, 24 address bits, and 16 data bits
    Uint16 temp;
    //Start
    GpioDataRegs.GPCDAT.bit.GPIO67 = 0;

    SpibTransmit(0x02);
    temp = address >> 16;
    SpibTransmit(temp);
    temp = address >> 8;
    SpibTransmit(temp);
    temp = address;
    SpibTransmit(temp);
    SpibTransmit(data >> 8);
    SpibTransmit(data);

    DELAY_US(1);
    GpioDataRegs.GPCDAT.bit.GPIO67 = 1;
}

Uint16 ReadSRAM1(Uint32 address){
    //40 bits in total 8 instruction bits, 24 address bits, and 16 data bits
    Uint16 temp;
    //Start
    GpioDataRegs.GPCDAT.bit.GPIO67 = 0;

    SpibTransmit(0x03);
    temp = address >> 16;
    SpibTransmit(temp);
    temp = address >> 8;
    SpibTransmit(temp);
    temp = address;
    SpibTransmit(temp);
    SpibTransmit(0x00); //Dummy cycle
    temp = SpibTransmit(0x00); //Dummy data (lower byte)
    temp = (temp << 8) | (SpibTransmit(0x00)); //Dummy data (upper byte)

    DELAY_US(1);
    GpioDataRegs.GPCDAT.bit.GPIO67 = 1;

    return temp;
}

Uint16 ReadSRAM0(Uint32 address){
    //40 bits in total 8 instruction bits, 24 address bits, and 16 data bits
    Uint16 temp;
    //Start
    GpioDataRegs.GPCDAT.bit.GPIO66 = 0;

    SpibTransmit(0x03);
    temp = address >> 16;
    SpibTransmit(temp);
    temp = address >> 8;
    SpibTransmit(temp);
    temp = address;
    SpibTransmit(temp);
    SpibTransmit(0x00); //Dummy cycle
    temp = SpibTransmit(0x00); //Dummy data (lower byte)
    temp = (temp << 8) | (SpibTransmit(0x00)); //Dummy data (upper byte)

    DELAY_US(1);
    GpioDataRegs.GPCDAT.bit.GPIO66 = 1;

    return temp;
}

void SpiWrite(Uint32 address, Uint16 data){
    if (address < 0x20000){
        WriteSRAM0(address << 1, data);
    }
    else{
        address = address & 0xFDFFFF;
        WriteSRAM1(address << 1, data);
    }
}

Uint16 SpiRead(Uint32 address){
    if (address < 0x20000){
        return ReadSRAM0(address << 1);
    }
    else{
        address = address & 0xFDFFFF;
        return ReadSRAM1(address << 1);
    }
}

void initMcbspInt(void (*function)(void)){
    EALLOW;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.MCBSPB_RX_INT = function;
    PieCtrlRegs.PIEIER6.bit.INTx7 = 1;
    IER |= 0x20;
}
