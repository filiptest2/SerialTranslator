#include "serial.h"

const INT8U STX = 0x02;
const INT8U ETX = 0x03;

static TPacket SCI0Packet;

void Init(void)
{
  SCI0Packet.state = WaitForSTX;
  SCI0Packet.port = &PORTB;
  _asm cli;
}

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
  SCI0CR2 = 0x2C; //receive interrupt enabled      
}

void FIFOInit(void) 
{   
  INT8U savedCCR;
  StartCritical();
  PUTPT = GETPT = &Fifo[0];     
  EndCritical();
}

short FIFOPut(const INT8U data)
{
  INT8U *Ppt;
  INT8U savedCCR;  
  StartCritical();
  Ppt = PUTPT;
  *(Ppt++) = data;
  if (Ppt == &Fifo[FIFOSIZE])
      Ppt = &Fifo[0];
  
  if(Ppt == GETPT) 
  {
     EndCritical();
     return(0); //fifo full
  } 
  else
  {
    PUTPT = Ppt;
    EndCritical();
    return(1);  //successful
  }
}

short FIFOGet (INT8U *dataPtr)
{        
  INT8U savedCCR;
  if(PUTPT == GETPT)
    return (0); //sorry, fifo empty    
  else
  {  
    StartCritical();  
    *dataPtr = *(GETPT++);           
    if(GETPT == &Fifo[FIFOSIZE])
      GETPT = &Fifo[0];
    EndCritical();
    return(1);  //successful
  }
}

INT8S ConvertHexToASCII(INT8U letter)
{  
  return (letter |= 0x30);
}


void Sniff(const INT8U byt,TPacket *const packet)
{
  INT8 digit;
  switch((*packet).state) 
  {
    case WaitForSTX:
    if(byt == STX);
    (*packet).state = WaitForMSB;
    break;
    
    case WaitForMSB:
    digit = ConvertHexToASCII(byt);
    if(digit>= 0)
      {
        (*packet).data = digit <<4;
        (*packet).state = WaitForLSB;
      } 
    else 
      (*packet).state = WaitForSTX; //error, restart
    break;
    
    case WaitForLSB:
    digit = ConvertHexToASCII(byt);     
    if(digit>= 0)
      {
        (*packet).data |= digit;
        (*packet).state = WaitForETX;
      } 
    else
      (*packet).state = WaitForSTX; //error, restart
    break;
    
    case WaitForETX:
    if(byt == ETX)    
      *((*packet).port) = (*packet).data;
    (*packet).state = WaitForSTX;      
    break;        
  }
}

void interrupt 20 SCI0_ISR(void)
{  
  if (SCI0SR1_RDRF && SCI0CR2_RIE)
  {    
    Sniff(SCI0DRL,&SCI0Packet);
    if(FIFOPut(SCI0DRL)) //if there is space in buffer      
        SCI0CR2_SCTIE = 1;  //arm transmit interrupt, new data available              
  }
  
  if (SCI0SR1_TDRE && SCI0CR2_SCTIE) 
  {
    if(FIFOGet(&SCI0DRL) == 0)               
    SCI0CR2_SCTIE = 0;  //disarm transmit interrupt, no data available  
  }      
}