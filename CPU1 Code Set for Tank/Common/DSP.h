#ifndef DSP_H_
#define DSP_H_

void writeLCDCommand(Uint16 cmd);
void writeLCDData(Uint16 data);
void initializeLCD(void);
void writeString(char* str);
void InitMcBSPb();
void InitBigBangedCodecSPI();
void BitBangedCodecSpiTransmit(Uint16 data);
void InitAIC23();
void InitSPIA();
void SpiTransmit(uint16_t data);
void initSpib();
Uint16 SpibTransmit(Uint16 transmitdata);
void WriteSRAM0(Uint32 address, Uint16 data);
void WriteSRAM1(Uint32 address, Uint16 data);
Uint16 ReadSRAM1(Uint32 address);
Uint16 ReadSRAM0(Uint32 address);
void SpiWrite(Uint32 address, Uint16 data);
Uint16 SpiRead(Uint32 address);
void initMcbspInt(void (*function)(void));

#endif /* DSP_H_ */
