//*****************************************************************************
//
// Lab 1 - Interfacing an LCD screen
// Innovation: 1-bit Ocillopscope
// 
// By: David Bui & Allen Garcia
//
//*****************************************************************************

#include "helper.h"
#include "CU_TM4C123.h"

int main(void) {
    int i;

    // Initialize all components
    InitializeGPIOPorts();
    InitializeTimer0AForDelay();
    InitializeLCD();
    SendCommand(0xCUL);                 // Turn off cursor
    InitializeSpecialCharacters();

    // Shift the screen to the right by 16 times so that the end of the screen will have the address 0x80
    for (i=0; i<16; i++) {
        SendCommand(0x1C);
    }

    InitializeTimer0BForScroll();
    InitializeNVIC();
    TIMER0->CTL |= (0x1UL<<8);          // Enable timer B interrupt

    while (1) { }

} // End main().
