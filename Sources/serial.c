#include "serial.h"

const INT8U STX = 0x02;
const INT8U ETX = 0x03;

static TPacket SCI0Packet;

void Init(void)
{
  InitPorts();
  InitSCI();
  FIFOInit();
  //initialize comms states
  SCI0Packet.state = WaitForSTX;
  SCI0Packet.port = &PORTB;
  
  //enable interrupts globally
  _asm cli;
}

void InitPorts(void)
{
  //set ports to zero
  PORTB = 0x00;
  PTJ = 0x00;
  
  //ports B and J output ports
  DDRB = 0xFF;  
  DDRJ = 0xFF; 
}

void InitSCI(void)
{
  //must use 1A, in run mode Fosc = 8 MHz, i.e. now BR = 9600
  SCI0BDH = 0x00;
  SCI0BDL = 0x1A; 
  
  //8 bits, no parity, 1 stop bit receive interrupt enabled 
  SCI0CR1 = 0x00;
  SCI0CR2 = 0x2C; 
}

/*
Initialize the FIFO
Input: -  
Output: -
Conditions: -
*/
void FIFOInit(void) 
{   
  INT8U savedCCR;
  StartCritical();
  PUTPT = GETPT = &Fifo[0];     
  EndCritical();
}


/*
Put data into FIFO
Input: 8 bit data to put into FIFO
Output: 1 if successful, 0 if unsuccessful because FIFO is full
Conditions: -
*/
short FIFOPut(const INT8U data)
{
  INT8U *Ppt;
  INT8U savedCCR;  
  StartCritical();
  Ppt = PUTPT;
  *(Ppt++) = data;
  if (Ppt == &Fifo[FIFOSIZE])
      //wrap around
      Ppt = &Fifo[0];
  
  if(Ppt == GETPT) 
    {
      EndCritical();
      //fifo full
      return(0); 
    } 
  else
    {
      PUTPT = Ppt;
      EndCritical();
       //successful
      return(1); 
    }
}


/*
Get data from FIFO
Input: dataPtr is the pointer to where the 8-bit from the FIFO is to be stored
Output: 1 if successful, 0 if unsuccessful because FIFO was empty
Conditions: -
*/
short FIFOGet(INT8U *dataPtr)
{        
  INT8U savedCCR;
  if(PUTPT == GETPT)
      //sorry, fifo empty 
      return (0);    
  else
    {  
      StartCritical();  
      *dataPtr = *(GETPT++);           
      if(GETPT == &Fifo[FIFOSIZE])
        GETPT = &Fifo[0];
      EndCritical();
       //successful
      return(1); 
    }
}

/*
Converts hex value to its ASCII equivalent
Input: letter is the hex value to convert
Output: '0'-'9', 'A'-'F'
Conditions: -
*/
INT8S ConvertHexToASCII(INT8U letter)
{        
  if(letter > '9')
    {    
      letter += 9;
    }
  return (letter &= 0x0F);  
}

/*
"Sniffs" an incoming byte to check for protocol conformance, and if valid outputs the data byte to PORTB
Input: byt is the byte received, packet points to the SCI's state, data and port
Output: -
Conditions: -
*/
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
    //convert hex to ASCII
    digit = ConvertHexToASCII(byt);
    if(digit>= 0)
      {
        //port alignment
        (*packet).data = digit <<4;
        (*packet).state = WaitForLSB;
      } 
    else 
      //error, restart
      (*packet).state = WaitForSTX; 
    break;
    
    case WaitForLSB:
    //convert hex to ASCII
    digit = ConvertHexToASCII(byt);     
    if(digit>= 0)
      {
        //port alignment
        (*packet).data |= digit;        
        (*packet).state = WaitForETX;              
      } 
    else
      //error, restart
      (*packet).state = WaitForSTX; 
    break;
    
    case WaitForETX:
    //if ETX found, output data to PORTB
    if(byt == ETX)    
      *((*packet).port) = (*packet).data;
    //restart
    (*packet).state = WaitForSTX;      
    break;        
  }
}

void interrupt 20 SCI0_ISR(void)
{  
  if (SCI0SR1_RDRF && SCI0CR2_RIE)
  {    
    Sniff(SCI0DRL,&SCI0Packet);
    if(FIFOPut(SCI0DRL)) //check if there is space in buffer
        SCI0CR2_SCTIE = 1;  //arm transmit interrupt, new data available              
  }
  
  if (SCI0SR1_TDRE && SCI0CR2_SCTIE) 
  {
    if(FIFOGet(&SCI0DRL) == 0) //transmit until buffer becomes empty              
    SCI0CR2_SCTIE = 0;  //disarm transmit interrupt, no data available  
  }      
}