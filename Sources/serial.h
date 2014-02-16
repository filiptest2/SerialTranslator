#ifndef _SERIAL_H
#define _SERIAL_H

#include "derivative.h"      /* derivative-specific definitions */

typedef unsigned char INT8U;


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

//functionn declarations
void InitPorts(void);
void InitSCI(void);
void Sniff(INT8U byt);
void FIFOInit(void);
short FIFOPut(const INT8U data);  //const makes sure data is not modified within a function          
short FIFOGet(INT8U *dataPtr);

#endif