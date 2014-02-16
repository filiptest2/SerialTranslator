#ifndef _SERIAL_H
#define _SERIAL_H

#include "derivative.h"      /* derivative-specific definitions */

//make data types independent of compiler
typedef unsigned char INT8U;
typedef signed char   INT8S;
typedef char          INT8;
typedef unsigned int  INT16U;


#define FIFOSIZE 10
#define StartCritical(){ asm tpa;asm staa savedCCR; asm sei;}
#define EndCritical(){ asm ldaa savedCCR; asm tap;}


extern INT8U *PUTPT;
extern INT8U *GETPT;
/*
extern const INT8U STX;
extern const INT8U ETX;
*/
extern INT8U Fifo[FIFOSIZE];

typedef enum
{
  WaitForSTX, WaitForMSB,WaitForLSB,WaitForETX
}TCommsState;

typedef struct
{
  TCommsState state;
  INT8U data;
  INT8U *port;  
}TPacket;

//functionn declarations
void Init(void);
void InitPorts(void);
void InitSCI(void);
void Sniff(const INT8U byt,TPacket *const packet);
INT8S ConvertHexToASCII(INT8U letter);
void FIFOInit(void);
short FIFOPut(const INT8U data);  //const makes sure data is not modified within a function          
short FIFOGet(INT8U *dataPtr);

#endif