#include <stdint.h>

#ifndef _HELPER
#define _HELPER

// Initialization Functions
void InitializeGPIOPorts(void);
void InitializeTimer0AForDelay(void);       // Initialize TIMER0A for delay function
void InitializeTimer0BForScroll(void);      // Initialize Timer0B for timer interrupt (0.5ms)
void InitializeLCD(void);
void InitializeNVIC(void);                  // Initialize the interrupt vector
void InitializeSpecialCharacters(void);     // Write to CGRAM to setup special characters

// Utility Functions
void SendCommand(uint32_t instruction);     // Send instructions to the LCD
void SendInstruction(char data);            // Send character data to the LCD
void writeString(char *string);             // Write a string of characters
void delayMs(uint32_t delayTimeInMs);       // Delay function using TIMER0A (0.1ms)
void increment_scroll(void);
void decrement_scroll(void);
void TIMER0B_Handler (void);                // ISR for Timer0B interrupt
void ScrollDisplay(void);

// Functions to print special characters for the 1 bit osciliscope
void print_go_high(void);
void print_stay_high(void);
void print_go_low(void);
void print_stay_low(void);

#endif
