// Interrupt Library

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef ISR_H_
#define ISR_H_

#include <stdint.h>
#include <stdbool.h>
#include "asm.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

char* uitoa(uint32_t num);
char* inttohex(uint32_t num);
void busFaultISR();
void usageFaultISR();
void hardFaultISR();
void mpuFaultISR();
void pendsvISR();

#endif
