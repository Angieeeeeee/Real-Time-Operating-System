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

void busFaultISR(void);
void usageFaultISR(void);
void hardFaultISR(void);
void mpuFaultISR(void);
void pendsvISR(void);

#endif
