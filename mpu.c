// Memory allocation and permissions Library

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "mpu.h"
#include "isr.h"
#include "uart0.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

extern uint32_t pid;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// background rule allowing RWX for priv and unpriv
void setBackgroundRule(void)
{
    NVIC_MPU_NUMBER_R = 0;
    NVIC_MPU_BASE_R = 0x00000000;
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE | (31 << 1) | (0b11 << 24) | (1 << 28); // 4GB RW for priv and unpriv no X
}

// only read from flash for both priv and unpriv
void allowFlashAccess(void)
{
    NVIC_MPU_NUMBER_R = 5;
    NVIC_MPU_BASE_R = 0x00000000;
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE | (17 << 1) | (0b110 << 24); // 256KB only R for both
}

// take away access from private peripherals
void allowPeripheralAccess(void)
{
    NVIC_MPU_NUMBER_R = 6;                          // Region Number
    NVIC_MPU_BASE_R = 0xE0000000;                   // Base Address of Peripherals
    NVIC_MPU_ATTR_R |=   NVIC_MPU_ATTR_ENABLE |     // Enable Region
                        (28 << 1) |                 // Region Size: 512MB - 2^29 (0xE0000000 -> 0x100000000)
                        (0b001 << 24) |              // RW only for priv
                        (1 << 28);                  // XN: Execute Never
}

void setupSramAccess(void)
{
    // makes multiple MPU regions 8KiB, with 8 subregions of 1KiB
    // RW access for priv, no acces to unpriv 0b1
    // disable subregions at start
    // size = 2^(SIZE+1) so 8KiB = 13 -> SIZE = 12

    NVIC_MPU_NUMBER_R = 1;
    NVIC_MPU_BASE_R = 0x20000000;
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE | (12 << 1) | (0b001 << 24);// | (0xFF << 8);
    NVIC_MPU_ATTR_R &= ~(0xFF << 8); // enable region initially, make it 0

    NVIC_MPU_NUMBER_R = 2;
    NVIC_MPU_BASE_R = 0x20002000;
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE | (12 << 1) | (0b001 << 24);// | (0xFF << 8);
    NVIC_MPU_ATTR_R &= ~(0xFF << 8); // enable region initially

    NVIC_MPU_NUMBER_R = 3;
    NVIC_MPU_BASE_R = 0x20004000;
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE | (12 << 1) | (0b001 << 24);// | (0xFF << 8);
    NVIC_MPU_ATTR_R &= ~(0xFF << 8); // enable region initially

    NVIC_MPU_NUMBER_R = 4;
    NVIC_MPU_BASE_R = 0x20006000;
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE | (12 << 1) | (0b001 << 24);// | (0xFF << 8);
    NVIC_MPU_ATTR_R &= ~(0xFF << 8); // enable region initially
}

uint64_t createSramAccessMask(void)
{
    // returns value of srd bits to allow access to SRAM
    // 4 regions * 8 subregions = 32 bits to enable
    return 0x0000000000000000;   // when SRD = 1 for subregion then the access will fall to background rule (RW for priv and unpriv)
}

// applies the srdBitMask to all SRAM regions
void applySramAccessMask(uint64_t srdBitMask)
{
    NVIC_MPU_NUMBER_R = 1;
    NVIC_MPU_ATTR_R &= ~(0xFF << 8);
    NVIC_MPU_ATTR_R |= (uint32_t) (srdBitMask & 0xFF) << 8;  //R1: bits 0-7

    NVIC_MPU_NUMBER_R = 2;
    NVIC_MPU_ATTR_R &= ~(0xFF << 8);
    NVIC_MPU_ATTR_R |= (uint32_t) ((srdBitMask >> 8) & 0xFF) << 8;

    NVIC_MPU_NUMBER_R = 3;
    NVIC_MPU_ATTR_R &= ~(0xFF << 8);
    NVIC_MPU_ATTR_R |= (uint32_t) ((srdBitMask >> 16) & 0xFF) << 8;

    NVIC_MPU_NUMBER_R = 4;
    NVIC_MPU_ATTR_R &= ~(0xFF << 8);
    NVIC_MPU_ATTR_R |= (uint32_t) ((srdBitMask >> 24) & 0xFF) << 8;
}

// adds access to the requested SRAM address range
void addSramAccessWindow(uint64_t *srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes)
{
    if (size_in_bytes % 1024 != 0)
    {
        putsUart0("sram access window: size is wrong\n");
      return;
    }
    if ((uint32_t)baseAdd < 0x20001000 || (uint32_t)baseAdd + size_in_bytes > 0x20008000)
    {
        putsUart0("sram access window: incorrect range\n");
        return;
    }

    uint32_t start = ((uint32_t)baseAdd - 0x20000000) >> 10;                     // start subregion (find offset and divide)
    uint32_t end   = ((uint32_t)baseAdd - 0x20000000 + size_in_bytes) >> 10; // end subregion

    int i;
    for (i = start; i < end; i++)
    {
        *srdBitMask |= (uint64_t) 1 << i; // turns bit on at that subregion, gets RW access
    }
}


