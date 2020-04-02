/*
 * UART.c
 *
 *  Created on: Feb 1, 2020
 *      Author: Paiscaso
 */

#include "UART.h"

//
// scia_fifo_init - Configure SCIA FIFO
//
void scib_fifo_init(){
   ScibRegs.SCICCR.all = 0x0007;      // 1 stop bit,  No loopback
                                      // No parity,8 char bits,
                                      // async mode, idle-line protocol
   ScibRegs.SCICTL1.all = 0x0003;     // enable TX, RX, internal SCICLK,
                                      // Disable RX ERR, SLEEP, TXWAKE
   ScibRegs.SCICTL2.bit.TXINTENA = 1;
   ScibRegs.SCICTL2.bit.RXBKINTENA = 1;
   ScibRegs.SCIHBAUD.all = 0x0002;
   ScibRegs.SCILBAUD.all = 0x008B;
   ScibRegs.SCICCR.bit.LOOPBKENA = 0; // Disable loop back
   ScibRegs.SCIFFTX.all = 0xC022;
   ScibRegs.SCIFFRX.all = 0x0022;
   ScibRegs.SCIFFCT.all = 0x00;

   ScibRegs.SCICTL1.all = 0x0023;     // Relinquish SCI from Reset
   ScibRegs.SCIFFTX.bit.TXFIFORESET = 1;
   ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;
}

