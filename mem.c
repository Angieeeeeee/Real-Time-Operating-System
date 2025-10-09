// Memory allocation and permissions Library

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

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "mem.h"
#include "mpu.h"
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
 */

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

extern uint32_t pid;
uint64_t srdBitmask = 0x0000000000000000;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// simple memory manager that allocates memory from the global heap
void *malloc_heap (int size_in_bytes)
{
    if (!size_in_bytes || (size_in_bytes > 0x00002000)) return NULL;     // null if size zero or greater than a region

    int blocks = size_in_bytes / BLOCK_SIZE;
    if (size_in_bytes % BLOCK_SIZE > 0) blocks ++; // round up

    // look through table to see if there are that number of consecutive blocks that are free
    int i;
    for (i = 0; i < NUM_BLOCKS; i++)
    {
        if (blockArray[i].alloc) continue; // skip used blocks

        // i = 0->3 Region 0
        // i = 4->11 Region 1
        // i = 12->19 Region 2
        // i = 20->27 Region 3
        int startRegion = ((i - 4) / 8) + 1;
        if (i < 4) startRegion = 0;
        int freeCount = 1;

        // check subsequent blocks
        int j;
        for (j = i + 1; j < NUM_BLOCKS && freeCount < blocks; j++)
        {
            int jregion = ((j - 4) / 8) + 1;
            if (j < 4) jregion = 0;
            // stop if block isnt free or crosses region boundary
            if (blockArray[j].alloc || jregion != startRegion) break;
            freeCount++;
        }

        if (freeCount == blocks)
        {
            // populate BLOCK table
            int k;
            for (k = i; k < i + blocks; k++)
            {
                blockArray[k].alloc = true;
                blockArray[k].owner = pid;
                blockArray[k].size = blocks;
            }
            // make those blocks have SRD bits 1 (RW access)
            addSramAccessWindow(&srdBitmask, (void *)(HEAP_START + (i * BLOCK_SIZE)), blocks * BLOCK_SIZE);
            applySramAccessMask(srdBitmask);
            //putsUart0(inttohex(srdBitmask));
            return (void *)(HEAP_START + (i * BLOCK_SIZE)); // pointer to start address in mem
        }

        i += freeCount - 1; // if blocks not found, skip ahead to past the checked blocks
    }
    return NULL; // failed to find space
}

// deallocates the memory from the heap
void free_heap(void * p)
{
    int blockIndex = ((uint32_t)p - HEAP_START) / BLOCK_SIZE;

    if (blockIndex < 0 || blockIndex >= NUM_BLOCKS) return; // check if bad pointer, out of heap range
    if (blockArray[blockIndex].owner != pid || !blockArray[blockIndex].alloc) return; // not the owner of the memory or not allocated anyways

    int size = blockArray[blockIndex].size;



    // free blocks owned by pid starting at index within region
    int i;
    for (i = blockIndex; (i - blockIndex) < size ; i++)
    {
        blockArray[i].alloc = false;
        blockArray[i].owner = 0;
        blockArray[i].size = 0;
        // set that block to 0 in srdBitmask
        srdBitmask &= ~((uint64_t)(1 << (i + 4))); // makes 0 no RW access for unpriv
    }
    //putsUart0(inttohex(srdBitmask));
    applySramAccessMask(srdBitmask);
}

void dumpHeap(void)
{
    putsUart0("HEAP BLOCK ALLOCATIONS\n");
    putsUart0(" BLOCK |   ADDRESS   | REGION | ALLOC | SIZE | OWNER\n");

    int i;
    for (i = 0; i < NUM_BLOCKS; i++)
    {
        int address = 0x20001000 + (0x400 * i);
        int region = ((i - 4) / 8) + 2;
        if (i < 4) region = 1;
        int size = blockArray[i].size;
        int alloc = 0; if (blockArray[i].alloc == true) alloc = 1;
        int owner = blockArray[i].owner;

        putsUart0(uitoa(i));
        putsUart0("  | ");
        putsUart0(inttohex(address));
        putsUart0(" | ");
        putsUart0(uitoa(region));
        putsUart0("  | ");
        putsUart0(uitoa(alloc));
        putsUart0("  | ");
        putsUart0(uitoa(size));
        putsUart0("  | ");
        putsUart0(uitoa(owner));
        putcUart0('\n');
    }
}