/* NOTES TO SELF
 *
 * MPU regions [0:7]
 *
 * MPUBASE REGISTER offset 0xD9C
 *  [31:5]  ADDR    Bits 31:N (N = Log2(Region size in bytes)) the region base address. (N-1):5 are reserved.
 *     [4]  VALID   0 -> The MPUNUMBER not changed, ignore REGION
 *                  1 -> The MPUNUMBER is updated in REGION
 *   [2:0]  REGION  W -> contains value to be written in MPUNUMBER
 *                  R -> returns current region number to MPUNUMBER register
 *
 * MPUATTR REGISTER offset 0xDA0            p193
 *     [28] XN      0 -> instruction fetches are enabled
 *                  1 -> instruction fetches are disabled
 *  [26:24] AP      BITS  PRIV  UPRIV
 *                  000   0     0
 *                  001   RW    0
 *                  010   RW    R
 *                  011   RW    RW
 *                  101   R     0
 *                  110   R     R
 *                  111   R     R
 *  [21:19] TEX     |
 *     [18] S       |
 *     [17] C       |
 *     [16] B       |see table below
 *   [15:8] SRD     [7:0] each bit represents a subregion. 1 -> disable
 *    [5:1] SIZE    (Region size in bytes) = 2^(SIZE+1) ex. 32bytes would be 4 in size
 *      [0] ENABLE  0 -> region disabled
 *                  1 -> region enabled
 *
 *  | TEX | S  | C | B  | Memory Type       | Shareability  | Other Attributes                                                                  |
    | --- | -- | - | -- | ----------------- | ------------- | --------------------------------------------------------------------------------- |
    | 000 | xᵃ | 0 | 0  | Strongly Ordered  | Shareable     | -                                                                                 |
    | 000 | xᵃ | 0 | 1  | Device            | Shareable     | -                                                                                 |
    | 000 | 0  | 1 | 0  | Normal            | Not shareable | Outer and inner write-through. No write allocate.                                 |
    | 000 | 1  | 1 | 0  | Normal            | Shareable     |                                                                                   |
    | 000 | 0  | 1 | 1  | Normal            | Not shareable |                                                                                   |
    | 000 | 1  | 1 | 1  | Normal            | Shareable     |                                                                                   |
    | 001 | 0  | 0 | 0  | Normal            | Not shareable | Outer and inner non-cacheable.                                                    |
    | 001 | 1  | 0 | 0  | Normal            | Shareable     |                                                                                   |
    | 001 | xᵃ | 0 | 1  | Reserved encoding | -             | -                                                                                 |
    | 001 | xᵃ | 1 | 0  | Reserved encoding | -             | -                                                                                 |
    | 001 | 0  | 1 | 1  | Normal            | Not shareable | Outer and inner write-back. Write and read allocate.                              |
    | 001 | 1  | 1 | 1  | Normal            | Shareable     |                                                                                   |
    | 010 | xᵃ | 0 | 0  | Device            | Not shareable | Nonshared Device.                                                                 |
    | 010 | xᵃ | 0 | 1  | Reserved encoding | -             | -                                                                                 |
    | 010 | xᵃ | 1 | xᵃ | Reserved encoding | -             | -                                                                                 |
    | 1BB | 0  | A | A  | Normal            | Not shareable | Cached memory (BB = outer policy, AA = inner policy). See Table 3-4 for encoding. |
    | 1BB | 1  | A | A  | Normal            | Shareable     |
 *
 * ==========================================================================
 *               HEAP VISUALIZATION
 * ==========================================================================
 *  0x2000 8000  |----------------|
 *               |                |
 *               | 8kB - Region 4 |  8 subregions of 1024B each  - bits 32-39
 *  0x2000 6000  |----------------|
 *               |                |
 *               | 8kB - Region 3 |  8 subregions of 1024B each  - bits 24-31
 *  0x2000 4000  |----------------|
 *               |                |
 *               | 8kB - Region 2 |  8 subregions of 1024B each  - bits 16-23
 *  0x2000 2000  |----------------|
 *               | 8kB - Region 1 |
 *               | [4kB - OS]     |  8 subregions of 1024B each  - bits 8-15  [4kB of which (bits 8-11) are reserved for the OS]
 *  0x2000 0000  |----------------|
 *
 *
 *  0x1000 0000  |        ^       |
 *               | 4GB - Backgrnd |  8 subregions of 1024B each  - bits 0-7  RW for all
 *  0x0000 0000  |----------------|
 */
