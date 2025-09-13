; ASM Library
; Angelina Abuhilal

;-----------------------------------------------------------------------------
; Hardware Target
;-----------------------------------------------------------------------------

; Target Platform: EK-TM4C123GXL
; Target uC:       TM4C123GH6PM
; System Clock:    40 MHz

; Hardware configuration:
; 16 MHz external crystal oscillator

;-----------------------------------------------------------------------------
; Device includes, defines, and assembler directives
;-----------------------------------------------------------------------------

    .def getPsp
    .def getMsp
    .def getControl
    .def getIpsr
    .def setPsp
    .def setAsp

;-----------------------------------------------------------------------------
; Register values and large immediate values
;-----------------------------------------------------------------------------

.thumb
.text

getPsp:
    MRS     r0, PSP
    BX      lr

getMsp:
    MRS     r0, MSP
    BX      lr

getControl:
    MRS     r0, CONTROL
    BX      lr

getIpsr:
    MRS     r0, IPSR
    BX      lr

; the value to be set will be passed in r0
setPsp:
    MSR     PSP, r0
    BX      lr

setAsp:
    MRS     r0, CONTROL
    ORR     r0, r0, #0x2
    MSR     CONTROL, r0
    ISB
    BX      lr
