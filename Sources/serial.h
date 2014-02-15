#ifndef _SERIAL_H
#define _SERIAL_H

#include "derivative.h"      /* derivative-specific definitions */

typedef unsigned char INT8U;

//functionn declarations
void InitPorts(void);
void InitSCI(void);
void Sniff(const INT8U byt);

#endif