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
  SCI0CR2 = 0x2C; //receive interrupt enabled
  _asm cli;
}

void FIFOInit(void) 
{ 
  //extern INT8U savedCCR;  
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

void Sniff(INT8U byt)
{  
  PORTB = byt;  
}

void interrupt 20 SCI0_ISR(void)
{  
  if (SCI0SR1_RDRF && SCI0CR2_RIE)
  {    
    if (FIFOPut(SCI0DRL)) //if there is space in buffer
      {
        SCI0CR2_SCTIE = 1;  //arm transmit interrupt, new data available        
      }
  }
  
  if (SCI0SR1_TDRE && SCI0CR2_SCTIE) 
  {
    if(FIFOGet(&SCI0DRL) == 0)               
    SCI0CR2_SCTIE = 0;  //disarm transmit interrupt, no data available  
  }      
}