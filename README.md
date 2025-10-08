# Real-Time-Operating-System
# Fall 2025 Mini Project (Memory Protection Unit and Memory Manager)

## Overview
The goal of this assignment is to introduce discrete privileged and unprivileged operation modes, provide
memory management, and implement memory protection that will be used in the RTOS project.

## Hardware Description
Microcontroller:
An ARM M4F core (TM4C123GH6PMI microcontroller) is required.
Serial interface:
If using the EK-TM4C123GXL evaluation board, then the UART0 tx/rx pair is routed to the ICDI that
provides a virtual COM port through a USB endpoint.
3.3V supply:
The circuit is powered completely from the 3.3V regulator output on the evaluation board.

## Software Requirements
A virtual COM port using a 115200 baud, 8N1 protocol with no hardware handshaking is used to
communicate with this project.

If a bus fault ISR occurs, display “Bus fault in process N”, where N will be a variable provided by the OS.
Or now, just use a variable named pid.

If a usage fault ISR occurs, display “Usage fault in process N”, where N will be a variable provided by the
OS. Or now, just use a variable named pid.

If a hard fault ISR occurs, display “Hard fault in process N”, where N will be a variable provided by the
OS. Or now, just use a variable named pid. Also, provide the value of the PSP, MSP, and mfault flags (in
hex). Also, print the offending instruction. Display the process stack dump (xPSR, PC, LR, R0-3, R12.

If an MPU fault ISR occurs, display “MPU fault in process N”, where N will be a variable provided by the
OS. Or now, just use a variable named pid. Also, provide the value of the PSP, MSP, and mfault flags (in
hex). Also, print the offending instruction and data addresses. Display the process stack dump (xPSR,
PC, LR, R0-3, R12. Clear the MPU fault pending bit and trigger a pendsv ISR call.

If a pendsv ISR occurs, display “Pendsv in process N”. If the MPU DERR or IERR bits are set, clear them
and display the message “called from MPU”.

Set up the microcontroller with a background rule that allows read and write access to both privileged and unprivileged functions and MPU regions at the heap that takes away RW access from unprivileged processes. 
Create a malloc that looks for memory blocks (1024 bytes), returning the pointer to the start of the blocks in memory and granting unprivileged RW access to that process 
Also create a free that takes away RW access and marks the blocks of memory as free
