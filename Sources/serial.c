#include "serial.h"

void InitPorts(void)
{
  PORTB = 0x00;
  PTJ = 0x00;
  DDRB = 0xFF;  //ports B and J output ports
  DDRJ = 0xFF; 
}

void InitSCI(void)
{
  SCI0BDH = 0x00;
  SCI0BDL = 0x1A;  //must use 1A, in run mode Fosc = 8 MHz, i.e. now BR = 9600
  SCI0CR1 = 0x00;
  SCI0CR2 = 0xAC; //receive and transmit interrupt enabled
  _asm cli;
}

void Sniff(const INT8U byt)
{
  PORTB = byt | 0x30;
}

void interrupt 20 SCI0_ISR(void)
{
  if (SCI0SR1 & 0x20)    
    Sniff(SCI0DRL);
  
  if (SCI0SR1 & 0x80)
    SCI0DRL = PORTB;    
}