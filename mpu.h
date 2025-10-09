// Memory functions
// Angelina

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

#ifndef MPU_H_
#define MPU_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "isr.h"
#include "uart0.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void setBackgroundRule(void);
void allowFlashAccess(void);
void allowPeripheralAccess(void);
void setupSramAccess(void);
uint64_t createSramAccessMask(void);
void applySramAccessMask(uint64_t srdBitMask);
void addSramAccessWindow(uint64_t *srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes);

#endif
