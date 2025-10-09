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

#ifndef MEM_H_
#define MEM_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "isr.h"
#include "uart0.h"

/*
 * ==========================================================================
 *               HEAP VISUALIZATION
 * ==========================================================================
 *  0x2000 8000  |----------------|
 *               |                |
 *               | 8kB - Region 3 |  8 subregions of 1024B each  - bits 24-31
 *  0x2000 6000  |----------------|
 *               |                |
 *               | 8kB - Region 2 |  8 subregions of 1024B each  - bits 16-23
 *  0x2000 4000  |----------------|
 *               |                |
 *               | 8kB - Region 1 |  8 subregions of 1024B each  - bits 8-15
 *  0x2000 2000  |----------------|
 *               | 8kB - Region 0 |
 *               | [4kB - OS]     |  8 subregions of 1024B each  - bits 0-7  [4kB of which (bits 0-3) are reserved for the OS]
 *  0x2000 0000  |----------------|
 *
 * ==========================================================================
 *               HEAP TABLE VISUALIZATION
 * ==========================================================================
 * BLOCK |  ADDRESS   | REGION |     ALLOC     |   OWNER
 * ----------------------------------------------------------
 * 0     | 0x20001000 |  0     |     T/F       |    PID
  * ----------------------------------------------------------
 * 1     | 0x20001400 |  0     |     T/F       |    PID
 *
 */

// keeping track if blocks were assigned and to which pid
typedef struct _BLOCK
{
    bool alloc;      // 1 allocated, 0 not allocated
    uint32_t owner;  // pid that owns it
    uint32_t size;
} BLOCK;

#define NUM_BLOCKS  (HEAP_SIZE / BLOCK_SIZE) //32 blocks

#define HEAP_START  0x20001000 // note: 0x20000000 -> 0x20001000 is for OS
#define HEAP_END    0x20008000
#define HEAP_SIZE   0x7000
#define BLOCK_SIZE  1024
#define NUM_BLOCKS  (HEAP_SIZE / BLOCK_SIZE) // heap is 32 but 28 usable

BLOCK blockArray[NUM_BLOCKS];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void *malloc_heap (int size_in_bytes);
void free_heap(void * p);
void dumpHeap(void);

#endif
