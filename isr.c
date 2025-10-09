// Interrupt Handler Library

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// Interrupt Handlers:
// short low cost programs that return information about the system failure
// when an interrupt is triggered

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "isr.h"
#include "asm.h"
#include "uart0.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

extern uint32_t pid;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// int to char* function for putsUart0() to print out the values of the registers IDK IF I CAN USE STDLIB IT DONT ASK ME
char* uitoa(uint32_t num)
{
    static char newString[11];          //unit32_t has max value of 4,294,967,295 so 10 spots (plus null)
    char* ptr = &newString[10];
    *ptr = '\0';

    if (num == 0)
    {
        *(--ptr) = '0';
    }

    while (num != 0)
    {
        // decrement ptr, will return with pointer pointing to beginning of the array
        *(--ptr) = (num % 10) + '0';
        num /= 10;
    }
    return ptr;
}

char* inttohex(uint32_t num)
{
    static char array[11]; // 32 bits "0xFFFFFFFF"
    char *ptr = &array[10];
    *ptr = '\0';
    int curr;
    if (num == 0) *(--ptr) = '0';
    while (num > 0)
    {
        curr = num % 16;
        if (curr > 10) *(--ptr) = 'A' + (curr - 10);
        else *(--ptr) = '0' + curr;
        num /= 16;
    }
    *(--ptr) = 'x';
    *(--ptr) = '0';

    return ptr;
}

void busFaultISR()
{
    putsUart0("Bus fault in process "); putsUart0(uitoa(pid)); putcUart0('\n');

    while(1);
}

void usageFaultISR()
{
    putsUart0("Usage fault in process ");
    putsUart0(uitoa(pid));
    putcUart0('\n');

    while(1);
}

void hardFaultISR()
{
    uint32_t r0, r1, r2, r3, r12, lr, pc, xpsr, mfault, address;
    uint32_t *psp, *msp;

    // go up the stack to collect values to output
    psp  = getPsp();
    msp  = getMsp();
    r0   = *psp;
    r1   = *(psp + 1);
    r2   = *(psp + 2);
    r3   = *(psp + 3);
    r12  = *(psp + 4);
    lr   = *(psp + 5);
    pc   = *(psp + 6);
    xpsr = getIpsr();
    mfault = NVIC_FAULT_STAT_R & 0xFF;       // mem fault  bits [7:0] in FAULTSTAT
    address = pc;
    uint32_t opcode = *((uint32_t*)(address));

    putsUart0("Hard fault in process  "); putsUart0(uitoa(pid));                  putcUart0('\n');
    putsUart0("PSP:                   "); putsUart0(inttohex((uint32_t)psp));        putcUart0('\n');
    putsUart0("MSP:                   "); putsUart0(inttohex((uint32_t)msp));        putcUart0('\n');
    putsUart0("MFault Flags:          "); putsUart0(inttohex(mfault));            putcUart0('\n');
    putsUart0("Offending Instruction: "); putsUart0(inttohex((uint32_t)opcode));  putcUart0('\n');
    putsUart0("Address of Instruction:"); putsUart0(inttohex(address));              putcUart0('\n');
    putsUart0("Stack Dump!!");                                                    putcUart0('\n');
    putsUart0("xPSR:                  "); putsUart0(inttohex(xpsr));                 putcUart0('\n');
    putsUart0("PC:                    "); putsUart0(inttohex(pc));                   putcUart0('\n');
    putsUart0("LR:                    "); putsUart0(inttohex(lr));                   putcUart0('\n');
    putsUart0("R12:                   "); putsUart0(inttohex(r12));                  putcUart0('\n');
    putsUart0("R3:                    "); putsUart0(inttohex(r3));                   putcUart0('\n');
    putsUart0("R2:                    "); putsUart0(inttohex(r2));                   putcUart0('\n');
    putsUart0("R1:                    "); putsUart0(inttohex(r1));                   putcUart0('\n');
    putsUart0("R0:                    "); putsUart0(inttohex(r0));                   putcUart0('\n');

    while(1);
}

void mpuFaultISR()
{
    uint32_t r0, r1, r2, r3, r12, lr, pc, xpsr, mfault, address;
    uint32_t *psp, *msp;
    //char *instruction;

    // go up the stack to collect values to output
    psp  = getPsp();
    msp  = getMsp();
    r0   = *psp;
    r1   = *(psp + 1);
    r2   = *(psp + 2);
    r3   = *(psp + 3);
    r12  = *(psp + 4);
    lr   = *(psp + 5);
    pc   = *(psp + 6);
    xpsr = getIpsr();
    mfault = NVIC_FAULT_STAT_R & 0xFF;       // mem fault  bits [7:0] in FAULTSTAT
    // offending instruction will be stored inside register NVIC_MM_ADDR_R which can only be accessed in priv
    address = pc;
    uint32_t opcode = *((uint32_t*)(address));

    putsUart0("MPU fault in process   "); putsUart0(uitoa(pid));                  putcUart0('\n');
    putsUart0("PSP:                   "); putsUart0(inttohex((uint32_t)psp));        putcUart0('\n');
    putsUart0("MSP:                   "); putsUart0(inttohex((uint32_t)msp));        putcUart0('\n');
    putsUart0("MFault Flags:          "); putsUart0(inttohex(mfault));            putcUart0('\n');
    putsUart0("Offending Instruction: "); putsUart0(inttohex((uint32_t)opcode));  putcUart0('\n');
    putsUart0("Address of Instruction:"); putsUart0(inttohex(address));              putcUart0('\n');
    putsUart0("Stack Dump!!");                                                    putcUart0('\n');
    putsUart0("xPSR:                  "); putsUart0(inttohex(xpsr));                 putcUart0('\n');
    putsUart0("PC:                    "); putsUart0(inttohex(pc));                   putcUart0('\n');
    putsUart0("LR:                    "); putsUart0(inttohex(lr));                   putcUart0('\n');
    putsUart0("R12:                   "); putsUart0(inttohex(r12));                  putcUart0('\n');
    putsUart0("R3:                    "); putsUart0(inttohex(r3));                   putcUart0('\n');
    putsUart0("R2:                    "); putsUart0(inttohex(r2));                   putcUart0('\n');
    putsUart0("R1:                    "); putsUart0(inttohex(r1));                   putcUart0('\n');
    putsUart0("R0:                    "); putsUart0(inttohex(r0));                   putcUart0('\n');

    while(1);
}

void pendsvISR()
{
    putsUart0("PendSV in process "); putsUart0(uitoa(pid)); putcUart0('\n');
    // If the MPU DERR or IERR bits are set, clear them and display the message “called from MPU”
    if (NVIC_FAULT_STAT_R & (NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR))
    {
        NVIC_FAULT_STAT_R |= (NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR); // clear flags
        putsUart0("called from MPU");
    }
}





