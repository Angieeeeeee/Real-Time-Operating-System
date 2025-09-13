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
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "isr.h"
#include "asm.h"
#include "uart0.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

int pid = 12;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void busFaultISR()
{
    putsUart0("Bus fault in process ");
    putsUart0(itoa(pid));
}

void usageFaultISR()
{
    putsUart0("Usage fault in process ");
    putsUart0(itoa(pid));
}

void hardFaultISR()
{
    uint32_t *psp = getPsp();
    uint32_t *msp = getMsp();
    // Memory Management Fault Status (MFAULTSTAT), bits 7:0
    uint32_t mfault = NVIC_FAULT_STAT_R & 0xFF;
    // offending instruction derived from pc
    uint16_t *instruction = (uint16_t *)pc;
    uint32_t xpsr = getIpsr();
    uint32_t r0 = psp[0];
    uint32_t r1 = psp[1];
    uint32_t r2 = psp[2];
    uint32_t r3 = psp[3];
    uint32_t r12 = psp[4];
    uint32_t lr = psp[5];
    uint32_t pc = psp[6];
    
    putsUart0("Hard fault in process ");   putsUart0(itoa(pid));
    putsUart0("\n  psp                 "); putsUart0(itoa((uint32_t)psp));
    putsUart0("\n  msp                 "); putsUart0(itoa((uint32_t)msp));
    putsUart0("\n  mfault flags        "); putsUart0(itoa(mfault));
    putsUart0("\n  offending instruction "); putsUart0(itoa((uint32_t)instruction));
    putsUart0("\n  process stack dump:");
    putsUart0("\n  ipsr                "); putsUart0(itoa(xpsr));
    putsUart0("\n  pc                  "); putsUart0(itoa(pc));
    putsUart0("\n  lr                  "); putsUart0(itoa(lr));
    putsUart0("\n  r0                  "); putsUart0(itoa(r0));
    putsUart0("\n  r1                  "); putsUart0(itoa(r1));
    putsUart0("\n  r2                  "); putsUart0(itoa(r2));
    putsUart0("\n  r3                  "); putsUart0(itoa(r3));
    putsUart0("\n  r12                 "); putsUart0(itoa(r12));
    putsUart0("\n");

    // while loop forever to allow user to see output and not return to faulty process
    while(1);
}

void mpuFaultISR()
{
    uint32_t *psp = getPsp();
    uint32_t *msp = getMsp();
    // Memory Management Fault Status (MFAULTSTAT), bits 7:0
    uint32_t mfault = NVIC_FAULT_STAT_R & 0xFF;
    // offending instruction derived from pc
    uint16_t *instruction = (uint16_t *)pc;
    // address that caused the fault
    uint32_t data_address = NVIC_MMFAR_R;
    uint32_t xpsr = getIpsr();
    uint32_t pc = psp[6];
    uint32_t lr = psp[5];
    uint32_t r0 = psp[0];
    uint32_t r1 = psp[1];
    uint32_t r2 = psp[2];
    uint32_t r3 = psp[3];
    uint32_t r12 = psp[4];

    putsUart0("MPU fault in process "); putsUart0(itoa(pid));
    putsUart0("\n  psp                 "); putsUart0(itoa((uint32_t)psp));
    putsUart0("\n  msp                 "); putsUart0(itoa((uint32_t)msp));
    putsUart0("\n  mfault flags        "); putsUart0(itoa(mfault));
    putsUart0("\n  offending instruction "); putsUart0(itoa((uint32_t)instruction));
    putsUart0("\n  data address       "); putsUart0(itoa(data_address));
    putsUart0("\n  process stack dump:");
    putsUart0("\n  ipsr                "); putsUart0(itoa(xpsr));
    putsUart0("\n  pc                  "); putsUart0(itoa(pc));
    putsUart0("\n  lr                  "); putsUart0(itoa(lr));
    putsUart0("\n  r0                  "); putsUart0(itoa(r0));
    putsUart0("\n  r1                  "); putsUart0(itoa(r1));
    putsUart0("\n  r2                  "); putsUart0(itoa(r2));
    putsUart0("\n  r3                  "); putsUart0(itoa(r3));
    putsUart0("\n  r12                 "); putsUart0(itoa(r12));
    putsUart0("\n");

    // Clear the MPU fault pending bit and trigger a pendsv ISR call
    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV; // trigger pendsv
    NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_MEM; // clear the mem fault
}

void pendsvISR()
{
    putsUart0("PendSV fault in process ");
    putsUart0(itoa(pid));

    // If the MPU DERR or IERR bits are set, clear them and display the message “called from MPU”
    if (NVIC_FAULT_STAT_R & (NVIC_FAULT_STAT_MPU_DERR | NVIC_FAULT_STAT_MPU_IERR))
    {
        NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_MEM; // clear the mem fault
        putsUart0("\n  called from MPU");
    }
}
